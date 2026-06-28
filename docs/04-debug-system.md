# Sistema de Debug

## LED de Heartbeat

| LED | Color | Significado |
|-----|-------|-------------|
| **PC13** | Verde (onboard) | Parpadeo cada 200 ms — firmware ejecutándose |

El LED commuta en cada iteración del bucle principal en
[`main.c:28`](../src/main.c#L28):
```c
gpio_toggle(GPIOC, GPIO13);
```

> **Nota**: El LED solo indica que el bucle principal está corriendo. No
> distingue entre estado normal y failsafe. Durante failsafe el LED sigue
> parpadeando.

## Consola Serie

| Parámetro | Valor |
|-----------|-------|
| **USART** | USART1 |
| **Pines** | TX: PA9 (AF7), RX: PA10 (AF7) |
| **Baudrate** | 115 200 bps |
| **Formato** | 8N1, sin control de flujo |

### Formato de Salida

Cada 200 ms se imprime una línea con los 16 canales RC y las estadísticas
de enlace:

```
=== CRSF Demo (CRSF / USART6 PA12) ===
CH01: 992 CH02: 992 CH03: 172 CH04: 992 CH05: 992 CH06: 992 CH07: 992 CH08: 992 CH09: 992 CH10: 992 CH11: 992 CH12: 992 CH13: 992 CH14: 992 CH15: 992 CH16: 992 |LQ: 99%  RSSI: -65 dBm
```

En ausencia de señal (failsafe):
```
[FAILSAFE] Sin señal del emisor
```

### Formato de Canales

| Campo | Formato | Ejemplo |
|-------|---------|---------|
| **CH01–CH16** | `%4d` (ancho fijo 4) | ` 992`, `1811` |
| **Link Quality** | `%3d%%` | ` 99%` |
| **RSSI** | `%4d dBm` | ` -65 dBm` |

Impresión en [`main.c:39-47`](../src/main.c#L39-L47).

## Syscall `_write`

La salida de `printf` se redirige a USART1 mediante la reimplementación de
`_write()` en [`usart.c:3-25`](../src/usart.c#L3-L25):

| Característica | Detalle |
|---------------|---------|
| **Método** | `usart_send_blocking(USART1, *ptr)` |
| **Buffer** | Ninguno (envío byte a byte) |
| **\r automático** | Comentado — el código espera `\r` explícito en el formato |
| **Null bytes** | Interrumpen la salida (el bucle comprueba `*ptr`) |

> **⚠️ Advertencia**: `usart_send_blocking` bloquea hasta que el byte se
> transmite. A 115 200 bps, cada byte tarda ~87 µs. Una línea completa
> (~200 bytes) bloquea el bucle principal durante ~17 ms. Si se añadieran
> más datos a imprimir, podría ser necesario pasar a transmisión por DMA o
> usar un buffer circular con interrupción TXE.

## OpenOCD

El proyecto incluye configuración para reset y upload vía STLink en
[`openocd_reset.cfg`](../openocd_reset.cfg). Se usa como flag de upload
en `platformio.ini`:
```ini
upload_flags = -f openocd_reset.cfg
```

---
*Documento generado el 2026-06-27. Ver también [Arquitectura Software](02-software-architecture.md), [Comunicaciones](03-communications.md).*
