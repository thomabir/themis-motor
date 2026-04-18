# Themis Encoder Controller V1

A prototype board to prove the encoder concept.

Goal: Measure at the precision required for telescope tracking. Find the limiting factors.

## Concept

- Generate two 0.6 Vpp sines with DDS, in the 100 kHz to 3 MHz region (λ/10 < 5 m)
  - TX1: f0 carrier, amplitude modulated with f1, resonant with the encoder
  - TX2: f0 carrier, amplitude modulated with f2, resonant with the encoder
  - keep f2, f1 < 20 kHz, such that it's comfortably in the 32 kHz BW of the ADC
- Amplifiy all to 5 Vpp, 50 mA compliance
- TX signals are fed into the encoder via STP instrumentation cables
- RX signals are fed out of the encoder with STP instrumentation cables
- A differential amplifier receives the RX signals and amplifies them to a few Vpp (variable gain, depending on encoder used)
- Two mixers demodulate the AM signals
- An active band-pass filter removes the LO and DC and outputs the signal in ± 1V range
- A simultaneous sampling sigma-delta ADC samples the downmixed RX signals
- The uC demodulates in the digital domain
- The DDS clock and ADC sampling are phase-coherent, such that the freuquencies arrinving at the uC are deterministic.

## Advantages

- Processing at AC instead of at DC -> no susceptibility to slow drifts and 1/f noise
- High pole count -> higher resolution and lower susceptibility to random errors
- Tuned RF section for highest coupling efficiency, and low coupling of noise at other freuquencies

## Challenges

- Clocks synchronization strategy for DDS, ADC, uC etc.
  - The DDSs run on their own clocks (e.g. from clock generator), small drifts don't matter
  - The uC generates
    - a 8.129 MHz for the Sigma-Delta ADC clock
    - the PWM signals that control the amplitude modulation of the DDSs
      - the sin/cos LUT used for demodulation are also used for generating the PWM amplitude modulation signal
  - The uC runs on its own clock, and samples on /DRDY from the ADC (interrupt)
- crosstalk between all pairwise combinations of the four channels (2x RX, 2x TX) has to be low, so good shielding is essential
  - Be aware of returns currents in the ground plane
