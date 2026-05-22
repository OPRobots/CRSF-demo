#include <stdio.h>

#include "crsf.h"
#include "setup.h"

void sys_tick_handler(void) {
  clock_tick();
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

    delay(200);
  }
}