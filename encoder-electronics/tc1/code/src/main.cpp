// TC1 bringup firmware — serial command shim for interactive DDS control.
// Not intended as production firmware; see AD9834.h for the reusable driver.

#include <Arduino.h>
#include "dds_controller.hpp"
#include "cmd_parser.hpp"

static constexpr int     PIN_FS_ADJUST         = 6;
static constexpr int     PIN_RESET             = 9;
static constexpr int     PIN_FSYNC             = 10;
static constexpr long    MCLK_HZ              = 75000000L;
// FS_ADJUST is current-sink: raw=0 → max output amplitude, raw=230 → min.
static constexpr uint8_t AMP_RAW_AT_MAX_OUTPUT = 0;
static constexpr uint8_t AMP_RAW_AT_MIN_OUTPUT = 230;
static constexpr int     CARRIER_AMP_AC_INIT   = 50;

static DdsController dds(PIN_FSYNC, PIN_RESET, MCLK_HZ,
                         PIN_FS_ADJUST, AMP_RAW_AT_MAX_OUTPUT, AMP_RAW_AT_MIN_OUTPUT);
static CmdParser     parser;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  dds.begin(1e6f, CARRIER_AMP_AC_INIT);

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
        parser.handle(buf, dds);
        pos = 0;
      }
    } else if (pos < sizeof(buf) - 1) {
      buf[pos++] = c;
    }
  }
}
