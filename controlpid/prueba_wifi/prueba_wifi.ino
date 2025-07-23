#include <WiFi.h>
#include <WebServer.h>

// ===== CONFIGURACIÓN WIFI =====
const char* ssid = "motoedge50fusion_1817";
const char* password = "69122422";

// ===== VALORES PID (por defecto) =====
float kp = 1.0;
float ki = 0.5;
float kd = 0.2;
float ref = 0.2;

// ===== LED indicador =====
const int LED_PIN = 2;

// ===== Servidor HTTP =====
WebServer server(80);


// ===== HANDLER para actualizar PID =====
void handleUpdate() {
  // Leer parámetros de la URL si existen
  if (server.hasArg("kp")) kp = server.arg("kp").toFloat();
  if (server.hasArg("ki")) ki = server.arg("ki").toFloat();
  if (server.hasArg("kd")) kd = server.arg("kd").toFloat();
  if (server.hasArg("dist")) ref = server.arg("dist").toFloat();

  String message = "Valores PID actualizados:\n";
  message += "Kp = " + String(kp, 4) + "\n";
  message += "Ki = " + String(ki, 4) + "\n";
  message += "Kd = " + String(kd, 4) + "\n";
  message += "Ref = " + String(ref, 4) + "\n";

  server.send(200, "text/plain", message);
  Serial.println(message);
}

// ===== HANDLER para raíz =====
void handleRoot() {
  String message = "Valores actuales:\n";
  message += "Kp = " + String(kp, 4) + "\n";
  message += "Ki = " + String(ki, 4) + "\n";
  message += "Kd = " + String(kd, 4) + "\n";
  message += "Ref = " + String(ref, 4) + "\n";
  server.send(200, "text/plain", message);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // LED apagado inicialmente
  
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  
  // Tiempo máximo de espera en milisegundos
  unsigned long tiempoInicio = millis();
  unsigned long tiempoEspera = 5000; // 5 segundos
  
  while (WiFi.status() != WL_CONNECTED && (millis() - tiempoInicio) < tiempoEspera) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi conectado.");
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_PIN, HIGH); // LED encendido
    // Configurar rutas del servidor
    server.on("/", handleRoot);
    server.on("/update", handleUpdate);
    server.begin();
    Serial.println("Servidor HTTP iniciado.");
  } else {
    Serial.println("");
    Serial.println("⚠️ No se pudo conectar al WiFi en el tiempo esperado. Continuando...");
  }
}

void loop() {
  // Verificar conexión WiFi
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH); // sigue encendido
  } else {
    digitalWrite(LED_PIN, LOW); // apagado si se pierde la conexión
  }

  // Atender peticiones HTTP entrantes
  server.handleClient();

  // Aquí podrías usar los valores kp, ki, kd para tu control

  Serial.println(kp);

  delay(1000);
}
