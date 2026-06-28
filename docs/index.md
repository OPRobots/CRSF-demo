# CRSF-demo

Demo de recepción y decodificación del protocolo **CRSF** (CrossFire Serial
Protocol) con un receptor **BETAFPV ELRS Lite** y un microcontrolador
**STM32F401CC** (BlackPill), usando **LibOpenCM3** y **PlatformIO**.

Decodifica los 16 canales RC y las estadísticas de enlace, detecta pérdida de
señal (**failsafe**) e imprime los valores por la consola serie. Incluye
transmisión de telemetría hacia la emisora (batería, actitud, modo de vuelo,
barómetro y GPS).

---

## ⚙️ Hardware

| Característica | Detalle |
|---------------|---------|
| **Microcontrolador** | STM32F401CCU6 (BlackPill) @ 84 MHz |
| **Receptor** | BETAFPV ELRS Lite (CRSF 420 000 bps) |
| **Consola** | USART1 @ 115 200 bps |
| **Debug** | STLink SWD + LED heartbeat PC13 |
| **Framework** | LibOpenCM3 |

---

## 💻 Software

| Componente | Detalle |
|-----------|---------|
| **Lenguaje** | C11 |
| **Framework** | LibOpenCM3 |
| **Entorno** | PlatformIO |
| **Bucle principal** | 5 Hz (200 ms por iteración) |
| **Protocolo** | CRSF (CrossFire Serial Protocol) |
| **Canales RC** | 16 canales × 11 bits (172–1811) |
| **Telemetría** | 5 tipos de frame (batería, actitud, flight mode, baro, GPS) |
| **Failsafe** | Timeout 500 ms sin trama RC |
| **Debug** | Consola serie + LED heartbeat |

---

## 📚 Documentación

- [Hardware](01-hardware.md) — MCU, pines, reloj, alimentación, diagrama de bloques
- [Arquitectura Software](02-software-architecture.md) — Bucle principal, ISRs, prioridades, parser CRSF
- [Comunicaciones](03-communications.md) — Protocolo CRSF, recepción de canales, transmisión de telemetría
- [Debug](04-debug-system.md) — Consola serie, LED heartbeat, syscall `_write`
- [Problemas Conocidos](05-known-issues.md) — 6 issues documentados (0 críticos, 3 moderados, 3 leves)

---

## 🔧 Stack Tecnológico

| Componente | Tecnología |
|-----------|-----------|
| **MCU** | STM32F401CCU6 (ARM Cortex-M4) |
| **Framework** | LibOpenCM3 |
| **Build System** | PlatformIO |
| **Lenguaje** | C11 |
| **Debug Probe** | STLink v2 (SWD) |
| **Protocolo** | CRSF (ELRS) |

---

*Documento generado el 2026-06-27. Ver también [Hardware](01-hardware.md).*
