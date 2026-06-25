/*
  ESP32-S3 TOUCH LCD 3.5B - NTP Clock con pantalla

  Proyecto: ESP32_S3_TOUCH_LCD_35B_07_NTP_CLOCK

  Verifica:
  - Conexion a una red Wi-Fi conocida
  - Resolucion DNS desde la placa
  - Sincronizacion de fecha y hora mediante NTP
  - Hora local Argentina UTC-3
  - Salida por Serial Monitor
  - Salida por pantalla incorporada
  - Reconexion si se pierde la conexion Wi-Fi

  IMPORTANTE:
  Crear un archivo config_wifi.h en la misma carpeta del sketch.
*/

#include <WiFi.h>
#include <time.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "TCA9554.h"
#include "config_wifi.h"

/************************************************************
 *  SECCION: CONFIGURACION GENERAL
 ************************************************************/
const unsigned long INTERVALO_REINTENTO_NTP_MS = 60000;
unsigned long ultimoIntentoNTP = 0;

const unsigned long INTERVALO_REPORTE_MS = 5000;
const int MAX_INTENTOS_WIFI = 30;

unsigned long ultimoReporte = 0;
bool pantallaLista = false;
bool horaSincronizada = false;

// Zona horaria Argentina: UTC-3
const long GMT_OFFSET_SEC = -3 * 60 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

// Servidores NTP
const char* NTP_SERVER_1 = "ar.pool.ntp.org";
const char* NTP_SERVER_2 = "time.google.com";
const char* NTP_SERVER_3 = "time.cloudflare.com";

// DNS externos para evitar problemas de resolucion en algunas redes.
IPAddress DNS_PRIMARIO(8, 8, 8, 8);
IPAddress DNS_SECUNDARIO(1, 1, 1, 1);

/************************************************************
 *  SECCION: CONFIGURACION DE PANTALLA WAVESHARE
 ************************************************************/

#define LCD_QSPI_CS 12
#define LCD_QSPI_CLK 5
#define LCD_QSPI_D0 1
#define LCD_QSPI_D1 2
#define LCD_QSPI_D2 3
#define LCD_QSPI_D3 4
#define GFX_BL 6

// Orientacion de la pantalla.
// 0: vertical normal. Luego podemos probar 1, 2 o 3 si queremos rotarla.
#define ROTATION 0

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 480;

// Colores RGB565 basicos.
const uint16_t COLOR_BG = 0x0000;
const uint16_t COLOR_PANEL = 0x2104;
const uint16_t COLOR_WHITE = 0xFFFF;
const uint16_t COLOR_GRAY = 0x8410;
const uint16_t COLOR_CYAN = 0x07FF;
const uint16_t COLOR_GREEN = 0x07E0;
const uint16_t COLOR_YELLOW = 0xFFE0;
const uint16_t COLOR_RED = 0xF800;
const uint16_t COLOR_BLUE = 0x001F;

// Expansor usado por la placa para reset/habilitacion de la pantalla.
TCA9554 TCA(0x20);

// Configuracion tomada del ejemplo oficial 08_gfx_helloworld de Waveshare.
Arduino_DataBus* bus = new Arduino_ESP32QSPI(
  LCD_QSPI_CS,
  LCD_QSPI_CLK,
  LCD_QSPI_D0,
  LCD_QSPI_D1,
  LCD_QSPI_D2,
  LCD_QSPI_D3);

Arduino_GFX* panel = new Arduino_AXS15231B(
  bus,
  -1,  // RST
  0,   // rotacion interna del driver
  false,
  SCREEN_WIDTH,
  SCREEN_HEIGHT);

Arduino_Canvas* gfx = new Arduino_Canvas(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  panel,
  0,
  0,
  ROTATION);

/************************************************************
 *  SECCION: FUNCIONES AUXILIARES GENERALES
 ************************************************************/

void imprimirSeparador() {
  Serial.println("------------------------------------------------------------");
}

