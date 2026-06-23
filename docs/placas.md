# Placas

## ESP32 DEVKIT V1 / ESP-WROOM-32

Primera placa usada en este repositorio.

Características generales:

- Módulo: ESP-WROOM-32
- Familia: ESP32 clásico
- Wi-Fi integrado
- Bluetooth integrado
- GPIO de 3.3 V
- Programación por USB desde Arduino IDE 2

## Configuración usada en Arduino IDE 2

Placa recomendada:

```text
Tools → Board → esp32 → DOIT ESP32 DEVKIT V1
```

Alternativa compatible:

```text
Tools → Board → esp32 → ESP32 Dev Module
```

Monitor serie:

```text
115200 baud
```

## Advertencias importantes

Los pines GPIO del ESP32 trabajan a 3.3 V. No se deben conectar señales de 5 V directamente a los pines de entrada/salida.

La alimentación por USB es suficiente para las pruebas básicas. Para proyectos con sensores, pantallas o motores, conviene revisar el consumo total y la forma de alimentación.

## LED integrado

Muchas placas ESP32 DEVKIT V1 tienen un LED integrado conectado al GPIO 2. No todas las placas lo tienen o lo conectan igual, por eso las pruebas principales de este repositorio usan también el Serial Monitor.
