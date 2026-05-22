#include "setup.h"

void sys_tick_handler(void) {
  clock_tick();
}

int main(void) {
  setup();

  while (true) {
    gpio_toggle(GPIOC, GPIO13);
    delay(500);
  }
}