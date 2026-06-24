# 004 - Web Status

![Arduino IDE 2](https://img.shields.io/badge/Arduino%20IDE-2.x-00979D?logo=arduino\&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-WROOM--32-blue)
![Board](https://img.shields.io/badge/Board-ESP32%20DEVKIT%20V1-orange)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![Web Server](https://img.shields.io/badge/Web%20Server-Local-lightgrey)
![License: MIT](https://img.shields.io/badge/License-MIT-green)

Servidor web local usando una placa ESP32 DEVKIT V1 con módulo ESP-WROOM-32.

El proyecto crea una red Wi-Fi propia en modo Access Point, permite acceder a una página de estado desde el navegador y, si también logra conectarse a una red Wi-Fi doméstica, intenta sincronizar fecha y hora mediante NTP.

## Objetivo

Crear una página web de estado servida directamente desde el ESP32, accesible aunque la red doméstica tenga subredes separadas o aislamiento entre dispositivos.

Esta prueba confirma:

* Compilación correcta del sketch.
* Carga correcta desde Arduino IDE 2.
* Funcionamiento del Serial Monitor.
* Creación de una red Wi-Fi propia desde el ESP32.
* Acceso a una página web servida por el ESP32.
* Funcionamiento básico de un servidor web local.
* Modo combinado Access Point + Station.
* Conexión opcional a una red Wi-Fi doméstica.
* Diagnóstico DNS desde el ESP32.
* Sincronización horaria mediante NTP, si hay acceso a internet.
* Visualización de estado de red, señal Wi-Fi, uptime, fecha y hora.
* Disponibilidad de la página aunque la hora todavía no esté sincronizada.

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
* Teléfono, tablet o computadora con Wi-Fi.
* Red Wi-Fi doméstica disponible, si se quiere sincronizar hora por NTP.
* Nombre y contraseña de la red Wi-Fi doméstica.

No requiere sensores, pantalla ni conexiones externas.

## Librerías necesarias

El proyecto usa:

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
```

La librería `WiFi.h` viene incluida con el core `esp32 by Espressif Systems`.

La librería `WebServer.h` permite crear un servidor web simple dentro del ESP32.

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
projects/DOIT_ESP32_DEVKIT_V1_10_WEB_STATUS/DOIT_ESP32_DEVKIT_V1_10_WEB_STATUS.ino
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

El proyecto incluye un archivo:

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

## Modo Access Point

Este proyecto crea una red Wi-Fi propia desde el ESP32.

Datos configurados en el sketch:

```text
SSID: ESP32_WEB_STATUS
Password: esp32status
IP: 192.168.10.1
```

Para acceder a la página web:

1. Cargar el sketch en la placa.
2. Abrir el Serial Monitor.
3. Esperar a que aparezca la red `ESP32_WEB_STATUS`.
4. Conectar el teléfono o la computadora a esa red.
5. Usar la contraseña `esp32status`.
6. Abrir en el navegador:

```text
http://192.168.10.1/
```

También se puede probar la ruta simple:

```text
http://192.168.10.1/ping
```

Si funciona correctamente, debe responder:

```text
pong
```

Algunos teléfonos pueden indicar que la red `ESP32_WEB_STATUS` no tiene acceso a internet. En ese caso, elegir la opción para mantener la conexión o usar esa red de todos modos.

## Modo Station

Además de crear su propia red, el ESP32 intenta conectarse a la red Wi-Fi doméstica configurada en `config_wifi.h`.

Esta conexión se usa para:

* Obtener acceso a internet.
* Resolver nombres de dominio.
* Consultar servidores NTP.
* Sincronizar fecha y hora.

La página web no depende de que la red doméstica sea accesible desde otros dispositivos. Aunque la red de la casa tenga varias subredes, la página se puede abrir desde la red propia del ESP32:

```text
http://192.168.10.1/
```

Si la conexión doméstica también es accesible desde la red local, el Serial Monitor puede mostrar una segunda URL con la IP asignada por el router.

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
2. Configura el ESP32 en modo `Access Point + Station`.
3. Crea la red Wi-Fi propia `ESP32_WEB_STATUS`.
4. Asigna al ESP32 la IP fija `192.168.10.1` para el Access Point.
5. Inicia el servidor web local.
6. Intenta conectarse a la red Wi-Fi doméstica configurada.
7. Si la conexión doméstica es exitosa, muestra IP, gateway, DNS y señal RSSI.
8. Realiza un diagnóstico DNS resolviendo servidores NTP.
9. Intenta sincronizar fecha y hora mediante NTP.
10. Mantiene la página web disponible aunque la hora no haya sincronizado.
11. Muestra una página de estado con datos de red, señal, hora y uptime.
12. Actualiza automáticamente la página cada 5 segundos.
13. Reintenta sincronizar la hora si todavía está pendiente.
14. Si se pierde la conexión Wi-Fi doméstica, intenta reconectar periódicamente.

## Ejemplo de salida

```text
==============================================
DOIT ESP32 DEVKIT V1 - WEB STATUS
==============================================

Iniciando red propia del ESP32...
Access Point iniciado correctamente.
SSID AP: ESP32_WEB_STATUS
Password AP: esp32status
IP AP: 192.168.10.1
URL AP: http://192.168.10.1

Servidor web iniciado.
URL desde red propia del ESP32: http://192.168.10.1

Conectando a Wi-Fi domestico...
SSID STA: Red_Casa
....
Conexion Wi-Fi domestica establecida correctamente.
IP STA: 192.168.0.100
Gateway STA: 192.168.0.1
DNS 1: 192.168.0.1
DNS 2: 0.0.0.0
RSSI: -39 dBm

Diagnostico DNS:
Resolviendo ar.pool.ntp.org ... OK -> 170.155.148.1
Resolviendo south-america.pool.ntp.org ... OK -> 162.159.200.123
Resolviendo pool.ntp.org ... OK -> 170.210.222.2
Resolviendo time.nist.gov ... OK -> 129.6.15.25

Sincronizando hora mediante NTP...
Servidor 1: ar.pool.ntp.org
Servidor 2: south-america.pool.ntp.org
Servidor 3: pool.ntp.org
.
Hora sincronizada correctamente.
Fecha y hora actual: 23/06/2026 19:10:42

------------------------------------------------------------
Web Status - Estado general
------------------------------------------------------------
Modo:                    Access Point + Station
SSID AP:                 ESP32_WEB_STATUS
IP AP:                   192.168.10.1
URL AP:                  http://192.168.10.1
Estado Wi-Fi:            Conectado
SSID domestico:          Red_Casa
IP STA:                  192.168.0.100
Gateway STA:             192.168.0.1
DNS 1:                   192.168.0.1
DNS 2:                   0.0.0.0
Estado hora:             Sincronizada
Fecha y hora:            23/06/2026 19:10:42
Dia semana:              Martes
Senal RSSI:              -39 dBm
Calidad:                 Excelente
Uptime:                  0 d 00:00:12
------------------------------------------------------------
```

## Página web

La página muestra:

* Modo de funcionamiento.
* SSID del Access Point.
* IP del Access Point.
* URL de acceso.
* Estado de la conexión Wi-Fi doméstica.
* SSID doméstico.
* IP asignada por el router.
* Gateway.
* DNS.
* MAC.
* Canal Wi-Fi.
* Intensidad de señal RSSI.
* Calidad estimada de señal.
* Estado de sincronización horaria.
* Fecha y hora local.
* Día de la semana.
* Zona horaria.
* Uptime.

La página se actualiza automáticamente cada 5 segundos.

## Archivos principales

| Archivo                                  | Descripción                                                   |
| ---------------------------------------- | ------------------------------------------------------------- |
| `DOIT_ESP32_DEVKIT_V1_10_WEB_STATUS.ino` | Sketch principal del proyecto.                                |
| `config_wifi.h`                          | Archivo local con las credenciales de la red Wi-Fi doméstica. |

## Estado del proyecto

* [x] Prueba básica
* [x] Funcional
* [x] Access Point propio
* [x] Servidor web local
* [x] Página accesible desde navegador
* [x] Ruta `/ping`
* [x] Modo Access Point + Station
* [x] Conexión Wi-Fi doméstica
* [x] Diagnóstico DNS
* [x] Sincronización NTP
* [x] Visualización de fecha y hora
* [x] Día de la semana en español
* [x] Lectura de datos de red
* [x] Lectura de señal RSSI
* [x] Uptime formateado
* [x] Página disponible aunque la hora esté pendiente
* [x] Documentado

## Notas

Si el sketch queda esperando con el mensaje `Connecting........` durante la carga, mantener presionado el botón `BOOT` de la placa hasta que comience la transferencia.

Para acceder a la página web, conectar el teléfono o la computadora a la red:

```text
ESP32_WEB_STATUS
```

Luego abrir:

```text
http://192.168.10.1/
```

Si el teléfono avisa que la red no tiene internet, elegir la opción para permanecer conectado.

Si la conexión Wi-Fi doméstica funciona pero la hora no se sincroniza, revisar:

* Que el router tenga acceso a internet.
* Que no haya bloqueo de acceso a servidores NTP.
* Que los servidores configurados estén disponibles.
* Que la placa haya obtenido una dirección IP válida.
* Que la resolución DNS funcione correctamente desde el ESP32.

Si el diagnóstico DNS muestra una salida como:

```text
ERROR DNS -> 0.0.0.0
```

significa que el ESP32 está conectado a la red, pero no pudo resolver correctamente el nombre del servidor. En ese caso, conviene revisar la configuración DNS, el router o probar servidores DNS externos.

Este proyecto es una base útil para paneles de estado, servidores web embebidos, estaciones meteorológicas, configuradores Wi-Fi, interfaces de sensores y dispositivos ESP32 que necesiten mostrar información desde un navegador.
