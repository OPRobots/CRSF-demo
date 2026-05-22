# CRSF-demo

Demo de recepción y decodificación del protocolo **CRSF** (CrossFire Serial Protocol) con un receptor **BETAFPV ELRS Lite** y un microcontrolador **STM32F401CC** (BlackPill), usando **LibOpenCM3** y **PlatformIO**.

Decodifica los 16 canales RC y las estadísticas de enlace, detecta pérdida de señal (**failsafe**) e imprime los valores por la consola serie vía USART1.

---

## Hardware

| Señal     | STM32F401CC | Descripción                                    |
| --------- | ----------- | ---------------------------------------------- |
| USART6 RX | PA12 (AF8)  | ← TX del receptor ELRS (CRSF 420 000 bps)      |
| USART6 TX | PA11 (AF8)  | → RX del receptor ELRS (no usado en esta demo) |
| USART1 TX | PA9 (AF7)   | → Consola serie PC (115 200 bps)               |
| USART1 RX | PA10 (AF7)  | ← Consola serie PC (opcional)                  |
| LED       | PC13        | Parpadeo cada 200 ms (señal de vida)           |

USART6 configurado en **modo solo RX**, 8N1, sin control de flujo. Con APB2 = 84 MHz el divisor de baudrate es exactamente 200 → error 0%.

## Protocolo CRSF

El receptor ELRS serializa los canales RC sobre UART usando el protocolo **CRSF** (CrossFire Serial Protocol):

```
[SYNC 0xC8][LEN][TYPE][PAYLOAD…][CRC-8 DVB-S2]
```

| TYPE   | Payload  | Contenido                                               |
| ------ | -------- | ------------------------------------------------------- |
| `0x16` | 22 bytes | 16 canales RC × 11 bits, little-endian (rango 172–1811) |
| `0x14` | 10 bytes | RSSI, Link Quality (%), SNR, modo RF, potencia TX       |

El CRC-8 (polinomio `0xD5`) se calcula sobre `TYPE + PAYLOAD`. La trama RC llega a la misma frecuencia que la tasa RF del enlace ELRS (configurable: 50–500 Hz).

## Software

### `crsf_data_t`

```c
typedef struct {
    uint16_t channels[16];  // Canales RC (172 = ~1000 µs, 992 = ~1500 µs, 1811 = ~2000 µs)
    uint8_t link_quality;   // Calidad de enlace 0–100 %
    int8_t signal_strength; // RSSI en dBm (valor negativo)
    int8_t signal_to_noise; // SNR en dB
    bool failsafe;          // true si no se recibe trama RC en > 500 ms
} crsf_data_t;
```

### API (`crsf.h`)

```c
void crsf_init(void);
void parser_data_feed(uint8_t byte);
const crsf_data_t *crsf_get_data(void);
```

| Función                  | Descripción                                                                     |
| ------------------------ | ------------------------------------------------------------------------------- |
| `crsf_init()`            | Inicializa el estado interno del parser; llamada desde `setup_usart_crsf()`     |
| `parser_data_feed(byte)` | Alimenta la máquina de estados byte a byte desde la ISR de USART6               |
| `crsf_get_data()`        | Devuelve puntero a los últimos datos decodificados; evalúa failsafe por timeout |

> [!WARNING]
> `crsf_get_data()` **no es segura desde una ISR** porque puede leer el struct `crsf_data_t` mientras la ISR de USART6 lo está actualizando. Llámala siempre desde el bucle principal o protege la lectura deshabilitando `NVIC_USART6_IRQ` brevemente.

### Inicialización en `setup.c`

```c
void setup_usart_crsf(void) {
    usart_set_baudrate(USART6, 420000);
    usart_set_mode(USART6, USART_MODE_RX);
    usart_enable_rx_interrupt(USART6);
    usart_enable(USART6);
    crsf_init();
}

void usart6_isr(void) {
    uint32_t sr  = USART_SR(USART6);
    uint8_t  dr  = (uint8_t)usart_recv(USART6);
    if (sr & USART_SR_RXNE) parser_data_feed(dr);
}
```

### Prioridades NVIC

| IRQ     | Prioridad    |
| ------- | ------------ |
| SysTick | 1 (más alta) |
| USART1  | 2            |
| USART6  | 4            |

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
