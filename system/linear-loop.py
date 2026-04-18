import numpy as np
import control as ct
import matplotlib.pyplot as plt

# TODOs
# write down requirements: Time to settle, residual noise RMS
# include constraints: max torque, max current, max speed, max acceleration
# model encoder noise
# use self-consistent model: Plant = 1/(Js^2), encoder estimates y and v
# add model for motor and current loop
# model current measurement noise

# parameters
J = 0.5  # moment of inertia, kg m^2
Td = 0.1  # torque disturbance, N m

s = ct.tf("s")
P = 1 / (J * s)  # telescope transfer function: torque to angular velocity

#
# Velocity loop
#

omega_v = 30 * 2 * np.pi  # desired natural frequency of the velocity loop, rad/s

# PD controller
kp_v = J * omega_v
ki_v = 0.01 * J * omega_v**2
kd_v = 0
C_v = kp_v + ki_v / s + kd_v * s  # PID controller
print(f"Velocity loop parameters: omega_n={omega_v / (2 * np.pi)} Hz")
print(f"Velocity PID gains: kp={kp_v}, ki={ki_v}, kd={kd_v}")

# transfer functions
L_v = C_v * P  # open-loop transfer function
S_v = P / (1 + L_v)  # load sensitivity function (disturbance to output)
CL_v = ct.feedback(L_v, 1)  # closed-loop transfer function (setpoint to output)

# bode plot of S and CL
# omega = np.geomspace(0.1, 100, 100)
# mag_S, phase_S, omega_S = ct.frequency_response(S, omega)
# mag_CL, phase_CL, omega_CL = ct.frequency_response(CL, omega)
# mag_L, phase_L, omega_L = ct.frequency_response(L, omega)
# mag_T, phase_T, omega_T = ct.frequency_response(T, omega)
# mag_C, phase_C, omega_C = ct.frequency_response(C, omega)
# fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))
# ax1.semilogx(omega_T, 20 * np.log10(mag_T), label='Plant T')
# ax1.semilogx(omega_C, 20 * np.log10(mag_C), label='Controller C')
# ax1.semilogx(omega_L, 20 * np.log10(mag_L), label='Loop L')
# ax1.semilogx(omega_S, 20 * np.log10(mag_S), label='Load sensitivity S')
# ax1.semilogx(omega_CL, 20 * np.log10(mag_CL), label='Closed-loop CL')

# ax1.set_ylabel('Magnitude [dB]')
# ax1.grid(which='both', linestyle='--', linewidth=0.5)
# ax1.legend()
# ax2.semilogx(omega_T, phase_T * (180 / np.pi), label='Plant T')
# ax2.semilogx(omega_C, phase_C * (180 / np.pi), label='Controller C')
# ax2.semilogx(omega_L, phase_L * (180 / np.pi), label='Loop L')
# ax2.semilogx(omega_S, phase_S * (180 / np.pi), label='Load sensitivity S')
# ax2.semilogx(omega_CL, phase_CL * (180 / np.pi), label='Closed-loop CL')
# ax2.set_ylabel('Phase [deg]')
# ax2.set_xlabel('Frequency [rad/s]')
# ax2.grid(which='both', linestyle='--', linewidth=0.5)
# plt.suptitle('Bode Plots of Sensitivity and Closed-Loop Transfer Function')
# plt.show()

# step response
# t = np.linspace(0, 1, 200)
# t, y_closed = ct.step_response(CL, t)
# t, y_open = ct.step_response(T, t)
# fig, ax = plt.subplots()
# ax.plot(t, y_closed, label='Closed-loop')
# ax.plot(t, np.ones_like(t), 'k--', label='Setpoint')
# ax.set_title('Step Response to Setpoint Change')
# ax.set_xlabel('Time [s]')
# ax.set_ylabel('Angular Position [rad]')
# plt.grid()
# plt.legend()
# plt.show()

