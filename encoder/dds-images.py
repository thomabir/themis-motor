import numpy as np
import matplotlib.pyplot as plt

def image_amplitude(f, f_out, f_s):
    """
    Calculate the amplitude of the image at frequency f.
    
    Parameters:
    f (float): Frequency at which to calculate the image amplitude.
    f_out (float): Frequency of the original signal.
    f_s (float): Sampling frequency.
    
    Returns:
    float: Amplitude of the image at frequency f.
    """
    return f_out / f * np.sin(np.pi * f / f_s) / np.sin(np.pi * f_out / f_s)

# plotting frequency range
f_max = 100e6 # Hz
f = np.linspace(0, f_max, 1000) # Hz

# DDS parameters and signal frequency
f_out = 2.5e6 # signal frequency, Hz
f_s = 16e6 # sampling frequency, Hz

# calculate image amplitude envelope
A = image_amplitude(f, f_out, f_s)
A = np.abs(A)

# calculate image positions
# f_s - f_out, fs + f_out, 2*f_s - f_out, 2*f_s + f_out, ...
n_img_half = int(f_max // f_s) + 1
f_image = np.arange(f_s, (n_img_half + 1) * f_s, f_s)
f_image = np.concatenate((f_image - f_out, f_image + f_out))
# sort and remove duplicates
f_image = np.unique(np.sort(f_image))

# remove images above f_max
f_image = f_image[f_image <= f_max]


fig, ax = plt.subplots()
ax.plot(f/1e6, A)
# draw vertical lines at image frequencies, with the line going from 0 to the amplitude at that frequency
for freq in f_image:
    # ax.axvline(freq/1e6, color='red', linestyle='--')
    ax.semilogy([freq/1e6, freq/1e6], [0, A[np.argmin(np.abs(f - freq))]], color='red')
# add nyquist
f_nyq = f_s / 2
ax.axvline(f_nyq/1e6, color='green', linestyle='--')
# add signal frequency
ax.axvline(f_out/1e6, color='blue')
# ylim: from min to max of A, ignoring nan or inf
A_min = np.min(A[np.isfinite(A) & (A > 0)])
A_max = np.max(A[np.isfinite(A)])
ax.set_ylim(1e-3, 2)
ax.set_xlabel('Frequency (MHz)')
ax.set_ylabel('Amplitude')
plt.show()

# simulate the signal in time domain
t = np.arange(0, 1e-5, 1/(1*f_s)) # seconds
signal = np.cos(2 * np.pi * f_out * t)
# add images
for freq in f_image:
    # amplitude at this frequency
    A_image = image_amplitude(freq, f_out, f_s)
    # add the image signal
    # signal += A_image * np.sin(2 * np.pi * freq * t)
    # print image frequency and amplitude
    print(f"{20*np.log10(np.abs(A_image)):.2f} dB at {freq/1e6:.2f} MHz")

# plot the time domain signal
fig, ax = plt.subplots()
ax.plot(t * 1e3, signal) # time in milliseconds
ax.set_xlabel('Time (ms)')
ax.set_ylabel('Amplitude')
plt.show()