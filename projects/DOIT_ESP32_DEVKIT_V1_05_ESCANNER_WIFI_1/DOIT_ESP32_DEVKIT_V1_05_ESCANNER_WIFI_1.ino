/*
  ESP32 DEVKIT V1 - Escaner Wi-Fi con salida tabulada

  Verifica:
  - Funcionamiento del Wi-Fi
  - Deteccion de redes cercanas
  - Salida ordenada en el Serial Monitor
*/

#include <WiFi.h>

void imprimirSeparador() {
  Serial.println("---------------------------------------------------------------");
}

void imprimirEncabezadoTabla() {
  imprimirSeparador();
  Serial.printf("%-4s %-32s %8s   %-10s\n", "Nro", "SSID", "Senal", "Seguridad");
  imprimirSeparador();
}

const char* tipoSeguridad(wifi_auth_mode_t tipo) {
  if (tipo == WIFI_AUTH_OPEN) {
    return "Abierta";
  } else {
    return "Protegida";
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 - Escaneo de redes Wi-Fi");
  Serial.println("================================");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Iniciando escaneo...");
}

void loop() {
  int cantidadRedes = WiFi.scanNetworks();

  Serial.println();

  if (cantidadRedes == 0) {
    Serial.println("No se encontraron redes.");
  } else {
    Serial.print("Redes encontradas: ");
    Serial.println(cantidadRedes);

    imprimirEncabezadoTabla();

    for (int i = 0; i < cantidadRedes; i++) {
      Serial.printf(
        "%-4d %-32s %5d dBm   %-10s\n",
        i + 1,
        WiFi.SSID(i).c_str(),
        WiFi.RSSI(i),
        tipoSeguridad(WiFi.encryptionType(i))
      );

      delay(10);
    }

    imprimirSeparador();
  }

  Serial.println("Nuevo escaneo en 10 segundos...");
  Serial.println();

  delay(10000);
}