# response to torque disturbance (impulse of 1 at t=0)
# t, y_dist = ct.step_response(S, t)
# fig, ax = plt.subplots()
# ax.plot(t, y_dist, label='Disturbance Response')
# ax.set_title('Response to Torque Disturbance')
# ax.set_xlabel('Time [s]')
# ax.set_ylabel('Angular Position [rad]')
# plt.grid()
# plt.legend()
# plt.show()

#
# Position loop
#

# requirement: 0.5 arcseconds settle
y_req_arcsec = 0.5  # arcseconds
y_req = y_req_arcsec / 3600 * np.pi / 180  # radians

# the inner loop is CL_v, which goes from torque to angular velocity
# we need to add an integrator to get from velocity to position
P_inner = CL_v * 1 / s  # velocity loop transfer function, extracting position

# P controller
zeta_pos = 1  # desired damping ratio of the outer loop
omega_pos = omega_v / (2 * zeta_pos)  # critical damping
kp_pos = omega_pos**2 / omega_v
ki_pos = 0
# Td_pos = (2 * zeta_pos * omega_pos - omega_v) / omega_pos**2
# kd_pos = Td_pos * kp_pos
kd_pos = 0
C_p = kp_pos + ki_pos / s + kd_pos * s
print(f"Position loop params: zeta={zeta_pos}, omega_n={omega_pos / (2 * np.pi)} Hz")
print(f"Position PID gains: kp={kp_pos}, ki={ki_pos}, kd={kd_pos}")

# transfer functions
L_pos = C_p * P_inner # open-loop transfer function
CL_pos = ct.feedback(L_pos, 1) # closed-loop transfer function (setpoint to output)

# step response plot: step response and error
t = np.linspace(0, 5, 200)
t, y_closed = ct.step_response(CL_pos, t)
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
ax = axs[0]
ax.plot(t, y_closed, label="Closed-loop position")
ax.plot(t, np.ones_like(t), "k--", label="Setpoint")
ax.set_ylabel("Angular Position [rad]")
ax.legend()
ax.grid()
ax = axs[1]
error = np.abs(1 - y_closed)
ax.semilogy(t, error, label="Position Error")
ax.axhline(y=y_req, color='r', linestyle='--', label=f"Requirement ({y_req_arcsec} arcsec)")
ax.legend()
ax.set_xlabel("Time [s]")
ax.set_ylabel("Absolute Error [rad]")
plt.grid()
plt.show()

# ramp response plot: 7.2e-5 rad/s = 15 arcsec/s sidereal rate
v_sidereal = 15 / 3600 * np.pi / 180  # rad/s
y_target = v_sidereal * t
t, y_ramp = ct.step_response(CL_pos * v_sidereal / s, t)
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
ax = axs[0]
ax.plot(t, y_ramp, label="Closed-loop position")
ax.plot(t, y_target, "k--", label="Setpoint")
ax.set_ylabel("Angular Position [rad]")
ax.legend()
ax.grid()
ax = axs[1]
error = np.abs(y_target - y_ramp)
ax.semilogy(t, error, label="Position Error")
ax.axhline(y=y_req, color='r', linestyle='--', label=f"Requirement ({y_req_arcsec} arcsec)")
ax.legend()

# disturbance response
S_taud_to_y = P / s / (1 + C_v * P / s * (s + C_p))
t, y_dist = ct.step_response(S_taud_to_y, t)
fig, axs = plt.subplots(2, 1, figsize=(8, 8), sharex=True)
ax = axs[0]
ax.plot(t, y_dist, label="Disturbance Response")
ax.set_ylabel("Angular Position [rad]")
ax.legend()
ax.grid()
ax = axs[1]
error = np.abs(y_dist)
ax.semilogy(t, error, label="Position Error")
ax.axhline(y=y_req, color='r', linestyle='--', label=f"Requirement ({y_req_arcsec} arcsec)")
ax.legend()
ax.set_xlabel("Time [s]")
ax.set_ylabel("Absolute Error [rad]")
plt.grid()
plt.show()