String estadoWiFi(wl_status_t estado) {
  switch (estado) {
    case WL_CONNECTED:
      return "Conectado";
    case WL_NO_SSID_AVAIL:
      return "SSID no disponible";
    case WL_CONNECT_FAILED:
      return "Conexion fallida";
    case WL_CONNECTION_LOST:
      return "Conexion perdida";
    case WL_DISCONNECTED:
      return "Desconectado";
    case WL_IDLE_STATUS:
      return "En espera";
    default:
      return "Estado desconocido";
  }
}

String recortarTexto(const String& texto, int maxCaracteres) {
  if (texto.length() <= maxCaracteres) {
    return texto;
  }

  if (maxCaracteres <= 3) {
    return texto.substring(0, maxCaracteres);
  }

  return texto.substring(0, maxCaracteres - 3) + "...";
}

uint16_t colorEstadoWiFi() {
  wl_status_t estado = WiFi.status();

  if (estado == WL_CONNECTED) {
    return COLOR_GREEN;
  }

  if (estado == WL_DISCONNECTED || estado == WL_CONNECTION_LOST) {
    return COLOR_YELLOW;
  }

  return COLOR_RED;
}

uint16_t colorEstadoNTP() {
  return horaSincronizada ? COLOR_GREEN : COLOR_YELLOW;
}

/************************************************************
 *  SECCION: FUNCIONES DE FECHA Y HORA
 ************************************************************/

bool obtenerTiempoLocal(struct tm* infoTiempo, uint32_t timeoutMs = 100) {
  return getLocalTime(infoTiempo, timeoutMs);
}

String obtenerFecha() {
  struct tm infoTiempo;

  if (!obtenerTiempoLocal(&infoTiempo)) {
    return "--/--/----";
  }

  char buffer[16];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y", &infoTiempo);

  return String(buffer);
}

String obtenerHora() {
  struct tm infoTiempo;

  if (!obtenerTiempoLocal(&infoTiempo)) {
    return "--:--:--";
  }

  char buffer[16];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", &infoTiempo);

  return String(buffer);
}

String obtenerFechaHora() {
  struct tm infoTiempo;

  if (!obtenerTiempoLocal(&infoTiempo)) {
    return "Hora no sincronizada";
  }

  char buffer[32];
  strftime(buffer, sizeof(buffer), "%d/%m/%Y %H:%M:%S", &infoTiempo);

  return String(buffer);
}

String obtenerDiaSemana() {
  struct tm infoTiempo;

  if (!obtenerTiempoLocal(&infoTiempo)) {
    return "No disponible";
  }

  const char* dias[] = {
    "Domingo",
    "Lunes",
    "Martes",
    "Miercoles",
    "Jueves",
    "Viernes",
    "Sabado"
  };

  return String(dias[infoTiempo.tm_wday]);
}

/************************************************************
 *  SECCION: FUNCIONES DE PANTALLA
 ************************************************************/

bool inicializarPantalla() {
  Wire.begin(21, 22);

  TCA.begin();
  TCA.pinMode1(1, OUTPUT);

  // Secuencia de reset/habilitacion usada por el ejemplo oficial.
  TCA.write1(1, 1);
  delay(10);
  TCA.write1(1, 0);
  delay(10);
  TCA.write1(1, 1);
  delay(200);

  if (!gfx->begin()) {
    Serial.println("Error: gfx->begin() fallo.");
    return false;
  }

  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);

  gfx->fillScreen(COLOR_BG);
  gfx->flush();

  return true;
}

void limpiarPantalla() {
  if (!pantallaLista) return;

  gfx->fillScreen(COLOR_BG);
}

void dibujarEncabezado(const String& titulo, const String& subtitulo, uint16_t colorTitulo) {
  if (!pantallaLista) return;

  gfx->fillRect(0, 0, SCREEN_WIDTH, 58, COLOR_PANEL);
  gfx->drawFastHLine(0, 58, SCREEN_WIDTH, COLOR_CYAN);

  gfx->setCursor(12, 10);
  gfx->setTextSize(2);
  gfx->setTextColor(colorTitulo);
  gfx->println(titulo);

  gfx->setCursor(12, 35);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println(subtitulo);
}

