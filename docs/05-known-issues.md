# Problemas Conocidos

> **Fecha de análisis**: 2026-06-27
> **Última actualización**: 2026-06-27
> **Total**: 6 issues — 0 críticos, 3 moderados, 3 leves

---

## Correcciones Recientes ✅

| ID | Fecha | Descripción |
|----|-------|-------------|
| — | — | — |

---

## 🔴 Críticos

*No se han encontrado issues críticos.*

---

## 🟡 Moderados

#### SW-01 — `delay()` vulnerable a wraparound de `clock_ticks`
- **Archivo**: [`delay.c:17-22`](../src/delay.c#L17-L22)
- **Descripción**: La comparación `while (awake > clock_ticks)` falla cuando
  `clock_ticks` desborda `UINT32_MAX` (~49.7 días a 1 kHz). En el instante del
  wraparound, `awake` (calculado antes del desborde, valor ~UINT32_MAX) será
  mayor que `clock_ticks` (~0), por lo que el delay retorna inmediatamente en
  lugar de esperar. Esto puede causar que una tarea periódica se ejecute a
  máxima velocidad durante un ciclo tras ~49 días de uptime.
- **Impacto**: Medio — solo relevante en operación continua prolongada.
  La mayoría de robots no superan 1 día de uptime.
- **Mitigación actual**: Ninguna.
- **Solución propuesta**: Usar resta con signo para manejar wraparound:
  `while ((int32_t)(awake - clock_ticks) > 0) {}`

#### SW-02 — Race condition ISR↔main en `crsf_get_data()`
- **Archivo**: [`crsf_rx.c:117-121`](../src/crsf_rx.c#L117-L121),
  [`main.c:34-47`](../src/main.c#L34-L47)
- **Descripción**: `crsf_get_data()` retorna un puntero a `crsf_data_t`
  (estructura volátil de ~40 bytes modificada por la ISR de USART6). El bucle
  principal desreferencia el puntero para leer `failsafe`, `channels[0..15]`,
  `link_quality` y `signal_strength` en accesos individuales. Entre dos accesos,
  la ISR puede actualizar la estructura con datos de una nueva trama. Esto puede
  causar: canales de tramas distintas mezclados en la misma línea impresa, o
  inconsistencia entre `failsafe` y los canales mostrados.
- **Impacto**: Medio — en una demo de consola la integridad de datos no es
  crítica, pero si el código se reutiliza como base para control de vuelo real,
  la inconsistencia entre canales podría causar comandos de actuador corruptos.
- **Mitigación actual**: Ninguna. El README advierte que `crsf_get_data()` no es
  segura desde ISR, pero no menciona la race condition en el bucle principal.
- **Solución propuesta**: Hacer una copia atómica de la estructura completa
  deshabilitando brevemente `NVIC_USART6_IRQ` durante la copia, o usar un
  mecanismo de doble buffer con flag de actualización.

#### SW-03 — `delay_us()` usa aritmética de punto flotante innecesaria
- **Archivo**: [`delay.c:28-34`](../src/delay.c#L28-L34)
- **Descripción**: `delay_us()` calcula `sleep_cycles` con `(float)us` y
  `(float)MICROSECONDS_PER_SECOND`, forzando una conversión a FP y división
  flotante. En un Cortex-M4 con FPU el coste es moderado, pero la operación
  es innecesaria — puede resolverse con aritmética entera.
- **Impacto**: Medio — la división flotante consume ~14 ciclos en FPU del M4.
  Si `delay_us()` se usa desde ISR, añade latencia innecesaria. Sin FPU
  (compilación soft-float), el impacto sería mucho mayor por emulación software.
- **Mitigación actual**: Ninguna.
- **Solución propuesta**: Reemplazar con aritmética entera:
  `uint32_t sleep_cycles = (uint32_t)(((uint64_t)SYSCLK_FREQUENCY_HZ * us) / MICROSECONDS_PER_SECOND);`

---

## 🟢 Leves

#### CM-01 — Parser CRSF sin timeout en estado STATE_DATA
- **Archivo**: [`crsf_rx.c:107-113`](../src/crsf_rx.c#L107-L113)
- **Descripción**: Si ruido en la línea genera un byte `0xC8` seguido de una
  longitud aparentemente válida (2–64), el parser entra en STATE_DATA y espera
  indefinidamente a que lleguen exactamente `data_length + 2` bytes. No existe
  un timeout que fuerce el retorno a STATE_SYNC. En un entorno con ruido
  electromagnético (motores, ESCs), esto puede causar que el parser quede
  atascado hasta que llegue otro `0xC8` por casualidad.
- **Impacto**: Bajo — en la práctica, el stream de bytes a 420k bps es denso y
  cualquier secuencia de bytes completa el frame y falla el CRC, forzando el
  retorno a SYNC. La ventana de atascamiento es pequeña.
- **Mitigación actual**: Ninguna explícita. El CRC actúa como protección
  secundaria.
- **Solución propuesta**: Añadir un contador de bytes que fuerce SYNC si el
  frame excede `CRSF_MAX_FRAME_SIZE` bytes sin completarse.

#### DB-01 — Inconsistencia en terminaciones `\r\n` de la consola
- **Archivo**: [`main.c:37`](../src/main.c#L37) vs
  [`main.c:39-42`](../src/main.c#L39-L42),
  [`usart.c:18-19`](../src/usart.c#L18-L19)
- **Descripción**: La línea de failsafe usa `\r\n` mientras que la línea de
  canales usa solo `\n`. Simultáneamente, el código que insertaba `\r`
  automáticamente en `_write()` está comentado. En terminales que no expanden
  `\n` a `\r\n`, la salida de canales produce stair-step (cada línea empieza
  donde terminó la anterior en lugar de al principio).
- **Impacto**: Bajo — la mayoría de terminales modernas (screen, minicom, PuTTY)
  convierten `\n` a `\r\n` automáticamente.
- **Mitigación actual**: Ninguna.
- **Solución propuesta**: Unificar el criterio: usar `\r\n` en todos los
  `printf`, o descomentar la inserción automática de `\r` en `_write()`.

#### SW-04 — Include guards con doble underscore (identificadores reservados)
- **Archivo**: [`config.h:1`](../include/config.h#L1),
  [`setup.h:1`](../include/setup.h#L1),
  [`crsf_rx.h:1`](../include/crsf_rx.h#L1),
  [`crsf_tx.h:1`](../include/crsf_tx.h#L1),
  [`delay.h:1`](../include/delay.h#L1),
  [`usart.h:1`](../include/usart.h#L1)
- **Descripción**: Según el estándar C (7.1.3), los identificadores que
  comienzan con doble underscore (`__`) están reservados para la implementación
  del compilador. Usarlos en código de aplicación es comportamiento indefinido.
  En la práctica, ningún compilador ARM usa `__CONFIG_H` como macro interna,
  pero es una violación técnica del estándar. `utils.h` sí usa `UTILS_H`
  correctamente.
- **Impacto**: Bajo — riesgo práctico casi nulo con GCC/ARM. Solo relevante
  para certificación (MISRA C:2012 Dir 4.10).
- **Mitigación actual**: Ninguna.
- **Solución propuesta**: Renombrar a `CONFIG_H`, `SETUP_H`, etc. sin
  underscores iniciales.

---
*Documento generado el 2026-06-27. Los issues se actualizan automáticamente mediante `/doc-review` al auditar el código fuente.*
