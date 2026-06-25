/*
  ESP32-S3 TOUCH LCD 3.5B - WiFi Connect con pantalla

  Proyecto: ESP32_S3_TOUCH_LCD_35B_06_WIFI_CONNECT

  Verifica:
  - Conexion a una red Wi-Fi conocida
  - Obtencion de direccion IP
  - Lectura de intensidad de senal
  - Datos basicos de red
  - Salida por Serial Monitor
  - Salida por pantalla incorporada
  - Reconexion si se pierde la conexion

  IMPORTANTE:
  Crear un archivo config_wifi.h en la misma carpeta del sketch.
*/

#include <WiFi.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "TCA9554.h"
#include "config_wifi.h"

/************************************************************
 *  SECCION: CONFIGURACION GENERAL
 ************************************************************/

const unsigned long INTERVALO_ESTADO_MS = 5000;
const int MAX_INTENTOS_WIFI = 30;

unsigned long ultimoReporte = 0;
bool pantallaLista = false;

/************************************************************
 *  SECCION: CONFIGURACION DE PANTALLA WAVESHARE
 ************************************************************/

#define LCD_QSPI_CS   12
#define LCD_QSPI_CLK  5
#define LCD_QSPI_D0   1
#define LCD_QSPI_D1   2
#define LCD_QSPI_D2   3
#define LCD_QSPI_D3   4
#define GFX_BL        6

// Orientacion de la pantalla.
// 0: vertical normal. Luego podemos probar 1, 2 o 3 si queremos rotarla.
#define ROTATION      0

const int SCREEN_WIDTH = 320;
const int SCREEN_HEIGHT = 480;

// Colores RGB565 basicos.
const uint16_t COLOR_BG       = 0x0000;
const uint16_t COLOR_PANEL    = 0x2104;
const uint16_t COLOR_WHITE    = 0xFFFF;
const uint16_t COLOR_GRAY     = 0x8410;
const uint16_t COLOR_CYAN     = 0x07FF;
const uint16_t COLOR_GREEN    = 0x07E0;
const uint16_t COLOR_YELLOW   = 0xFFE0;
const uint16_t COLOR_RED      = 0xF800;
const uint16_t COLOR_BLUE     = 0x001F;

// Expansor usado por la placa para reset/habilitacion de la pantalla.
TCA9554 TCA(0x20);

// Configuracion tomada del ejemplo oficial 08_gfx_helloworld de Waveshare.
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
  LCD_QSPI_CS,
  LCD_QSPI_CLK,
  LCD_QSPI_D0,
  LCD_QSPI_D1,
  LCD_QSPI_D2,
  LCD_QSPI_D3
);

Arduino_GFX *panel = new Arduino_AXS15231B(
  bus,
  -1,       // RST
  0,        // rotacion interna del driver
  false,
  SCREEN_WIDTH,
  SCREEN_HEIGHT
);

Arduino_Canvas *gfx = new Arduino_Canvas(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  panel,
  0,
  0,
  ROTATION
);

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

void mostrarPantallaInicio() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("WiFi Connect", "ESP32-S3 Touch LCD 3.5B", COLOR_WHITE);

  gfx->setCursor(20, 100);
  gfx->setTextSize(2);
  gfx->setTextColor(COLOR_WHITE);
  gfx->println("Iniciando prueba...");

  gfx->setCursor(20, 140);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Salida por Serial Monitor y pantalla.");

  gfx->flush();
}

void mostrarPantallaConectando(int intento) {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("WiFi Connect", "Conectando a la red", COLOR_YELLOW);

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
  dibujarEncabezado("WiFi Connect", "No se pudo conectar", COLOR_RED);

  escribirCampo(90, "SSID: ", recortarTexto(WIFI_SSID, 18), COLOR_WHITE);
  escribirCampo(125, "Estado: ", estadoWiFi(WiFi.status()), COLOR_RED);

  gfx->setCursor(14, 190);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Revisar SSID, clave, senal o red 2.4 GHz.");

  gfx->flush();
}

void mostrarDatosConexionEnPantalla() {
  if (!pantallaLista) return;

  limpiarPantalla();
  dibujarEncabezado("WiFi Connect", "Estado de conexion", colorEstadoWiFi());

  int y = 78;
  const int salto = 34;

  escribirCampo(y, "Estado: ", estadoWiFi(WiFi.status()), colorEstadoWiFi());
  y += salto;

  escribirCampo(y, "SSID: ", recortarTexto(WiFi.SSID(), 18), COLOR_WHITE);
  y += salto;

  escribirCampo(y, "IP: ", WiFi.localIP().toString(), COLOR_WHITE);
  y += salto;

  escribirCampo(y, "Gateway: ", WiFi.gatewayIP().toString(), COLOR_WHITE);
  y += salto;

  escribirCampo(y, "DNS: ", WiFi.dnsIP().toString(), COLOR_WHITE);
  y += salto;

  escribirCampo(y, "MAC: ", WiFi.macAddress(), COLOR_WHITE);
  y += salto;

  escribirCampo(y, "Canal: ", String(WiFi.channel()), COLOR_WHITE);
  y += salto;

  escribirCampo(y, "RSSI: ", String(WiFi.RSSI()) + " dBm", COLOR_WHITE);
  y += salto;

  escribirCampo(y, "Uptime: ", String(millis() / 1000) + " s", COLOR_WHITE);

  gfx->setCursor(14, 445);
  gfx->setTextSize(1);
  gfx->setTextColor(COLOR_GRAY);
  gfx->println("Actualiza cada 5 segundos.");

  gfx->flush();
}

/************************************************************
 *  SECCION: FUNCIONES DE WIFI
 ************************************************************/

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
  WiFi.disconnect();
  delay(300);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int intentos = 0;
  mostrarPantallaConectando(intentos);

  while (WiFi.status() != WL_CONNECTED && intentos < MAX_INTENTOS_WIFI) {
    delay(500);
    Serial.print(".");
    intentos++;
    mostrarPantallaConectando(intentos);
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conexion establecida correctamente.");
    imprimirDatosConexion();
    mostrarDatosConexionEnPantalla();
  } else {
    Serial.println("No se pudo conectar a la red Wi-Fi.");
    Serial.print("Estado final: ");
    Serial.println(estadoWiFi(WiFi.status()));
    Serial.println();
    mostrarPantallaErrorConexion();
  }
}

/************************************************************
 *  SECCION: SETUP
 ************************************************************/

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("==============================================");
  Serial.println("ESP32-S3 TOUCH LCD 3.5B - WIFI CONNECT");
  Serial.println("==============================================");

  pantallaLista = inicializarPantalla();

  if (pantallaLista) {
    Serial.println("Pantalla inicializada correctamente.");
    mostrarPantallaInicio();
    delay(1200);
  } else {
    Serial.println("La pantalla no pudo inicializarse. Continua solo por Serial.");
  }

  conectarWiFi();
}

/************************************************************
 *  SECCION: LOOP PRINCIPAL
 ************************************************************/

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
      mostrarDatosConexionEnPantalla();
    }
  }
}
