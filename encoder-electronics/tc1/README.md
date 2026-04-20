# Themis TC1

Goal: Figure out whether 10 kHz amplitude modulaiton works with the AD9834 DDS via the FS_ADJUST pin.

Secondary goal (optional): Have a minimal "sender" circuit for the encoder, test the differential driver

- Interfaces:
  - Power in: 4V lab PSU, ±13V lab PSU
  - Signal out: differential amplified DDS signal, can power the existing encoder coils
  - Digital IO: Every useful digital pin from the DDS is accessible via a pin header, for easy debugging
  - Testpoints: Every single analog signal is accessible, and jumpers between main sections are used for easy disconnecting of blocks.

- Strategy
  - Passives: Use 0Ωs and DNPs to keep it flexible. 0805 everything for easy modification by hand.
  - Using screw terminals for power and encoder, pin headers for digital. Not fancy, but cheap and quick.

- Power:
  - ±12 V: LT3097
    - 10 CHF/pc (= 5 CHF/rail)
    - 500 mA bipolar adjustable ultra-low-noise LDO
    - It's the best. Slightly pricy, but just works, no fuss.
    - Basically excludes LDOs as a noise source -> faster prototyping.
    - (Hurts a bit to buy fancy parts for a prototype)
  - 3.3 V: MAX8887
    - I've used it before -> copy-paste design
    - Decently low-noise
- DDS: AD9834
  - The big unknown: Can it modulate quickly and with low noise?
  - Does the differential output work as expected?
  - How to program it? Will figure out once I get it
- Clock: Active crystal oscillator at 75 MHz
- Diff-amp to drive encoder: AD8390
  - Lots of power, cheap, fast
  - Unknown: Made for ADSL, let's see if this works well in this application? Spec wise, it should work.

## How to run

Upload code to Teensy: `~/.platformio/penv/bin/pio run -e teensy41 -t upload`