- It might be hard to get the analog signal chain right the first time on the PCB, without prototyping on breadboard first
  - Put [pin headers](https://www.digikey.ch/de/products/detail/w%C3%BCrth-elektronik/61300211121/4846823) and [jumpers](https://www.digikey.ch/de/products/detail/sullins-connector-solutions/QPC02SXGN-RC/2618262) on every important signal and power connection, such that it is essentially modularized
  - Can easily probe sections in isolation without having to cut traces or desolder

## Key components

### Power (LDOs)

### Signal generation

Requirement: Up to 3 MHz sine.

#### DDS

- Current choice: [AD9834](https://www.analog.com/media/en/technical-documentation/data-sheets/AD9834.pdf)
  - 14 CHF * 2 = 28 CHF total
  - Supply 2.3 to 5.5 V (AVDD and DVDD indpendent)
  - 75 MS/s, 10 Bit
  - 2.5 MHz signal: strongest image -30 dB
  - MCLK: up to 75 MHz
  - Vout: 30 mV to 0.6 V
  - It has an FSadjust pin, which can be used for amplitude modulation
    - App note: [It works at DC](https://www.analog.com/media/en/reference-design-documentation/reference-designs/cn0156.pdf)
    - May have to decrease COMP cap for 10 kHz modulation: [EEVblog forum](https://www.eevblog.com/forum/projects/amplitude-modulation-of-ad9834-dds-failed-but-why/25/)
    - Differential outputs: [EEVblog forum](https://www.eevblog.com/forum/repair/low-cost-signal-generator-with-modulation-is-that-a-pink-elephant)
- AD9838
  - BW: 8 MHz
  - 7 CHF
  - no FS ADJUST -> not an option
- AD9837
  - 6 CHF, hard to notice any practical difference to AD9838
  - 8 MHz bandwidth
  - 0.6 Vpp typ
  - 200Ω Rout (current source with 200Ω to ground after it -> Thevenin is a voltage source with 200Ω output impedance)
    - needs a buffer to drive the encoder (1µH, 1Ω, 1nF)
  - has image at nyquist - fout, but that will be filtered out during downconversion
    - however, LO has to be clean. Should think about this a bit more
  - 2.5 MHz signal: strongest image at -15 dB, 13.5 MHz
  - 2.3 to 5.5 V power supply
  - No FS ADJUST -> not an option

#### Clock generator

- [TI CDCE6214](https://www.ti.com/lit/ds/symlink/cdce6214.pdf?HQS=dis-dk-null-digikeymode-dsf-pf-null-wwe&ts=1760891217001&ref_url=https%253A%252F%252Fwww.ti.com%252Fgeneral%252Fdocs%252Fsuppproductinfo.tsp%253FdistId%253D10%2526gotoUrl%253Dhttps%253A%252F%252Fwww.ti.com%252Flit%252Fgpn%252Fcdce6214)
  - 5.70 CHF
  - same as in analog discovery 3
- [Si5351A](https://cdn-shop.adafruit.com/datasheets/Si5351.pdf): 1.7 CHF, can generate 4 clocks from kHz to 10s of MHz
  - [Adafruit breakout board](https://www.adafruit.com/product/2045)
  - Z_out: 50 Ω
  - Supply 3.3 V
  - RMS phase jitter (12 kHz to 20 MHz): 3.5 ps typ
  - Pk cycle-to-cycle jitter (10 k cycles): 30 ps typ
  - Crystal requirements: 25 to 27 MHz, 6 to 12 pF

### Amplifiers

- Requirement
  - Gain 10 at 3 MHz (30 MHz GBWP)
  - Slew rate: `2π * 5V * 3 MHz` = 100 V/µs
  - Power: ±18 V min.
- Main op-amp: [LM6172](https://www.ti.com/lit/ds/symlink/lm6172.pdf)
  - stable with 1 nF load
  - dual opamp
  - -65 dB crosstalk rejection at 3 MHz
  - fast enough
  - 50 mA continuous output current
  - DIP package available for easy breadboarding
  - 33 CHF for 10 pc
- stronger TX amplifier/buffer: OPA2673
  - Supplies: ± 3.5 to ± 6.5 V -> analog chain is ± 5 V, because nice number
  - Output: 700 mA, 3500 V/µs (way overpowered most likely, very conservative)
  - 600 MHZ unity gain bandwidth
- full-wave diode mixer (can be skipped with jumpers, then can use breadboard mixer as alternative)

### ADC

- [ADS131M03](https://www.ti.com/lit/ds/symlink/ads131m03-q1.pdf)
  - 3ch 64 kS/s Sigma-Delta
  - 8.192 MHz MCLK in
  - Supply: 2.7 to 3.6 V -> using 3.3V
  - Integrated charge pump to allow negative input voltages
    - Note that this is unusual, but simplifies the setup considerably.
    - Max absolute ratings on AINP, AINN: AGND – 1.6V to AVDD + 0.3V
    - Recommended voltage  on AINP, AINN: AGND – 1.3V to AVDD
    - Current limit to ±10mA max: ±6V into 1kΩ yield 6mA, should be safe
  - Integrated reference: 1.2V
  - Full scale range: ±1.2V (1x gain) to ±9.3mV (128x gain)
  - 330 kΩ input impedance at gains from 1 to 4
  - Noise and dynamic range (p. 16 datasheet)
    - At  1x gain, 64 kS/s: 80 dB dynamic range, 75 µV RMS noise
    - At 32x gain, 64 kS/s: 60 dB dynamic range, 5.58 µV RMS noise
    - don't go higher in gain, no benefit
  - channel corsstalk < -125 dB
  - power: < 3mA analog, < 0.5 mA digital apparently (p. 15 of datasheet)

## Signals and noise

- DDS: 600 mVpp (300 mV amplitude)
- TX amps: amplify to 5 Vpp
- encoder board: 0.3x gain (measured at 2.5 MHz with 10 nF, see lab notebook 2025-11-03)
- RX amps: switchable from 1x to 10x gain (or potentiometer)
- model mixer in LTspice, then low-pass
- ADC: ± 1 V amplitude target pseudo-differential (connect GND to AINN) (headroom to ±1.2V reference)

## Power and voltages

- Analog voltages: ±9V (except ADC)
- ADC and DDS: +3.3V (both analog and digital)
- ADC inputs ±1.2V differential max.
- Protect ADC against overvoltage
