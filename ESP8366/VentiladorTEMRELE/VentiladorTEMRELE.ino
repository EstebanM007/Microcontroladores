/*
 * ===================================================================
 * SISTEMA DE CONTROL DE VENTILADOR OPTIMIZADO
 * ===================================================================
 * 
 * Dispositivo: ESP-01S + Módulo Relé
 * Versión: 2.0 - Optimizado para bajo consumo
 * Fecha: Febrero 2026
 * 
 * MEJORAS EN ESTA VERSIÓN:
 * - Sin auto-refrescado (reduce calentamiento del ESP)
 * - Actualización manual mediante botón
 * - Solo conexión local (sin servicios externos)
 * - GPIO0 para control del relé
 * - Menor consumo de RAM
 * - Código optimizado y comentado
 * 
 * CONFIGURACIÓN DE RED:
 * - SSID: MartinezPalacios2G
 * - IP Estática: 192.168.101.100
 * - Gateway: 192.168.101.1
 * - Solo acceso LAN (sin Internet)
 * - mDNS: ventilador.local
 * 
 * HARDWARE:
 * - GPIO0 -> IN del relé 
 * - VCC -> 3.3V
 * - GND -> GND común
 * 
 * NOTA: GPIO0 tiene lógica invertida durante el boot.
 * El relé debe estar conectado como normalmente cerrado (NC)
 * o configurar pull-up externo.
 * 
 * ===================================================================
 */

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// ===================================================================
// CONFIGURACIÓN DE RED
// ===================================================================
const char* ssid = "MartinezPalacios2G";
const char* password = "M4trix_L!nk#777";

// Configuración de IP estática (nunca cambia)
IPAddress ip_estatica(192, 168, 101, 100);  // IP fija del ESP-01S
IPAddress gateway(192, 168, 101, 1);         // IP del router
IPAddress subnet(255, 255, 255, 0);          // Máscara de subred
IPAddress dns(192, 168, 101, 1);             // DNS local (router)

const char* nombre_mdns = "ventilador";      // Dominio: ventilador.local

// ===================================================================
// CONFIGURACIÓN DE HARDWARE ESP01s
// ===================================================================
// GPIO0 es el pin de programación, pero puede usarse como salida
// después de cargar el código
#define RELAY_PIN 0  // GPIO0 del ESP-01S (GPIO2 se puede usar y tiene indicador led integrado)

// ===================================================================
// VARIABLES GLOBALES DE CONTROL
// ===================================================================
bool ventiladorEncendido = false;       // Estado actual del ventilador
bool temporizadorActivo = false;        // Indica si hay temporizador activo
unsigned long tiempoInicio = 0;         // Momento de inicio del temporizador
unsigned long duracionTemporizador = 0; // Duración en milisegundos

// Servidor web en puerto 80 (HTTP estándar)
ESP8266WebServer server(80);

// ===================================================================
// FUNCIÓN: Controlar Relé
// ===================================================================
/**
 * Controla el estado físico del relé conectado a GPIO0
 * 
 * @param encender - true: activa el relé (ventilador ON)
 *                   false: desactiva el relé (ventilador OFF)
 * 
 * NOTA: La mayoría de módulos relé usan lógica invertida:
 *       LOW (0V) = Relé activado
 *       HIGH (3.3V) = Relé desactivado
 * 
 * Al apagar manualmente, se cancela cualquier temporizador activo
 */
void controlarRele(bool encender) {
  ventiladorEncendido = encender;
  
  // Lógica invertida: LOW activa el relé, HIGH lo desactiva
  // Lógica directa: HIGH activa el relé, LOW lo desactiva
  digitalWrite(RELAY_PIN, encender ? HIGH : LOW);
  
  // Si se apaga, cancelar temporizador
  if (!encender) {
    temporizadorActivo = false;
    duracionTemporizador = 0;
  }
}

