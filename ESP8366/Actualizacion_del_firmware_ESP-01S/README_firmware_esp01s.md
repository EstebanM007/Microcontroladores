# 🔧 Actualización de Firmware ESP-01S (ESP8266)

> Guía completa para actualizar el firmware AT del módulo ESP-01S usando Flash Download Tools de Espressif. Basada en el tutorial en video y los recursos oficiales de RemoteXY y Espressif.

📺 **Video de referencia:** [Actualización de Firmware ESP-01S](https://www.youtube.com/watch?v=31TQMxeyFL4&t=1160s)

---

## 📋 Tabla de Contenidos

- [¿Por qué actualizar el firmware?](#-por-qué-actualizar-el-firmware)
- [Recursos y Herramientas](#-recursos-y-herramientas)
- [Hardware Necesario](#-hardware-necesario)
- [Diagrama de Conexiones](#-diagrama-de-conexiones)
- [Paso 1 — Verificar firmware actual con Hercules](#paso-1--verificar-firmware-actual-con-hercules)
- [Paso 2 — Descargar el firmware oficial](#paso-2--descargar-el-firmware-oficial)
- [Paso 3 — Configurar Flash Download Tools](#paso-3--configurar-flash-download-tools)
- [Paso 4 — Flashear el firmware](#paso-4--flashear-el-firmware)
- [Paso 5 — Verificar la actualización](#paso-5--verificar-la-actualización)
- [Comandos AT Útiles](#-comandos-at-útiles)
- [Configuración para RemoteXY](#-configuración-para-remotexy)
- [Solución de Problemas](#-solución-de-problemas)

---

## ❓ ¿Por qué actualizar el firmware?

Los módulos ESP-01S que se consiguen en el mercado frecuentemente traen firmware desactualizado o de versión desconocida. Actualizar el firmware ofrece:

- ✅ Soporte para comandos AT más recientes (mínimo v0.40 para RemoteXY)
- ✅ Corrección de errores y mejoras de estabilidad
- ✅ Compatibilidad garantizada con librerías modernas
- ✅ Firmware oficial de Espressif (el fabricante del chip)
- ✅ Posibilidad de recuperar un módulo que no responde correctamente

> ⚠️ Actualizar el firmware siempre conlleva un pequeño riesgo. Solo hazlo si es necesario y si tienes claro el procedimiento.

---

## 🔗 Recursos y Herramientas

| Recurso | Descripción | Enlace |
|---|---|---|
| **RemoteXY WiFi ESP8266** | Documentación oficial para usar ESP8266 con RemoteXY | [remotexy.com/en/help/esp8266](https://remotexy.com/en/help/esp8266/) |
| **Guía de actualización RemoteXY** | Tutorial de conexión y flasheo de RemoteXY | [remotexy.com/en/help/esp8266-firmware-update](https://remotexy.com/en/help/esp8266-firmware-update/) |
| **Flash Download Tools** | Herramienta oficial de Espressif para flashear ESP8266/ESP32 | [espressif.com/en/products](https://www.espressif.com/en/products/socs/esp8266) |
| **Firmware ESP8266 (GitHub)** | Repositorio oficial del firmware NONOS SDK de Espressif | [github.com/espressif/ESP8266_NONOS_SDK](https://github.com/espressif/ESP8266_NONOS_SDK) |
| **Hercules Utility** | Terminal serie para probar comandos AT | [hw-group.com/software/hercules](https://www.hw-group.com/software/hercules) |
| **AT Instruction Set** | Manual oficial de comandos AT del ESP8266 | [Espressif AT Instruction Set (PDF)](https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf) |

---

## 🔧 Hardware Necesario

| Componente | Descripción |
|---|---|
| ESP-01S | Módulo WiFi ESP8266 a actualizar |
| Adaptador USB-UART | Conversor USB a TTL **a 3.3V** (CH340, CP2102, FT232, etc.) |
| Fuente 3.3V | Mínimo **200mA** de corriente (el USB del adaptador puede no ser suficiente) |
| Cables Dupont | Para realizar las conexiones entre pines |
| PC con Windows | Flash Download Tools solo está disponible para Windows |

> ⚠️ **Crítico:** El ESP8266 opera a **3.3V**. Usar 5V puede dañarlo permanentemente. Asegurarse de que el adaptador USB-UART tenga salida a 3.3V, **no a 5V**.

---

## 🔌 Diagrama de Conexiones


### Modo Flash / Programación (actualizar firmware)

```
ESP-01S          Adaptador USB-UART
──────────       ──────────────────
VCC   ────────►  3.3V
GND   ────────►  GND
TX    ────────►  TX
RX    ────────►  RX
GPIO0 ────────►  GND  ← PUENTE para entrar en modo flash
RST    (libre o reset manual)
```

> 📌 **Regla clave:**
> - `GPIO0` en **HIGH** (libre/flotante) → Modo normal, responde AT commands
> - `GPIO0` en **LOW** (unido a GND) → Modo flash, listo para recibir firmware

---

## Paso 1 — Verificar firmware actual con Hercules

Antes de flashear, comprobar qué versión de firmware tiene el módulo.

### Conexión en modo normal
1. Conectar el ESP-01S al adaptador USB-UART **sin** poner GPIO0 a GND
2. Conectar el adaptador USB al PC
3. Abrir **Hercules** (Serial tab)

### Configuración del puerto serie en Hercules

| Parámetro | Valor |
|---|---|
| Puerto | El COM asignado al adaptador (verificar en Administrador de dispositivos) |
| Baud rate | `115200` |
| Data bits | `8` |
| Parity | `None` |
| Stop bits | `1` |
| Handshake | `None` |

### Comandos de verificación

```
AT
```
Respuesta esperada: `OK`

```
AT+GMR
```
Respuesta esperada (ejemplo):
```
AT version:1.7.4.0(May 11 2020 19:13:04)
SDK version:3.0.4(9532ceb)
compile time:...
Bin version(Wroom 02):1.7.4
OK
```

> ✅ Para uso con RemoteXY se requiere **AT version mínima v0.40**. Se recomienda usar la versión **1.7.4** o superior.

---

## Paso 2 — Descargar el firmware oficial

### Opción A — Desde GitHub oficial de Espressif

1. Ir al repositorio: [github.com/espressif/ESP8266_NONOS_SDK](https://github.com/espressif/ESP8266_NONOS_SDK)
2. Descargar la rama `master` o la versión más reciente disponible
3. Descomprimir en una ruta **sin espacios ni caracteres especiales**
   - ✅ `C:\ESP8266\firmware\`
   - ❌ `C:\Mis Documentos\descargas\`

### Archivos bin necesarios (directorio `/bin`)

Para ESP-01S con **Flash de 1MB** (512KB+512KB):

| Archivo | Dirección |
|---|---|
| `boot_v1.2.bin` | `0x00000` |
| `at/512+512/user1.1024.new.2.bin` | `0x01000` |
| `blank.bin` | `0x7E000` |
| `esp_init_data_default_v08.bin` | `0x7C000` |
| `blank.bin` | `0xFE000` |

> 📝 Los archivos y direcciones exactas pueden variar según la versión del SDK descargada. Consultar el archivo `README.md` incluido en el SDK o la guía de RemoteXY para confirmarlo.

---

## Paso 3 — Configurar Flash Download Tools

1. Descargar desde: [espressif.com — Flash Download Tools](https://www.espressif.com/en/products/socs/esp8266)
2. Ejecutar el `.exe` (no requiere instalación)
3. En la ventana inicial seleccionar:
   - **ChipType:** `ESP8266`
   - **WorkMode:** `Developer`

### Configuración en la interfaz

```
┌────────────────────────────────────────────────┐
│  ESP8266 DOWNLOAD TOOL                         │
│                                                 │
│  [✓] boot_v1.2.bin              @ 0x00000      │
│  [✓] user1.1024.new.2.bin       @ 0x01000      │
│  [✓] esp_init_data_default.bin  @ 0x7C000      │
│  [✓] blank.bin                  @ 0x7E000      │
│  [✓] blank.bin                  @ 0xFE000      │
│                                                 │
│  SPI SPEED:  40 MHz                             │
│  SPI MODE:   DIO                                │
│  FLASH SIZE: 8Mbit  (1MB)                       │
│                                                 │
│  COM: COM3 (el que corresponda)                 │
│  BAUD: 115200                                   │
└────────────────────────────────────────────────┘
```

> ⚠️ Asegurarse de marcar las **checkboxes** a la izquierda de cada archivo. Sin el tilde, el archivo no se cargará aunque esté seleccionado.

---

## Paso 4 — Flashear el firmware

### Secuencia completa

1. **Conectar GPIO0 a GND** para entrar en modo flash
2. **Conectar** el adaptador USB al PC
3. **Resetear** el ESP-01S (conectar brevemente RST a GND o desconectar y reconectar alimentación)
4. En Flash Download Tools presionar el botón **`START`**
5. Observar el progreso en la barra inferior — debe mostrar avance
6. Esperar a que aparezca el mensaje **`FINISH`** en color verde

### Indicadores de progreso normales

```
Connecting...
Erasing flash...
Writing at 0x00000... (10%)
Writing at 0x01000... (40%)
...
Leaving...
FINISH
```

### Al finalizar

1. **Desconectar GPIO0 de GND** (quitar el puente)
2. Resetear el ESP-01S nuevamente
3. El módulo arranca en modo normal y ya está listo

---

## Paso 5 — Verificar la actualización

1. Abrir **Hercules** con la misma configuración del Paso 1
2. Conectar en modo normal (GPIO0 libre)
3. Enviar:

```
AT
```
Respuesta: `OK`

```
AT+GMR
```
Verificar que la versión mostrada coincide con el firmware que se cargó.

```
AT+RST
```
Reinicia el módulo — debe mostrar el mensaje de arranque y terminar en `ready`.

---

## 📡 Comandos AT Útiles

| Comando | Función |
|---|---|
| `AT` | Test básico de comunicación → responde `OK` |
| `AT+GMR` | Muestra versión del firmware y SDK |
| `AT+RST` | Reinicia el módulo |
| `AT+CWMODE=1` | Modo estación (cliente WiFi) |
| `AT+CWMODE=2` | Modo Access Point |
| `AT+CWMODE=3` | Modo mixto (AP + cliente) |
| `AT+CWLAP` | Lista redes WiFi disponibles |
| `AT+CWJAP="SSID","password"` | Conecta a una red WiFi |
| `AT+CIFSR` | Muestra la IP asignada |
| `AT+UART_DEF=9600,8,1,0,0` | Cambia la velocidad del puerto serie a 9600 (permanente) |
| `AT+UART_DEF=115200,8,1,0,0` | Restaura la velocidad a 115200 (permanente) |

> 📖 Referencia completa: [Espressif AT Instruction Set](https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf)

---

## 🖥️ Configuración para RemoteXY

RemoteXY requiere que el ESP8266 tenga **firmware AT versión mínima 0.40**.

### Velocidad de comunicación

- Si se usa **HardwareSerial** (pines 0 y 1 en Arduino UNO): se puede usar `115200` baud
- Si se usa **SoftwareSerial**: la velocidad máxima estable es `19200` baud

Para cambiar la velocidad del ESP permanentemente:
```
AT+UART_DEF=19200,8,1,0,0
```

### Verificación de compatibilidad

```
AT+GMR
```
La línea `AT version:X.X.X.X` debe ser **v0.40 o superior**.

> 📚 Más información: [RemoteXY WiFi ESP8266](https://remotexy.com/en/help/esp8266/) | [Guía actualización RemoteXY](https://remotexy.com/en/help/esp8266-firmware-update/)

---

## 🛠️ Solución de Problemas

| Síntoma | Causa probable | Solución |
|---|---|---|
| Hercules no recibe respuesta a `AT` | Baudrate incorrecto | Probar con 9600, 19200, 57600 y 115200 |
| Hercules no recibe respuesta a `AT` | TX/RX invertidos | Intercambiar los cables TX↔RX |
| Flash Download Tools no conecta | GPIO0 no está en GND | Verificar el puente GPIO0→GND |
| Flash Download Tools no conecta | Reset no aplicado | Resetear el módulo justo después de presionar START |
| Error de sincronización del chip | Fuente de alimentación insuficiente | Usar fuente externa de 3.3V con al menos 200mA |
| El módulo arranca pero no responde AT | Firmware corrupto | Repetir el proceso de flasheo |
| `FINISH` pero el módulo no responde | GPIO0 aún conectado a GND | Quitar el puente y resetear |
| Velocidad cambia sola al reiniciar | Se usó `AT+UART_CUR` en lugar de `AT+UART_DEF` | Usar `AT+UART_DEF` para cambios permanentes |
| SoftwareSerial recibe datos corruptos | Velocidad demasiado alta | Cambiar a 9600 o 19200 con `AT+UART_DEF` |

---

## 📁 Archivos Relacionados

```
esp01s-firmware-update/
│
├── README.md                        ← Este archivo
├── firmware/
│   └── ESP8266_NONOS_SDK/           ← SDK descargado de GitHub
│       └── bin/
│           ├── boot_v1.2.bin
│           ├── at/512+512/
│           │   └── user1.1024.new.2.bin
│           ├── blank.bin
│           └── esp_init_data_default_v08.bin
└── tools/
    └── flash_download_tool_x.x.x/  ← Flash Download Tools de Espressif
```

---

## 📄 Referencias

- 📺 Video tutorial: [youtube.com/watch?v=31TQMxeyFL4](https://www.youtube.com/watch?v=31TQMxeyFL4)
- 🌐 RemoteXY ESP8266: [remotexy.com/en/help/esp8266](https://remotexy.com/en/help/esp8266/)
- 🌐 Actualización firmware RemoteXY: [remotexy.com/en/help/esp8266-firmware-update](https://remotexy.com/en/help/esp8266-firmware-update/)
- 🔧 Flash Download Tools: [espressif.com/en/products](https://www.espressif.com/en/products/socs/esp8266)
- 💾 Firmware ESP8266: [github.com/espressif/ESP8266_NONOS_SDK](https://github.com/espressif/ESP8266_NONOS_SDK)
- 🖥️ Hercules Utility: [hw-group.com/software/hercules](https://www.hw-group.com/software/hercules)
- 📖 AT Instruction Set: [Espressif AT Commands PDF](https://www.espressif.com/sites/default/files/documentation/4a-esp8266_at_instruction_set_en.pdf)

---

*ESP-01S Firmware Update Guide | ESP8266 NONOS SDK | Flash Download Tools v2.4+*
