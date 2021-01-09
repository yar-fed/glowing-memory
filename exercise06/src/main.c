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


int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	gpio_setup();
	systick_setup();
	keys_setup(&key, 1);

	int i = 0;
	int j;

	while (1) {
		if (key_pressed(&key, 0)) {
			i = (i + 1) % 5;
			if (i != 0)
				gpio_set(GPIOD, led_pins[i - 1]);
			else
				for (j = 0; j < sizeof(led_pins); ++j)
					gpio_clear(GPIOD, led_pins[j]);
		}
	}

	return 0;
}