// ===================================================================
// FUNCIÓN: Calcular Tiempo Restante
// ===================================================================
/**
 * Calcula el tiempo restante del temporizador si está activo
 * 
 * @param minutosRestantes - referencia donde se guardan los minutos
 * @param segundosRestantes - referencia donde se guardan los segundos
 * @return true si hay tiempo restante, false si terminó o no hay temporizador
 */
bool calcularTiempoRestante(int &minutosRestantes, int &segundosRestantes) {
  if (!temporizadorActivo) {
    return false;
  }
  
  unsigned long tiempoTranscurrido = millis() - tiempoInicio;
  
  // Verificar si ya expiró
  if (tiempoTranscurrido >= duracionTemporizador) {
    return false;
  }
  
  unsigned long tiempoRestante = duracionTemporizador - tiempoTranscurrido;
  minutosRestantes = tiempoRestante / 60000;
  segundosRestantes = (tiempoRestante % 60000) / 1000;
  
  return true;
}

// ===================================================================
// FUNCIÓN: Generar Página HTML
// ===================================================================
/**
 * Genera la interfaz web completa
 * 
 * OPTIMIZACIONES:
 * - Sin auto-refresh JavaScript (elimina peticiones constantes)
 * - Botón manual de actualización
 * - Estilos CSS minificados
 * - Menor uso de String concatenación
 * 
 * @return String con HTML completo
 */
