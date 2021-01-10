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

/* 8399 == 100%
 * 6299 == 75%
 * 4199 == 50%
 * 2099 == 25%
 */
static uint32_t led_brightness[] = {8399, 6299, 4199, 2099};
static uint32_t tim4_ocs[] = {TIM_OC1, TIM_OC2, TIM_OC3, TIM_OC4};

static volatile uint32_t reading_user_key = false;
static volatile uint32_t user_key_down = false;

static volatile uint32_t tick_ms_count;

static void gpio_setup()
{
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_mode_setup(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE,
			GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_100MHZ,
				GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_set_af(GPIOD, GPIO_AF2, GPIO12 | GPIO13 | GPIO14 | GPIO15);
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

static void timer10_setup(void)
{
	rcc_periph_clock_enable(RCC_TIM10);
	timer_set_mode(TIM10, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_counter(TIM10, 1);
	timer_set_prescaler(TIM10, 21000);
	timer_disable_preload(TIM10);
	timer_set_period(TIM10, 100);
	timer_enable_irq(TIM10, TIM_DIER_UIE);
	timer_enable_counter(TIM10);
}

static void timer4_setup(void)
{
	rcc_periph_clock_enable(RCC_TIM4);
	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM4, 0);
	timer_set_period(TIM4, 8399);
	timer_set_repetition_counter(TIM4, 0);

	timer_set_oc_mode(TIM4, TIM_OC4, TIM_OCM_PWM2);
	timer_set_oc_mode(TIM4, TIM_OC3, TIM_OCM_PWM2);
	timer_set_oc_mode(TIM4, TIM_OC2, TIM_OCM_PWM2);
	timer_set_oc_mode(TIM4, TIM_OC1, TIM_OCM_PWM2);

	timer_set_oc_polarity_low(TIM4, TIM_OC4);
	timer_set_oc_polarity_low(TIM4, TIM_OC3);
	timer_set_oc_polarity_low(TIM4, TIM_OC2);
	timer_set_oc_polarity_low(TIM4, TIM_OC1);

	timer_enable_oc_preload(TIM4, TIM_OC4);
	timer_enable_oc_preload(TIM4, TIM_OC3);
	timer_enable_oc_preload(TIM4, TIM_OC2);
	timer_enable_oc_preload(TIM4, TIM_OC1);

	timer_set_oc_value(TIM4, TIM_OC1, led_brightness[0]);
	timer_set_oc_value(TIM4, TIM_OC2, led_brightness[1]);
	timer_set_oc_value(TIM4, TIM_OC3, led_brightness[2]);
	timer_set_oc_value(TIM4, TIM_OC4, led_brightness[3]);

	timer_enable_oc_output(TIM4, TIM_OC1);
	timer_enable_oc_output(TIM4, TIM_OC2);
	timer_enable_oc_output(TIM4, TIM_OC3);
	timer_enable_oc_output(TIM4, TIM_OC4);

	timer_enable_counter(TIM4);
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
	static float i = 0.9;

	int j;
	for (j = 0; j < 4; ++j)
		timer_set_oc_value(TIM4, tim4_ocs[j], led_brightness[j] * i);
	i = (i <= 0) ? 1.0 : i - 0.1;
}

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	rcc_set_ppre2(RCC_CFGR_PPRE_DIV_16);
	systick_setup();
	gpio_setup();
	keys_setup(&key, 1);
	nvic_setup();
	timer10_setup();
	timer4_setup();

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
