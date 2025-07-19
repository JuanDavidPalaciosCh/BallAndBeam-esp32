#include <WiFi.h>

// 📌 Ajusta estos datos a tu red WiFi del celular
const char* ssid     = "motoedge50fusion_1817";   // ⚠️ pon aquí el nombre de tu hotspot
const char* password = "69122422";         // ⚠️ pon aquí la contraseña

// Valores predeterminados si no hay conexión
float kp_default = -5.0;
float ki_default = -2.0;
float kd_default = -1.0;
float dist_default = 0.2;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Iniciando WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando a WiFi");
  unsigned long startAttempt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 10000) { // 10s timeout
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Conectado a WiFi!");
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n⚠️ No se pudo conectar a WiFi, usando valores predeterminados.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Estamos conectados: imprime datos de conexión
    Serial.println("📡 WiFi conectado");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("RSSI (dBm): ");
    Serial.println(WiFi.RSSI());
    Serial.println("--------------------");
  } else {
    // No hay conexión: usa valores predeterminados
    Serial.println("❌ Sin WiFi, usando valores predeterminados:");
    Serial.print("Kp = "); Serial.println(kp_default);
    Serial.print("Ki = "); Serial.println(ki_default);
    Serial.print("Kd = "); Serial.println(kd_default);
    Serial.print("Distancia = "); Serial.println(dist_default);
    Serial.println("--------------------");
  }

  delay(1000); // cada segundo
}
