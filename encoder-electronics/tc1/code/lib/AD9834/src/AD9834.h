#pragma once

// AD9834 75 MHz DDS driver for Teensy 4.1 (Arduino/SPI).
// References: AD9834 Rev. D datasheet, Table 6 (control register),
//             Table 11 (frequency register write sequence),
//             Table 17 (SIGN BIT OUT behaviour).
// SPI_MODE2: AD9834 clocks data on SCLK falling edge, SCLK idles high —
//            see datasheet p. 27 (68HC11 interface). Matches Arduino MODE2.

#include <SPI.h>

// Waveform output mode.
// Sine and Triangle affect the IOUT/IOUTB DAC output.
// Square enables SIGN BIT OUT (OPBITEN=1, DIV2=1) — square wave at the set
// frequency derived from the MSB of the phase accumulator. This is a separate
// pin from IOUT; the TC1 amplifier chain is on IOUT.
enum class DdsMode : uint8_t { Sine, Triangle, Square };

class AD9834 {
 public:
  // Default constructor is a no-op. Call begin() inside Arduino setup().
  // This makes it safe to declare at file scope without risking I/O before
  // SPI.begin() or before setup() is entered.
  AD9834() = default;

  // Initialise SPI, configure pins, and hold the chip in reset.
  // Call update_freq() to release reset and start generating output.
  void begin(int fsync_pin, long clock_freq = 75000000);

  // Set output frequency in Hz. Clamped to [1, clock_freq/2].
  // Must be called at least once before update_mode().
  void update_freq(float freq);

  // Switch waveform mode. Sends one control word — does not re-emit frequency.
  // Silently ignored if update_freq() has not been called yet.
  void update_mode(DdsMode mode);

  float   get_freq() const { return _last_freq; }
  DdsMode get_mode() const { return _mode; }

 private:
  // AD9834 control register bits (datasheet Table 6, bits 15:14 = 00).
  static constexpr uint16_t kCtrlB28     = 1u << 13; // 28-bit freq write mode
  static constexpr uint16_t kCtrlFSEL1  = 1u << 11; // select FREQ1 register
  static constexpr uint16_t kCtrlReset  = 1u <<  8; // reset phase accumulator
  static constexpr uint16_t kCtrlOPBITEN = 1u << 5; // enable SIGN BIT OUT
  static constexpr uint16_t kCtrlDIV2   = 1u <<  3; // DB3: 1=MSB direct, 0=MSB/2
  static constexpr uint16_t kCtrlMODE   = 1u <<  1; // DB1: 0=sine, 1=triangle (DB0 reserved)

  // Frequency register address bits (datasheet Table 11).
  static constexpr uint16_t kAddrFreq0  = 0x4000;
  static constexpr uint16_t kAddrFreq1  = 0x8000;

  int         _fsync_pin  = -1;
  long        _clock_freq = 0;
  SPISettings _spi_settings;

  // When true, the next update_freq() writes to FREQ1; when false, to FREQ0.
  // Alternates on every call so the new value is always written to the idle
  // register before FSELECT is swapped — glitch-free updates (datasheet p. 20).
  bool _write_next_to_reg1 = false;

  float   _last_freq = -1.0f;       // sentinel: update_freq() not yet called
  DdsMode _mode      = DdsMode::Sine;

  // Returns the FSEL control bit for the currently active frequency register.
  // Only meaningful after update_freq() has been called at least once.
  uint16_t _active_fsel() const;

  // Returns the mode-specific control bits (no B28, no FSEL).
  // Invariant: kCtrlMODE and kCtrlOPBITEN are mutually exclusive (Table 18).
  uint16_t _mode_bits() const;

  // Send a 16-bit control register word to the chip.
  void _write_control(uint16_t ctrl);

  // Send a 28-bit frequency word to the given register (LSB half first, then MSB
  // half), as required by Table 11. Caller must have sent the B28 preamble first.
  void _write_freq28(bool use_reg1, uint32_t freq_word);

  // Low-level: send one 16-bit SPI frame, FSYNC-framed.
  void _transfer16(uint16_t data);
};
