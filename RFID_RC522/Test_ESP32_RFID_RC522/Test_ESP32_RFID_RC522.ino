/*
 * Programa de prueba para módulo RFID-RC522 con ESP32 de 30 pines
 * 
 * Conexiones:
 * RFID RC522 -> ESP32
 * SDA  -> D4 (GPIO 4)
 * SCK  -> D18 (GPIO 18)
 * MOSI -> D23 (GPIO 23)
 * MISO -> D19 (GPIO 19)
 * GND  -> GND
 * RST  -> D15 (GPIO 15)
 * 3.3V -> 3.3V
 * 
 * LED integrado -> GPIO 2
 * Botón integrado (BOOT) -> GPIO 0
 */

#include <SPI.h>
#include <MFRC522.h>

// Pines del RFID
#define SS_PIN 4
#define RST_PIN 15

// Pines integrados en la ESP32
#define LED_PIN 2      // LED integrado (azul)
#define BUTTON_PIN 0   // Botón BOOT integrado

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;

// Variables para almacenar hasta 5 tarjetas autorizadas
byte authorizedUIDs[5][4];
int authorizedCount = 0;
bool learningMode = false;

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Inicializar SPI
  SPI.begin();
  
  // Inicializar RFID
  rfid.PCD_Init();
  
  // Preparar la clave (por defecto FFFFFFFFFFFFh)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  // Señal de inicio (3 parpadeos rápidos)
  for(int i = 0; i < 3; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
  
  Serial.println("\n=================================");
  Serial.println("  RFID-RC522 Sistema de Prueba");
  Serial.println("=================================");
  Serial.println("\nMódulo RFID inicializado correctamente");
  Serial.println("Versión: " + String(rfid.PCD_ReadRegister(rfid.VersionReg), HEX));
  Serial.println("\n--- INSTRUCCIONES ---");
  Serial.println("• Acerca una tarjeta/tag al lector para leerla");
  Serial.println("• Mantén presionado el botón BOOT y acerca");
  Serial.println("  una tarjeta para registrarla (máx. 5)");
  Serial.println("• Las tarjetas registradas encenderán el LED");
  Serial.println("=================================\n");
}

void loop() {
  // Verificar si se presiona el botón para entrar en modo aprendizaje
  if (digitalRead(BUTTON_PIN) == LOW && !learningMode) {
    learningMode = true;
    Serial.println("\n>>> MODO APRENDIZAJE ACTIVADO <<<");
    Serial.println("Acerca una tarjeta para registrarla...");
    
    // Parpadeo rápido mientras se mantiene el botón
    for(int i = 0; i < 5; i++) {
      digitalWrite(LED_PIN, HIGH);
      delay(100);
      digitalWrite(LED_PIN, LOW);
      delay(100);
    }
  }
  
  if (digitalRead(BUTTON_PIN) == HIGH && learningMode) {
    learningMode = false;
    Serial.println(">>> Modo aprendizaje desactivado <<<\n");
  }
  
  // Verificar si hay una tarjeta presente
  if (!rfid.PICC_IsNewCardPresent())
    return;
  
  // Intentar leer la tarjeta
  if (!rfid.PICC_ReadCardSerial())
    return;
  
  // Mostrar información de la tarjeta
  Serial.println("\n--- Tarjeta detectada ---");
  Serial.print("UID: ");
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  
  Serial.print("Tipo: ");
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
  
  // Modo aprendizaje: guardar la tarjeta
  if (learningMode && authorizedCount < 5) {
    bool alreadyExists = false;
    
    // Verificar si ya está registrada
    for(int i = 0; i < authorizedCount; i++) {
      if (compareUID(rfid.uid.uidByte, authorizedUIDs[i], rfid.uid.size)) {
        alreadyExists = true;
        break;
      }
    }
    
    if (!alreadyExists) {
      // Guardar nueva tarjeta
      for(byte i = 0; i < rfid.uid.size; i++) {
        authorizedUIDs[authorizedCount][i] = rfid.uid.uidByte[i];
      }
      authorizedCount++;
      
      Serial.println("✓ Tarjeta registrada exitosamente!");
      Serial.println("Total registradas: " + String(authorizedCount) + "/5");
      
      // Parpadeo de confirmación
      for(int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(300);
        digitalWrite(LED_PIN, LOW);
        delay(300);
      }
    } else {
      Serial.println("⚠ Esta tarjeta ya está registrada");
      digitalWrite(LED_PIN, HIGH);
      delay(1000);
      digitalWrite(LED_PIN, LOW);
    }
  } 
  // Modo normal: verificar acceso
  else {
    bool authorized = false;
    
    // Verificar si está en la lista
    for(int i = 0; i < authorizedCount; i++) {
      if (compareUID(rfid.uid.uidByte, authorizedUIDs[i], rfid.uid.size)) {
        authorized = true;
        break;
      }
    }
    
    if (authorized) {
      Serial.println("✓ ACCESO AUTORIZADO");
      digitalWrite(LED_PIN, HIGH);
      delay(2000);
      digitalWrite(LED_PIN, LOW);
    } else {
      Serial.println("✗ ACCESO DENEGADO");
      // Parpadeo rápido de denegación
      for(int i = 0; i < 5; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
    }
  }
  
  Serial.println("-------------------------\n");
  
  // Detener la lectura
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  
  delay(500);
}

// Función para imprimir en hexadecimal
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

// Función para comparar UIDs
bool compareUID(byte *uid1, byte *uid2, byte size) {
  for(byte i = 0; i < size; i++) {
    if(uid1[i] != uid2[i]) {
      return false;
    }
  }
  return true;
}