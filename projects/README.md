# 001 - WiFi Scan

Escaneo de redes Wi-Fi cercanas usando una placa ESP32 DEVKIT V1 con módulo ESP-WROOM-32.

El proyecto muestra en el Serial Monitor una tabla con las redes encontradas, su intensidad de señal y si la red está abierta o protegida.

## Objetivo

Verificar que la placa ESP32 esté correctamente configurada en Arduino IDE 2 y que el módulo Wi-Fi funcione.

Esta prueba confirma:

- Compilación correcta del sketch.
- Carga correcta desde Arduino IDE 2.
- Funcionamiento del Serial Monitor.
- Funcionamiento básico del Wi-Fi integrado.
- Lectura ordenada de redes cercanas.

## Placa utilizada

- Placa: ESP32 DEVKIT V1
- Módulo: ESP-WROOM-32
- Entorno: Arduino IDE 2
- Board usada: `DOIT ESP32 DEVKIT V1`
- Board alternativa: `ESP32 Dev Module`
- Monitor serie: `115200 baud`

## Componentes necesarios

- Placa ESP32 DEVKIT V1 o compatible.
- Cable USB de datos.
- Computadora con Arduino IDE 2.

No requiere sensores, pantalla ni conexiones externas.

## Librerías necesarias

El proyecto usa:

```cpp
#include <WiFi.h>
```

La librería `WiFi.h` viene incluida con el core `esp32 by Espressif Systems`.

## Cómo abrir el proyecto

Desde Arduino IDE 2:

```text
File → Open...
```

Abrir el archivo:

```text
projects/001-wifi-scan/wifi_scan/wifi_scan.ino
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

## Funcionamiento general

El programa realiza estos pasos:

1. Inicia el puerto serie.
2. Configura el Wi-Fi en modo estación.
3. Desconecta cualquier conexión Wi-Fi previa.
4. Escanea las redes disponibles.
5. Muestra los resultados en una tabla.
6. Espera 10 segundos.
7. Repite el escaneo.

## Ejemplo de salida

```text
Redes encontradas: 5
---------------------------------------------------------------
Nro  SSID                                Senal   Seguridad
---------------------------------------------------------------
1    Red_Casa                              -40 dBm   Protegida
2    Invitados                             -66 dBm   Protegida
3    Oficina                               -72 dBm   Protegida
4    Red_Abierta                           -79 dBm   Abierta
5    Otra_Red                              -85 dBm   Protegida
---------------------------------------------------------------
Nuevo escaneo en 10 segundos...
```

## Archivos principales

| Archivo | Descripción |
| --- | --- |
| `wifi_scan.ino` | Sketch principal del proyecto. |

## Estado del proyecto

- [x] Prueba básica
- [x] Funcional
- [x] Salida tabulada
- [x] Documentado
- [ ] Ampliado con más datos de cada red
- [ ] Reutilizable como módulo auxiliar

## Posibles mejoras

- Mostrar canal de cada red.
- Mostrar tipo de seguridad con más detalle.
- Mostrar BSSID/MAC del punto de acceso.
- Ordenar redes por intensidad de señal.
- Filtrar redes duplicadas.
- Mostrar la salida en una pantalla conectada al ESP32.
- Usar este proyecto como base para un selector de red Wi-Fi.

## Notas

Si el sketch queda esperando con el mensaje `Connecting........` durante la carga, mantener presionado el botón `BOOT` de la placa hasta que comience la transferencia.
