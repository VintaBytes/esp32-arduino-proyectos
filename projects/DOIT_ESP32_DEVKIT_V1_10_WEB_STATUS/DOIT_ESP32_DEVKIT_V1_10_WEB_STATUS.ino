/*
  Proyecto: DOIT_ESP32_DEVKIT_V1_10_WEB_STATUS

  Nombre visible: 004 - Web Status

  Placa: ESP32 DEVKIT V1 / ESP-WROOM-32
  Entorno: Arduino IDE 2

  Objetivo:
  - Crear una red Wi-Fi propia en modo Access Point.
  - Permitir acceder siempre a la pagina web desde http://192.168.10.1/
  - Conectar tambien el ESP32 a una red Wi-Fi conocida, si esta disponible.
  - Verificar resolucion DNS desde la placa.
  - Sincronizar fecha y hora usando NTP.
  - Mostrar una pagina de estado desde el navegador.
  - Mostrar la pagina aunque la hora aun no haya sincronizado.
  - Informar estado AP, estado Wi-Fi, IPs, RSSI, uptime y hora local.

  IMPORTANTE:
  Crear un archivo config_wifi.h en la misma carpeta del sketch.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include "config_wifi.h"

// ------------------------------------------------------------
// CONFIGURACION GENERAL
// ------------------------------------------------------------

const unsigned long INTERVALO_REPORTE_MS = 10000;
const unsigned long INTERVALO_REINTENTO_NTP_MS = 60000;
const unsigned long TIMEOUT_NTP_TOTAL_MS = 30000;

unsigned long ultimoReporte = 0;
unsigned long ultimoIntentoNTP = 0;

bool horaSincronizada = false;
bool servidorIniciado = false;

// ------------------------------------------------------------
// CONFIGURACION ACCESS POINT
// ------------------------------------------------------------

const char* AP_SSID = "ESP32_WEB_STATUS";
const char* AP_PASSWORD = "esp32status";

// Usamos 192.168.10.1 para evitar conflictos con redes domesticas comunes
IPAddress AP_IP(192, 168, 10, 1);
IPAddress AP_GATEWAY(192, 168, 10, 1);
IPAddress AP_SUBNET(255, 255, 255, 0);

// ------------------------------------------------------------
// CONFIGURACION NTP
// ------------------------------------------------------------

// Zona horaria Argentina: UTC-3
const long GMT_OFFSET_SEC = -3 * 60 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

// Servidores NTP
const char* NTP_SERVER_1 = "ar.pool.ntp.org";
const char* NTP_SERVER_2 = "south-america.pool.ntp.org";
const char* NTP_SERVER_3 = "pool.ntp.org";

// DNS externos para la conexion STA
IPAddress DNS_PRIMARIO(8, 8, 8, 8);
IPAddress DNS_SECUNDARIO(1, 1, 1, 1);

// Servidor web en puerto 80
WebServer servidor(80);

// ------------------------------------------------------------
// FUNCIONES AUXILIARES
// ------------------------------------------------------------

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

bool obtenerTiempoLocal(struct tm* infoTiempo, uint32_t timeoutMs = 100) {
  return getLocalTime(infoTiempo, timeoutMs);
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

String obtenerUptime() {
  unsigned long totalSegundos = millis() / 1000;

  unsigned long dias = totalSegundos / 86400;
  totalSegundos %= 86400;

  unsigned long horas = totalSegundos / 3600;
  totalSegundos %= 3600;

  unsigned long minutos = totalSegundos / 60;
  unsigned long segundos = totalSegundos % 60;

  char buffer[40];
  snprintf(buffer, sizeof(buffer), "%lu d %02lu:%02lu:%02lu", dias, horas, minutos, segundos);

  return String(buffer);
}

String calidadSenal(int rssi) {
  if (WiFi.status() != WL_CONNECTED) {
    return "No disponible";
  }

  if (rssi >= -50) {
    return "Excelente";
  } else if (rssi >= -60) {
    return "Muy buena";
  } else if (rssi >= -70) {
    return "Buena";
  } else if (rssi >= -80) {
    return "Regular";
  } else {
    return "Debil";
  }
}

String htmlEscape(String texto) {
  texto.replace("&", "&amp;");
  texto.replace("<", "&lt;");
  texto.replace(">", "&gt;");
  texto.replace("\"", "&quot;");
  texto.replace("'", "&#39;");
  return texto;
}

String obtenerRSSITexto() {
  if (WiFi.status() != WL_CONNECTED) {
    return "No disponible";
  }

  return String(WiFi.RSSI()) + " dBm";
}

String generarEstadoHora() {
  if (horaSincronizada) {
    return "Sincronizada";
  } else {
    return "Pendiente";
  }
}

// ------------------------------------------------------------
// ACCESS POINT
// ------------------------------------------------------------

void iniciarAccessPoint() {
  Serial.println();
  Serial.println("Iniciando red propia del ESP32...");

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  if (!WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET)) {
    Serial.println("No se pudo configurar la IP del Access Point.");
  }

  bool apIniciado = WiFi.softAP(AP_SSID, AP_PASSWORD);

  if (apIniciado) {
    Serial.println("Access Point iniciado correctamente.");
    Serial.print("SSID AP: ");
    Serial.println(AP_SSID);
    Serial.print("Password AP: ");
    Serial.println(AP_PASSWORD);
    Serial.print("IP AP: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("URL AP: http://");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("No se pudo iniciar el Access Point.");
  }

  Serial.println();
}

// ------------------------------------------------------------
// WIFI STA
// ------------------------------------------------------------

void conectarWiFiSTA() {
  Serial.println();
  Serial.println("Conectando a Wi-Fi domestico...");
  Serial.print("SSID STA: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);

  // Mismo esquema usado en el proyecto NTP que funciono correctamente.
  if (!WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, DNS_PRIMARIO, DNS_SECUNDARIO)) {
    Serial.println("No se pudo configurar DNS personalizados.");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;

  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    Serial.print(".");
    servidor.handleClient();
    intentos++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexion Wi-Fi domestica establecida correctamente.");
    Serial.print("IP STA: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway STA: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS 1: ");
    Serial.println(WiFi.dnsIP(0));
    Serial.print("DNS 2: ");
    Serial.println(WiFi.dnsIP(1));
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("No se pudo conectar al Wi-Fi domestico.");
    Serial.print("Estado final: ");
    Serial.println(estadoWiFi(WiFi.status()));
    Serial.println("La pagina web seguira disponible desde la red propia del ESP32.");
  }

  Serial.println();
}

// ------------------------------------------------------------
// DIAGNOSTICO DNS
// ------------------------------------------------------------

bool probarDNS(const char* servidorNTP) {
  IPAddress ip;

  Serial.print("Resolviendo ");
  Serial.print(servidorNTP);
  Serial.print(" ... ");

  if (WiFi.hostByName(servidorNTP, ip) && ip != IPAddress(0, 0, 0, 0)) {
    Serial.print("OK -> ");
    Serial.println(ip);
    return true;
  } else {
    Serial.print("ERROR DNS -> ");
    Serial.println(ip);
    return false;
  }
}

bool diagnosticarDNS() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No se puede probar DNS sin conexion Wi-Fi domestica.");
    return false;
  }

  Serial.println("Diagnostico DNS:");

  bool ok1 = probarDNS(NTP_SERVER_1);
  bool ok2 = probarDNS(NTP_SERVER_2);
  bool ok3 = probarDNS(NTP_SERVER_3);
  bool ok4 = probarDNS("time.nist.gov");

  Serial.println();

  return ok1 || ok2 || ok3 || ok4;
}

// ------------------------------------------------------------
// NTP
// ------------------------------------------------------------

bool sincronizarHoraNTP(bool mostrarDetalle = true) {
  if (WiFi.status() != WL_CONNECTED) {
    if (mostrarDetalle) {
      Serial.println("No se puede sincronizar NTP sin conexion Wi-Fi domestica.");
    }
    return false;
  }

  if (mostrarDetalle) {
    Serial.println("Sincronizando hora mediante NTP...");
    Serial.print("Servidor 1: ");
    Serial.println(NTP_SERVER_1);
    Serial.print("Servidor 2: ");
    Serial.println(NTP_SERVER_2);
    Serial.print("Servidor 3: ");
    Serial.println(NTP_SERVER_3);
  }

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    NTP_SERVER_1,
    NTP_SERVER_2,
    NTP_SERVER_3
  );

  struct tm infoTiempo;
  unsigned long inicio = millis();
  unsigned long ultimoPunto = 0;

  while (millis() - inicio < TIMEOUT_NTP_TOTAL_MS) {
    servidor.handleClient();

    if (getLocalTime(&infoTiempo, 100)) {
      horaSincronizada = true;

      if (mostrarDetalle) {
        Serial.println();
        Serial.println("Hora sincronizada correctamente.");
        Serial.print("Fecha y hora actual: ");
        Serial.println(obtenerFechaHora());
        Serial.println();
      }

      return true;
    }

    if (mostrarDetalle && millis() - ultimoPunto >= 500) {
      Serial.print(".");
      ultimoPunto = millis();
    }

    delay(10);
  }

  if (mostrarDetalle) {
    Serial.println();
    Serial.println("No se pudo sincronizar la hora despues de 30 segundos.");
    Serial.println("La pagina web quedara disponible igualmente.");
    Serial.println();
  }

  return false;
}

// ------------------------------------------------------------
// PAGINA WEB
// ------------------------------------------------------------

String generarFila(String etiqueta, String valor) {
  String fila = "";

  fila += "<tr>";
  fila += "<td class='etiqueta'>" + htmlEscape(etiqueta) + "</td>";
  fila += "<td class='valor'>" + htmlEscape(valor) + "</td>";
  fila += "</tr>";

  return fila;
}

String generarPaginaHTML() {
  String html = "";

  html += "<!DOCTYPE html>";
  html += "<html lang='es'>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>ESP32 Web Status</title>";

  html += "<style>";
  html += "body{font-family:Arial,sans-serif;background:#f2f4f8;margin:0;padding:20px;color:#222;}";
  html += ".contenedor{max-width:820px;margin:0 auto;background:white;border-radius:14px;padding:24px;box-shadow:0 4px 18px rgba(0,0,0,0.12);}";
  html += "h1{margin-top:0;color:#1f3b57;}";
  html += ".subtitulo{color:#555;margin-bottom:22px;}";
  html += ".badge{display:inline-block;background:#00979D;color:white;padding:6px 10px;border-radius:20px;font-size:0.85rem;margin-bottom:14px;}";
  html += ".ok{display:inline-block;background:#d8f5df;color:#126426;padding:6px 10px;border-radius:8px;font-weight:bold;}";
  html += ".pendiente{display:inline-block;background:#fff2c2;color:#735600;padding:6px 10px;border-radius:8px;font-weight:bold;}";
  html += "table{width:100%;border-collapse:collapse;margin-top:16px;}";
  html += "td{padding:10px;border-bottom:1px solid #e6e6e6;}";
  html += ".etiqueta{font-weight:bold;width:38%;color:#444;}";
  html += ".valor{font-family:monospace;color:#111;}";
  html += ".footer{margin-top:20px;font-size:0.85rem;color:#666;}";
  html += "</style>";

  html += "</head>";
  html += "<body>";
  html += "<div class='contenedor'>";

  html += "<span class='badge'>ESP32 DEVKIT V1 / ESP-WROOM-32</span>";
  html += "<h1>004 - Web Status</h1>";
  html += "<p class='subtitulo'>Estado general del ESP32 publicado desde un servidor web local.</p>";

  if (horaSincronizada) {
    html += "<p><span class='ok'>Hora sincronizada</span></p>";
  } else {
    html += "<p><span class='pendiente'>Hora pendiente de sincronizacion</span></p>";
  }

  html += "<table>";

  html += generarFila("Modo", "Access Point + Station");
  html += generarFila("SSID AP", AP_SSID);
  html += generarFila("IP AP", WiFi.softAPIP().toString());
  html += generarFila("URL AP", "http://" + WiFi.softAPIP().toString());

  html += generarFila("Estado Wi-Fi domestico", estadoWiFi(WiFi.status()));
  html += generarFila("SSID domestico", WiFi.SSID());
  html += generarFila("IP STA", WiFi.localIP().toString());
  html += generarFila("Gateway STA", WiFi.gatewayIP().toString());
  html += generarFila("DNS 1", WiFi.dnsIP(0).toString());
  html += generarFila("DNS 2", WiFi.dnsIP(1).toString());
  html += generarFila("MAC", WiFi.macAddress());
  html += generarFila("Canal", String(WiFi.channel()));
  html += generarFila("RSSI", obtenerRSSITexto());
  html += generarFila("Calidad de senal", calidadSenal(WiFi.RSSI()));

  html += generarFila("Estado hora", generarEstadoHora());
  html += generarFila("Fecha y hora", obtenerFechaHora());
  html += generarFila("Dia semana", obtenerDiaSemana());
  html += generarFila("Zona horaria", "Argentina UTC-3");
  html += generarFila("Uptime", obtenerUptime());

  html += "</table>";

  html += "<p class='footer'>La pagina se actualiza automaticamente cada 5 segundos.</p>";
  html += "</div>";
  html += "</body>";
  html += "</html>";

  return html;
}

// ------------------------------------------------------------
// SERVIDOR WEB
// ------------------------------------------------------------

void manejarRaiz() {
  Serial.println("Peticion recibida en /");
  servidor.send(200, "text/html; charset=utf-8", generarPaginaHTML());
}

void manejarPing() {
  Serial.println("Peticion recibida en /ping");
  servidor.send(200, "text/plain; charset=utf-8", "pong");
}

void manejarNoEncontrado() {
  String mensaje = "Ruta no encontrada\n\n";
  mensaje += "URI: ";
  mensaje += servidor.uri();
  mensaje += "\n";

  servidor.send(404, "text/plain; charset=utf-8", mensaje);
}

void iniciarServidorWeb() {
  if (!servidorIniciado) {
    servidor.on("/", manejarRaiz);
    servidor.on("/ping", manejarPing);
    servidor.onNotFound(manejarNoEncontrado);
    servidor.begin();

    servidorIniciado = true;
  }

  Serial.println("Servidor web iniciado.");
  Serial.print("URL desde red propia del ESP32: http://");
  Serial.println(WiFi.softAPIP());

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("URL desde red domestica, si es accesible: http://");
    Serial.println(WiFi.localIP());
  }

  Serial.println();
}

// ------------------------------------------------------------
// REPORTE SERIAL
// ------------------------------------------------------------

void imprimirReporteSerial() {
  imprimirSeparador();
  Serial.println("Web Status - Estado general");
  imprimirSeparador();

  String urlAP = "http://" + WiFi.softAPIP().toString();
  String ipSTA = WiFi.localIP().toString();

  Serial.printf("%-24s %s\n", "Modo:", "Access Point + Station");
  Serial.printf("%-24s %s\n", "SSID AP:", AP_SSID);
  Serial.printf("%-24s %s\n", "IP AP:", WiFi.softAPIP().toString().c_str());
  Serial.printf("%-24s %s\n", "URL AP:", urlAP.c_str());

  Serial.printf("%-24s %s\n", "Estado Wi-Fi:", estadoWiFi(WiFi.status()).c_str());
  Serial.printf("%-24s %s\n", "SSID domestico:", WiFi.SSID().c_str());
  Serial.printf("%-24s %s\n", "IP STA:", ipSTA.c_str());
  Serial.printf("%-24s %s\n", "Gateway STA:", WiFi.gatewayIP().toString().c_str());
  Serial.printf("%-24s %s\n", "DNS 1:", WiFi.dnsIP(0).toString().c_str());
  Serial.printf("%-24s %s\n", "DNS 2:", WiFi.dnsIP(1).toString().c_str());

  Serial.printf("%-24s %s\n", "Estado hora:", generarEstadoHora().c_str());
  Serial.printf("%-24s %s\n", "Fecha y hora:", obtenerFechaHora().c_str());
  Serial.printf("%-24s %s\n", "Dia semana:", obtenerDiaSemana().c_str());
  Serial.printf("%-24s %s\n", "Senal RSSI:", obtenerRSSITexto().c_str());
  Serial.printf("%-24s %s\n", "Calidad:", calidadSenal(WiFi.RSSI()).c_str());
  Serial.printf("%-24s %s\n", "Uptime:", obtenerUptime().c_str());

  imprimirSeparador();
  Serial.println();
}

// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================================");
  Serial.println("DOIT ESP32 DEVKIT V1 - WEB STATUS");
  Serial.println("==============================================");

  iniciarAccessPoint();
  iniciarServidorWeb();

  conectarWiFiSTA();

  if (WiFi.status() == WL_CONNECTED) {
    diagnosticarDNS();
    sincronizarHoraNTP(true);
  }

  imprimirReporteSerial();
  ultimoReporte = millis();
  ultimoIntentoNTP = millis();
}

// ------------------------------------------------------------
// LOOP PRINCIPAL
// ------------------------------------------------------------

void loop() {
  servidor.handleClient();

  unsigned long ahora = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!horaSincronizada && ahora - ultimoIntentoNTP >= INTERVALO_REINTENTO_NTP_MS) {
      ultimoIntentoNTP = ahora;

      Serial.println("Reintentando sincronizacion NTP...");
      diagnosticarDNS();
      sincronizarHoraNTP(false);
    }
  } else {
    static unsigned long ultimoIntentoWiFi = 0;

    if (ahora - ultimoIntentoWiFi >= 30000) {
      ultimoIntentoWiFi = ahora;

      Serial.println("Wi-Fi domestico desconectado. Intentando reconectar...");
      conectarWiFiSTA();

      if (WiFi.status() == WL_CONNECTED) {
        diagnosticarDNS();
        sincronizarHoraNTP(true);
        ultimoIntentoNTP = millis();
      }
    }
  }

  if (ahora - ultimoReporte >= INTERVALO_REPORTE_MS) {
    ultimoReporte = ahora;
    imprimirReporteSerial();
  }
}