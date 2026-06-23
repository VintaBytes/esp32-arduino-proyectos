# Instalación y configuración en Arduino IDE 2

Esta guía resume la configuración usada para programar una placa ESP32 DEVKIT V1 con módulo ESP-WROOM-32 desde Arduino IDE 2.

## 1. Agregar soporte para ESP32

En Arduino IDE 2, abrir:

```text
File → Preferences
```

En el campo:

```text
Additional Boards Manager URLs
```

agregar:

```text
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

## 2. Instalar el core ESP32

Abrir:

```text
Tools → Board → Boards Manager...
```

Buscar:

```text
esp32
```

Instalar:

```text
esp32 by Espressif Systems
```

## 3. Seleccionar la placa

Para una placa etiquetada como ESP32 DEVKIT V1 con módulo ESP-WROOM-32, probar primero:

```text
Tools → Board → esp32 → DOIT ESP32 DEVKIT V1
```

Si esa opción no aparece o genera problemas, usar:

```text
Tools → Board → esp32 → ESP32 Dev Module
```

## 4. Seleccionar el puerto

Conectar la placa por USB y seleccionar el puerto correspondiente:

```text
Tools → Port
```

En Linux normalmente aparece como algo similar a:

```text
/dev/ttyUSB0
```

o:

```text
/dev/ttyACM0
```

## 5. Monitor serie

Para las pruebas de este repositorio, salvo indicación contraria, abrir:

```text
Tools → Serial Monitor
```

Configurar:

```text
115200 baud
```

## 6. Problema frecuente: queda en Connecting...

Si durante la carga aparece el mensaje:

```text
Connecting........
```

mantener presionado el botón `BOOT` de la placa hasta que Arduino IDE empiece a subir el programa. Luego se puede soltar.

## 7. Permisos en Linux

Si Arduino IDE detecta el puerto pero no tiene permiso para usarlo, agregar el usuario al grupo `dialout`:

```bash
sudo usermod -aG dialout $USER
```

Después cerrar sesión y volver a entrar, o reiniciar el sistema.
