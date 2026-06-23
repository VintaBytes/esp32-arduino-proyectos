# 002 - WiFi Connect

![Arduino IDE 2](https://img.shields.io/badge/Arduino%20IDE-2.x-00979D?logo=arduino\&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue)
![Board](https://img.shields.io/badge/Board-ESP32%20DEVKIT%20V1-orange)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![License: MIT](https://img.shields.io/badge/License-MIT-green)

Conexión de una placa ESP32 DEVKIT V1 con módulo ESP-WROOM-32 a una red Wi-Fi conocida.

El proyecto muestra en el Serial Monitor el estado de la conexión, la dirección IP local asignada, el gateway, el DNS, la dirección MAC, el canal Wi-Fi, la intensidad de señal y el tiempo de funcionamiento del programa.

## Objetivo

Verificar que la placa ESP32 pueda conectarse correctamente a una red Wi-Fi real desde Arduino IDE 2.

Esta prueba confirma:

* Compilación correcta del sketch.
* Carga correcta desde Arduino IDE 2.
* Funcionamiento del Serial Monitor.
* Conexión a una red Wi-Fi conocida.
* Obtención de dirección IP local.
* Lectura de datos básicos de red.
* Lectura de intensidad de señal Wi-Fi.
* Reconexión básica si se pierde la conexión.

## Placa utilizada

* Placa: ESP32 DEVKIT V1
* Módulo: ESP-WROOM-32
* Entorno: Arduino IDE 2
* Board usada: `DOIT ESP32 DEVKIT V1`
* Board alternativa: `ESP32 Dev Module`
* Monitor serie: `115200 baud`

## Componentes necesarios

* Placa ESP32 DEVKIT V1 o compatible.
* Cable USB de datos.
* Computadora con Arduino IDE 2.
* Red Wi-Fi disponible.
* Nombre y contraseña de la red Wi-Fi.

No requiere sensores, pantalla ni conexiones externas.

## Librerías necesarias

El proyecto usa:

```cpp
#include <WiFi.h>
```

La librería `WiFi.h` viene incluida con el core `esp32 by Espressif Systems`.

También usa un archivo local de configuración:

```cpp
#include "config_wifi.h"
```

## Cómo abrir el proyecto

Desde Arduino IDE 2:

```text
File → Open...
```

Abrir el archivo:

```text
projects/DOIT_ESP32_DEVKIT_V1_06_WIFI_CONNECT/DOIT_ESP32_DEVKIT_V1_06_WIFI_CONNECT.ino
```

## Configuración en Arduino IDE 2

Seleccionar la placa:

```text
Tools → Board → esp32 → DOIT ESP32 DEVKIT V1
```

Si esa opción no está disponible, usar:

```text
Tools → Board → esp32 → ESP32 Dev Module
```

Seleccionar el puerto:

```text
Tools → Port
```

Abrir el monitor serie:

```text
Tools → Serial Monitor
```

Configurar:

```text
115200 baud
```

## Configuración de la red Wi-Fi

El proyecto incluye un archivo de ejemplo:

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
2. Configura el ESP32 en modo estación Wi-Fi.
3. Intenta conectarse a la red configurada.
4. Muestra puntos en el Serial Monitor mientras intenta conectar.
5. Si la conexión es exitosa, muestra una tabla con los datos de red.
6. Cada 5 segundos vuelve a mostrar el estado de conexión.
7. Si se pierde la conexión, intenta reconectar automáticamente.

## Ejemplo de salida

```text
==============================================
DOIT ESP32 DEVKIT V1 - WIFI CONNECT
==============================================

Conectando a Wi-Fi...
SSID: Red_Casa
.....
Conexion establecida correctamente.
------------------------------------------------------------
Estado de conexion Wi-Fi
------------------------------------------------------------
Estado:            Conectado
SSID:              Red_Casa
IP local:          192.168.1.45
Gateway:           192.168.1.1
DNS:               192.168.1.1
MAC:               A0:B7:65:12:34:56
Canal:             6
Senal RSSI:        -42 dBm
Uptime:            4 s
------------------------------------------------------------
```

## Archivos principales

| Archivo                                    | Descripción                                                      |
| ------------------------------------------ | ---------------------------------------------------------------- |
| `DOIT_ESP32_DEVKIT_V1_06_WIFI_CONNECT.ino` | Sketch principal del proyecto.                                   |
| `config_wifi.h`                            | Archivo local con credenciales. No debe compartirse.             |

## Estado del proyecto

* [x] Prueba básica
* [x] Funcional
* [x] Conexión Wi-Fi exitosa
* [x] Lectura de datos de red
* [x] Reconexión básica
* [x] Salida tabulada
* [x] Documentado

## Notas

Si el sketch queda esperando con el mensaje `Connecting........` durante la carga, mantener presionado el botón `BOOT` de la placa hasta que comience la transferencia.

Si el programa compila pero no conecta a la red Wi-Fi, revisar:

* Que el archivo `config_wifi.h` exista.
* Que `WIFI_SSID` tenga exactamente el nombre de la red.
* Que `WIFI_PASSWORD` tenga la contraseña correcta.
* Que la red sea de 2.4 GHz, ya que el ESP32 clásico no se conecta a redes Wi-Fi de 5 GHz.
* Que la placa esté dentro del alcance del router.

Este proyecto es una base útil para trabajos posteriores con APIs, servidores web locales, NTP, sensores conectados a internet o paneles de estado.
