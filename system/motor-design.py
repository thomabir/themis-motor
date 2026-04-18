from pint import UnitRegistry
import numpy as np

ureg = UnitRegistry()
Q_ = ureg.Quantity
ureg.default_format = ".2f~P" # 2 decimal places, pretty print

B_peak = Q_("0.8 T")  # peak air-gap flux density, changes sinusoidally with position

# geometry
L_stack = Q_("5 cm")  # stack lengh
r_air_gap = Q_("7 cm")  # distance between middle of air-gap and shaft
h_stack = Q_("1 cm")  # height of the stack
p = 20  # pole pairs (not poles)
n_slots = 20  # number of slots

# wire
# N_turns = 180  # turns per phase
wire_d = Q_("0.8 mm")  # wire diameter
rho_Cu = Q_("1.72e-8 Ω m")  # resistivity of copper, 20C
k_w = 0.87  # winding number (between 0 and 1)
A_wire = np.pi * (wire_d / 2) ** 2  # cross-sectional area of wire

# fill factors and packing
A_tot = np.pi * (
    r_air_gap**2 - (r_air_gap - h_stack) ** 2
)  # total area available for winding
print(f"A_tot = {A_tot.to('cm^2')}")
slot_fill = 0.5  # fraction of total area filled that is available for copper
fill_factor = 0.6  # fraction of available area that is filled with copper
A_avail = A_tot * slot_fill * fill_factor  # area available for copper
print(f"A_avail = {A_avail.to('cm^2')}")
N_turns_per_phase = np.floor(
    A_avail / (6 * A_wire)
)  # number of turns per phase that fit in the winding area
print(N_turns_per_phase.to(""))

# turns per slot
N_turns = 3 * N_turns_per_phase  # total number of turns
print(f"N_turns = {N_turns.to('')}")
N_turns_per_slot = N_turns / n_slots
print(f"N_turns_per_slot = {N_turns_per_slot.to('')}")


I_peak = Q_("5 A")  # phase current

torque = 3 * N_turns_per_phase * B_peak * k_w * r_air_gap * L_stack * I_peak
print(f"torque = {torque.to('N m')}")

# resistance of single phase
inactive_factor = (
    1.4  # factor to account for extra length of wire due to winding and connections
)
L_ph = 2 * N_turns_per_phase * L_stack * inactive_factor
R_ph = rho_Cu * L_ph / A_wire
print(f"R_ph = {R_ph.to('ohm')}")

# thermal power loss in single phase
I_rms = I_peak / np.sqrt(2)
P_ph = I_rms**2 * R_ph
P_total = 3 * P_ph
print(f"P_total = {P_total.to('W')}")

# torque constant
K_t = torque / I_peak
print(f"K_t = {K_t.to('N m / A')}")

# back-EMF constant
K_e = K_t.to("V s / rad")
print(f"K_e = {K_e}")

# rotational speed
n_max = Q_("300 deg/s")
omega_max = n_max.to("rad/s")
print(f"omega_max = {omega_max.to('rpm')} = {omega_max.to('deg/s')}")
E_peak = K_e * omega_max
E_rms = E_peak / np.sqrt(2)
E_LL_rms = E_rms * np.sqrt(3)
print(f"E_LL_rms = {E_LL_rms.to('V')}")

# copper weight
V_Cu = A_wire * L_ph * 3  # volume of copper in all three phases
rho_Cu_mass = Q_("8.96 g/cm^3")  # density of copper
m_Cu = V_Cu * rho_Cu_mass
print(f"m_Cu = {m_Cu.to('kg')}")