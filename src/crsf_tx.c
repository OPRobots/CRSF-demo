#include "crsf_tx.h"

#define TELEMETRY_FRAME_BATTERY 0
#define TELEMETRY_FRAME_ATTITUDE 1
#define TELEMETRY_FRAME_FLIGHT_MODE 2
#define TELEMETRY_FRAME_BARO 3
#define TELEMETRY_FRAME_GPS 4

static uint8_t telemetry_index = 0;
static uint32_t telemetry_last_ms = 0;

static bool telemetry_dirty[CRSF_TELEMETRY_FRAME_COUNT];

static struct {
  uint16_t voltage_mv;
  uint16_t current_ma;
  uint32_t capacity_mah;
  uint8_t remaining_pct;
} telemetry_battery;

static struct {
  int16_t pitch;
  int16_t roll;
  int16_t yaw;
} telemetry_attitude;

static char telemetry_flight_mode[16];

static struct {
  int32_t altitude_cm;
  int16_t vspeed_cms;
} telemetry_barometer;

static struct {
  int32_t lat;
  int32_t lon;
  uint16_t speed;
  uint16_t heading;
  uint16_t altitude_m;
  uint8_t sats;
} telemetry_gps;

static void crsf_tx_frame(uint8_t type, const uint8_t *payload, uint8_t len) {
  uint8_t frame[34];
  frame[0] = 0xC8;
  frame[1] = (uint8_t)(len + 2);
  frame[2] = type;
  for (uint8_t i = 0; i < len; i++) {
    frame[3 + i] = payload[i];
  }
  frame[3 + len] = crsf_crc(&frame[2], (uint8_t)(1 + len));
  uint8_t total = (uint8_t)(4 + len);
  for (uint8_t i = 0; i < total; i++) {
    usart_send_blocking(USART6, frame[i]);
  }
}

static void crsf_send_battery(void) {
  uint8_t payload[8];
  uint16_t voltage = telemetry_battery.voltage_mv / 100U;
  uint16_t current = telemetry_battery.current_ma / 100U;
  uint32_t capacity = telemetry_battery.capacity_mah;
  payload[0] = (uint8_t)(voltage >> 8);
  payload[1] = (uint8_t)voltage;
  payload[2] = (uint8_t)(current >> 8);
  payload[3] = (uint8_t)current;
  payload[4] = (uint8_t)(capacity >> 16);
  payload[5] = (uint8_t)(capacity >> 8);
  payload[6] = (uint8_t)capacity;
  payload[7] = telemetry_battery.remaining_pct;
  crsf_tx_frame(0x08, payload, sizeof(payload));
}

static void crsf_send_attitude(void) {
  uint8_t payload[6];
  payload[0] = (uint8_t)((uint16_t)telemetry_attitude.pitch >> 8);
  payload[1] = (uint8_t)telemetry_attitude.pitch;
  payload[2] = (uint8_t)((uint16_t)telemetry_attitude.roll >> 8);
  payload[3] = (uint8_t)telemetry_attitude.roll;
  payload[4] = (uint8_t)((uint16_t)telemetry_attitude.yaw >> 8);
  payload[5] = (uint8_t)telemetry_attitude.yaw;
  crsf_tx_frame(0x1E, payload, sizeof(payload));
}

static void crsf_send_flight_mode(void) {
  uint8_t len = (uint8_t)(strlen(telemetry_flight_mode) + 1);
  crsf_tx_frame(0x21, (const uint8_t *)telemetry_flight_mode, len);
}

static void crsf_send_baro(void) {
  int16_t alt_dm = (int16_t)(telemetry_barometer.altitude_cm / 10) + 10000;
  uint8_t payload[4];
  payload[0] = (uint8_t)((uint16_t)alt_dm >> 8);
  payload[1] = (uint8_t)alt_dm;
  payload[2] = (uint8_t)((uint16_t)telemetry_barometer.vspeed_cms >> 8);
  payload[3] = (uint8_t)telemetry_barometer.vspeed_cms;
  crsf_tx_frame(0x09, payload, sizeof(payload));
}

