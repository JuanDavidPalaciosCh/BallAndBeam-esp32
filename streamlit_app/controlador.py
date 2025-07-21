import numpy as np
import control as ctrl


def calcular_pid_y_simular(wn, zeta, dist=0.2, tmax=10, n=3):
    # Parámetros físicos
    m = 0.021  # masa bola [kg]
    g = 9.81  # gravedad [m/s^2]
    d = 0.05  # distancia eje a haz [m]
    L = 0.4  # longitud del haz [m]
    J = 9.99e-6  # momento de inercia de la bola [kg*m^2]
    Radio = 0.03  # radio de la bola [m]

    k = -(m * g * d) / (L * (J / Radio**2 + m))
    P = ctrl.TransferFunction([k], [1, 0, 0])  # planta k/s^2

    alpha = k

    # Calcular PID
    kp = (wn**2 * (2 * zeta * n + 1)) / alpha
    ki = (n * wn**3) / alpha
    kd = (wn * (2 * zeta + n)) / alpha

    C = ctrl.TransferFunction([kd, kp, ki], [1, 0])
    T = ctrl.feedback(C * P, 1)

    # Simular respuesta al escalón
    t = np.linspace(0, tmax, 200)
    t, y = ctrl.step_response(dist * T, T=t)

    # Calcular señal de control u(t)
    e = dist - y
    dt = t[1] - t[0]
    integral = 0.0
    u = np.zeros_like(e)
    for k_i in range(1, len(t)):
        integral += 0.5 * (e[k_i] + e[k_i - 1]) * dt
        derivative = (e[k_i] - e[k_i - 1]) / dt
        u[k_i] = kp * e[k_i] + ki * integral + kd * derivative

    u_deg = -np.rad2deg(u) + 90

    return {"kp": kp, "ki": ki, "kd": kd, "t": t, "y": y, "u_deg": u_deg, "dist": dist}
