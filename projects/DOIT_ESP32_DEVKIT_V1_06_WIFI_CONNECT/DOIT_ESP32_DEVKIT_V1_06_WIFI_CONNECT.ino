/*
  ESP32 DEVKIT V1 - Conexion Wi-Fi con estado

  Proyecto: DOIT_ESP32_DEVKIT_V1_06_WIFI_CONNECT

  Verifica:
  - Conexion a una red Wi-Fi conocida
  - Obtencion de direccion IP
  - Lectura de intensidad de señal
  - Datos basicos de red
  - Reconexión si se pierde la conexión

  IMPORTANTE:
  Crear un archivo config_wifi.h en la misma carpeta del sketch.
*/

#include <WiFi.h>
#include "config_wifi.h"

const unsigned long INTERVALO_ESTADO_MS = 5000;
unsigned long ultimoReporte = 0;

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

void imprimirDatosConexion() {
  imprimirSeparador();
  Serial.println("Estado de conexion Wi-Fi");
  imprimirSeparador();

  Serial.printf("%-18s %s\n", "Estado:", estadoWiFi(WiFi.status()).c_str());
  Serial.printf("%-18s %s\n", "SSID:", WiFi.SSID().c_str());
  Serial.printf("%-18s %s\n", "IP local:", WiFi.localIP().toString().c_str());
  Serial.printf("%-18s %s\n", "Gateway:", WiFi.gatewayIP().toString().c_str());
  Serial.printf("%-18s %s\n", "DNS:", WiFi.dnsIP().toString().c_str());
  Serial.printf("%-18s %s\n", "MAC:", WiFi.macAddress().c_str());
  Serial.printf("%-18s %d\n", "Canal:", WiFi.channel());
  Serial.printf("%-18s %d dBm\n", "Senal RSSI:", WiFi.RSSI());
  Serial.printf("%-18s %lu s\n", "Uptime:", millis() / 1000);

  imprimirSeparador();
  Serial.println();
}

void conectarWiFi() {
  Serial.println();
  Serial.println("Conectando a Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;

  while (WiFi.status() != WL_CONNECTED && intentos < 30) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexion establecida correctamente.");
    imprimirDatosConexion();
  } else {
    Serial.println("No se pudo conectar a la red Wi-Fi.");
    Serial.print("Estado final: ");
    Serial.println(estadoWiFi(WiFi.status()));
    Serial.println();
  }
}

// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 - Conexion Wi-Fi");
  Serial.println("================================");

  conectarWiFi();
}

// ------------------------------------------------------------
// LOOP PRINCIPAL
// ------------------------------------------------------------

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi desconectado. Intentando reconectar...");
    conectarWiFi();
  }

  unsigned long ahora = millis();

  if (ahora - ultimoReporte >= INTERVALO_ESTADO_MS) {
    ultimoReporte = ahora;

    if (WiFi.status() == WL_CONNECTED) {
      imprimirDatosConexion();
    }
  }
}