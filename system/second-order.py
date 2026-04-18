import numpy as np
import control as ct
import matplotlib.pyplot as plt

# parameters
zeta = 1  # damping ratio
wn = 2 * np.pi      # natural frequency, rad/s

# Classic control engineer's goal: closed-loop transfer function is a second-order system
s = ct.tf('s')
CL = wn**2 / (s**2 + 2*zeta*wn*s + wn**2)  


# Bode plot
omega = np.geomspace(0.1, 100, 100)
mag_CL, phase_CL, omega_CL = ct.frequency_response(CL, omega)
# fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))
# ax1.semilogx(omega_CL, 20 * np.log10(mag_CL), label='CL')
# ax1.set_ylabel('Magnitude [dB]')
# ax1.grid()
# ax1.legend()
# ax2.semilogx(omega_CL, phase_CL * (180 / np.pi), label='CL')
# ax2.set_ylabel('Phase [deg]')
# ax2.set_xlabel('Frequency [rad/s]')
# ax2.grid()
# ax2.legend()
# plt.suptitle('Bode Plot of 2nd order system')
# plt.show()

# Step response
t = np.linspace(0, 5, 10000)
t, y_closed = ct.step_response(CL, t)
fig, ax = plt.subplots()
ax.plot(t, y_closed, label='Closed-loop')
ax.plot(t, np.ones_like(t), 'k--', label='Setpoint')
ax.set_title('Step Response to Setpoint Change')
ax.set_xlabel('Time [s]')
ax.set_ylabel('Angular Position [rad]')
plt.grid()
plt.legend()
plt.show()

# Rise time and overshoot
tr1 = np.where(y_closed >= 0.1)[0][0]
tr2 = np.where(y_closed >= 0.9)[0][0]
rise_time = t[tr2] - t[tr1]
overshoot = (np.max(y_closed) - 1) * 100  # in percent
print(f'Rise time: {rise_time:.3f} s')
print(f'Overshoot: {overshoot:.2f} %')


# Settling time, analytical and simulated
settle_times = []
tolerances = [0.999, 0.1, 0.01, 0.001, 0.0001, 0.00001, 0.000001]
for tol in tolerances:
    idx = np.where(np.abs(y_closed - 1) > tol)[0]
    if len(idx) == 0:
        settle_time = 0
    else:
        settle_time = t[idx[-1]]  # last time it is outside the tolerance
    settle_times.append(settle_time)
    print(f'Settling time to within {tol*100:.5f}%: {settle_time:.3f} s')

settle_time_analytical = np.exp(-t*wn)*(1+wn*t)

# plot the trend
fig, ax = plt.subplots()
ax.semilogy(settle_times, tolerances, marker='o', label='Simulated')
ax.semilogy(t, settle_time_analytical, label='Analytical')
ax.set_ylabel('Tolerance')
ax.set_xlabel('Settling Time [s]')
ax.set_title('Settling Time vs Tolerance')
ax.grid(which='both', linestyle='--', linewidth=0.5)
ax.legend()
plt.show()