void escribirCampo(int y, const String& etiqueta, const String& valor, uint16_t colorValor = COLOR_WHITE) {
  if (!pantallaLista) return;

  gfx->setCursor(14, y);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_CYAN);
  gfx->print(etiqueta);

  gfx->setTextColor(colorValor);
  gfx->println(valor);
}

void escribirCampoPequeno(int y, const String& etiqueta, const String& valor, uint16_t colorValor = COLOR_WHITE) {
  if (!pantallaLista) return;

  gfx->setCursor(14, y);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_CYAN);
  gfx->print(etiqueta);

  gfx->setTextColor(colorValor);
  gfx->println(valor);
}

void mostrarPantallaInicio() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "ESP32-S3 Touch LCD 3.5B", COLOR_WHITE);

  gfx->setCursor(20, 100);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->println("Iniciando reloj...");

  gfx->setCursor(20, 140);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Wi-Fi + DNS + NTP + pantalla.");

  gfx->flush();
}

void mostrarPantallaConectando(int intento) {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "Conectando a Wi-Fi", COLOR_YELLOW);

  escribirCampo(90, "SSID: ", recortarTexto(WIFI_SSID, 18), COLOR_WHITE);
  escribirCampo(125, "Intento: ", String(intento) + " / " + String(MAX_INTENTOS_WIFI), COLOR_YELLOW);

  gfx->setCursor(14, 180);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_GRAY);
  gfx->print("Esperando");

  int puntos = intento % 10;
  for (int i = 0; i < puntos; i++) {
    gfx->print(".");
  }

  gfx->flush();
}

void mostrarPantallaErrorConexion() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "No se pudo conectar", COLOR_RED);

  escribirCampo(90, "SSID: ", recortarTexto(WIFI_SSID, 18), COLOR_WHITE);
  escribirCampo(125, "Estado: ", estadoWiFi(WiFi.status()), COLOR_RED);

  gfx->setCursor(14, 190);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Revisar SSID, clave, senal o red 2.4 GHz.");

  gfx->flush();
}

void mostrarPantallaDNS(const String& servidor, bool ok, const String& ipTexto) {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "Diagnostico DNS", ok ? COLOR_GREEN : COLOR_RED);

  escribirCampo(90, "Servidor:", "", COLOR_WHITE);

  gfx->setCursor(14, 122);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->println(recortarTexto(servidor, 22));

  escribirCampo(175, "Estado: ", ok ? "OK" : "ERROR", ok ? COLOR_GREEN : COLOR_RED);
  escribirCampo(210, "IP: ", ipTexto, ok ? COLOR_WHITE : COLOR_RED);

  gfx->flush();
}

void mostrarPantallaSincronizandoNTP() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "Sincronizando hora", COLOR_YELLOW);

  escribirCampo(90, "NTP 1: ", recortarTexto(NTP_SERVER_1, 17), COLOR_WHITE);
  escribirCampo(125, "NTP 2: ", recortarTexto(NTP_SERVER_2, 17), COLOR_WHITE);
  escribirCampo(160, "NTP 3: ", recortarTexto(NTP_SERVER_3, 17), COLOR_WHITE);

  gfx->setCursor(14, 225);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Esperando respuesta...");

  gfx->flush();
}

void mostrarPantallaErrorNTP() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "Hora no sincronizada", COLOR_RED);

  escribirCampo(90, "Wi-Fi: ", estadoWiFi(WiFi.status()), colorEstadoWiFi());
  escribirCampo(125, "DNS: ", WiFi.dnsIP().toString(), COLOR_WHITE);

  gfx->setCursor(14, 190);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Revisar acceso a internet, DNS o servidores NTP.");

  gfx->flush();
}

