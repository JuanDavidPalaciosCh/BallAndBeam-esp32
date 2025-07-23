#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <math.h>

// lib wifi
#include <WiFi.h>
#include <WebServer.h>


// ==== PID por defecto ====
float Kp = -4.330;
float Ki =  -1.396;
float Kd =  -4.398;
float Ts = 0.1;
float ref = 0.2;
float distancia1 = 0.2;
float error = 0.0;
int angulo = 90;

int usarReferenciaFisica = 0;

// ==== PID Interno ====
float integral = 0.0;
float error_anterior = 0.0;

// ==== Servo ====
Servo miServo;
int pinServo = 18;

// === Pines XSHUT de cada VL53L0X ===
#define SDA_PIN 21
#define SCL_PIN 22
#define XSHUT1_PIN 23
#define XSHUT2_PIN 19

// ==== Objetos para cada sensor ====
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X();
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X();


// === Variables de filtrado ===
const int N = 5;  // tamaño de ventana
float ref_buffer[N];
int ref_index = 0;
float ref_sum = 0;
int ref_count = 0;


// === Conexion internet ===
// ===== LED indicador =====
const int LED_PIN = 2;

// ===== Servidor HTTP =====
WebServer server(80);

// ===== CONFIGURACIÓN WIFI =====
const char* ssid = "lab_control";
const char* password = "lab_control";

bool serverIniciado = false;


void setup() {
  Serial.begin(115200);
  // WiFi
  startWifi();

  // Servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  miServo.setPeriodHertz(50);
  miServo.attach(pinServo, 500, 2400);
  miServo.write(90);

  // I2C
  Wire.begin(SDA_PIN, SCL_PIN);

  inicializarSensoresLaser();

}

void loop() {
  // ===== WiFi ====
  // Verificar conexión WiFi
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_PIN, HIGH);
    // Si antes no estaba iniciado el servidor, iniciarlo ahora
    if (!serverIniciado) {
      server.on("/update", handleUpdate);
      server.on("/datos", handleDatos);
      server.begin();
      Serial.println("Servidor HTTP iniciado (loop).");
      serverIniciado = true;
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    serverIniciado = false;
  }

  // Atender peticiones HTTP entrantes
  if (serverIniciado){
    server.handleClient();
  }

  
  // ===== Lecturas =====
  distancia1 = leerSensor(lox1);

  if (distancia1 < 0){
    distancia1 = 0;
  }

  if(WiFi.status() != WL_CONNECTED || usarReferenciaFisica){
    float distancia2 = leerSensor(lox2);
    ref = distancia2 + 0.01;

    if (ref < 0){
      ref = 0;
    }
  }

  // ===== Calcular error =====
  error = ref - distancia1;

  // ===== Zona muerta =====
  if (fabs(error) < 0.001) {
    error = 0;
  }

  // ====== PID ======
  float proporcional = Kp * error;
  float integral_provisional = integral + error * Ki * Ts;
  float derivativo = Kd * (error - error_anterior) / Ts;

  float control_rad = (proporcional + integral_provisional + derivativo);

  Serial.println(control_rad);


  float control_deg = -1* control_rad * 180.0f / (float)M_PI;

  control_deg = constrain(control_deg, -90, 90);

  // ====== Normalizar y saturar ======
  angulo = control_deg + 90;
  

  miServo.write((int)angulo);
  error_anterior = error;

  // ===== Monitor =====
  Serial.print(distancia1, 3);
  Serial.print('\t');        // tabulación
  Serial.print(ref, 3);
  Serial.print('\t');
  //Serial.println(angulo, 1); // última con salto de línea


  delay((int)(Ts * 1000));
}




// === Funciones ===


// ==== Función para leer sensor láser ====
float leerSensor(Adafruit_VL53L0X &lox) {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);

  //Serial.println(measure.RangeStatus);
  if (measure.RangeStatus != 4) {
    return 0.45 - ((measure.RangeMilliMeter) / 1000.0);
  }
  return 0.0;
}


// === Funcion para inicializar sensores laser ===
void inicializarSensoresLaser() {
  // Pines XSHUT
  pinMode(XSHUT1_PIN, OUTPUT);
  pinMode(XSHUT2_PIN, OUTPUT);

  // Apagar ambos sensores
  digitalWrite(XSHUT1_PIN, LOW);
  digitalWrite(XSHUT2_PIN, LOW);
  delay(1000);

  // Inicializar sensor 1
  digitalWrite(XSHUT1_PIN, HIGH);
  delay(1000);
  if (!lox1.begin(0x30)) {
    Serial.println(F("¡Fallo sensor 1!"));
    while (1);
  }
  Serial.println(F("Sensor 1 listo"));

  delay(500);

  // Inicializar sensor 2
  digitalWrite(XSHUT2_PIN, HIGH);
  delay(100);
  if (!lox2.begin(0x31)) {
    Serial.println(F("¡Fallo sensor 2!"));
    while (1);
  }
  Serial.println(F("Sensor 2 listo"));
  delay(1000);
}

// === Funciones wifi ===
// ===== HANDLER para actualizar PID =====
void handleUpdate() {
  // Leer parámetros de la URL si existen
  if (server.hasArg("kp")) Kp = server.arg("kp").toFloat();
  if (server.hasArg("ki")) Ki = server.arg("ki").toFloat();
  if (server.hasArg("kd")) Kd = server.arg("kd").toFloat();

  if (server.hasArg("ref_fisica")) {
    usarReferenciaFisica = (server.arg("ref_fisica") == "1");
  }
  
  // Solo actualizar ref si NO estamos usando referencia física
  if (!usarReferenciaFisica && server.hasArg("dist")) {
    ref = server.arg("dist").toFloat();
  }

  String message = "Valores PID actualizados:\n";
  message += "Kp = " + String(Kp, 4) + "\n";
  message += "Ki = " + String(Ki, 4) + "\n";
  message += "Kd = " + String(Kd, 4) + "\n";
  message += "Ref = " + String(ref, 4) + "\n";
  message += "RefFisica = " + String(usarReferenciaFisica ? "Sí" : "No") + "\n";

  server.send(200, "text/plain", message);
  Serial.println(message);
}


// === HANDLER para mandar datos ===
void handleDatos() {
  String json = "{";
  json += "\"distancia\":" + String(distancia1, 3) + ",";
  json += "\"ref\":" + String(ref, 3) + ",";
  json += "\"error\":" + String(error, 3) + ",";
  json += "\"angulo\":" + String(angulo, 1);
  json += "}";
  server.send(200, "application/json", json);
}


void startWifi() {
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
    server.on("/update", handleUpdate);
    server.on("/datos", handleDatos);
    
    server.begin();
    serverIniciado = true;
    Serial.println("Servidor HTTP iniciado.");
  } else {
    Serial.println("");
    Serial.println("⚠️ No se pudo conectar al WiFi en el tiempo esperado. Continuando...");
  }
  
}
