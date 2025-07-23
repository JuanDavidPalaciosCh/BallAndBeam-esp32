import requests


def enviar_pid_wifi(
    kp: float,
    ki: float,
    kd: float,
    dist: float,
    ip_esp32: str,
    timeout: int = 3,
    referencia_fisica: bool = False,
):
    """
    Envía los parámetros PID y la distancia de referencia al ESP32 mediante una petición HTTP GET.

    Parámetros:
    - kp, ki, kd: valores del controlador
    - dist: distancia de referencia en metros
    - ip_esp32: dirección IP o hostname del ESP32, ej. '192.168.1.50'
    - timeout: tiempo máximo de espera en segundos

    Retorna:
    - (True, mensaje) si el envío fue exitoso
    - (False, mensaje) si hubo error
    """
    url = f"http://{ip_esp32}/update"

    if referencia_fisica:
        params = {"kp": kp, "ki": ki, "kd": kd, "dist": dist, "ref_fisica": "1"}
    else:
        params = {"kp": kp, "ki": ki, "kd": kd, "dist": dist, "ref_fisica": "0"}
    try:
        resp = requests.get(url, params=params, timeout=timeout)
        if resp.status_code == 200:
            return True, (
                f"PID y referencia enviados correctamente: "
                f"Kp={kp:.3f}, Ki={ki:.3f}, Kd={kd:.3f}, Ref={dist:.3f}, ref_fisica={int(referencia_fisica)}"
            )
        else:
            return False, f"Error {resp.status_code} al enviar al ESP32"
    except Exception as e:
        return False, f"No se pudo enviar: {e}"
