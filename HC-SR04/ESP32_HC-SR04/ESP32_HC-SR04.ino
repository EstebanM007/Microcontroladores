// Código de prueba para sensor ultrasónico HC-SR04 con ESP32
// Pinout:
// VCC -> VIN
// Trig -> GPIO 5
// Echo -> GPIO 18
// GND -> GND

#define TRIG_PIN 5
#define ECHO_PIN 18

// Variables para el cálculo de distancia
long duration;
float distance;

void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  Serial.println("=================================");
  Serial.println("Sensor HC-SR04 con ESP32");
  Serial.println("=================================");
  delay(1000);
}

void loop() {
  // Limpiar el pin TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Generar pulso de 10 microsegundos en TRIG
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Leer el tiempo de respuesta en ECHO
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calcular distancia en centímetros
  // Velocidad del sonido: 343 m/s = 0.0343 cm/µs
  // Distancia = (tiempo * velocidad) / 2
  distance = (duration * 0.0343) / 2;
  
  // Mostrar resultado en el monitor serial
  Serial.print("Distancia: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Verificar si la distancia está fuera de rango
  if (distance > 400 || distance < 2) {
    Serial.println("⚠️  Fuera de rango");
  }
  
  // Esperar antes de la siguiente medición
  delay(500);
}