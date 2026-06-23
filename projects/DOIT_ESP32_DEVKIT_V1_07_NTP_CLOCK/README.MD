# 003 - Reloj de Red

![Arduino IDE 2](https://img.shields.io/badge/Arduino%20IDE-2.x-00979D?logo=arduino\&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue)
![Board](https://img.shields.io/badge/Board-ESP32%20DEVKIT%20V1-orange)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![License: MIT](https://img.shields.io/badge/License-MIT-green)

Sincronización de fecha y hora desde internet usando una placa ESP32 DEVKIT V1 con módulo ESP-WROOM-32.

El proyecto conecta el ESP32 a una red Wi-Fi conocida, obtiene la hora actual mediante NTP y muestra en el Serial Monitor un reporte con fecha, hora local, estado de conexión, IP, señal Wi-Fi y tiempo de funcionamiento.

## Objetivo

Verificar que la placa ESP32 pueda conectarse a internet y obtener fecha y hora reales mediante servidores NTP.

Esta prueba confirma:

* Compilación correcta del sketch. 
* Carga correcta desde Arduino IDE 2. 
* Funcionamiento del Serial Monitor. 
* Conexión a una red Wi-Fi conocida. 
* Acceso a internet desde el ESP32. 
* Resolución DNS desde el ESP32.
* Sincronización horaria mediante NTP.
* Visualización de fecha y hora local. 
* Lectura de datos básicos de red. 
* Reconexión básica si se pierde la conexión Wi-Fi.

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
* Acceso a internet.

No requiere sensores, pantalla ni conexiones externas.

## Librerías necesarias

El proyecto usa:

```cpp
#include <WiFi.h>
#include <time.h>
```

La librería `WiFi.h` viene incluida con el core `esp32 by Espressif Systems`.

La librería `time.h` permite trabajar con fecha y hora usando las funciones disponibles en el entorno del ESP32.

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
projects/DOIT_ESP32_DEVKIT_V1_07_NTP_CLOCK/DOIT_ESP32_DEVKIT_V1_07_NTP_CLOCK.ino
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

El proyecto incluye un archivo

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


## Zona horaria

El proyecto está configurado para usar la hora local de Argentina:

```cpp
const long GMT_OFFSET_SEC = -3 * 60 * 60;
const int DAYLIGHT_OFFSET_SEC = 0;
```

Esto corresponde a:

```text
Argentina UTC-3
```

Si se quiere usar otra zona horaria, hay que modificar esos valores en el sketch principal.

## Funcionamiento general

El programa realiza estos pasos:

1. Inicia el puerto serie. 
2. Configura el ESP32 en modo estación Wi-Fi. 
3. Intenta conectarse a la red configurada. 
4. Muestra puntos en el Serial Monitor mientras intenta conectar. 
5. Si la conexión Wi-Fi es exitosa, muestra IP local, gateway, DNS y señal RSSI. 
6. Realiza un diagnóstico DNS resolviendo varios servidores NTP. 
7. Si la resolución DNS funciona, inicia la sincronización NTP. 
8. Consulta servidores de hora en internet. 
9. Obtiene fecha y hora local. 
10. Muestra un reporte general en el Serial Monitor. 
11. Cada 5 segundos vuelve a mostrar fecha, hora, estado de Wi-Fi y datos de red. 
12. Si se pierde la conexión Wi-Fi, intenta reconectar automáticamente.

## Ejemplo de salida

```text
==============================================
DOIT ESP32 DEVKIT V1 - NTP CLOCK
==============================================

Conectando a Wi-Fi...
SSID: Red_Casa
....
Conexion Wi-Fi establecida correctamente.
IP local: 192.168.4.173
Gateway: 192.168.4.1
DNS 1: 192.168.4.1
DNS 2: 0.0.0.0
RSSI: -41 dBm

Diagnostico DNS:
Resolviendo ar.pool.ntp.org ... OK -> 170.155.148.1
Resolviendo south-america.pool.ntp.org ... OK -> 162.159.200.123
Resolviendo pool.ntp.org ... OK -> 170.210.222.2
Resolviendo time.nist.gov ... OK -> 129.6.15.25

Sincronizando hora mediante NTP...
Servidor 1: ar.pool.ntp.org
Servidor 2: south-america.pool.ntp.org
Servidor 3: pool.ntp.org
Hora sincronizada correctamente.
Fecha y hora actual: 23/06/2026 18:44:28

------------------------------------------------------------
Reloj NTP - Estado general
------------------------------------------------------------
Fecha y hora:      23/06/2026 18:44:28
Dia semana:        Martes
Zona horaria:      Argentina UTC-3
Uptime:            8 s
Estado Wi-Fi:      Conectado
SSID:              Red_Casa
IP local:          192.168.4.173
Gateway:           192.168.4.1
DNS:               192.168.4.1
MAC:               AC:67:B2:31:8C:48
Canal:             11
Senal RSSI:        -42 dBm
------------------------------------------------------------
```

## Archivos principales

| Archivo                                 | Descripción                                                      |
| --------------------------------------- | ---------------------------------------------------------------- |
| `DOIT_ESP32_DEVKIT_V1_07_NTP_CLOCK.ino` | Sketch principal del proyecto.                                   |
| `config_wifi.h`                         | Archivo local con credenciales reales.                           |

## Estado del proyecto

* [x] Prueba básica
* [x] Funcional
* [x] Conexión Wi-Fi exitosa
* [x] Acceso a internet
* [x] Sincronización NTP
* [x] Visualización de fecha y hora
* [x] Lectura de datos de red
* [x] Reconexión básica
* [x] Salida tabulada
* [x] Documentado
* [x] Diagnóstico DNS


## Notas

Si el sketch queda esperando con el mensaje `Connecting........` durante la carga, mantener presionado el botón `BOOT` de la placa hasta que comience la transferencia.

Si el programa compila pero no conecta a la red Wi-Fi, revisar:

* Que el archivo `config_wifi.h` exista.
* Que `WIFI_SSID` tenga exactamente el nombre de la red.
* Que `WIFI_PASSWORD` tenga la contraseña correcta.
* Que la red sea de 2.4 GHz, ya que el ESP32 clásico no se conecta a redes Wi-Fi de 5 GHz.
* Que la placa esté dentro del alcance del router.
* Que la red tenga acceso a internet.

Si la conexión Wi-Fi funciona pero la hora no se sincroniza, revisar:

* Que el router tenga acceso a internet.
* Que no haya bloqueo de acceso a servidores NTP.
* Que los servidores configurados estén disponibles.
* Que la placa haya obtenido una dirección IP válida.
* Que la resolución DNS funcione correctamente desde el ESP32.

Si el diagnóstico DNS muestra una salida como:

```text
ERROR DNS -> 0.0.0.0
```

significa que el ESP32 está conectado a la red, pero no pudo resolver correctamente el nombre del servidor. En ese caso, conviene revisar la configuración DNS, el router o probar servidores DNS externos. Este proyecto muestra los días de la semana en español mediante una función propia, por lo que no depende del idioma interno del entorno de ejecución.

Este proyecto es una base útil para relojes, estaciones meteorológicas, dataloggers, servidores web locales, registros de eventos y cualquier sistema ESP32 que necesite fecha y hora reales.

