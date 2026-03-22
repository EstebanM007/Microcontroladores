# 🌀 Control de Ventilador ESP-01S

> Sistema de control remoto por red local para ventilador, usando ESP-01S + Módulo Relé. Interfaz web accesible desde cualquier dispositivo en la red LAN, sin dependencias de internet.

---

## 📋 Tabla de Contenidos

- [Características](#-características)
- [Hardware Requerido](#-hardware-requerido)
- [Diagrama de Conexiones](#-diagrama-de-conexiones)
- [Configuración de Red](#-configuración-de-red)
- [Instalación y Carga del Código](#-instalación-y-carga-del-código)
- [Uso de la Interfaz Web](#-uso-de-la-interfaz-web)
- [Rutas HTTP Disponibles](#-rutas-http-disponibles)
- [Temporizador](#-temporizador)
- [Optimizaciones de Consumo](#-optimizaciones-de-consumo)
- [Notas Técnicas](#-notas-técnicas)
- [Solución de Problemas](#-solución-de-problemas)

---

## ✅ Características

- **Control ON/OFF** del ventilador desde cualquier dispositivo en la red local
- **Temporizador** con presets de 15, 30, 45 min, 1h, 2h y 3h, más valor personalizado hasta 12 horas
- **IP estática** para acceso siempre en la misma dirección
- **mDNS** para acceder vía `http://ventilador.local` (sin recordar la IP)
- **Interfaz web responsiva** compatible con celulares y tablets
- **Sin auto-refresh** — actualización manual para reducir calentamiento del chip
- **Reconexión WiFi automática** ante cortes de señal
- **Versión 2.0 optimizada**: menor consumo, menos calor, código completamente comentado

---

## 🔧 Hardware Requerido

| Componente | Especificación |
|---|---|
| Microcontrolador | ESP-01S (ESP8266) |
| Módulo relé | 1 canal, 5V, activación por nivel LOW o HIGH |
| Fuente de alimentación | 3.3V para el ESP-01S / 5V para el relé (según módulo) |
| Cables de conexión | Dupont macho-macho |
| Ventilador / carga | Compatible con el relé utilizado |

---

## 🔌 Diagrama de Conexiones

```
ESP-01S          Módulo Relé
─────────        ───────────
GPIO0   ──────►  IN
VCC(3.3V)──────► VCC  (verificar si tu módulo requiere 5V)
GND     ──────►  GND
```

```
Módulo Relé          Ventilador / Carga
───────────          ──────────────────
COM      ──────────── Cable de línea (fase)
NC / NO  ──────────── Entrada del ventilador
```

> ⚠️ **Importante:** GPIO0 tiene lógica especial durante el arranque del ESP8266. Si el relé se activa momentáneamente al encender el dispositivo, usar la terminal **NC (Normalmente Cerrado)** o añadir un pull-up externo de 10kΩ a 3.3V.

---

## 📡 Configuración de Red

| Parámetro | Valor |
|---|---|
| SSID | `MartinezPalacios2G` |
| IP Estática | `192.168.101.100` |
| Gateway | `192.168.101.1` |
| Máscara de subred | `255.255.255.0` |
| DNS | `192.168.101.1` |
| Dominio mDNS | `ventilador.local` |
| Puerto HTTP | `80` |

Acceso desde el navegador:
```
http://192.168.101.100
http://ventilador.local   ← (requiere soporte mDNS en el cliente)
```

> 📝 Para cambiar la red WiFi, editar las variables `ssid` y `password` en el código fuente antes de cargar.

---

## 💻 Instalación y Carga del Código

### Requisitos previos

- [Arduino IDE](https://www.arduino.cc/en/software) con soporte para ESP8266
- Paquete de placas ESP8266 instalado en el gestor de tarjetas:
  ```
  https://arduino.esp8266.com/stable/package_esp8266com_index.json
  ```
- Adaptador USB-UART (CH340, CP2102, FT232, etc.) para programar el ESP-01S

### Librerías necesarias

Todas incluidas en el paquete ESP8266 para Arduino:
- `ESP8266WiFi`
- `ESP8266WebServer`
- `ESP8266mDNS`

### Pasos para cargar el firmware

1. Conectar el ESP-01S al adaptador USB-UART en modo programación:
   - `GPIO0` → `GND` (para entrar en modo flash)
   - `TX` → `TX` del adaptador
   - `RX` → `RX` del adaptador
   - `VCC` → `3.3V`
   - `GND` → `GND`

2. Seleccionar en Arduino IDE:
   - **Placa:** `Generic ESP8266 Module`
   - **Flash Size:** `1MB (FS: 64KB OTA: ~470KB)`
   - **Upload Speed:** `115200`

3. Cargar el sketch y esperar a que finalice.

4. Desconectar `GPIO0` de `GND` y reiniciar el ESP.

---

## 🖥️ Uso de la Interfaz Web

Al ingresar a `http://192.168.101.100` se muestra el panel de control:

```
┌─────────────────────────────────┐
│     🌀 Control de Ventilador    │
│      ESP-01S v2.0 Optimizado    │
│                                 │
│   ✅ VENTILADOR ENCENDIDO       │  ← Estado actual
│                                 │
│  [ ⚡ Encender ] [ ⏻ Apagar ]   │  ← Control manual
│                                 │
│  [ 🔄 Actualizar Estado ]       │  ← Actualización manual
│                                 │
│  ⏰ Temporizador                │
│  [15min][30min][45min]          │
│  [1hora][2horas][3horas]        │
│  [___minutos___] [Iniciar]      │
│                                 │
│  📡 Red: MartinezPalacios2G     │
│  🌐 IP: 192.168.101.100         │
└─────────────────────────────────┘
```

---

## 🌐 Rutas HTTP Disponibles

| Ruta | Método | Descripción |
|---|---|---|
| `/` | GET | Página principal con panel de control |
| `/encender` | GET | Enciende el ventilador y redirige a `/` |
| `/apagar` | GET | Apaga el ventilador, cancela el temporizador y redirige a `/` |
| `/temporizador?minutos=XX` | GET | Enciende el ventilador durante XX minutos (1–720) |

Ejemplo de uso directo desde un browser o scripts:
```
http://192.168.101.100/encender
http://192.168.101.100/apagar
http://192.168.101.100/temporizador?minutos=60
```

---

## ⏰ Temporizador

El temporizador enciende el ventilador y lo apaga automáticamente al expirar el tiempo.

- **Rango válido:** 1 a 720 minutos (12 horas)
- **Estado:** Se muestra en la página principal el tiempo restante (minutos y segundos)
- **Cancelación:** Al presionar "Apagar" se cancela el temporizador activo
- **Seguridad:** Si el ESP se reinicia, el temporizador se pierde y el ventilador queda apagado

---

## ⚡ Optimizaciones de Consumo

Esta versión (2.0) incluye mejoras significativas frente a versiones anteriores:

| Aspecto | v1.x | v2.0 |
|---|---|---|
| Auto-refresh JavaScript | Cada 10 segundos | ❌ Eliminado |
| Actualización de estado | Automática | Manual (botón) |
| Temperatura operación | 50–60 °C | 35–45 °C |
| Consumo promedio | ~90–100 mA | ~75–80 mA |
| Consumo pico (sirviendo página) | ~150 mA | ~120 mA |
| Escrituras en flash WiFi | Frecuentes | Desactivadas (`persistent(false)`) |
| Strings en RAM | Sin optimizar | Macro `F()` → almacenadas en flash |
| Reserva de memoria HTML | Dinámica | `String.reserve(8000)` anticipado |

---

## 📝 Notas Técnicas

### GPIO0 y el relé
GPIO0 es el pin de programación del ESP8266. Se puede usar como salida digital normal **después** de que el firmware ha cargado, pero tiene una restricción: debe estar en **HIGH** durante el arranque para que el chip no entre en modo flash. Tenerlo conectado a un relé de lógica directa puede causar activación momentánea al encender.

**Soluciones:**
- Usar la terminal **NC** del relé (Normalmente Cerrado) si la lógica lo permite
- Agregar resistencia pull-up de 10kΩ entre GPIO0 y 3.3V
- Usar GPIO2 como alternativa (tiene LED integrado en el ESP-01S)

### Lógica del relé
La mayoría de módulos relé de bajo costo usan **lógica invertida**:
- `LOW (0V)` → Relé **activado**
- `HIGH (3.3V)` → Relé **desactivado**

El código utiliza lógica directa (`HIGH` = ON). Verificar el módulo utilizado y ajustar la función `controlarRele()` si es necesario.

### mDNS
El dominio `ventilador.local` funciona en la mayoría de sistemas modernos (Windows 10+, macOS, Linux con Avahi, Android con aplicaciones compatibles). En algunos dispositivos Android puede no estar disponible; usar la IP directa como alternativa.

---

## 🛠️ Solución de Problemas

| Síntoma | Causa probable | Solución |
|---|---|---|
| El ESP no conecta al WiFi | SSID o contraseña incorrectos | Verificar `ssid` y `password` en el código |
| No se accede por `ventilador.local` | mDNS no soportado en el cliente | Usar la IP directa `192.168.101.100` |
| El relé se activa al encender | GPIO0 en LOW durante boot | Usar terminal NC del relé o pull-up 10kΩ |
| La IP no es la esperada | IP ya ocupada en la red | Cambiar la IP estática en el código |
| El temporizador no apaga | ESP reiniciado durante el temporizador | El temporizador vive en RAM, se pierde al reiniciar |
| Página muy lenta | Señal WiFi débil | Acercar el ESP al router o usar antena externa |
| ESP muy caliente | Versión anterior con auto-refresh | Cargar versión 2.0 del firmware |

---

## 📁 Estructura del Proyecto

```
ventilador-esp01s/
│
└── ventilador_esp01s_v2.ino    # Código fuente principal (único archivo)
```

---

## 📄 Licencia

Proyecto de uso personal / doméstico. Libre para modificar y adaptar según necesidad.

---

*ESP-01S v2.0 | GPIO0 → Relé | IP: 192.168.101.100 | ventilador.local*
