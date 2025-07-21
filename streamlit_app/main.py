import streamlit as st
import matplotlib.pyplot as plt
from controlador import calcular_pid_y_simular
from enviar_esp import enviar_pid_wifi

st.set_page_config(page_title="PID Ball-Beam", layout="wide")

st.title("Simulador de PID Ball-Beam")

# Controles en barra lateral
st.sidebar.header("Parámetros del controlador")
wn = st.sidebar.slider(
    "Frecuencia natural (wn)", min_value=0.1, max_value=3.0, value=0.799, step=0.1
)
zeta = st.sidebar.slider(
    "Amortiguamiento (ζ)", min_value=0.1, max_value=1.0, value=0.95, step=0.01
)
dist = st.sidebar.slider(
    "Referencia (m)", min_value=0.00, max_value=0.35, value=0.2, step=0.01
)

# Calcular
resultados = calcular_pid_y_simular(wn, zeta, dist)

st.subheader("Parámetros calculados")
col1, col2, col3 = st.columns(3)
col1.metric("Kp", f"{resultados['kp']:.3f}")
col2.metric("Ki", f"{resultados['ki']:.3f}")
col3.metric("Kd", f"{resultados['kd']:.3f}")

# Graficar salida y control
fig, axes = plt.subplots(2, 1, figsize=(8, 6), sharex=True)

# Salida
axes[0].plot(resultados["t"], resultados["y"], "b", label="Salida y(t)")
axes[0].axhline(resultados["dist"], color="r", linestyle="--", label="Referencia")
axes[0].set_ylabel("Posición [m]")
axes[0].set_title("Respuesta al escalón - Salida")
axes[0].legend()
axes[0].grid(True)

# Control
axes[1].plot(resultados["t"], resultados["u_deg"], "y", label="Entrada u(t) [°]")
axes[1].axhline(180, color="red", linestyle="--", label="Límite superior")
axes[1].axhline(0, color="red", linestyle="--", label="Límite inferior")
axes[1].axhline(90, color="green", linestyle="--", label="Centro servo")
axes[1].set_xlabel("Tiempo [s]")
axes[1].set_ylabel("Ángulo [°]")
axes[1].set_title("Señal de control al servo")
axes[1].legend()
axes[1].grid(True)

plt.tight_layout()
st.pyplot(fig)


# Envío a la ESP32 wifi
if st.sidebar.button("Enviar PID al ESP32"):
    ok, mensaje = enviar_pid_wifi(
        resultados["kp"],
        resultados["ki"],
        resultados["kd"],
        resultados["dist"],
        ip_esp32="192.168.1.50",  # Cambiar IP
    )

    if ok:
        st.sidebar.success(mensaje)
    else:
        st.sidebar.error(mensaje)