void mostrarRelojEnPantalla() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("NTP Clock", "Hora local Argentina UTC-3", colorEstadoNTP());

  // Hora principal.
  gfx->setCursor(24, 88);
  gfx->setTextSize(4);
  gfx->setTextColor(horaSincronizada ? COLOR_GREEN : COLOR_YELLOW);
  gfx->println(obtenerHora());

  // Fecha y dia.
  gfx->setCursor(24, 150);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->println(obtenerDiaSemana());

  gfx->setCursor(24, 180);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->println(obtenerFecha());

  // Datos de estado.
  int y = 235;
  const int salto = 26;

  escribirCampoPequeno(y, "NTP: ", horaSincronizada ? "sincronizado" : "pendiente", colorEstadoNTP());
  y += salto;

  escribirCampoPequeno(y, "Wi-Fi: ", estadoWiFi(WiFi.status()), colorEstadoWiFi());
  y += salto;

  escribirCampoPequeno(y, "SSID: ", recortarTexto(WiFi.SSID(), 28), COLOR_WHITE);
  y += salto;

  escribirCampoPequeno(y, "IP: ", WiFi.localIP().toString(), COLOR_WHITE);
  y += salto;

  escribirCampoPequeno(y, "DNS: ", WiFi.dnsIP().toString(), COLOR_WHITE);
  y += salto;

  escribirCampoPequeno(y, "RSSI: ", String(WiFi.RSSI()) + " dBm", COLOR_WHITE);
  y += salto;

  escribirCampoPequeno(y, "Uptime: ", String(millis() / 1000) + " s", COLOR_WHITE);

  gfx->setCursor(14, 450);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Actualiza cada 5 segundos.");

  gfx->flush();
}

/************************************************************
 *  SECCION: FUNCIONES DE SERIAL MONITOR
 ************************************************************/

void imprimirDatosRed() {
  Serial.printf("%-18s %s\n", "Estado Wi-Fi:", estadoWiFi(WiFi.status()).c_str());
  Serial.printf("%-18s %s\n", "SSID:", WiFi.SSID().c_str());
  Serial.printf("%-18s %s\n", "IP local:", WiFi.localIP().toString().c_str());
  Serial.printf("%-18s %s\n", "Gateway:", WiFi.gatewayIP().toString().c_str());
  Serial.printf("%-18s %s\n", "DNS:", WiFi.dnsIP().toString().c_str());
  Serial.printf("%-18s %s\n", "MAC:", WiFi.macAddress().c_str());
  Serial.printf("%-18s %d\n", "Canal:", WiFi.channel());
  Serial.printf("%-18s %d dBm\n", "Senal RSSI:", WiFi.RSSI());
}

void imprimirDatosHora() {
  Serial.printf("%-18s %s\n", "Fecha y hora:", obtenerFechaHora().c_str());
  Serial.printf("%-18s %s\n", "Dia semana:", obtenerDiaSemana().c_str());
  Serial.printf("%-18s %s\n", "Zona horaria:", "Argentina UTC-3");
  Serial.printf("%-18s %s\n", "Estado NTP:", horaSincronizada ? "Sincronizado" : "Pendiente");
  Serial.printf("%-18s %lu s\n", "Uptime:", millis() / 1000);
}

void imprimirReporte() {
  imprimirSeparador();
  Serial.println("Reloj NTP - Estado general");
  imprimirSeparador();

  imprimirDatosHora();
  imprimirDatosRed();

  imprimirSeparador();
  Serial.println();
}

/************************************************************
 *  SECCION: WIFI
 ************************************************************/

bool esperarConexionWiFi(int maxIntentos, bool actualizarPantalla) {
  int intentos = 0;

  if (actualizarPantalla) {
    mostrarPantallaConectando(intentos);
  }

  while (WiFi.status() != WL_CONNECTED && intentos < maxIntentos) {
    delay(500);
    Serial.print(".");
    intentos++;

    if (actualizarPantalla) {
      mostrarPantallaConectando(intentos);
    }
  }

  Serial.println();
  return WiFi.status() == WL_CONNECTED;
}

