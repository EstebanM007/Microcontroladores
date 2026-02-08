/*
 * Código de Prueba para LCD 16x2 I2C con ESP32
 * Compatible con la librería LiquidCrystal_I2C instalada
 * 
 * Conexiones:
 * - SDA -> GPIO 21 (verde)
 * - SCL -> GPIO 22 (azul)  
 * - VCC -> 5V (rojo)
 * - GND -> GND (negro)
 */

#include <LiquidCrystal_I2C.h>

// Dirección I2C del LCD (normalmente 0x27 o 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando prueba de LCD...");
  
  // Inicializar el LCD
  lcd.begin();  // Para ESP32 usa los pines por defecto (21-SDA, 22-SCL)
  lcd.backlight();
  
  // Mensaje de bienvenida
  lcd.setCursor(0, 0);
  lcd.print("  Hola Mundo!  ");
  lcd.setCursor(0, 1);
  lcd.print(" ESP32 + LCD ");
  
  delay(3000);
}

void loop() {
  // Prueba 1: Texto simple
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prueba 1:");
  lcd.setCursor(0, 1);
  lcd.print("Texto simple");
  delay(2000);
  
  // Prueba 2: Contador
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prueba 2:");
  lcd.setCursor(0, 1);
  lcd.print("Contador: ");
  
  for(int i = 0; i <= 10; i++) {
    lcd.setCursor(10, 1);
    lcd.print(i);
    lcd.print("  ");
    delay(500);
  }
  
  // Prueba 3: Texto desplazándose
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prueba 3:");
  delay(1000);
  
  String mensaje = "Texto desplazandose por la pantalla...";
  for(int pos = 0; pos < mensaje.length() - 15; pos++) {
    lcd.setCursor(0, 1);
    lcd.print(mensaje.substring(pos, pos + 16));
    delay(300);
  }
  
  delay(1000);
  
  // Prueba 4: Backlight on/off
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Prueba 4:");
  lcd.setCursor(0, 1);
  lcd.print("Backlight test");
  delay(1000);
  
  for(int i = 0; i < 3; i++) {
    lcd.noBacklight();
    delay(500);
    lcd.backlight();
    delay(500);
  }
  
  // Prueba 5: Caracteres especiales
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Caracteres:");
  lcd.setCursor(0, 1);
  lcd.print("!@#$%&*()+-=");
  delay(2000);
  
  // Prueba 6: Simulación de sensor
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temperatura:");
  
  for(int i = 20; i <= 30; i++) {
    lcd.setCursor(0, 1);
    lcd.print(i);
    lcd.print(" C  ");
    delay(500);
  }
  
  delay(1000);
  
  // Prueba 7: Display on/off
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Display:");
  lcd.setCursor(0, 1);
  lcd.print("Encender/Apagar");
  delay(1500);
  
  for(int i = 0; i < 3; i++) {
    lcd.noDisplay();
    delay(500);
    lcd.display();
    delay(500);
  }
  
  delay(1000);
}

/*
 * SOLUCIÓN DE PROBLEMAS:
 * 
 * 1. Si la pantalla está en blanco:
 *    - Ajusta el potenciómetro azul en el módulo I2C
 *    - Gíralo lentamente hasta que veas los caracteres
 * 
 * 2. Si no funciona con dirección 0x27:
 *    - Cambia la línea 14 a: LiquidCrystal_I2C lcd(0x3F, 16, 2);
 * 
 * 3. Para encontrar tu dirección I2C, sube este código:
 */

// ESCÁNER I2C - Copia este código en un nuevo sketch:
/*
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.println("\n=== Escáner I2C ===");
  Serial.println("Buscando dispositivos...\n");
  
  byte count = 0;
  
  for(byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();
    
    if(error == 0) {
      Serial.print("Dispositivo encontrado en 0x");
      if(addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.println(" <-- USA ESTA DIRECCIÓN");
      count++;
    }
  }
  
  if(count == 0) {
    Serial.println("No se encontraron dispositivos");
    Serial.println("Verifica las conexiones:");
    Serial.println("- SDA -> GPIO 21");
    Serial.println("- SCL -> GPIO 22");
    Serial.println("- VCC -> 5V");
    Serial.println("- GND -> GND");
  } else {
    Serial.print("\nTotal de dispositivos: ");
    Serial.println(count);
  }
}

void loop() {}
*/