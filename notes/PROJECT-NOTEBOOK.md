# Themis Project Notebook

## 2026-06-20

- Measured clock: 75 MHz, ≈ 3 Vpp (not great probing, so amplitude not certain)
  - Problem: This is a discrepency with schematic, which says 50 MHz. Not a big issue
  - BOM part number is a 75 MHz part, so the procurement was correct. Updating schematic.
  - Changed MCLK_HZ in code to 75 MHz
- Measuring SIGNOUT pin
  - 8.7 MHz 3.3V clock, distorted probably from bad probing
  - After configuring for 1 MHz with Teensy: Still 8.7 MHz.
  - This is not as expected, something's wrong. Should be 1 MHz
- Soldered 6.8 kΩ resistor between FSADJUST and GND.
- Clock still as before
- SIGNOUT now no output, both before and after config from uC.
- Did the part die?
- Prepared second board, also soldered in 6.8 kΩ resistor between FS_ADJUST and GND
  - Clock good
  - no SIGNOUT output before teensy config
  - no SIGNOUT output after teensy config
- Removng 6.8 kΩ resistor again
  - clock still good
  - still no SIGNOUT output
- What happened here? Did both boards die from soldering in a resistor?

- Desoldered 6.8 kΩ again on one of the boards. I broke out these pins on the PCB, so the resistor has big pads and is nowhere near the IC. No bridges possible here
- Inspected both boards under microscope. No problems visible. Some crud from probing the SIGNOUT pin at the IC (forgot to break that one out), but no bridging. Cleaned up a bit anyway with small scaper.
  - Next board iteration: break out SIGNOUT pin
- ICs are not getting warm. Lab PSU shows < 10 mA on analog rails. No current measurement on digital rails unfortunately. Switching Lab PSU to get that as well.
- Digital rail draws 13 mA, probably not a short.
- Next: Checking SPI
- Notes for next board
  - make connectors quick-disconnectable (use the plug-in type screw terminals instead of regular ones)
  - Break more pins for easy testing

- From AD9834 datasheet:
  "SIGN BIT OUT PIN
  The AD9834 offers a variety of outputs from the chip. The digital outputs are available from the SIGN BIT OUT pin. The available outputs are the comparator output or the MSB of the DAC data. The bits controlling the SIGN BIT OUT pin are outlined in Table 17.
  This pin must be enabled before use. The enabling/disabling of this pin is controlled by the Bit OPBITEN (DB5) in the control register. When OPBITEN = 1, this pin is enabled. Note that the MODE bit (DB1) in the control register should be set to 0 if OPBITEN = 1."
- It appears what I measured before (8.7 MHz SIGNOUT) may have been some parasitic coupling.

- Scoping SPI signals with AD3 now to see if they are as expected.
- Not stable, very noisy, one capture:
  `h4101 h1907 h01B5 h1971`
  SCLK freq ≈ 10 MHz
- Weird, I've used 10 MHz SPI over jumpers in the past, no problem.
- SPI clock slowed down to 1 MHz
  - still garbled data, changes on every transmission
  - Disconnected DUT now - just probing SPI from teensy directly.
  - That clock frequency is not a problem now - 1 MHz is very slow
- Found a problem: was probing SPI wrong. In AD9834 datasheet: "SCK idles high between write operations (CPOL = 0), Data is valid on the SCK falling edge (CPHA = 1). notFSYNC (active low) frames each 16-bit transfer individually."
- When I set the logic analyzer up for that, it's a bit more stable:
  ```
  b0010000000000000
  bXX01110100000011
  bXX00000011011010
  b0000X00000111000
  ```
  (X means unstable, jumping around)

- Just realized: RESET pin was floating the entire time. Setting it now and reuploading code.
- SIGNOUT is alive! 9.7 MHz 3.3V
- Power consumption just changed: 17 mA digital.
- DDS+ and DDS- show a clean 1 MHz signal!
- That fixed it, floating reset pin was the culprit.

## 2026-04-18

TC1 testings

[X] Voltage regulators are working
  [X] +12 V (less than 10 mA idle)
  [X] -12 V (less than 10 mA idle)
  [X] +3.3 V

Should have broken out the SIGNBIT pin from the AD9834 for proof-of-life, since there is no SDO.

Currently, using datasheet-suggested 6.8 kΩ resistor from FSADJUST to GND instead of PWM.

Setting up for 1 MHz sine wave. Output not good. See spikes at 1 MHz, but a lot of noise. Not a sine wave.

## 2026-04-17

Assembling TC1 (Transmitter)

