import streamlit as st
from streamlit_autorefresh import st_autorefresh
import requests


def monitor_esp32(ip_esp32: str):
    """
    Consulta periódicamente los datos del ESP32 y los muestra en Streamlit.
    Usa st_autorefresh para actualizar cada 0.15 s.
    """

    st.title("Monitor ESP32 en tiempo real")

    # Esto provoca que la app se vuelva a ejecutar cada 200 ms
    st_autorefresh(interval=10000, limit=None, key="refresh")

    # Direccion IP del ESP32
    dir = "http://" + ip_esp32 + "/datos"

    print(dir)

    # Un contenedor que se actualizará cada ciclo
    placeholder = st.empty()

    try:
        response = requests.get(dir, timeout=5)
        if response.status_code == 200:
            data = response.json()
            with placeholder.container():
                st.metric("📏 Distancia", f"{data['distancia']} cm")
                st.metric("🎯 Ref", f"{data['ref']}")
                st.metric("⚖️ Error", f"{data['error']}")
                st.metric("📐 Ángulo", f"{data['angulo']}°")
        else:
            with placeholder.container():
                st.error(f"Error al obtener datos: {response.status_code}")

    except Exception as e:
        with placeholder.container():
            st.error(f"No se pudo conectar con el ESP32: {e}")