static void crsf_send_gps(void) {
  uint8_t payload[15];
  payload[0] = (uint8_t)((uint32_t)telemetry_gps.lat >> 24);
  payload[1] = (uint8_t)((uint32_t)telemetry_gps.lat >> 16);
  payload[2] = (uint8_t)((uint32_t)telemetry_gps.lat >> 8);
  payload[3] = (uint8_t)(telemetry_gps.lat);
  payload[4] = (uint8_t)((uint32_t)telemetry_gps.lon >> 24);
  payload[5] = (uint8_t)((uint32_t)telemetry_gps.lon >> 16);
  payload[6] = (uint8_t)((uint32_t)telemetry_gps.lon >> 8);
  payload[7] = (uint8_t)(telemetry_gps.lon);
  payload[8] = (uint8_t)(telemetry_gps.speed >> 8);
  payload[9] = (uint8_t)telemetry_gps.speed;
  payload[10] = (uint8_t)(telemetry_gps.heading >> 8);
  payload[11] = (uint8_t)telemetry_gps.heading;
  uint16_t alt = telemetry_gps.altitude_m + 1000;
  payload[12] = (uint8_t)(alt >> 8);
  payload[13] = (uint8_t)alt;
  payload[14] = telemetry_gps.sats;
  crsf_tx_frame(0x02, payload, sizeof(payload));
}

void crsf_telemetry_set_battery(uint16_t voltage_mv, uint16_t current_ma, uint32_t capacity_mah, uint8_t remaining_pct) {
  telemetry_battery.voltage_mv = voltage_mv;
  telemetry_battery.current_ma = current_ma;
  telemetry_battery.capacity_mah = capacity_mah;
  telemetry_battery.remaining_pct = remaining_pct;
  telemetry_dirty[TELEMETRY_FRAME_BATTERY] = true;
}

void crsf_telemetry_set_attitude(int16_t pitch, int16_t roll, int16_t yaw) {
  telemetry_attitude.pitch = pitch;
  telemetry_attitude.roll = roll;
  telemetry_attitude.yaw = yaw;
  telemetry_dirty[TELEMETRY_FRAME_ATTITUDE] = true;
}

void crsf_telemetry_set_flight_mode(const char *mode_str) {
  strncpy(telemetry_flight_mode, mode_str, sizeof(telemetry_flight_mode) - 1);
  telemetry_flight_mode[sizeof(telemetry_flight_mode) - 1] = '\0';
  telemetry_dirty[TELEMETRY_FRAME_FLIGHT_MODE] = true;
}

void crsf_telemetry_set_baro(int32_t altitude_cm, int16_t vspeed_cms) {
  telemetry_barometer.altitude_cm = altitude_cm;
  telemetry_barometer.vspeed_cms = vspeed_cms;
  telemetry_dirty[TELEMETRY_FRAME_BARO] = true;
}

void crsf_telemetry_set_gps(int32_t lat_deg7, int32_t lon_deg7, uint16_t speed_kph10, uint16_t heading_deg100, uint16_t altitude_m, uint8_t sats) {
  telemetry_gps.lat = lat_deg7;
  telemetry_gps.lon = lon_deg7;
  telemetry_gps.speed = speed_kph10;
  telemetry_gps.heading = heading_deg100;
  telemetry_gps.altitude_m = altitude_m;
  telemetry_gps.sats = sats;
  telemetry_dirty[TELEMETRY_FRAME_GPS] = true;
}
void crsf_telemetry_update(void) {
  uint32_t now = get_clock_ticks();
  if ((now - telemetry_last_ms) < (uint32_t)CRSF_TELEMETRY_FRAME_INTERVAL_MS) {
    return;
  }

  uint8_t count = 0;
  while (!telemetry_dirty[telemetry_index]) {
    telemetry_index = (uint8_t)((telemetry_index + 1) % CRSF_TELEMETRY_FRAME_COUNT);
    count++;
    if (count >= CRSF_TELEMETRY_FRAME_COUNT) {
      return;
    }
  }

  telemetry_dirty[telemetry_index] = false;
  telemetry_last_ms = now;

  switch (telemetry_index) {
    case TELEMETRY_FRAME_BATTERY:
      crsf_send_battery();
      break;
    case TELEMETRY_FRAME_ATTITUDE:
      crsf_send_attitude();
      break;
    case TELEMETRY_FRAME_FLIGHT_MODE:
      crsf_send_flight_mode();
      break;
    case TELEMETRY_FRAME_BARO:
      crsf_send_baro();
      break;
    case TELEMETRY_FRAME_GPS:
      crsf_send_gps();
      break;
  }
  telemetry_index = (uint8_t)((telemetry_index + 1) % CRSF_TELEMETRY_FRAME_COUNT);
}
