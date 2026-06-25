# 005 - ESP32-S3 Touch LCD 3.5B - WiFi Connect

![Arduino IDE 2](https://img.shields.io/badge/Arduino%20IDE-2.x-00979D?logo=arduino\&logoColor=white)
![ESP32-S3](https://img.shields.io/badge/ESP32-S3-blue)
![Board](https://img.shields.io/badge/Board-ESP32S3%20Dev%20Module-orange)
![Display](https://img.shields.io/badge/Display-Waveshare%203.5B-green)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![License: MIT](https://img.shields.io/badge/License-MIT-green)

Adaptacion del [proyecto WiFi Connect para la placa Waveshare ESP32-S3 Touch LCD 3.5B](https://github.com/VintaBytes/esp32-arduino-proyectos/blob/main/projects/DOIT_ESP32_DEVKIT_V1_06_WIFI_CONNECT).

El proyecto conecta la placa a una red Wi-Fi conocida y muestra los datos de conexion tanto en el Serial Monitor como en la pantalla incorporada.

## Objetivo

Verificar que la placa ESP32-S3 Touch LCD 3.5B pueda conectarse correctamente a una red Wi-Fi desde Arduino IDE 2 y mostrar informacion basica de red en pantalla.

Esta prueba confirma:

* Compilacion correcta del sketch.
* Carga correcta desde Arduino IDE 2.
* Funcionamiento del Serial Monitor.
* Inicializacion de la pantalla incorporada.
* Conexión a una red Wi-Fi conocida.
* Obtencion de direccion IP local.
* Lectura de datos basicos de red.
* Lectura de intensidad de senal Wi-Fi.
* Reconexion basica si se pierde la conexion.

## Placa utilizada

* Placa: Waveshare ESP32-S3 Touch LCD 3.5B
* Modulo: ESP32-S3R8
* Entorno: Arduino IDE 2
* Board usada: `ESP32S3 Dev Module`
* Monitor serie: `115200 baud`

## Componentes necesarios

* Placa Waveshare ESP32-S3 Touch LCD 3.5B.
* Cable USB-C de datos.
* Computadora con Arduino IDE 2.
* Red Wi-Fi disponible de 2.4 GHz.
* Nombre y contraseña de la red Wi-Fi.

No requiere sensores ni conexiones externas.

## Librerias necesarias

El proyecto usa:

```cpp
#include <WiFi.h>
#include <Wire.h>
#include <Arduino_GFX_Library.h>
#include "TCA9554.h"
#include "config_wifi.h"
```

Instalar desde el Library Manager de Arduino IDE:

* `GFX Library for Arduino`
* `TCA9554`

La libreria `WiFi.h` viene incluida con el core `esp32 by Espressif Systems`.

## Configuracion en Arduino IDE 2

Seleccionar la placa:

```text
Tools -> Board -> esp32 -> ESP32S3 Dev Module
```

Configuracion recomendada:

```text
USB CDC On Boot: Enabled
CPU Frequency: 240MHz (WiFi)
Flash Mode: QIO 80MHz
Flash Size: 16MB (128Mb)
Partition Scheme: 16M Flash (3MB APP/9.9MB FATFS)
PSRAM: OPI PSRAM
Upload Mode: UART0 / Hardware CDC
Upload Speed: 921600
USB Mode: Hardware CDC and JTAG
```

Seleccionar el puerto:

```text
Tools -> Port
```

Abrir el monitor serie:

```text
Tools -> Serial Monitor
```

Configurar:

```text
115200 baud
```

## Configuracion de la red Wi-Fi

El proyecto usa un archivo local:

```text
config_wifi.h
```

Antes de compilar, completar los datos de la red Wi-Fi:

```cpp
#ifndef CONFIG_WIFI_H
#define CONFIG_WIFI_H

const char* WIFI_SSID = "NOMBRE_DE_TU_RED";
const char* WIFI_PASSWORD = "CONTRASENA_DE_TU_RED";

#endif
```

El archivo `config_wifi.h` debe quedar en la misma carpeta que el sketch principal.

## Funcionamiento general

El programa realiza estos pasos:

1. Inicia el puerto serie.
2. Inicializa la pantalla incorporada.
3. Configura el ESP32-S3 en modo estacion Wi-Fi.
4. Intenta conectarse a la red configurada.
5. Muestra el progreso de conexion por Serial Monitor y por pantalla.
6. Si la conexion es exitosa, muestra datos de red en ambos medios.
7. Cada 5 segundos vuelve a mostrar el estado de conexion.
8. Si se pierde la conexion, intenta reconectar automaticamente.

## Datos mostrados

El proyecto muestra:

* Estado de conexion.
* SSID.
* IP local.
* Gateway.
* DNS.
* Direccion MAC.
* Canal Wi-Fi.
* Intensidad de senal RSSI.
* Tiempo de funcionamiento del programa.

## Archivos principales

| Archivo | Descripcion |
| ------- | ----------- |
| `ESP32_S3_TOUCH_LCD_35B_01_WIFI_CONNECT.ino` | Sketch principal del proyecto. |
| `config_wifi.h` | Archivo local con credenciales Wi-Fi. No debe compartirse con datos reales. |

## Estado del proyecto

* [x] Adaptacion inicial preparada
* [x] Salida por Serial Monitor
* [x] Salida por pantalla incorporada
* [x] Documentado
* [x] Compilado en Arduino IDE 2
* [x] Probado en placa real
* [x] Conexion Wi-Fi validada
* [x] Reconexion basica validada

## Notas

Este proyecto no usa todavia la pantalla tactil, la IMU, la bateria, la microSD ni LVGL.

La primera version usa `Arduino_GFX` para mostrar texto de forma simple en pantalla. LVGL queda reservado para proyectos posteriores con interfaz grafica, botones, menus y pantallas multiples.

Si el programa compila pero no conecta a la red Wi-Fi, revisar:

* Que el archivo `config_wifi.h` exista.
* Que `WIFI_SSID` tenga exactamente el nombre de la red.
* Que `WIFI_PASSWORD` tenga la contraseña correcta.
* Que la red sea de 2.4 GHz.
* Que la placa este dentro del alcance del router.
