#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_VL53L0X.h>
#include <math.h>

// ==== PID ====
float Kp = -5.384351559633028 ;
float Ki = -2.7733211009174314;
float Kd = -5.037629357798164;
float Ts = 0.1;

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

// ==== Función para leer sensor láser ====
float leerSensor(Adafruit_VL53L0X &lox) {
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  if (measure.RangeStatus != 4) {
    return 0.4 - (measure.RangeMilliMeter - 40) / 1000.0;
  }
  return 0.0;
}

void setup() {
  Serial.begin(115200);

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

  // Pines XSHUT
  pinMode(XSHUT1_PIN, OUTPUT);
  pinMode(XSHUT2_PIN, OUTPUT);

  // Apagar ambos sensores
  digitalWrite(XSHUT1_PIN, LOW);
  digitalWrite(XSHUT2_PIN, LOW);
  delay(1000);

  // Inicializar sensor 1
  digitalWrite(XSHUT1_PIN, HIGH);
  delay(100);
  if (!lox1.begin(0x30)) {
    Serial.println(F("¡Fallo sensor 1!"));
    while (1);
  }
  Serial.println(F("Sensor 1 listo"));

  delay(500);
}

void loop() {
  // ===== Lecturas =====
  float distancia1 = leerSensor(lox1);

  // Valor de referencia fijo por ahora
  float distancia2 = 0.2;
  float referencia = distancia2;

  // ===== Calcular error =====
  float error = referencia - distancia1;

  // ===== Zona muerta de ±15 mm =====
  if (fabs(error) < 0.015) {
    error = 0;
  }

  // ====== PID ======
  float proporcional = Kp * error;
  float integral_provisional = integral + error * Ki * Ts;
  float derivativo = Kd * (error - error_anterior) / Ts;

  float control_rad = (proporcional + integral_provisional + derivativo);

  // ====== Limitar señal ======
  const float limiteRad = M_PI / 2.0f;
  if (control_rad > limiteRad) control_rad = limiteRad;
  if (control_rad < -limiteRad) control_rad = -limiteRad;

  if (fabs(proporcional + derivativo) < limiteRad) {
    integral = integral_provisional;
  }

  float control_deg = control_rad * 180.0f / (float)M_PI + 90.0f;

  // ====== Normalizar y saturar ======
  float angulo = fmod(control_deg, 360.0f);
  if (angulo < 0) angulo += 360.0f;
  if (angulo > 180.0f) angulo = 180.0f;
  if (angulo < 0.0f) angulo = 0.0f;

  miServo.write((int)angulo);
  error_anterior = error;

  // ===== Monitor =====
  Serial.print("Distancia: "); Serial.print(distancia1, 3);
  Serial.print("  Ref: "); Serial.print(referencia, 3);
  Serial.print("  Error: "); Serial.print(error, 3);
  Serial.print("  Angulo: "); Serial.println(angulo, 1);

  delay((int)(Ts * 1000));
}
