#include <WiFi.h>

const char* ssid = "FAMILIA_ERASO_CHAVEZ_PLUS";
const char* password = "Juanda280904";

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println(WiFi.localIP());
}

void loop() {
  delay(1000);
}
