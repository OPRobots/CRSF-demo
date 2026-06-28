# CRSF-demo

Demo de recepción y decodificación del protocolo CRSF (CrossFire Serial
Protocol) con receptor BETAFPV ELRS Lite y STM32F401CC (BlackPill).

## Stack tecnológico
- **Microcontrolador**: STM32F401CCU6 (ARM Cortex-M4, 84 MHz)
- **Framework**: LibOpenCM3
- **Entorno**: PlatformIO
- **Lenguaje**: C11
- **Protocolo**: CRSF (CrossFire Serial Protocol) @ 420 000 bps

## Estructura del repositorio
- `src/` — firmware (main, setup, CRSF RX/TX, delay, USART, utils)
- `include/` — headers (config, setup, CRSF RX/TX, delay, USART, utils)
- `docs/` — documentación técnica (MkDocs Material)
- `platformio.ini` — configuración de build
- `openocd_reset.cfg` — configuración OpenOCD para STLink

## Convenciones de código
- Lenguaje: C11
- Formateo: clang-format (ColumnLimit: 0, IndentCaseLabels: true)
- Framework: LibOpenCM3 (bare-metal, sin HAL)
- ISRs: nomenclatura estándar de LibOpenCM3 (`usart6_isr`)
- Include guards: `__FILE_H` (ver SW-04 en known issues)

## Documentación
- Usa `/doc-init` para regenerar la documentación desde cero
- Usa `/doc-review` para revisar cambios incrementales y actualizar known issues
- La documentación sigue el estándar OPRobots (MkDocs Material + estilo ZoroBot3)
- Despliegue en GitHub Pages (docs.oprobots.org) vía monorepo OPRobots/docs

## Notas
- USART6 opera a 420 000 bps — velocidad no estándar; divisor exacto solo con
  APB2 = 84 MHz (divisor = 200)
- `crsf_get_data()` retorna puntero a datos volátiles modificados por ISR —
  no llamar desde otra ISR ni asumir atomicidad
- Los slots de telemetría no actualizados no consumen ancho de banda
- El failsafe se activa tras 500 ms sin trama RC válida