bool aplicarDNSExternosConIPActual() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se pueden aplicar DNS externos sin conexion Wi-Fi.");
    return false;
  }

  IPAddress ipActual = WiFi.localIP();
  IPAddress gatewayActual = WiFi.gatewayIP();
  IPAddress subnetActual = WiFi.subnetMask();

  if (ipActual == IPAddress(0, 0, 0, 0) || gatewayActual == IPAddress(0, 0, 0, 0)) {
    Serial.println("No se pudo obtener IP/Gateway validos para configurar DNS externos.");
    return false;
  }

  Serial.println("Aplicando DNS externos con la IP obtenida por DHCP...");
  Serial.print("IP conservada: ");
  Serial.println(ipActual);
  Serial.print("Gateway conservado: ");
  Serial.println(gatewayActual);
  Serial.print("Subnet conservada: ");
  Serial.println(subnetActual);
  Serial.print("DNS primario: ");
  Serial.println(DNS_PRIMARIO);
  Serial.print("DNS secundario: ");
  Serial.println(DNS_SECUNDARIO);

  WiFi.disconnect(false);
  delay(700);

  if (!WiFi.config(ipActual, gatewayActual, subnetActual, DNS_PRIMARIO, DNS_SECUNDARIO)) {
    Serial.println("No se pudo configurar IP actual con DNS externos.");
    return false;
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  bool reconectado = esperarConexionWiFi(MAX_INTENTOS_WIFI, false);

  if (reconectado) {
    Serial.println("Reconectado con DNS externos.");
    Serial.print("DNS 1 final: ");
    Serial.println(WiFi.dnsIP(0));
    Serial.print("DNS 2 final: ");
    Serial.println(WiFi.dnsIP(1));
  } else {
    Serial.println("No se pudo reconectar despues de aplicar DNS externos.");
  }

  return reconectado;
}

void conectarWiFi() {
  Serial.println();
  Serial.println("Conectando a Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);

  // Primer intento: mismo esquema del proyecto ESP32 DevKit V1.
  // Se intenta mantener DHCP y forzar DNS externos.
  if (!WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, DNS_PRIMARIO, DNS_SECUNDARIO)) {
    Serial.println("No se pudo configurar DNS personalizados antes de conectar.");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  bool conectado = esperarConexionWiFi(MAX_INTENTOS_WIFI, true);

  if (conectado) {
    // En algunas combinaciones ESP32-S3/core/router, el DHCP puede
    // pisar los DNS configurados con INADDR_NONE y dejar el DNS del router.
    // Si ocurre, conservamos la IP obtenida por DHCP y reconectamos
    // usando esa misma IP, pero con DNS externos explícitos.
    if (WiFi.dnsIP(0) != DNS_PRIMARIO) {
      Serial.println("El DNS entregado por la red no coincide con el DNS externo esperado.");
      Serial.println("Se intentara conservar la IP DHCP y aplicar DNS externos explicitamente.");
      conectado = aplicarDNSExternosConIPActual();
    }
  }

  if (WiFi.status() == WL_CONNECTED && conectado) {
    Serial.println("Conexion Wi-Fi establecida correctamente.");
    Serial.print("IP local: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS 1: ");
    Serial.println(WiFi.dnsIP(0));
    Serial.print("DNS 2: ");
    Serial.println(WiFi.dnsIP(1));
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("No se pudo conectar a la red Wi-Fi.");
    Serial.print("Estado final: ");
    Serial.println(estadoWiFi(WiFi.status()));
    mostrarPantallaErrorConexion();
  }

  Serial.println();
}

/************************************************************
 *  SECCION: DIAGNOSTICO DNS
 ************************************************************/

bool probarDNS(const char* servidor) {
  IPAddress ip;

  Serial.print("Resolviendo ");
  Serial.print(servidor);
  Serial.print(" ... ");

  bool ok = WiFi.hostByName(servidor, ip) && ip != IPAddress(0, 0, 0, 0);

  if (ok) {
    Serial.print("OK -> ");
    Serial.println(ip);
  } else {
    Serial.print("ERROR DNS -> ");
    Serial.println(ip);
  }

  mostrarPantallaDNS(String(servidor), ok, ip.toString());
  delay(900);

  return ok;
}

void diagnosticarDNS() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se puede probar DNS sin conexion Wi-Fi.");
    return;
  }

  Serial.println("Diagnostico DNS:");
  probarDNS(NTP_SERVER_1);
  probarDNS(NTP_SERVER_2);
  probarDNS(NTP_SERVER_3);
  probarDNS("time.nist.gov");
  Serial.println();
}

