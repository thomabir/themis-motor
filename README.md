# Themis Telescope Mount

Hardware and software for a direct-drive brushless DC motor and encoder for a telescope mount

## Control Scheme

- PWM
  - 50 to 100 kHz
  - Input: desired PWM duty cycle
  - Output: PWM signals to gate drivers
- Inner Loop: Current Control (FOC)
  - Controls flux and torque currents.
  - Runs at 10–20 kHz.
  - Inputs: Torque target (from position control loop), Phase current measurements (from current sensor), rotor electrical angle (from encoder).
  - Outputs: PWM duty cycles via inverse Park/Clarke
- Outer Loop: Position Control
  - Controls angular position and/or velocity, which is commanded from the observatory
  - Runs at 100–500 Hz.
  - Inputs: Position setpoint, measured position from encoder
  - Output: Torque command

## Motor Specifications

- Type: BLDC, direct drive (salvaged from a hoverboard)
- Quantity: 2 motors
- Electrical: 3 Phases, 30 Poles, 27 Slots, 36 V, 13 A RMS peak, 7A RMS continuous, 0.45 Ω, Y
- Mechanical: ≈ 10 Nm continuous, ≈ 1.43 Nm/A, ≈ 16 rpm/V, 350 W (estimates, not necessarily accurate)

## Encoder

DIY inductosyn, 0.05 arcseconds resolution, 1000 Hz sampling rate, custom SPI-like interface with absolute position, latency 1 ms.

## System Requirements

- Telescope Alt/Az axis control
- Tracking speed: 1 rev / 24 hr
- Slewing speed: 1 rev / 10 s
- Tracking error: < 1 arcsecond RMS
- Power: Mains
- System inertia: Low (balanced 8" SCT)

Thermal: motor will be at low currents most of the time, so convection will probably be fine, with short peaks when slewing. (Motors are from a hoverboard, which was also only passively cooled)

Mechanical: Very low backlash; bearing friction (esp. static) likely to influence control scheme.
