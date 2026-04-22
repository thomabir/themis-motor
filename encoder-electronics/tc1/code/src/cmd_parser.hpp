#pragma once

#include <Arduino.h>
#include "AD9834.h"

struct CmdParser {
  float   freq;
  uint8_t amp;
  int     amp_ac; // -1 when unused (raw 'a' command was last used)

  const float   freq_min;
  const float   freq_max;
  const uint8_t amp_raw_at_max_output;
  const uint8_t amp_raw_at_min_output;

  CmdParser(float freq_init, uint8_t amp_init,
            float freq_min, float freq_max,
            uint8_t amp_raw_at_max_output, uint8_t amp_raw_at_min_output)
    : freq(freq_init), amp(amp_init), amp_ac(-1),
      freq_min(freq_min), freq_max(freq_max),
      amp_raw_at_max_output(amp_raw_at_max_output),
      amp_raw_at_min_output(amp_raw_at_min_output) {}

  void print_state(const AD9834& dds) const {
    if (amp_ac >= 0)
      Serial.printf("freq=%.0f Hz  amp=%d  ac=%d  mode=%s\n",
                    freq, amp, amp_ac, mode_name(dds.get_mode()));
    else
      Serial.printf("freq=%.0f Hz  amp=%d  mode=%s\n",
                    freq, amp, mode_name(dds.get_mode()));
  }

  void handle(const char* line, AD9834& dds, int pin_fs_adjust) {
    while (*line == ' ' || *line == '\t') ++line;
    if (*line == '\0') return;

    char       cmd = tolower((unsigned char)*line);
    const char* arg = line + 1;
    while (*arg == ' ' || *arg == '\t') ++arg;

    if (cmd == 'f') {
      cmd_freq(arg, dds);
    } else if (cmd == 'a' && tolower((unsigned char)arg[0]) == 'c') {
      cmd_amp_ac(arg + 1, dds, pin_fs_adjust);
    } else if (cmd == 'a') {
      cmd_amp(arg, dds, pin_fs_adjust);
    } else if (cmd == 'm') {
      cmd_mode(arg, dds);
    } else {
      print_help();
    }
  }

  static void print_help() {
    Serial.println("Commands: f <Hz>  a <0-255>  ac <0-100>  m <sine|tri|square>");
  }

private:
  static const char* mode_name(DdsMode m) {
    switch (m) {
      case DdsMode::Triangle: return "tri";
      case DdsMode::Square:   return "square";
      default:                return "sine";
    }
  }

  // Maps corrected amplitude [0, 100] to raw PWM.
  // Span is negative: FS_ADJUST sink current is inversely proportional to output amplitude.
  uint8_t amp_ac_to_raw(int ac) const {
    const int span = int(amp_raw_at_max_output) - int(amp_raw_at_min_output);
    return static_cast<uint8_t>(int(amp_raw_at_min_output) + span * ac / 100);
  }

  void cmd_freq(const char* arg, AD9834& dds) {
    char* end;
    float val = strtof(arg, &end);
    if (end == arg) {
      Serial.println("Error: expected a number after 'f'");
      return;
    }
    if (val < freq_min || val > freq_max) {
      Serial.printf("Error: frequency must be %.0f–%.0f Hz\n", freq_min, freq_max);
      return;
    }
    freq = val;
    dds.update_freq(freq);
    print_state(dds);
  }

  void cmd_amp_ac(const char* arg, AD9834& dds, int pin_fs_adjust) {
    while (*arg == ' ' || *arg == '\t') ++arg;
    char* end;
    long val = strtol(arg, &end, 10);
    if (end == arg) {
      Serial.println("Error: expected a number after 'ac'");
      return;
    }
    if (val < 0 || val > 100) {
      Serial.println("Error: corrected amplitude must be 0–100");
      return;
    }
    amp_ac = (int)val;
    amp    = amp_ac_to_raw(amp_ac);
    analogWrite(pin_fs_adjust, amp);
    print_state(dds);
  }

  void cmd_amp(const char* arg, AD9834& dds, int pin_fs_adjust) {
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
    amp    = (uint8_t)val;
    amp_ac = -1;
    analogWrite(pin_fs_adjust, amp);
    print_state(dds);
  }

  void cmd_mode(const char* arg, AD9834& dds) {
    char arg_lower[16] = {};
    for (size_t i = 0; i < sizeof(arg_lower) - 1 && arg[i]; ++i)
      arg_lower[i] = tolower((unsigned char)arg[i]);

    DdsMode mode;
    if      (strcmp(arg_lower, "sine")   == 0) mode = DdsMode::Sine;
    else if (strcmp(arg_lower, "tri")    == 0) mode = DdsMode::Triangle;
    else if (strcmp(arg_lower, "square") == 0) mode = DdsMode::Square;
    else {
      Serial.println("Error: mode must be sine, tri, or square");
      return;
    }
    dds.update_mode(mode);
    print_state(dds);
  }
};
