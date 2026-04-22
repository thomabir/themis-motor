// TC1 bringup firmware — serial command shim for interactive DDS control.
// Not intended as production firmware; see AD9834.h for the reusable driver.

#include <Arduino.h>
#include "AD9834.h"
#include "cmd_parser.hpp"

static constexpr int   PIN_FS_ADJUST = 6;
static constexpr int   PIN_RESET     = 9;
static constexpr int   PIN_FSYNC     = 10;
static constexpr long  MCLK_HZ      = 75000000L;

// FS_ADJUST is current-sink: raw=0 → max output amplitude, raw=230 → min output amplitude.
static constexpr uint8_t AMP_RAW_AT_MAX_OUTPUT = 0;
static constexpr uint8_t AMP_RAW_AT_MIN_OUTPUT = 230;

static AD9834    dds;
static CmdParser parser(
  /*freq_init*/            1e6f,
  /*amp_init*/             128,
  /*freq_min*/             1.0f,
  /*freq_max*/             MCLK_HZ / 2.0f,
  /*amp_raw_at_max_output*/AMP_RAW_AT_MAX_OUTPUT,
  /*amp_raw_at_min_output*/AMP_RAW_AT_MIN_OUTPUT
);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  // Drive RESET low (deasserted) before initialising the DDS.
  // A floating RESET holds the chip in reset; all SPI writes are silently ignored.
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, LOW);

  analogWriteFrequency(PIN_FS_ADJUST, 1000000); // 1 MHz: above LPF corner, clean DC bias
  analogWriteResolution(8);
  analogWrite(PIN_FS_ADJUST, parser.amp);

  dds.begin(PIN_FSYNC, MCLK_HZ);
  dds.update_freq(parser.freq);

  CmdParser::print_help();
  parser.print_state(dds);
}

void loop() {
  static char    buf[64];
  static uint8_t pos = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (pos > 0) {
        buf[pos] = '\0';
        parser.handle(buf, dds, PIN_FS_ADJUST);
        pos = 0;
      }
    } else if (pos < sizeof(buf) - 1) {
      buf[pos++] = c;
    }
  }
}
