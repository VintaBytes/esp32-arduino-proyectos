# Proyectos ESP32 con Arduino IDE

Repositorio personal de proyectos y pruebas con placas basadas en ESP32 usando Arduino IDE 2.

El objetivo es reunir ejercicios, experimentos, módulos reutilizables y proyectos completos relacionados con conectividad Wi-Fi, sensores, pantallas, almacenamiento, baterías y dispositivos interactivos.

## Descripción breve para GitHub

Proyectos y pruebas con placas ESP32 usando Arduino IDE 2: Wi-Fi, sensores, pantallas, configuración y módulos reutilizables.

## Contenido del repositorio

```text
esp32-arduino-proyectos/
├── README.md
├── LICENSE
├── .gitignore
├── docs/
│   ├── instalacion-arduino-ide.md
│   ├── librerias.md
│   ├── notas-tecnicas.md
│   └── placas.md
├── projects/
│   └── 001-wifi-scan/
│       ├── README.md
│       └── wifi_scan/
│           └── wifi_scan.ino
└── assets/
    └── README.md
```

## Proyectos

| Nº | Proyecto | Descripción | Estado |
| --: | --- | --- | --- |
| 001 | [WiFi Scan](projects/001-wifi-scan/) | Escaneo de redes Wi-Fi cercanas desde una placa ESP32 DEVKIT V1 / ESP-WROOM-32, con salida tabulada en el Serial Monitor. | Funcional |

## Placas utilizadas

Por ahora el repositorio incluye pruebas realizadas con:

- ESP32 DEVKIT V1
- Módulo ESP-WROOM-32

Más adelante se pueden agregar proyectos para otras placas ESP32, ESP32-S3, pantallas táctiles, sensores externos y módulos de alimentación.

## Entorno de desarrollo

Los proyectos están pensados para:

- Arduino IDE 2
- Core `esp32 by Espressif Systems`
- Monitor serie a `115200 baud`, salvo que el proyecto indique otra configuración

La configuración general de Arduino IDE está documentada en [`docs/instalacion-arduino-ide.md`](docs/instalacion-arduino-ide.md).

## Organización del código

Cada proyecto vive dentro de `projects/` y tiene su propio `README.md`.

En los proyectos Arduino, la carpeta del sketch y el archivo `.ino` deben tener el mismo nombre. Por ejemplo:

```text
wifi_scan/
└── wifi_scan.ino
```

Esto permite abrir el proyecto directamente desde Arduino IDE usando:

```text
File → Open...
```

## Manejo de credenciales

Las contraseñas Wi-Fi, claves de API y otros datos sensibles no deben subirse al repositorio.

Este repositorio ignora archivos comunes para credenciales locales, como:

- `arduino_secrets.h`
- `secrets.h`
- `config_local.h`
- `wifi_credentials.h`

Cuando un proyecto necesite credenciales reales, debería incluirse un archivo de ejemplo sin datos privados.

## Próximos proyectos posibles

- Conexión a una red Wi-Fi conocida.
- Portal de configuración Wi-Fi desde el ESP32.
- Lectura de sensores ambientales.
- Estación meteorológica con datos locales y datos web.
- Uso de pantalla táctil en ESP32-S3.
- Rotación de pantalla usando IMU.

## Licencia

Este repositorio se publica con fines educativos y personales bajo licencia MIT. Ver [`LICENSE`](LICENSE).
