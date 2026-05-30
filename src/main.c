#include <stdio.h>

#include "crsf_rx.h"
#include "crsf_tx.h"
#include "setup.h"

void sys_tick_handler(void) {
  clock_tick();
}

static void telemetry_demo(void) {
  uint32_t t = get_clock_ticks();

  uint32_t batt_t = t % 60000;
  crsf_telemetry_set_battery((uint16_t)(14800 - batt_t * 400 / 60000), (uint16_t)(10000 + (t % 4000) * 40000 / 4000), (t / 10000) * 3, (uint8_t)(100 - batt_t * 30 / 60000));

  crsf_telemetry_set_attitude((int16_t)(t % 8000), (int16_t)(2500 + (int32_t)((t % 20000) * 4000 / 20000)), (int16_t)((t % 10000) * 32000 / 10000));

  static const char *const states[] = {"IDLE", "ARMED", "SPINNING", "BRAKING"};
  crsf_telemetry_set_flight_mode(states[(t / 4000) % 4]);

  // crsf_telemetry_set_baro((int32_t)((t % 8000) * 200 / 8000), 25);
  // crsf_telemetry_set_gps(404238000L, -37120000L, 0, 0, 0, 0);
}

int main(void) {
  setup();

  printf("=== CRSF Demo (CRSF / USART6 PA12) ===\r\n");

  while (true) {
    gpio_toggle(GPIOC, GPIO13);

    const crsf_data_t *data = crsf_get_data();

    if (data->failsafe) {
      printf("[FAILSAFE] Sin señal del emisor\r\n");
    } else {
      printf(
          "CH01:%4d CH02:%4d CH03:%4d CH04:%4d CH05:%4d CH06:%4d CH07:%4d CH08:%4d "
          "CH09:%4d CH10:%4d CH11:%4d CH12:%4d CH13:%4d CH14:%4d CH15:%4d CH16:%4d |"
          "LQ:%3d%%  RSSI:%4d dBm\n",
          data->channels[0], data->channels[1], data->channels[2], data->channels[3],
          data->channels[4], data->channels[5], data->channels[6], data->channels[7],
          data->channels[8], data->channels[9], data->channels[10], data->channels[11],
          data->channels[12], data->channels[13], data->channels[14], data->channels[15],
          data->link_quality, data->signal_strength);
    }

    telemetry_demo();
    crsf_telemetry_update();
    delay(200);
  }
}