/************************************************************
 *  SECCION: NTP
 ************************************************************/

void sincronizarHoraNTP() {
  horaSincronizada = false;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se puede sincronizar NTP sin conexion Wi-Fi.");
    return;
  }

  Serial.println("Sincronizando hora mediante NTP...");
  Serial.print("Servidor 1: ");
  Serial.println(NTP_SERVER_1);
  Serial.print("Servidor 2: ");
  Serial.println(NTP_SERVER_2);
  Serial.print("Servidor 3: ");
  Serial.println(NTP_SERVER_3);

  mostrarPantallaSincronizandoNTP();

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    NTP_SERVER_1,
    NTP_SERVER_2,
    NTP_SERVER_3);

  struct tm infoTiempo;

  if (getLocalTime(&infoTiempo, 30000)) {
    horaSincronizada = true;
    Serial.println("Hora sincronizada correctamente.");
    Serial.print("Fecha y hora actual: ");
    Serial.println(obtenerFechaHora());
  } else {
    horaSincronizada = false;
    Serial.println("No se pudo sincronizar la hora despues de 30 segundos.");
    mostrarPantallaErrorNTP();
  }

  Serial.println();
}


/************************************************************
 *  SECCION: DIAGNOSTICO DE MEMORIA
 ************************************************************/

void imprimirMemoria(const char* etapa) {
  Serial.println();
  Serial.print("Memoria - ");
  Serial.println(etapa);

  Serial.print("Heap libre: ");
  Serial.println(ESP.getFreeHeap());

  Serial.print("Heap minimo libre: ");
  Serial.println(ESP.getMinFreeHeap());

  Serial.print("PSRAM libre: ");
  Serial.println(ESP.getFreePsram());

  Serial.println();
}

/************************************************************
 *  SECCION: SETUP
 ************************************************************/

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================================");
  Serial.println("ESP32-S3 TOUCH LCD 3.5B - NTP CLOCK");
  Serial.println("==============================================");

  imprimirMemoria("inicio");

  conectarWiFi();
  imprimirMemoria("despues de WiFi");

  if (WiFi.status() == WL_CONNECTED) {
    diagnosticarDNS();
    sincronizarHoraNTP();
    ultimoIntentoNTP = millis();
  }
  imprimirMemoria("despues de NTP");

  pantallaLista = inicializarPantalla();

  if (pantallaLista) {
    Serial.println("Pantalla inicializada correctamente.");
    mostrarPantallaInicio();
    delay(1200);
  } else {
    Serial.println("La pantalla no pudo inicializarse. Continua solo por Serial.");
  }
  imprimirMemoria("despues de pantalla");

  imprimirReporte();
  mostrarRelojEnPantalla();
}

/************************************************************
 *  SECCION: LOOP PRINCIPAL
 ************************************************************/

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Intentando reconectar...");
    conectarWiFi();

    if (WiFi.status() == WL_CONNECTED) {
      diagnosticarDNS();
      sincronizarHoraNTP();
      ultimoIntentoNTP = millis();
    }
  }

  unsigned long ahora = millis();

  // Si hay Wi-Fi pero la hora todavía no se sincronizó,
  // reintentamos NTP cada cierto intervalo.
  if (WiFi.status() == WL_CONNECTED && !horaSincronizada) {
    if (ahora - ultimoIntentoNTP >= INTERVALO_REINTENTO_NTP_MS) {
      Serial.println("Hora pendiente. Reintentando sincronizacion NTP...");
      diagnosticarDNS();
      sincronizarHoraNTP();
      ultimoIntentoNTP = ahora;
    }
  }

  if (ahora - ultimoReporte >= INTERVALO_REPORTE_MS) {
    ultimoReporte = ahora;
    imprimirReporte();
    mostrarRelojEnPantalla();
  }
}