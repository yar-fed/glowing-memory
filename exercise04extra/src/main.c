#include <stddef.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "../libopencm3-drivers/include/keys.h"


static struct keys_s key = {.port = GPIOA,
			    .gpio = GPIO0,
			    .pup = false,
			    .nc = true};

static const uint32_t led_switch_period = 250;
static uint32_t led_switch_ms;
static uint32_t button_press_ms = UINT32_MAX;
static uint32_t led_pins[] = {GPIO12, GPIO13, GPIO14, GPIO15};

// Variable that used for counting ms
static volatile uint32_t tick_ms_count;

void gpio_setup()
{
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
				GPIO12 | GPIO13 | GPIO14 | GPIO15);

	// gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO0);
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

int _inc(int i)
{
	return i + 1;
}

int _dec(int i)
{
	return i + 4 - 1;
}


static int (*counter_func)(int) = _inc;

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	gpio_setup();
	systick_setup();
	keys_setup(&key, 1);

	int i = 0;

	gpio_set(GPIOD, GPIO12);
	led_switch_ms = tick_ms_count;
	while (1) {
		if (key_pressed(&key, 0)) {
			if (button_press_ms == UINT32_MAX) {
				button_press_ms = tick_ms_count;
			} else if (tick_ms_count - button_press_ms
				   > led_switch_period) {
				if (counter_func != _inc)
					counter_func = _inc;
				else
					counter_func = _dec;
				button_press_ms = UINT32_MAX;
			}
		} else {
			button_press_ms = UINT32_MAX;
		}

		if ((tick_ms_count - led_switch_ms) > led_switch_period) {
			gpio_clear(GPIOD, led_pins[i]);
			i = counter_func(i) % 4;
			gpio_set(GPIOD, led_pins[i]);
			led_switch_ms = tick_ms_count;
		}
	}

	return 0;
}

