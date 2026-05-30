#include "setup.h"

static void setup_clock(void) {
  rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);

  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOC);

  rcc_periph_clock_enable(RCC_SYSCFG);

  rcc_periph_clock_enable(RCC_USART1);
  rcc_periph_clock_enable(RCC_USART6);

  dwt_enable_cycle_counter();
}

static void setup_systick(void) {
  systick_set_frequency(SYSTICK_FREQUENCY_HZ, SYSCLK_FREQUENCY_HZ);
  systick_counter_enable();
  systick_interrupt_enable();
}

static void setup_timer_priorities(void) {
  nvic_set_priority(NVIC_SYSTICK_IRQ, 16 * 1);
  nvic_set_priority(NVIC_USART1_IRQ, 16 * 2);
  nvic_set_priority(NVIC_USART6_IRQ, 16 * 4);

  nvic_enable_irq(NVIC_USART1_IRQ);
  nvic_enable_irq(NVIC_USART6_IRQ);
}

static void setup_usart(void) {
  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART1, USART_MODE_TX_RX);
  usart_enable(USART1);
}

static void setup_usart_crsf(void) {
  usart_set_baudrate(USART6, 420000);
  usart_set_databits(USART6, 8);
  usart_set_stopbits(USART6, USART_STOPBITS_1);
  usart_set_parity(USART6, USART_PARITY_NONE);
  usart_set_flow_control(USART6, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART6, USART_MODE_TX_RX);
  usart_enable_rx_interrupt(USART6);
  usart_enable(USART6);

  crsf_init();
}

void usart6_isr(void) {
  uint32_t status = USART_SR(USART6);
  uint8_t data = (uint8_t)usart_recv(USART6);

  if (status & USART_SR_RXNE) {
    parser_data_feed(data);
  }
}

static void setup_gpio(void) {
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
  gpio_set(GPIOC, GPIO13);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);

  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO11 | GPIO12);
  gpio_set_af(GPIOA, GPIO_AF8, GPIO11 | GPIO12);
}

void setup(void) {
  setup_clock();
  setup_systick();
  setup_timer_priorities();
  setup_gpio();
  setup_usart();
  setup_usart_crsf();
}