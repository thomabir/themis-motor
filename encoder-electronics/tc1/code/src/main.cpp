// TC1 bringup firmware — serial command shim for interactive DDS control.
// Not intended as production firmware; see AD9834.h for the reusable driver.

#include <Arduino.h>
#include "AD9834.h"

static constexpr int   PIN_FS_ADJUST = 6;
static constexpr int   PIN_RESET     = 9;
static constexpr int   PIN_FSYNC     = 10;
static constexpr long  MCLK_HZ      = 75000000L;
static constexpr float FREQ_MIN_HZ  = 1.0f;
static constexpr float FREQ_MAX_HZ  = MCLK_HZ / 2.0f;

static float   g_freq = 1e6f;
static uint8_t g_amp  = 128;

static AD9834 dds;

static const char* mode_name(DdsMode m) {
  switch (m) {
    case DdsMode::Triangle: return "tri";
    case DdsMode::Square:   return "square";
    default:                return "sine";
  }
}

static void print_state() {
  Serial.printf("freq=%.0f Hz  amp=%d  mode=%s\n",
                g_freq, g_amp, mode_name(dds.get_mode()));
}

static void handle(const char* line) {
  while (*line == ' ' || *line == '\t') ++line;
  if (*line == '\0') return;

  char cmd = tolower((unsigned char)*line);
  const char* arg = line + 1;
  while (*arg == ' ' || *arg == '\t') ++arg;

  if (cmd == 'f') {
    char* end;
    float val = strtof(arg, &end);
    if (end == arg) {
      Serial.println("Error: expected a number after 'f'");
      return;
    }
    if (val < FREQ_MIN_HZ || val > FREQ_MAX_HZ) {
      Serial.printf("Error: frequency must be %.0f–%.0f Hz\n", FREQ_MIN_HZ, FREQ_MAX_HZ);
      return;
    }
    g_freq = val;
    dds.update_freq(g_freq);
    print_state();

  } else if (cmd == 'a') {
    char* end;
    long val = strtol(arg, &end, 10);
    if (end == arg) {
      Serial.println("Error: expected a number after 'a'");
      return;
    }
    if (val < 0 || val > 255) {
      Serial.println("Error: amplitude must be 0–255");
      return;
    }
    g_amp = (uint8_t)val;
    analogWrite(PIN_FS_ADJUST, g_amp);
    print_state();

  } else if (cmd == 'm') {
    char arg_lower[16] = {};
    for (size_t i = 0; i < sizeof(arg_lower) - 1 && arg[i]; ++i)
      arg_lower[i] = tolower((unsigned char)arg[i]);

    DdsMode mode;
    if (strcmp(arg_lower, "sine") == 0) {
      mode = DdsMode::Sine;
    } else if (strcmp(arg_lower, "tri") == 0) {
      mode = DdsMode::Triangle;
    } else if (strcmp(arg_lower, "square") == 0) {
      mode = DdsMode::Square;
    } else {
      Serial.println("Error: mode must be sine, tri, or square");
      return;
    }
    dds.update_mode(mode);
    print_state();

  } else {
    Serial.println("Commands: f <Hz>  a <0-255>  m <sine|tri|square>");
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  // Drive RESET low (deasserted) before initialising the DDS.
  // A floating RESET holds the chip in reset; all SPI writes are silently ignored.
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, LOW);

  analogWriteFrequency(PIN_FS_ADJUST, 1000000); // 1 MHz: above LPF corner, clean DC bias
  analogWriteResolution(8);
  analogWrite(PIN_FS_ADJUST, g_amp);

  dds.begin(PIN_FSYNC, MCLK_HZ);
  dds.update_freq(g_freq);

  Serial.println("Commands: f <Hz>  a <0-255>  m <sine|tri|square>");
  print_state();
}

void loop() {
  static char    buf[64];
  static uint8_t pos = 0;

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (pos > 0) {
        buf[pos] = '\0';
        handle(buf);
        pos = 0;
      }
    } else if (pos < sizeof(buf) - 1) {
      buf[pos++] = c;
    }
  }
}
