#include "utils.h"

static uint8_t crc8_update(uint8_t crc, uint8_t data) {
  crc ^= data;
  for (int i = 0; i < 8; i++) {
    crc = (crc & 0x80) ? ((uint8_t)((crc << 1) ^ 0xD5)) : (uint8_t)(crc << 1);
  }
  return crc;
}

uint8_t crsf_crc(const uint8_t *buffer, uint8_t length) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < length; i++) {
    crc = crc8_update(crc, buffer[i]);
  }
  return crc;
}