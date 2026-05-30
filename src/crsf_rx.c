#include "crsf_rx.h"

#define CRSF_SYNC_BYTE 0xC8
#define CRSF_MAX_LEN 64
#define CRSF_MAX_FRAME_SIZE 66

#define CRSF_TYPE_RC_CHANNELS 0x16
#define CRSF_TYPE_LINK_STATS 0x14

#define CRSF_RC_PAYLOAD_LEN 22
#define CRSF_LINK_PAYLOAD_LEN 10

typedef enum {
  STATE_SYNC,
  STATE_LEN,
  STATE_DATA,
} crsf_feed_state_t;

static crsf_feed_state_t feed_state = STATE_SYNC;
static uint8_t data_buffer[CRSF_MAX_FRAME_SIZE];
static uint8_t data_index;
static uint8_t data_length;

static volatile crsf_data_t crsf_data;
static volatile uint32_t last_data_ms;

static void parse_channels(const uint8_t *payload) {
  crsf_data.channels[0] = ((uint16_t)(payload[0] | ((uint16_t)payload[1] << 8))) & 0x07FF;
  crsf_data.channels[1] = ((uint16_t)(payload[1] >> 3 | ((uint16_t)payload[2] << 5))) & 0x07FF;
  crsf_data.channels[2] = ((uint16_t)(payload[2] >> 6 | ((uint16_t)payload[3] << 2) | ((uint16_t)payload[4] << 10))) & 0x07FF;
  crsf_data.channels[3] = ((uint16_t)(payload[4] >> 1 | ((uint16_t)payload[5] << 7))) & 0x07FF;
  crsf_data.channels[4] = ((uint16_t)(payload[5] >> 4 | ((uint16_t)payload[6] << 4))) & 0x07FF;
  crsf_data.channels[5] = ((uint16_t)(payload[6] >> 7 | ((uint16_t)payload[7] << 1) | ((uint16_t)payload[8] << 9))) & 0x07FF;
  crsf_data.channels[6] = ((uint16_t)(payload[8] >> 2 | ((uint16_t)payload[9] << 6))) & 0x07FF;
  crsf_data.channels[7] = ((uint16_t)(payload[9] >> 5 | ((uint16_t)payload[10] << 3))) & 0x07FF;
  crsf_data.channels[8] = ((uint16_t)(payload[11] | ((uint16_t)payload[12] << 8))) & 0x07FF;
  crsf_data.channels[9] = ((uint16_t)(payload[12] >> 3 | ((uint16_t)payload[13] << 5))) & 0x07FF;
  crsf_data.channels[10] = ((uint16_t)(payload[13] >> 6 | ((uint16_t)payload[14] << 2) | ((uint16_t)payload[15] << 10))) & 0x07FF;
  crsf_data.channels[11] = ((uint16_t)(payload[15] >> 1 | ((uint16_t)payload[16] << 7))) & 0x07FF;
  crsf_data.channels[12] = ((uint16_t)(payload[16] >> 4 | ((uint16_t)payload[17] << 4))) & 0x07FF;
  crsf_data.channels[13] = ((uint16_t)(payload[17] >> 7 | ((uint16_t)payload[18] << 1) | ((uint16_t)payload[19] << 9))) & 0x07FF;
  crsf_data.channels[14] = ((uint16_t)(payload[19] >> 2 | ((uint16_t)payload[20] << 6))) & 0x07FF;
  crsf_data.channels[15] = ((uint16_t)(payload[20] >> 5 | ((uint16_t)payload[21] << 3))) & 0x07FF;
}

static void parse_stats(const uint8_t *payload) {
  crsf_data.signal_strength = -(int8_t)payload[0];
  crsf_data.link_quality = payload[2];
  crsf_data.signal_to_noise = (int8_t)payload[3];
}

static void process_data(void) {
  uint8_t type = data_buffer[2];
  uint8_t payload_length = data_length - 2;

  uint8_t calc_crc = crsf_crc(&data_buffer[2], data_length - 1);
  uint8_t data_crc = data_buffer[2 + data_length - 1];

  if (calc_crc != data_crc) {
    return;
  }

  const uint8_t *payload = &data_buffer[3];

  switch (type) {
    case CRSF_TYPE_RC_CHANNELS:
      if (payload_length >= CRSF_RC_PAYLOAD_LEN) {
        parse_channels(payload);
        crsf_data.failsafe = false;
        last_data_ms = get_clock_ticks();
      }
      break;
    case CRSF_TYPE_LINK_STATS:
      if (payload_length >= CRSF_LINK_PAYLOAD_LEN) {
        parse_stats(payload);
      }
      break;
  }
}

void crsf_init(void) {
  crsf_data.failsafe = true;
  crsf_data.link_quality = 0;
  crsf_data.signal_strength = -100;
  crsf_data.signal_to_noise = 0;
  last_data_ms = get_clock_ticks();
}

void parser_data_feed(uint8_t byte) {
  switch (feed_state) {
    case STATE_SYNC:
      if (byte == CRSF_SYNC_BYTE) {
        data_buffer[0] = byte;
        feed_state = STATE_LEN;
      }
      break;
    case STATE_LEN:
      if (byte >= 2 && byte <= CRSF_MAX_LEN) {
        data_buffer[1] = byte;
        data_length = byte;
        data_index = 2;
        feed_state = STATE_DATA;
      } else {
        feed_state = STATE_SYNC;
      }
      break;
    case STATE_DATA:
      data_buffer[data_index++] = byte;
      if (data_index >= 2 + data_length) {
        process_data();
        feed_state = STATE_SYNC;
      }
      break;
  }
}

const crsf_data_t *crsf_get_data(void) {
  if ((get_clock_ticks() - last_data_ms) > CRSF_FAILSAFE_TIMEOUT_MS) {
    crsf_data.failsafe = true;
  }
  return (const crsf_data_t *)&crsf_data;
}