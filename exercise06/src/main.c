#include <stddef.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f4/nvic.h>
#include "../libopencm3-drivers/include/keys.h"


static struct keys_s key = {.port = GPIOA,
			    .gpio = GPIO0,
			    .pup = false,
			    .nc = true};

static uint32_t led_pins[] = {GPIO12, GPIO13, GPIO14, GPIO15};

static volatile uint32_t reading_user_key = false;
static volatile uint32_t user_key_down = false;

static volatile uint32_t tick_ms_count;

static void gpio_setup()
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

static void nvic_setup(void)
{
	nvic_enable_irq(NVIC_TIM1_UP_TIM10_IRQ);
	nvic_set_priority(NVIC_TIM1_UP_TIM10_IRQ, 1);
}

static void timer_setup(void)
{
	rcc_set_ppre2(RCC_CFGR_PPRE_DIV_16);
	rcc_periph_clock_enable(RCC_TIM10);
	timer_set_mode(TIM10, TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_counter(TIM10, 1);
	timer_set_prescaler(TIM10, 21000);
	timer_disable_preload(TIM10);
	timer_set_period(TIM10, 100);
	timer_enable_irq(TIM10, TIM_DIER_UIE);
	timer_enable_counter(TIM10);
}

void sys_tick_handler(void)
{
	tick_ms_count++;
}

void tim1_up_tim10_isr(void)
{
	if (timer_get_flag(TIM10, TIM_SR_UIF)) {
		timer_clear_flag(TIM10, TIM_SR_UIF);
		if (reading_user_key) {
			if (key_pressed(&key, 0)) {
				user_key_down = true;
			}
			reading_user_key = false;
		}
	}
}

static void change_led_state(void)
{
	static int i = 0;

	i = (i + 1) % 5;
	if (i != 0) {
		gpio_set(GPIOD, led_pins[i - 1]);
	} else {
		int j;
		for (j = 0; j < 4; ++j)
			gpio_clear(GPIOD, led_pins[j]);
	}
}

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	systick_setup();
	gpio_setup();
	keys_setup(&key, 1);
	nvic_setup();
	timer_setup();

	while (1) {
		if (key_pressed(&key, 0)) {
			if (!reading_user_key && !user_key_down) {
				reading_user_key = true;
				timer_set_counter(TIM10, 0);
			}
		} else {
			if (user_key_down) {
				user_key_down = false;
				change_led_state();
			}
		}
	}

	return 0;
}
