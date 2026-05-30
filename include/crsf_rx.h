#ifndef __CRSF_RX_H
#define __CRSF_RX_H

#include <stdbool.h>
#include <stdint.h>

#include <libopencm3/stm32/usart.h>

#include "delay.h"
#include "utils.h"

#define CRSF_NUM_CHANNELS 16

#define CRSF_CHANNEL_MIN 172  // ~1000 µs
#define CRSF_CHANNEL_MID 992  // ~1500 µs
#define CRSF_CHANNEL_MAX 1811 // ~2000 µs

#define CRSF_FAILSAFE_TIMEOUT_MS 500

typedef struct {
  uint16_t channels[CRSF_NUM_CHANNELS];
  uint8_t link_quality;
  int8_t signal_strength;
  int8_t signal_to_noise;
  bool failsafe;
} crsf_data_t;

void crsf_init(void);
void parser_data_feed(uint8_t byte);
const crsf_data_t *crsf_get_data(void);

#endif
