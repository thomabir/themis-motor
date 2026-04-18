import numpy as np
import control as ct

def second_order_filter(x_step, a_max=0, v_max=0):
    omega = min(np.sqrt(a_max/x_step), 2 * v_max / x_step)
    s = ct.tf("s")
    F = omega**2 / (s**2 + 2 * omega * s + omega**2)
    return F
