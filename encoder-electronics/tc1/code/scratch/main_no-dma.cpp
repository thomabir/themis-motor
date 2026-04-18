
#include <Arduino.h>

const int pwmPin = 4;

const int pwmFreq = 2343750; // Hz, see https://www.pjrc.com/teensy/td_pulse.html
const int sampleFreq = 64000; // Hz, waveform update rate
const int lutSize = 64;      // Size of lookup table

uint16_t sineLUT[lutSize];
volatile uint8_t lutIndex = 0;
IntervalTimer sampleTimer;

void updatePWM() {
  analogWrite(pwmPin, sineLUT[lutIndex]);
  lutIndex = (lutIndex + 1) % lutSize;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000);

  Serial.println("Sine Wave PWM Generator (Step 2)");
  
  // Fill LUT
  for (int i = 0; i < lutSize; i++) {
    // 0-255 range for 8-bit PWM (implied by pwmFreq of ~2.34MHz on 600MHz clock)
    float angle = (2.0 * PI * i) / lutSize;
    sineLUT[i] = (uint16_t)(127.5 + 127.5 * sin(angle));
  }

  // PWM Setup
  analogWriteResolution(8);
  analogWriteFrequency(pwmPin, pwmFreq);
  analogWrite(pwmPin, 128); // Start at mid-point

  // Timer Setup
  float periodMicros = 1000000.0 / sampleFreq;
  if (sampleTimer.begin(updatePWM, periodMicros)) {
    Serial.printf("Timer started. Freq: %d Hz, Period: %f us\n", sampleFreq, periodMicros);
  } else {
    Serial.println("Failed to start timer!");
  }
}

void loop() {
  delay(1000);
}