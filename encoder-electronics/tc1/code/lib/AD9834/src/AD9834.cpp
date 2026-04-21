#include "AD9834.h"
#include <math.h>

static constexpr uint32_t kTwoPow28 = 1u << 28;

void AD9834::begin(int fsync_pin, long clock_freq) {
  _fsync_pin    = fsync_pin;
  _clock_freq   = clock_freq;
  _spi_settings = SPISettings(100000, MSBFIRST, SPI_MODE2);
  SPI.begin();
  pinMode(_fsync_pin, OUTPUT);
  digitalWriteFast(_fsync_pin, HIGH);
  _write_control(kCtrlReset); // hold chip in reset until update_freq() called
}

uint16_t AD9834::_active_fsel() const {
  // _write_next_to_reg1 = true  → next write goes to Reg1 → active = Reg0 → FSEL=0
  // _write_next_to_reg1 = false → next write goes to Reg0 → active = Reg1 → FSEL=1
  return _write_next_to_reg1 ? 0 : kCtrlFSEL1;
}

// Square: OPBITEN=1, DIV2=1 (DB3) → NCO MSB (not divided) on SIGN BIT OUT.
// SIGN/PIB=0 selects the NCO MSB path; DIV2=0 would give f_out/2 instead.
// Invariant: kCtrlMODE and kCtrlOPBITEN are mutually exclusive (Table 18).
uint16_t AD9834::_mode_bits() const {
  switch (_mode) {
    case DdsMode::Triangle: return kCtrlMODE;
    case DdsMode::Square:   return kCtrlOPBITEN | kCtrlDIV2;
    default:                return 0;
  }
}

void AD9834::update_freq(float freq) {
  // Clamp to valid range — Nyquist limit is MCLK/2.
  if (freq < 1.0f)                  freq = 1.0f;
  if (freq > _clock_freq / 2.0f)   freq = _clock_freq / 2.0f;

  // freq_word = round(f_out / f_MCLK * 2^28). Double arithmetic avoids overflow;
  // llround() gives the nearest representable frequency, not always the lower one.
  uint32_t freq_word = static_cast<uint32_t>(
      llround(static_cast<double>(freq) * kTwoPow28 / _clock_freq));

  // Write to the idle register, then switch FSELECT to it.
  _write_control(kCtrlB28 | _mode_bits());          // preamble: next 2 writes = freq word
  _write_freq28(_write_next_to_reg1, freq_word);
  _write_control((_write_next_to_reg1 ? kCtrlFSEL1 : 0) | _mode_bits());

  _write_next_to_reg1 = !_write_next_to_reg1;
  _last_freq = freq;
}

void AD9834::update_mode(DdsMode mode) {
  if (_last_freq < 0.0f) return; // update_freq() not yet called — ignore
  _mode = mode;
  // Send one control word: keeps the current FSEL, just changes the mode bits.
  // No frequency re-emit needed.
  _write_control(_active_fsel() | _mode_bits());
}

void AD9834::_write_control(uint16_t ctrl) {
  _transfer16(ctrl);
}

void AD9834::_write_freq28(bool use_reg1, uint32_t freq_word) {
  uint16_t addr = use_reg1 ? kAddrFreq1 : kAddrFreq0;
  _transfer16(addr | static_cast<uint16_t>(freq_word & 0x3FFF));        // 14 LSBs
  _transfer16(addr | static_cast<uint16_t>((freq_word >> 14) & 0x3FFF)); // 14 MSBs
}

void AD9834::_transfer16(uint16_t data) {
  SPI.beginTransaction(_spi_settings);
  digitalWriteFast(_fsync_pin, LOW);
  SPI.transfer16(data);
  digitalWriteFast(_fsync_pin, HIGH);
  SPI.endTransaction();
}
