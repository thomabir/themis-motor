#pragma once

// Hardware interface boundary for the TC1 signal generator.
// Owns the AD9834 DDS and the FS_ADJUST amplitude modulator.
// main.cpp and CmdParser include only this header — never AD9834.h or
// amplitude_modulator.hpp directly. Swapping the DDS IC means updating only
// this file.

#include "AD9834.h"
#include "amplitude_modulator.hpp"

class DdsController {
 public:
  // All hardware pin assignments and amplitude-mapping constants in one place.
  DdsController(int pin_fsync, int pin_reset, long mclk_hz,
                int pin_fs_adjust,
                uint8_t amp_raw_at_max, uint8_t amp_raw_at_min)
      : _dds(),
        _mod(pin_fs_adjust, amp_raw_at_max, amp_raw_at_min),
        _pin_fsync(pin_fsync),
        _pin_reset(pin_reset),
        _mclk_hz(mclk_hz) {}

  // Full hardware initialisation. Call from setup() after Serial.begin().
  // Configures RESET pin, DDS, and AM modulator (PWM + IntervalTimer).
  void begin(float freq_init, int carrier_amp_ac_init,
             float am_isr_rate_hz = 200000.0f) {
    pinMode(_pin_reset, OUTPUT);
    digitalWrite(_pin_reset, LOW); // deassert reset (active-low)
    _dds.begin(_pin_fsync, _mclk_hz);
    _dds.update_freq(freq_init);
    _mod.begin(carrier_amp_ac_init, am_isr_rate_hz);
  }

  // --- Carrier ---

  void set_freq(float hz)     { _dds.update_freq(hz); }
  void set_mode(DdsMode mode) { _dds.update_mode(mode); }

  // Corrected amplitude [0, 100] — 0 = off, 100 = maximum output.
  void set_carrier_amp_ac(int ac)       { _mod.set_carrier_amp_ac(ac); }

  // Raw PWM [0, 255]. Converted to corrected scale internally so modulation
  // math keeps working after an 'a' command.
  void set_carrier_amp_raw(uint8_t raw) { _mod.set_carrier_amp_raw(raw); }

  // --- Amplitude modulation ---

  void set_mod_enabled(bool en)   { _mod.set_mod_enabled(en); }
  void set_mod_index(float idx)   { _mod.set_mod_index(idx); }
  void set_mod_freq(float hz)     { _mod.set_mod_freq(hz); }
  void set_mod_phase(float deg)   { _mod.set_mod_phase(deg); }

  // --- Getters (used by CmdParser::print_state and freq validation) ---

  float   get_freq()     const { return _dds.get_freq(); }
  float   get_freq_min() const { return 1.0f; }
  float   get_freq_max() const { return float(_mclk_hz) / 2.0f; }
  DdsMode get_mode()     const { return _dds.get_mode(); }

  // Returns -1 when raw 'a' command was last used; [0, 100] otherwise.
  int     get_carrier_amp_ac()  const { return _mod.get_carrier_amp_ac(); }
  uint8_t get_carrier_amp_raw() const { return _mod.get_carrier_amp_raw(); }

  bool  get_mod_enabled()   const { return _mod.get_mod_enabled(); }
  float get_mod_index()     const { return _mod.get_mod_index(); }
  float get_mod_freq()      const { return _mod.get_mod_freq(); }
  float get_mod_phase_deg() const { return _mod.get_mod_phase_deg(); }

 private:
  AD9834             _dds;
  AmplitudeModulator _mod;
  int                _pin_fsync;
  int                _pin_reset;
  long               _mclk_hz;
};