Ibom had some wrong values, used to ones from the schematic instead.
  For C13, I'm using 10u instead of 2.2u
  For R12, R13 I'm using 22 Ω instead of 25 Ω

## 2026-01-01

TC1 sent off for manufacturing.
RX: Using AD8130 in-amp. Rest of chain I have in LTspice, still need an opamp choice, but not critical.
Work on ADC, schematic etc.

Firmware for DDS and ADC.

## 2025-12-30 Test Coupon 1: DDS Amplitude Modulation

Goal: Figure out whether 10 kHz amplitude modulaiton works with the AD9834 DDS via the FS_ADJUST pin.

Secondary goal (optional): Have a minimal "sender" circuit for the encoder, test the differential driver

- Interfaces:
  - Power in: 5V lab PSU
  - Signal out: amplified DDS signal, can power the encoder
  - Digital IO: Every single digital pin from the DDS is accessible via a pin header, for easy debugging
  - Testpoints: Every single signal is accessible, and jumpers between main sections are used for easy disconnecting of blocks.

- Power:
  - LDO for 5V -> 3.3V to supply DDS
  - LDO for ±X V -> supplies amplifier

## 2025-12-29 Getting un-stuck

- To avoid analysis paralysis, I have to get a board out for manufacturing soon.
- I'm setting a hypothetical hourly rate for myself, to avoid spending too much time in simulation and concept phase.
  - 10 CHF set-up fee (whenever I start working or thinking on the project)
  - 30 CHF/hour (min. 1h)
- Deadlines
  - [DONE] Today (Dec 29), I have identified the SGU (Single Greatest Unknown), and dedicate to building a TC (Test Coupon) to test this specific thing.
  - [TODO] Sunday 20:00 (Jan 4) at the latest, I send out Test Coupon #1 for manufacturing. If this does not happen, I trash the project.

- SGU (Single Greatest Unknowns):
  - Candidates:
    - Analog part choice
    - Filters
    - noise
    - amplification
    - oscillation/stability
    - uC interface to ADC/DAC
    - Power supply
    - Mounting/mechanical
    - DDS control via modulation port
    - Clock generator for DDS
    - Clock synchronization (DDS modulation -> ADC Sigma-Delta sampling clock)
    - uC internal code (DSP etc.)
    - Current return path / ground noise
    - Interference/EMI
    - Cables/shielding
    - Connectors
    - Encoder coils analog model
  - Shortlist:
    - Analog chain concept: Get a signal from a signal generator, amplify it to get it into the encoder, amplify and mix down the response, look at it on the scope. NO: I basically already did that on breadboard, only one buffer was missing honestly. I have high confidence this will work. I trust the concept.
    - Analog chain implementation: Instead of breadboard, use the real SMD components, characterise the noise levels, test the amplification gain of the stages. Make sure nothing oscillates. No digital parts on board. NO: I realise I am mostly anxious to test the digital part, which I don’t have any experience with.
    - Digital section: I have never worked with a crystal oscillator before (shocking, I know), and I have two DSS’s and an ADC, all of which need sync’d clocks or control inputs from the uC. Goal: Get the individual digital parts working with their clocks synchronised and correct control signals from the uC. Slap on a basic analog section, first guess, and be ready to just swap out the analog part for something better in a separate TC. Just measure SOMETHING on the ADC that shows that the clocks are in phase, disregard noise. Use a Teensy dev-board
    - No, keep it even simpler: Make a AD9834 Test Coupon: Just the DDS, a basic amplifier for the output, and break out every single pin. This is the “warm-up” exercise. Goal: Get the amplitude modulation working with the FS_ADJUST pin. EEVblog forum members write that I may have to reduce COMP capacitor to make it work for my modulation frequency. This is something that I can’t figure out in LTspice, the datasheet does not mention it, thus it needs a physical test board. It’s crucial to figure this out: The encoder concept depends on being able to modulate the DSS output at 10 kHz or so. If it works: use it as a small board that is part of the encoder, or copy-paste the PCB design in with confidence wherever needed. If it doesn't work: I saved a lot of design time by identifiying the cuplrit early, the rest of the encoder would not have worked anyway!

