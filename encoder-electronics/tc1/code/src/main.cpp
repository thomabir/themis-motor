#include <Arduino.h>
#include "AD9834.h"

static constexpr int PIN_FS_ADJUST = 6;
static constexpr int PIN_RESET     = 9;
static constexpr int PIN_FSYNC     = 10;
static constexpr long MCLK_HZ     = 75000000L;
static constexpr float OUT_FREQ_HZ = 1000000.0f;

AD9834* dds;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 2000) {}

  // Drive RESET low (deasserted) before initialising the DDS.
  pinMode(PIN_RESET, OUTPUT);
  digitalWrite(PIN_RESET, LOW);

  analogWriteFrequency(PIN_FS_ADJUST, 1000000);
  analogWriteResolution(8);
  analogWrite(PIN_FS_ADJUST, 128);

  dds = new AD9834(PIN_FSYNC, MCLK_HZ);

  while(true) {
    dds->update_freq(OUT_FREQ_HZ);
    delay(100);
  }
}

void loop() {}
