# TC1: DDS

## Concept

Outputs:

- 8.192 MHz clock for ADC
- 2x PWM signals for FS_ADJUST (kHz to 10 kHz range)

Inputs:

- Data_Ready from ADC (≈ 64 kHz, the ADC derives it from the 8.192 MHz clock)
- SPI data from ADC

Setup:

- Populate LUTs for FS_ADJUST
- Start ADC clock

Main loop (no DMA):

- Wait for DREADY
- Read ADC
- Demodulate and downsample data (using LUT values)
- Load new duty-cycle for FS_ADJUST from LUT
