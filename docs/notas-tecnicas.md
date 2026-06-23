# Notas técnicas

## Salida tabulada por Serial Monitor

Para mejorar la lectura de datos en el Serial Monitor, los sketches pueden usar `Serial.printf()`.

Ejemplo:

```cpp
Serial.printf("%-4d %-32s %5d dBm   %-10s\n", numero, ssid, rssi, seguridad);
```

Algunos formatos útiles:

| Formato | Uso |
| --- | --- |
| `%d` | Número entero |
| `%s` | Texto |
| `%5d` | Entero alineado en un ancho de 5 caracteres |
| `%-32s` | Texto alineado a la izquierda en un ancho de 32 caracteres |
| `\n` | Salto de línea |

## Longitud del SSID

Un nombre de red Wi-Fi, o SSID, puede tener hasta 32 caracteres. Por eso el proyecto de escaneo usa una columna de 32 caracteres para mostrarlo de forma ordenada.

## Seguridad Wi-Fi

La función `WiFi.encryptionType(i)` permite consultar el tipo de seguridad de una red encontrada durante el escaneo.

En el proyecto inicial se muestra una versión simple:

- `Abierta`
- `Protegida`

Más adelante puede ampliarse para mostrar tipos más específicos como WPA, WPA2 o WPA3, según el soporte disponible en la versión del core ESP32 usada.
