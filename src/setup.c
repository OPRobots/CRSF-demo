#include "setup.h"

/**
 * @brief Configura los relojes del sistema.
 *
 * SYSCLK: 84 MHz (HSE 25 MHz + PLL)
 * GPIO:   A, C
 * USART1: printf por PA9/PA10
 * SPI1:   SRAM 23AA04M por PA5/PA6/PA7
 * DWT:    contador de ciclos para delay_us
 */
static void setup_clock(void) {
  rcc_clock_setup_pll(&rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_84MHZ]);

  rcc_periph_clock_enable(RCC_GPIOA);
  rcc_periph_clock_enable(RCC_GPIOC);

  rcc_periph_clock_enable(RCC_SYSCFG);

  rcc_periph_clock_enable(RCC_USART1);

  dwt_enable_cycle_counter();
}

/**
 * @brief Configura el SysTick para interrupciones cada 1 ms.
 */
static void setup_systick(void) {
  systick_set_frequency(SYSTICK_FREQUENCY_HZ, SYSCLK_FREQUENCY_HZ);
  systick_counter_enable();
  systick_interrupt_enable();
}

static void setup_timer_priorities(void) {
  nvic_set_priority(NVIC_SYSTICK_IRQ, 16 * 1);
  nvic_set_priority(NVIC_USART1_IRQ, 16 * 2);

  nvic_enable_irq(NVIC_USART1_IRQ);
  // nvic_enable_irq(NVIC_EXTI15_10_IRQ);
}

static void setup_usart(void) {
  usart_set_baudrate(USART1, 115200);
  usart_set_databits(USART1, 8);
  usart_set_stopbits(USART1, USART_STOPBITS_1);
  usart_set_parity(USART1, USART_PARITY_NONE);
  usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
  usart_set_mode(USART1, USART_MODE_TX_RX);
  // USART_CR1(USART1) |= USART_CR1_RXNEIE;
  // usart_enable_tx_interrupt(USART1);
  usart_enable(USART1);
}

static void setup_gpio(void) {

  // LED
  gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
  gpio_set(GPIOC, GPIO13);

  // USART1
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO10);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO9 | GPIO10);
}

void setup(void) {
  setup_clock();
  setup_systick();
  setup_timer_priorities();
  setup_gpio();
  setup_usart();
}