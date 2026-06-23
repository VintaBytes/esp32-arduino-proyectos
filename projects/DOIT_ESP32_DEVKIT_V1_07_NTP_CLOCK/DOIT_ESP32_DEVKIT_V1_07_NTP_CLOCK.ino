/*
  Proyecto: DOIT_ESP32_DEVKIT_V1_07_NTP_CLOCK

  Placa: ESP32 DEVKIT V1 / ESP-WROOM-32
  Entorno: Arduino IDE 2

  Objetivo:
  - Conectar el ESP32 a una red Wi-Fi conocida.
  - Verificar resolucion DNS desde la placa.
  - Sincronizar fecha y hora desde internet usando NTP.
  - Mostrar la hora local en el Serial Monitor.
  - Mostrar estado de Wi-Fi, IP local, DNS, RSSI y uptime.
  - Intentar reconectar si se pierde la conexion Wi-Fi.

  IMPORTANTE:
  Crear un archivo config_wifi.h en la misma carpeta del sketch.
*/

#include <WiFi.h>
#include <time.h>
#include "config_wifi.h"

// ------------------------------------------------------------
// CONFIGURACION GENERAL
// ------------------------------------------------------------

const unsigned long INTERVALO_REPORTE_MS = 5000;
unsigned long ultimoReporte = 0;

// Zona horaria Argentina: UTC-3
const long GMT_OFFSET_SEC = -3 * 60 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;

// Servidores NTP
const char* NTP_SERVER_1 = "ar.pool.ntp.org";
const char* NTP_SERVER_2 = "south-america.pool.ntp.org";
const char* NTP_SERVER_3 = "pool.ntp.org";


IPAddress DNS_PRIMARIO(8, 8, 8, 8);
IPAddress DNS_SECUNDARIO(1, 1, 1, 1);


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

// ------------------------------------------------------------
// WIFI
// ------------------------------------------------------------

void conectarWiFi() {
  Serial.println();
  Serial.println("Conectando a Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);

  // Forzar DNS externos manteniendo IP por DHCP
  if (!WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, DNS_PRIMARIO, DNS_SECUNDARIO)) {
    Serial.println("No se pudo configurar DNS personalizados.");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;

  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
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
  }

  Serial.println();
}

// ------------------------------------------------------------
// DIAGNOSTICO DNS
// ------------------------------------------------------------

void probarDNS(const char* servidor) {
  IPAddress ip;

  Serial.print("Resolviendo ");
  Serial.print(servidor);
  Serial.print(" ... ");

  if (WiFi.hostByName(servidor, ip) && ip != IPAddress(0, 0, 0, 0)) {
    Serial.print("OK -> ");
    Serial.println(ip);
  } else {
    Serial.print("ERROR DNS -> ");
    Serial.println(ip);
  }
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

// ------------------------------------------------------------
// NTP
// ------------------------------------------------------------

void sincronizarHoraNTP() {
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

  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    NTP_SERVER_1,
    NTP_SERVER_2,
    NTP_SERVER_3
  );

  struct tm infoTiempo;

  if (getLocalTime(&infoTiempo, 30000)) {
    Serial.println("Hora sincronizada correctamente.");
    Serial.print("Fecha y hora actual: ");
    Serial.println(obtenerFechaHora());
  } else {
    Serial.println("No se pudo sincronizar la hora despues de 30 segundos.");
  }

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
  Serial.println("DOIT ESP32 DEVKIT V1 - NTP CLOCK");
  Serial.println("==============================================");

  conectarWiFi();

  if (WiFi.status() == WL_CONNECTED) {
    diagnosticarDNS();
    sincronizarHoraNTP();
  }

  imprimirReporte();
}

// ------------------------------------------------------------
// LOOP PRINCIPAL
// ------------------------------------------------------------

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Intentando reconectar...");
    conectarWiFi();

    if (WiFi.status() == WL_CONNECTED) {
      diagnosticarDNS();
      sincronizarHoraNTP();
    }
  }

  unsigned long ahora = millis();

  if (ahora - ultimoReporte >= INTERVALO_REPORTE_MS) {
    ultimoReporte = ahora;
    imprimirReporte();
  }
}