#pragma once

#include <Arduino.h>
#include "dds_controller.hpp"

// Pure serial command dispatcher — no member state, no hardware I/O.
// All hardware access goes through DdsController.
struct CmdParser {

  void handle(const char* line, DdsController& dds) {
    while (*line == ' ' || *line == '\t') ++line;
    if (*line == '\0') return;

    // Lex the first whitespace-delimited token, lower-cased.
    char tok[8] = {};
    size_t ti = 0;
    while (line[ti] && !isspace((unsigned char)line[ti]) && ti < sizeof(tok) - 1) {
      tok[ti] = tolower((unsigned char)line[ti]);
      ++ti;
    }
    tok[ti] = '\0';
    const char* arg = line + ti;
    while (isspace((unsigned char)*arg)) ++arg;

    if      (strcmp(tok, "f")  == 0) cmd_freq(arg, dds);
    else if (strcmp(tok, "ac") == 0) cmd_amp_ac(arg, dds);
    else if (strcmp(tok, "a")  == 0) cmd_amp(arg, dds);
    else if (strcmp(tok, "m")  == 0) cmd_mode(arg, dds);
    else if (strcmp(tok, "me") == 0) cmd_mod_enable(arg, dds);
    else if (strcmp(tok, "mi") == 0) cmd_mod_index(arg, dds);
    else if (strcmp(tok, "mf") == 0) cmd_mod_freq(arg, dds);
    else if (strcmp(tok, "mp") == 0) cmd_mod_phase(arg, dds);
    else print_help();
  }

  void print_state(const DdsController& dds) const {
    const int ac = dds.get_carrier_amp_ac();
    if (ac >= 0)
      Serial.printf("freq=%.0f Hz  amp=%d  ac=%d  mode=%s\n",
                    dds.get_freq(), dds.get_carrier_amp_raw(), ac,
                    mode_name(dds.get_mode()));
    else
      Serial.printf("freq=%.0f Hz  amp=%d  mode=%s\n",
                    dds.get_freq(), dds.get_carrier_amp_raw(),
                    mode_name(dds.get_mode()));

    Serial.printf("am: %s  index=%.3f  mf=%.1f Hz  mp=%.1f deg\n",
                  dds.get_mod_enabled() ? "on" : "off",
                  dds.get_mod_index(),
                  dds.get_mod_freq(),
                  dds.get_mod_phase_deg());
  }

  static void print_help() {
    Serial.println("Commands: f <Hz>  a <0-255>  ac <0-100>  m <sine|tri|square>");
    Serial.println("          me <on|off>  mi <0.0-1.0>  mf <Hz>  mp <deg>");
  }

 private:
  static const char* mode_name(DdsMode m) {
    switch (m) {
      case DdsMode::Triangle: return "tri";
      case DdsMode::Square:   return "square";
      default:                return "sine";
    }
  }

  void cmd_freq(const char* arg, DdsController& dds) {
    char* end;
    float val = strtof(arg, &end);
    if (end == arg) {
      Serial.println("Error: expected a number after 'f'");
      return;
    }
    if (val < dds.get_freq_min() || val > dds.get_freq_max()) {
      Serial.printf("Error: frequency must be %.0f–%.0f Hz\n",
                    dds.get_freq_min(), dds.get_freq_max());
      return;
    }
    dds.set_freq(val);
    print_state(dds);
  }

  void cmd_amp_ac(const char* arg, DdsController& dds) {
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
    dds.set_carrier_amp_ac((int)val);
    print_state(dds);
  }

  void cmd_amp(const char* arg, DdsController& dds) {
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
    dds.set_carrier_amp_raw((uint8_t)val);
    print_state(dds);
  }

  void cmd_mode(const char* arg, DdsController& dds) {
    char a[16] = {};
    for (size_t i = 0; i < sizeof(a) - 1 && arg[i]; ++i)
      a[i] = tolower((unsigned char)arg[i]);

    DdsMode mode;
    if      (strcmp(a, "sine")   == 0) mode = DdsMode::Sine;
    else if (strcmp(a, "tri")    == 0) mode = DdsMode::Triangle;
    else if (strcmp(a, "square") == 0) mode = DdsMode::Square;
    else {
      Serial.println("Error: mode must be sine, tri, or square");
      return;
    }
    dds.set_mode(mode);
    print_state(dds);
  }

  void cmd_mod_enable(const char* arg, DdsController& dds) {
    char a[8] = {};
    for (size_t i = 0; i < sizeof(a) - 1 && arg[i]; ++i)
      a[i] = tolower((unsigned char)arg[i]);

    if      (strcmp(a, "on")  == 0) dds.set_mod_enabled(true);
    else if (strcmp(a, "off") == 0) dds.set_mod_enabled(false);
    else {
      Serial.println("Error: me expects 'on' or 'off'");
      return;
    }
    print_state(dds);
  }

  void cmd_mod_index(const char* arg, DdsController& dds) {
    char* end;
    float val = strtof(arg, &end);
    if (end == arg) {
      Serial.println("Error: expected a number after 'mi'");
      return;
    }
    if (val < 0.0f || val > 1.0f) {
      Serial.println("Error: modulation index must be 0.0–1.0");
      return;
    }
    dds.set_mod_index(val);
    print_state(dds);
  }

  void cmd_mod_freq(const char* arg, DdsController& dds) {
    char* end;
    float val = strtof(arg, &end);
    if (end == arg) {
      Serial.println("Error: expected a number after 'mf'");
      return;
    }
    if (val < 0.0f) {
      Serial.println("Error: modulation frequency must be >= 0");
      return;
    }
    dds.set_mod_freq(val);
    print_state(dds);
  }

  void cmd_mod_phase(const char* arg, DdsController& dds) {
    char* end;
    float val = strtof(arg, &end);
    if (end == arg) {
      Serial.println("Error: expected a number after 'mp'");
      return;
    }
    dds.set_mod_phase(val);
    print_state(dds);
  }
};
