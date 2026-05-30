# CRSF-demo

Demo de recepción y decodificación del protocolo **CRSF** (CrossFire Serial Protocol) con un receptor **BETAFPV ELRS Lite** y un microcontrolador **STM32F401CC** (BlackPill), usando **LibOpenCM3** y **PlatformIO**.

Decodifica los 16 canales RC y las estadísticas de enlace, detecta pérdida de señal (**failsafe**) e imprime los valores por la consola serie vía USART1.

---

## Hardware

| Señal     | STM32F401CC | Descripción                               |
| --------- | ----------- | ----------------------------------------- |
| USART6 RX | PA12 (AF8)  | ← TX del receptor ELRS (CRSF 420 000 bps) |
| USART6 TX | PA11 (AF8)  | → RX del receptor ELRS (telemetría)       |
| USART1 TX | PA9 (AF7)   | → Consola serie PC (115 200 bps)          |
| USART1 RX | PA10 (AF7)  | ← Consola serie PC (opcional)             |
| LED       | PC13        | Parpadeo cada 200 ms (señal de vida)     |

USART6 configurado en modo TX_RX bidireccional, 8N1, sin control de flujo. Con APB2 = 84 MHz el divisor de baudrate es exactamente 200 → error 0%.

## Protocolo CRSF

El receptor ELRS serializa los canales RC y recibe telemetría usando el protocolo **CRSF** (CrossFire Serial Protocol):

```
[SYNC 0xC8][LEN][TYPE][PAYLOAD…][CRC-8 DVB-S2]
```

| TYPE   | Dirección | Payload  | Contenido                                               |
| ------ | --------- | -------- | ------------------------------------------------------- |
| `0x16` | RX        | 22 bytes | 16 canales RC × 11 bits, little-endian (rango 172–1811) |
| `0x14` | RX        | 10 bytes | RSSI, Link Quality (%), SNR, modo RF, potencia TX       |
| `0x08` | TX        | 8 bytes  | Batería: tensión, corriente, capacidad, %               |
| `0x1E` | TX        | 6 bytes  | Actitud: pitch, roll, yaw (int16, 0,0001 rad)           |
| `0x21` | TX        | variable | Modo de vuelo (string ASCII + `\0`)                     |
| `0x09` | TX        | 4 bytes  | Altitud barométrica y velocidad vertical                |
| `0x02` | TX        | 15 bytes | Posición GPS                                            |

El CRC-8 (polinomio `0xD5`) se calcula sobre `TYPE + PAYLOAD`.

## Software

### RX — Recepción de canales

**`crsf_data_t`** (`crsf_rx.h`)

```c
typedef struct {
    uint16_t channels[16];  // 172 = ~1000 µs · 992 = ~1500 µs · 1811 = ~2000 µs
    uint8_t link_quality;   // 0–100 %
    int8_t signal_strength; // RSSI en dBm
    int8_t signal_to_noise; // SNR en dB
    bool failsafe;          // true si no llega trama RC en > 500 ms
} crsf_data_t;
```

**API** (`crsf_rx.h`)

| Función                  | Descripción                                               |
| ------------------------ | --------------------------------------------------------- |
| `crsf_init()`            | Inicializa el parser; llamada desde `setup_usart_crsf()`  |
| `parser_data_feed(byte)` | Alimenta la máquina de estados byte a byte desde la ISR   |
| `crsf_get_data()`        | Devuelve puntero a los últimos datos; evalúa el failsafe  |

> [!WARNING]
> `crsf_get_data()` no es segura desde una ISR. Llámala siempre desde el bucle principal o protege la lectura deshabilitando `NVIC_USART6_IRQ` brevemente.

### TX — Telemetría hacia la emisora

Cada tipo de dato tiene un setter independiente. Llamar a un setter marca ese slot como *dirty*; `crsf_telemetry_update()` envía un slot por llamada con un mínimo de 50 ms entre frames, respetando la capacidad del enlace.

**API** (`crsf_tx.h`)

| Función                          | Frame  | Sensores EdgeTX          | Parámetros clave                                                   |
| -------------------------------- | ------ | ------------------------ | ------------------------------------------------------------------ |
| `crsf_telemetry_set_battery(…)`  | `0x08` | RxBt, Curr, Capa, Bat%   | `voltage_mv`, `current_ma`, `capacity_mah`, `remaining_pct`        |
| `crsf_telemetry_set_attitude(…)` | `0x1E` | Ptch, Roll, Yaw          | int16 × 0,0001 rad; reutilizable para RPM, temp ESC, heading       |
| `crsf_telemetry_set_flight_mode(…)` | `0x21` | FM                   | string ASCII ≤ 15 chars                                            |
| `crsf_telemetry_set_baro(…)`     | `0x09` | Alt, VSpd                | `altitude_cm` (int32), `vspeed_cms` (int16)                        |
| `crsf_telemetry_set_gps(…)`      | `0x02` | GPS, GSpd, Hdg, Alt, Sat | lat/lon × 1e7, speed × 10 km/h, heading × 100°, alt MSL m, sats   |
| `crsf_telemetry_update()`        | —      | —                        | Llamar en el bucle principal; envía el siguiente slot dirty si toca |

Solo los setters que se llaman generan frames. Los slots que nunca se actualizan no consumen ancho de banda.

## Consola serie

El bucle principal imprime una línea cada 200 ms a 115 200 bps:

```
=== CRSF Demo (CRSF / USART6 PA12) ===
CH01: 992 CH02: 992 CH03: 172 CH04: 992 CH05: 992 CH06: 992 CH07: 992 CH08: 992 CH09: 992 CH10: 992 CH11: 992 CH12: 992 CH13: 992 CH14: 992 CH15: 992 CH16: 992 |LQ: 99%  RSSI: -65 dBm
```

Sin señal (failsafe activo):

```
[FAILSAFE] Sin señal del emisor
```
