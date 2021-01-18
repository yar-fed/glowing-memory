#include <stddef.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "../libopencm3-drivers/include/hd44780.h"

static struct hd44780_bus lcd_bus = {
	.rs = {GPIOE, GPIO7},
	.e = {GPIOE, GPIO11},
	.rnw = {GPIOE, GPIO10},
	.db7 = {GPIOE, GPIO15},
	.db6 = {GPIOE, GPIO14},
	.db5 = {GPIOE, GPIO13},
	.db4 = {GPIOE, GPIO12},
};

static volatile uint32_t tick_ms_count;

void sleep_ms(uint32_t ms)
{
	uint32_t current_ms = tick_ms_count;
	while ((tick_ms_count - current_ms) < ms);
}

static void gpio_setup()
{
	rcc_periph_clock_enable(RCC_GPIOE);

	gpio_clear(GPIOE, GPIO9);

	gpio_mode_setup(GPIOE,
			GPIO_MODE_OUTPUT,
			GPIO_PUPD_NONE,
			GPIO9);
}

static void systick_setup(void)
{
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(21000 - 1);
	systick_interrupt_enable();
	systick_counter_enable();
}

void sys_tick_handler(void)
{
	tick_ms_count++;
}

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	systick_setup();
	gpio_setup();
	hd44780_init(&lcd_bus, 16, false, 2, false);

	gpio_set(GPIOE, GPIO9);

	hd44780_putchar('Y');
	sleep_ms(100);
	hd44780_putchar('a');
	sleep_ms(100);
	hd44780_putchar('r');
	sleep_ms(100);
	hd44780_putchar('o');
	sleep_ms(100);
	hd44780_putchar('s');
	sleep_ms(100);
	hd44780_putchar('l');
	sleep_ms(100);
	hd44780_putchar('a');
	sleep_ms(100);
	hd44780_putchar('v');
	sleep_ms(200);
	hd44780_putchar_xy(0, 1, 'F');
	sleep_ms(100);
	hd44780_putchar('e');
	sleep_ms(100);
	hd44780_putchar('d');
	sleep_ms(100);
	hd44780_putchar('o');
	sleep_ms(100);
	hd44780_putchar('r');
	sleep_ms(100);
	hd44780_putchar('i');
	sleep_ms(100);
	hd44780_putchar('a');
	sleep_ms(100);
	hd44780_putchar('c');
	sleep_ms(100);
	hd44780_putchar('h');
	sleep_ms(100);
	hd44780_putchar('e');
	sleep_ms(100);
	hd44780_putchar('n');
	sleep_ms(100);
	hd44780_putchar('k');
	sleep_ms(100);
	hd44780_putchar('o');
	sleep_ms(100);
	while (1) {
	}

	return 0;
}
