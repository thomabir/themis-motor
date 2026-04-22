#pragma once

#include <Arduino.h>
#include <math.h>

// Forward declaration for the ISR trampoline.
class AmplitudeModulator;
static AmplitudeModulator* _am_instance = nullptr;
static void _am_isr();

// Drives the FS_ADJUST PWM pin with an optional sinusoidal AM envelope.
//
// Hardware note: pin 6 on Teensy 4.1 maps to FLEXPWM2 sub-module 0 (PWMA).
// tick() writes directly to FLEXPWM2.SM[0].VAL3 to avoid analogWrite() overhead
// in the ISR. This is safe; begin() caches the period register (VAL1) after
// analogWrite() has configured the FlexPWM for 1 MHz / 8-bit.
//
// Concurrency: all volatile fields are 32-bit aligned. On ARMv7-M a naturally-
// aligned 32-bit store/load is single-bus-cycle (atomic at the instruction level).
// volatile prevents the compiler from caching values across the ISR boundary.
// _phase_accum and _pwm_period are only touched by tick() and begin() respectively,
// so they need no volatile.
class AmplitudeModulator {
 public:
  // pin:             Arduino pin number for FS_ADJUST (PWM).
  // amp_raw_at_max:  raw PWM value that produces maximum output amplitude (0).
  // amp_raw_at_min:  raw PWM value that produces minimum output amplitude (230).
  AmplitudeModulator(int pin, uint8_t amp_raw_at_max, uint8_t amp_raw_at_min)
      : _pin(pin),
        _amp_raw_at_max(amp_raw_at_max),
        _amp_raw_at_min(amp_raw_at_min),
        _isr_period_s(0.0f),
        _mod_enabled(false),
        _carrier_amp_ac(50),
        _mod_index(0.0f),
        _mod_freq(0.0f),
        _mod_phase_rad(0.0f),
        _amp_is_raw(false),
        _carrier_amp_raw(0),
        _phase_accum(0.0f) {}

  // Call from setup() — configures PWM frequency/resolution, writes initial level,
  // caches the FlexPWM period register, and starts the IntervalTimer.
  // isr_rate_hz = 200 kHz → 20 samples/cycle at 10 kHz mod → clean scope envelope.
  void begin(int carrier_amp_ac_init, float isr_rate_hz = 200000.0f) {
    _carrier_amp_ac = carrier_amp_ac_init;
    _isr_period_s   = 1.0f / isr_rate_hz;

    analogWriteResolution(8);
    analogWriteFrequency(_pin, 1000000);
    analogWrite(_pin, _ac_to_raw(_carrier_amp_ac));

    _am_instance = this;
    _timer.begin(_am_isr, 1000000.0f / isr_rate_hz);
  }

  // --- Setters (main-loop safe while ISR is running) ---

  void set_carrier_amp_ac(int ac) {
    if (ac < 0)   ac = 0;
    if (ac > 100) ac = 100;
    _carrier_amp_ac = ac;
    _amp_is_raw     = false;
  }

  // Converts raw to corrected scale so modulation math always operates in [0, 100].
  void set_carrier_amp_raw(uint8_t raw) {
    _carrier_amp_raw = raw;
    _carrier_amp_ac  = _raw_to_ac(raw);
    _amp_is_raw      = true;
  }

  void set_mod_enabled(bool en) { _mod_enabled = en; }

  void set_mod_index(float idx) {
    if (idx < 0.0f) idx = 0.0f;
    if (idx > 1.0f) idx = 1.0f;
    _mod_index = idx;
  }

  void set_mod_freq(float hz) {
    if (hz < 0.0f) hz = 0.0f;
    _mod_freq = hz;
  }

  void set_mod_phase(float deg) {
    float rad = deg * (float(M_PI) / 180.0f);
    rad = fmodf(rad, 2.0f * float(M_PI));
    if (rad < 0.0f) rad += 2.0f * float(M_PI);
    _mod_phase_rad = rad;
  }

  // --- Getters ---

  // Returns -1 when raw 'a' command was last used; [0, 100] otherwise.
  int     get_carrier_amp_ac()  const { return _amp_is_raw ? -1 : (int)_carrier_amp_ac; }
  uint8_t get_carrier_amp_raw() const {
    return _amp_is_raw ? _carrier_amp_raw : _ac_to_raw((int)_carrier_amp_ac);
  }
  bool  get_mod_enabled()    const { return _mod_enabled; }
  float get_mod_index()      const { return _mod_index; }
  float get_mod_freq()       const { return _mod_freq; }
  float get_mod_phase_deg()  const { return _mod_phase_rad * (180.0f / float(M_PI)); }

  // Called by the ISR trampoline — not for user code.
  void tick() {
    const int   carrier  = _carrier_amp_ac;
    const float index    = _mod_index;
    const float mod_freq = _mod_freq;
    const float phase    = _mod_phase_rad;

    if (!_mod_enabled) {
      _write_raw(_ac_to_raw(carrier));
      return;
    }

    // Advance and wrap the phase accumulator.
    _phase_accum += 2.0f * float(M_PI) * mod_freq * _isr_period_s;
    if (_phase_accum >= 2.0f * float(M_PI))
      _phase_accum -= 2.0f * float(M_PI);

    // a(t) = carrier * (1 + index * cos(phase_accum + phase_offset))
    float am_val = float(carrier) * (1.0f + index * cosf(_phase_accum + phase));

    int ac_clamped = int(am_val + 0.5f);
    if (ac_clamped < 0)   ac_clamped = 0;
    if (ac_clamped > 100) ac_clamped = 100;

    _write_raw(_ac_to_raw(ac_clamped));
  }

 private:
  int      _pin;
  uint8_t  _amp_raw_at_max;
  uint8_t  _amp_raw_at_min;
  float    _isr_period_s;

  // Volatile: written from main loop, read from ISR.
  volatile bool    _mod_enabled;
  volatile int     _carrier_amp_ac;   // corrected scale [0, 100]
  volatile float   _mod_index;
  volatile float   _mod_freq;
  volatile float   _mod_phase_rad;

  // Display-only state — never read from ISR, no volatile needed.
  bool    _amp_is_raw;
  uint8_t _carrier_amp_raw;

  float         _phase_accum;   // exclusively written/read by tick()
  IntervalTimer _timer;

  // Maps corrected amplitude [0, 100] to raw PWM.
  // Span is negative: FS_ADJUST is current-sink (higher PWM → lower amplitude).
  uint8_t _ac_to_raw(int ac) const {
    const int span = int(_amp_raw_at_max) - int(_amp_raw_at_min);
    return static_cast<uint8_t>(int(_amp_raw_at_min) + span * ac / 100);
  }

  // Inverse of _ac_to_raw — used when converting a raw 'a' command to ac-scale.
  int _raw_to_ac(uint8_t raw) const {
    const int span = int(_amp_raw_at_max) - int(_amp_raw_at_min);
    if (span == 0) return 0;
    int ac = (int(raw) - int(_amp_raw_at_min)) * 100 / span;
    if (ac < 0)   ac = 0;
    if (ac > 100) ac = 100;
    return ac;
  }

  void _write_raw(uint8_t raw) {
    analogWrite(_pin, raw);
  }
};

static void _am_isr() {
  if (_am_instance) _am_instance->tick();
}