- Oscillators 75 MHz for DDS: [digikey](https://www.digikey.ch/de/products/filter/oszillatoren/172?s=N4IgjCBcoEwOwDYqgMZQGYEMA2BnApgDQgD2UA2uHACxhgCsIAusQA4AuUIAyuwE4BLAHYBzEAF9iYAJwAOachBpIWPEVIUQABmaSQMBjuggOXAKpCB7APLoAsvky4Arn3whizrnYASALw8QAFthLjhGYiDMAA8wxj0YAGZE2UVlVQJiMkhKZPotaR0WE05IHn5hMT0AWgVjZX5ndWzKCCYamDSoRubNRna9JGMBABMuarAtTrZSkED2AE9WdzKR3DRiAEdFrglxIA)

## 2025-12-02

- Finished analog chain in LTspice
- Mixer works at 200 kHz as well as 2.5 MHz, that part should be fine
- The only adjustable gain I need is at the RX amp, since it depends strongly on encoder size.
- PCB layout plan
  - Put everything on one PCB, 10x10 cm minimum size (larger is fine for easier bodging)
  - Separate into logical sections: DDS/ADC/clock generator/analog chain
    - Connect with traces that have a jumper on them, so can easily interrupt traces and connect other signals
    - (SMT jumpers would be great to save time)
    - this keeps it modular and still functional, even if part of the board fails

## 2025-11-30

Laying out the analog board in LTspice, selecting gains.
Considering the LM6172 as the do-it-all amplifier (50 mA current, fast, drives 1nF well)

## 2025-11-03

- Continued from yesterday
  - Realised I used 1 nF caps instead of 10 nF yesterday.
  - If I use 10 nF for sender and receiver, 10Ω series at sender for current limit, I get the expeceted peak around 2.5 MHz, gain ≈ 0.35 peak.
  - Still weaker than what I had on 2025-08-11, but at least there's a peak and it's at the expected frequency.
  - K ≈ 0.1 in LTspice seems a decent match for the behaviour I see.

Does this give me all the information I need to prototype the analog chain?

- Max. frequency and bandwidth: yes, 2.5 MHz should be fine, let's say 3 MHz for margin.
- LTspice model: yes, I have one that seems to reproduce the behaviour well (two 500 nH inductors, K = 0.1)
- Capacitors for resonant circuit: 10 nF on RX and TX
- Current compliance: Get it from LTspice model, AD3 had no problems in resonance with 1Vpp + 10Ω series resistor
- Noise levels: Don't know yet, best to build a prototype and measure.

So, what is the next step?

- Analog chain prototype: Gain stages, mixers, testpoints.
  - Frequencies come from a different board or AD3/signal generator
  - ADC also on a different board, differential via STP wire
- In more detail
  - Three input ports for TX1, TX2, LO
  - Power amplifiers for TX1, TX2 (+LO, might as well):
    - DDS output is in the range of 0.6 Vpp + 200 Ω impedance
    - Selectable gain with resistor or jumper: 1x, 5x, 10x?
    - current compliance 100 mA is plenty
    - voltage level? 3.3 V would be nice as it's universal, but analog chain may well be ±12 V or so if needed.
  - Outputs for amplified TX1, TX2: STP wire
  - Inputs for RX1, RX2
  - Amplifiers for RX1, RX2: not much power needed (driving diodes, see LTspice), selectable gain again. May be worth making these differential, not sure
  - Diode mixers with LO: double Schottkys for maximum efficiency
  - Low-pass filter: Prepare for a 64 kS/s sigma-delat ADC, so 100 kHz filter more than adequate.
    - Blessed art thou, sigma-delta ADC, for thy inherent filtering simplifies my analog chaine
  - That's all, folks.

Potential pitfalls

- Interference, crosstalk: Keep everything well separated, lots of ground planes, use STP wires.
  - 3 MHz is a long wavelength, lambda/10 ≈ 10 m
- Noise
  - measure and adapt, check with LTspice. Trust the physics
- PCB errors
  - Keep it modular: testpoints and jumpers everywhere, so every module can be skipped and isolated
  - keep the PCB big, so repairs and bodges are feasible
  - Use parts that I can hand-solder if neeeded (0805, SOIC ...)
- Oscillations, capacitive loads
  - Use recommended isolation resistors at TX amps for capacitive loads
  - Keep feedback loops tight
  - simulate in LTspice

What I need

- STP wires + some way to connect them to the board. Maybe just screw terminals + a crocodile clamp for shield to keep it simple.
- Decide on diff-amp vs opamp at RX: what are the trade-offs?
- PCB design + reflow

## 2025-11-02

Goal: Figure out the information I need to design the encoder board.

- Measure transfer function of current encoder, in a configuration that would be used for practical motor control.
  - This defines the maximum required frequency of the signal generators, as well as the minimum expected coupling
  - (A larger encoder will resonate at a lower frequency and have better coupling coefficient)
  - Read through lessons learned of previous attempts at measuring the encoder, make a checklist.
- Build up the whole system as far as possible.
  - What is missing? Probably need buffer amps: How much gain, bandwidth, current compliance needed? Which capacitive + inductive load?
  - How are the noise levels? Probably insufficient -> where does the noise come from? Cables? DDS? ICs?

### Encoder characterisation measurements

Goals: Get all information I need to design encoder electronics that interfaces with the encoder disks (DDS, RX and TX amplifiers)

This means I need:

- Measurements of L, R, C of the encoder (no resonant circuit, with resonant circuit)
- Measured impedance curve of encoder (no resonant circuit, with resonant circuit)
- Choice of capacitor/inductor to make the encoder resonant
- LTspice model of single encoder disk (no resonant circuit)
- LTspice model of full encoder system of two disks (no resonant circuit)
- Measured transfer function of two coupled disks under maximum coupling

Watch out:

- Keep wires short to avoid their parasitic impedance
- Use short/open compensation
- Check crosstalk and keep it low, this messed up some previous measurements
- Find optimum configuration with resonant circuit, then characterize.

Experiment 1: Impedance measurement of single encoder disk

- Used Analog Discovery 3, impedance measurement mode, 10 Ω resistor
- performed open and short calibration
- Sweep 1 kHz to 10 MHz with 1 V amplitude
- Results
  - Series inductance 0.4 µH to 0.6 µH from 10 kHz to 10 MHz
  - Series resistance: 0.9 Ω to 1.0 Ω from 10 kHz to 2 MHz, sharp rise afterwards
- The resistance is as expected constant and low. The rise at the end is weird
- Inductance is as expected constant and low (small encoder).
- Measurements agree with my previous measurements on 2025-05-17 of the same encoder, except for series inductance (was 0.65 to 0.75 µH).
  - I did however shorten the cables of the encoder from 40 cm to 20 cm today.
- Conclusion: use 0.5 µH series impedance + 1.0 Ω series resistance for modelling the disk.

Trying to reproduce 2025-08-11

- No resonant circuit: Hard to measure, around 0.6 V/V at 4 to 5 MHz is peak coupling.
  - Used a 10 Ω resistor in series with source, 1V amplitude. Still lots of distortion at lower frequencies.
- With resonant circuit, 10 nF in parallel with both sender and receiver
  - Very weird! I don't get a resonance peak. Very suspicious, probably something wrong here.
    - Looks like resonance at ≈ 10 MHz with 1:1 coupling
    - Nevermind, just crosstalk, signal remains even when I unplug the measurement leads on the RX
  - Should try with inductor as well, maybe that fixes it. But have I really forgotten to write that down in the entry from 2025-08-11?
  - I don't see a match with f0 = 1 / 2π sqrt(LC), very odd! This is not a missing inductor, something different is wrong here.

### Encoder analog chain prototype

Goal:  Get all information I need to design encoder electronics

This means I need:

- Try to build the full analog chain
- Check where I need additional gain or current compliance
- Measure noise and compare with expectation

## 2025-10-20

Have to figure out what voltage levels I expect at the RX: Do I need an amplifier/buffer there?
Should find that experimentally with the encoder I have.

## 2025-10-19: Solving the synchronisation problem

New maxim: Optimise for finishing, not for cost/time/effort

- Don't try to save money
- Use components that are easy to use (low pin count)
- Make recovery from mistakes easy
- uC/FPGA on external dev-board, header pins
- jumper-friendly PCBs such that broken sections can be skipped and replaced with breadboards
- use good cables and connectors to reduce mess and interference
- Use familiar tools (Zynq, lab pc, ImGui)

**Proposed solution for the clock synchronization problem:**

- A clock generator IC generates four signals
  - 50 MHz clock for a DDS, to generate a clean LO free of harmonics
  - f1 for the encoder (square wave) (or use DDS as well)
  - f2 for the encoder (sqaure wave) (or use DDS as well)
  - 8.129 MHz for the Sigma-Delta ADC clock
- The microcontroller or FPGA runs on its own clock, and samples on /DRDY from the ADC (interrupt)
  - The internal sin/cos LUT also is incremented at every /DRDY pulse
  - Or can also get it from clock generator, but for modularity's sake, I'll keep them separate until the design is more mature.

**Boards for prototyping:**

- PYNQ Z2 dev board for FPGA
- Custom PCB
  - Power (LDOs), external
  - Clock generator: Si5351A
  - DDS: AD9834
  - TX amplifiers/buffers: OPA2673
  - RX buffer: same or something less powerful
  - full-wave diode mixer (can be skipped with jumpers, then can use breadboard mixer as alternative)
  - ADC: ADS131M03 (3ch 64 kS/s)

The third ADC channel would be unused in this design, and could be used to make the encoder absolute with an additional basic low-resolution encoder with different pole count.