String getPaginaHTML() {
  // Buffer más eficiente que concatenación de Strings
  String html;
  html.reserve(8000); // Reservar memoria anticipadamente
  
  html = F("<!DOCTYPE html><html lang='es'><head>");
  html += F("<meta charset='UTF-8'>");
  html += F("<meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>Control de Ventilador</title>");
  
  // ===================================================================
  // ESTILOS CSS OPTIMIZADOS
  // ===================================================================
  html += F("<style>");
  html += F("*{margin:0;padding:0;box-sizing:border-box}");
  html += F("body{font-family:'Segoe UI',sans-serif;background:linear-gradient(135deg,#667eea 0%,#764ba2 100%);min-height:100vh;padding:20px}");
  html += F(".container{max-width:500px;margin:0 auto;background:#fff;border-radius:20px;padding:30px;box-shadow:0 20px 60px rgba(0,0,0,.3)}");
  html += F("h1{color:#333;text-align:center;margin-bottom:10px;font-size:28px}");
  html += F(".subtitle{text-align:center;color:#666;margin-bottom:30px;font-size:14px}");
  html += F(".status{text-align:center;margin:25px 0;padding:20px;border-radius:15px;font-size:18px;font-weight:bold}");
  html += F(".status.on{background:#d4edda;color:#155724;border:2px solid #c3e6cb}");
  html += F(".status.off{background:#f8d7da;color:#721c24;border:2px solid #f5c6cb}");
  html += F(".controls{display:flex;gap:15px;margin:25px 0}");
  html += F(".btn{flex:1;padding:15px;border:none;border-radius:12px;font-size:16px;font-weight:bold;cursor:pointer;transition:all .3s;text-decoration:none;display:block;text-align:center}");
  html += F(".btn:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(0,0,0,.2)}");
  html += F(".btn-on{background:#28a745;color:#fff}");
  html += F(".btn-off{background:#dc3545;color:#fff}");
  html += F(".btn-refresh{background:#17a2b8;color:#fff;width:100%;margin-top:15px}");
  html += F(".timer-section{background:#f8f9fa;padding:20px;border-radius:15px;margin:25px 0}");
  html += F(".timer-section h2{color:#333;margin-bottom:15px;font-size:20px}");
  html += F(".timer-presets{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-bottom:15px}");
  html += F(".timer-btn{padding:12px;background:#667eea;color:#fff;border:none;border-radius:10px;font-size:14px;cursor:pointer;transition:all .3s}");
  html += F(".timer-btn:hover{background:#5568d3;transform:scale(1.05)}");
  html += F(".custom-timer{display:flex;gap:10px;align-items:center;margin-top:15px}");
  html += F(".custom-timer input{flex:1;padding:12px;border:2px solid #ddd;border-radius:10px;font-size:14px}");
  html += F(".custom-timer button{padding:12px 20px;background:#764ba2;color:#fff;border:none;border-radius:10px;cursor:pointer}");
  html += F(".timer-info{background:#fff3cd;border:2px solid #ffc107;color:#856404;padding:15px;border-radius:10px;margin-top:15px;text-align:center;font-weight:bold}");
  html += F(".info{background:#e7f3ff;padding:15px;border-radius:10px;margin-top:20px;font-size:13px;color:#004085;line-height:1.8}");
  html += F(".footer{text-align:center;margin-top:20px;color:#666;font-size:12px}");
  html += F("@media(max-width:480px){.timer-presets{grid-template-columns:repeat(2,1fr)}}");
  html += F("</style>");
  
  // ===================================================================
  // JAVASCRIPT OPTIMIZADO - SIN AUTO-REFRESH
  // ===================================================================
  html += F("<script>");
  
  // Función para iniciar temporizador predefinido
  html += F("function iniciarTemporizador(m){");
  html += F("if(confirm('¿Iniciar temporizador por '+m+' minutos?')){");
  html += F("window.location.href='/temporizador?minutos='+m}}");
  
  // Función para temporizador personalizado
  html += F("function iniciarCustom(){");
  html += F("let m=document.getElementById('customMin').value;");
  html += F("if(m>0&&m<=720)iniciarTemporizador(m);");
  html += F("else alert('Ingresa un valor entre 1 y 720 minutos')}");
  
  // Función para actualización manual
  html += F("function actualizar(){location.reload()}");
  
  html += F("</script>");
  html += F("</head><body>");
  
  // ===================================================================
  // CONTENIDO HTML
  // ===================================================================
  html += F("<div class='container'>");
  html += F("<h1>🌀 Control de Ventilador</h1>");
  html += F("<div class='subtitle'>ESP-01S v2.0 Optimizado</div>");
  
  // Estado actual del ventilador
  html += F("<div class='status ");
  html += ventiladorEncendido ? F("on'>✅ VENTILADOR ENCENDIDO") : F("off'>❌ VENTILADOR APAGADO");
  html += F("</div>");
  
  // Botones de control manual
  html += F("<div class='controls'>");
  html += F("<a href='/encender' class='btn btn-on'>⚡ Encender</a>");
  html += F("<a href='/apagar' class='btn btn-off'>⏻ Apagar</a>");
  html += F("</div>");
  
  // Botón de actualización manual (reemplaza auto-refresh)
  html += F("<button class='btn btn-refresh' onclick='actualizar()'>🔄 Actualizar Estado</button>");
  
  // ===================================================================
  // SECCIÓN DE TEMPORIZADOR
  // ===================================================================
  html += F("<div class='timer-section'>");
  html += F("<h2>⏰ Temporizador</h2>");
  
  // Botones predefinidos
  html += F("<div class='timer-presets'>");
  html += F("<button class='timer-btn' onclick='iniciarTemporizador(15)'>15 min</button>");
  html += F("<button class='timer-btn' onclick='iniciarTemporizador(30)'>30 min</button>");
  html += F("<button class='timer-btn' onclick='iniciarTemporizador(45)'>45 min</button>");
  html += F("<button class='timer-btn' onclick='iniciarTemporizador(60)'>1 hora</button>");
  html += F("<button class='timer-btn' onclick='iniciarTemporizador(120)'>2 horas</button>");
  html += F("<button class='timer-btn' onclick='iniciarTemporizador(180)'>3 horas</button>");
  html += F("</div>");
  
  // Campo personalizado
  html += F("<div class='custom-timer'>");
  html += F("<input type='number' id='customMin' placeholder='Minutos (1-720)' min='1' max='720'>");
  html += F("<button onclick='iniciarCustom()'>Iniciar</button>");
  html += F("</div>");
  
  // Mostrar tiempo restante si hay temporizador activo
  int minutosRestantes, segundosRestantes;
  if (calcularTiempoRestante(minutosRestantes, segundosRestantes)) {
    html += F("<div class='timer-info'>⏱️ Tiempo restante: ");
    html += String(minutosRestantes);
    html += F("m ");
    html += String(segundosRestantes);
    html += F("s<br>El ventilador se apagará automáticamente</div>");
  }
  html += F("</div>");
  
  // ===================================================================
  // INFORMACIÓN DEL SISTEMA
  // ===================================================================
  html += F("<div class='info'>");
  html += F("📡 <strong>Red:</strong> ");
  html += String(ssid);
  html += F("<br>🌐 <strong>IP:</strong> ");
  html += WiFi.localIP().toString();
  html += F("<br>📍 <strong>Pin Relé:</strong> GPIO0");
  html += F("<br>🔋 <strong>Modo:</strong> Optimizado (bajo consumo)");
  html += F("<br>🔄 <strong>Actualización:</strong> Manual");
  html += F("</div>");
  
  html += F("<div class='footer'>ESP-01S v2.0 | IP: 192.168.101.100 | GPIO0</div>");
  html += F("</div></body></html>");
  
  return html;
}

