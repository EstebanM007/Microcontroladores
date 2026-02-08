/*
 * TECLADO MATRICIAL 4x4 CON ESP32
 * ================================
 * 
 * DESCRIPCIÓN:
 * Código para lectura de teclado matricial 4x4 usando ESP32.
 * Detecta y muestra en el Monitor Serial cada tecla presionada.
 * 
 * HARDWARE:
 * - ESP32 DevKit
 * - Teclado matricial 4x4 (8 pines)
 * 
 * CONEXIONES (Pines lado izquierdo ESP32):
 * ----------------------------------------
 * FILAS (Pines 1-4 del keypad):
 *   Pin 1 (Fila 1) → GPIO 32
 *   Pin 2 (Fila 2) → GPIO 33
 *   Pin 3 (Fila 3) → GPIO 25
 *   Pin 4 (Fila 4) → GPIO 26
 * 
 * COLUMNAS (Pines 5-8 del keypad):
 *   Pin 5 (Columna 1) → GPIO 13
 *   Pin 6 (Columna 2) → GPIO 12
 *   Pin 7 (Columna 3) → GPIO 14
 *   Pin 8 (Columna 4) → GPIO 27
 * 
 * LIBRERÍA REQUERIDA:
 * - Keypad by Mark Stanley, Alexander Brevig
 *   (Instalar desde el Administrador de Bibliotecas de Arduino IDE)
 * 
 * CONFIGURACIÓN:
 * - Baudrate: 115200
 * - LED integrado (GPIO 2) parpadea al detectar tecla
 * 
 * NOTA IMPORTANTE:
 * El mapeo de la matriz se ajustó según el hardware específico del keypad.
 * Si usas otro keypad y las teclas no coinciden, ajusta la matriz 'keys'.
 */

#include <Keypad.h>

// ========== CONFIGURACIÓN DEL TECLADO ==========
const byte ROWS = 4;  // Cuatro filas
const byte COLS = 4;  // Cuatro columnas

// Mapeo de teclas según el layout físico del keypad
// IMPORTANTE: Esta matriz está ajustada para este keypad específico
char keys[ROWS][COLS] = {
  {'A','B','C','D'},  // Fila 1: A B C D
  {'3','6','9','#'},  // Fila 2: 3 6 9 #
  {'2','5','8','0'},  // Fila 3: 2 5 8 0
  {'1','4','7','*'}   // Fila 4: 1 4 7 *
};

// Pines GPIO conectados a las FILAS del keypad
byte rowPins[ROWS] = {32, 33, 25, 26};

// Pines GPIO conectados a las COLUMNAS del keypad
byte colPins[COLS] = {13, 12, 14, 27};

// Crear objeto Keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LED integrado para feedback visual
const int LED_BUILTIN = 2;

// ========== CONFIGURACIÓN INICIAL ==========
void setup() {
  // Inicializar comunicación serial
  Serial.begin(115200);
  
  // Configurar LED integrado como salida
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  // Esperar estabilización
  delay(1000);
  
  // Mensaje de inicio
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║  TECLADO MATRICIAL 4x4 - ESP32   ║");
  Serial.println("╚════════════════════════════════════╝");
  Serial.println("\nSistema inicializado correctamente.");
  Serial.println("Esperando entrada del teclado...\n");
}

// ========== BUCLE PRINCIPAL ==========
void loop() {
  // Leer tecla presionada
  char key = keypad.getKey();
  
  // Si se detecta una tecla
  if (key) {
    // Activar LED como feedback visual
    digitalWrite(LED_BUILTIN, HIGH);
    
    // Mostrar tecla presionada
    Serial.print("► Tecla: ");
    Serial.print(key);
    
    // Clasificar tipo de tecla
    if (key >= '0' && key <= '9') {
      Serial.println(" [NÚMERO]");
    } else if (key >= 'A' && key <= 'D') {
      Serial.println(" [LETRA]");
    } else if (key == '*') {
      Serial.println(" [ASTERISCO]");
    } else if (key == '#') {
      Serial.println(" [NUMERAL]");
    }
    
    // Apagar LED
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
  }
}