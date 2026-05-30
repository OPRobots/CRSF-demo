#ifndef __CRSF_TX_H
#define __CRSF_TX_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <libopencm3/stm32/usart.h>

#include "delay.h"
#include "utils.h"

#define CRSF_TELEMETRY_FRAME_INTERVAL_MS 50
#define CRSF_TELEMETRY_FRAME_COUNT 5

void crsf_telemetry_set_battery(uint16_t voltage_mv, uint16_t current_ma, uint32_t capacity_mah, uint8_t remaining_pct);
void crsf_telemetry_set_attitude(int16_t pitch, int16_t roll, int16_t yaw);
void crsf_telemetry_set_flight_mode(const char *mode_str);
void crsf_telemetry_set_baro(int32_t altitude_cm, int16_t vspeed_cms);
void crsf_telemetry_set_gps(int32_t lat_deg7, int32_t lon_deg7, uint16_t speed_kph10, uint16_t heading_deg100, uint16_t altitude_m, uint8_t sats);
void crsf_telemetry_update(void);

#endif