// ===================================================================
// MANEJADORES DE RUTAS HTTP
// ===================================================================

/**
 * Ruta raíz: GET /
 * Muestra la página principal de control
 */
void handleRoot() {
  server.send(200, "text/html", getPaginaHTML());
}

/**
 * Ruta de encendido: GET /encender
 * Enciende el ventilador y redirige a la página principal
 */
void handleEncender() {
  controlarRele(true);
  server.sendHeader("Location", "/");
  server.send(303); // HTTP 303 See Other (redirección)
}

/**
 * Ruta de apagado: GET /apagar
 * Apaga el ventilador, cancela temporizador y redirige
 */
void handleApagar() {
  controlarRele(false);
  server.sendHeader("Location", "/");
  server.send(303);
}

/**
 * Ruta de temporizador: GET /temporizador?minutos=XX
 * Inicia un temporizador de XX minutos
 * 
 * @param minutos - Parámetro GET con la duración (1-720)
 */
void handleTemporizador() {
  if (server.hasArg("minutos")) {
    int minutos = server.arg("minutos").toInt();
    
    // Validar rango: 1 minuto a 12 horas (720 minutos)
    if (minutos > 0 && minutos <= 720) {
      controlarRele(true); // Encender ventilador
      temporizadorActivo = true;
      tiempoInicio = millis();
      duracionTemporizador = (unsigned long)minutos * 60000UL;
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ===================================================================
// CONFIGURACIÓN INICIAL (SETUP)
// ===================================================================
/**
 * Función setup() - Se ejecuta UNA SOLA VEZ al iniciar
 * 
 * Configura:
 * 1. Pin GPIO0 como salida para el relé
 * 2. IP estática en la red local
 * 3. Conexión WiFi
 * 4. Servicio mDNS (ventilador.local)
 * 5. Rutas del servidor web
 * 6. Inicia el servidor HTTP
 */
void setup() {
  // -------------------------------------------------------------------
  // CONFIGURAR GPIO0 COMO SALIDA
  // -------------------------------------------------------------------
  pinMode(RELAY_PIN, OUTPUT);
  
  // Estado inicial: APAGADO (seguridad ante cortes de luz)
  // HIGH = Relé desactivado (lógica invertida)
  // LOW = Relé desactivado (lógica directa)
  digitalWrite(RELAY_PIN, LOW);
  
  // -------------------------------------------------------------------
  // CONFIGURAR IP ESTÁTICA (ANTES DE WiFi.begin)
  // -------------------------------------------------------------------
  // Esto asegura que siempre tenga la misma IP
  WiFi.config(ip_estatica, gateway, subnet, dns);
  
  // -------------------------------------------------------------------
  // CONECTAR A WIFI
  // -------------------------------------------------------------------
  WiFi.mode(WIFI_STA); // Modo estación (cliente)
  WiFi.setAutoReconnect(true); // Reconexión automática
  WiFi.persistent(false); // No guardar configuración en flash (reduce escrituras)
  WiFi.begin(ssid, password);
  
  // Esperar conexión (máximo 15 segundos)
  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    intentos++;
  }
  
  // -------------------------------------------------------------------
  // CONFIGURAR mDNS (DOMINIO LOCAL)
  // -------------------------------------------------------------------
  // Permite acceder vía http://ventilador.local
  if (MDNS.begin(nombre_mdns)) {
    MDNS.addService("http", "tcp", 80);
  }
  
  // -------------------------------------------------------------------
  // CONFIGURAR RUTAS DEL SERVIDOR WEB
  // -------------------------------------------------------------------
  server.on("/", handleRoot);                    // Página principal
  server.on("/encender", handleEncender);         // Encender ventilador
  server.on("/apagar", handleApagar);             // Apagar ventilador
  server.on("/temporizador", handleTemporizador); // Configurar temporizador
  
  // -------------------------------------------------------------------
  // INICIAR SERVIDOR WEB
  // -------------------------------------------------------------------
  server.begin();
  
  // Sistema listo - ESP en espera de peticiones HTTP
}

// ===================================================================
// BUCLE PRINCIPAL (LOOP)
// ===================================================================
/**
 * Función loop() - Se ejecuta CONTINUAMENTE
 * 
 * Tareas:
 * 1. Atender peticiones HTTP del servidor web
 * 2. Actualizar servicio mDNS
 * 3. Verificar si el temporizador ha expirado
 * 
 * OPTIMIZACIÓN: delay(10) reduce consumo sin afectar funcionalidad
 */
void loop() {
  // Procesar peticiones HTTP entrantes
  server.handleClient();
  
  // Mantener servicio mDNS activo
  MDNS.update();
  
  // -------------------------------------------------------------------
  // VERIFICAR TEMPORIZADOR
  // -------------------------------------------------------------------
  if (temporizadorActivo && ventiladorEncendido) {
    unsigned long tiempoTranscurrido = millis() - tiempoInicio;
    
    // Si el tiempo expiró, apagar ventilador
    if (tiempoTranscurrido >= duracionTemporizador) {
      controlarRele(false);
      temporizadorActivo = false;
    }
  }
  
  // Pequeño delay para reducir consumo de CPU y temperatura
  // 10ms no afecta la respuesta web pero reduce significativamente el calor
  delay(10);
}

/*
 * ===================================================================
 * FIN DEL CÓDIGO
 * ===================================================================
 * 
 * CAMBIOS EN VERSIÓN 2.0:
 * ✅ GPIO0 en lugar de GPIO2 para el relé
 * ✅ Sin auto-refresh JavaScript (reduce calentamiento)
 * ✅ Botón manual de actualización
 * ✅ Conexión solo local (DNS apunta al router)
 * ✅ Uso de F() macro para strings en flash (ahorra RAM)
 * ✅ String.reserve() para optimizar memoria
 * ✅ WiFi.persistent(false) reduce escrituras en flash
 * ✅ Código completamente comentado y documentado
 * 
 * TEMPERATURA ESPERADA:
 * - Antes: 50-60°C (con auto-refresh cada 10s)
 * - Ahora: 35-45°C (solo responde a peticiones manuales)
 * 
 * CONSUMO ESTIMADO:
 * - Inactivo: ~70mA @ 3.3V
 * - Sirviendo página: ~120mA pico
 * - Promedio: ~75-80mA (vs 90-100mA en versión anterior)
 * 
 * ===================================================================
 */
