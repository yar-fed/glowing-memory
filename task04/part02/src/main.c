#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f4/nvic.h>
#include "../libopencm3-drivers/include/keys.h"
#include "../libopencm3-drivers/include/hd44780.h"

#define STRING_SHIFT_PERIOD 250

static struct hd44780_bus lcd_bus = {
	.rs = {GPIOE, GPIO7},
	.e = {GPIOE, GPIO11},
	.rnw = {GPIOE, GPIO10},
	.db7 = {GPIOE, GPIO15},
	.db6 = {GPIOE, GPIO14},
	.db5 = {GPIOE, GPIO13},
	.db4 = {GPIOE, GPIO12},
};

static struct keys_s key = {.port = GPIOA,
			    .gpio = GPIO0,
			    .pup = false,
			    .nc = true};

static uint32_t led_pins[] = {GPIO12, GPIO13, GPIO14, GPIO15};

static volatile uint32_t reading_user_key = false;
static volatile uint32_t user_key_down = false;
static volatile uint32_t user_key_handled = false;

static volatile uint32_t tick_ms_count;

void sleep_ms(uint32_t ms)
{
	uint32_t current_ms = tick_ms_count;
	while ((tick_ms_count - current_ms) < ms)
		;
}

static void gpio_setup()
{
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOE);

	gpio_clear(GPIOD, GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
			GPIO12 | GPIO13 | GPIO14 | GPIO15);

	gpio_set_output_options(GPIOD, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ,
				GPIO12 | GPIO13 | GPIO14 | GPIO15);
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
	nvic_enable_irq(NVIC_TIM1_TRG_COM_TIM11_IRQ);
	nvic_set_priority(NVIC_TIM1_TRG_COM_TIM11_IRQ, 0);
	nvic_enable_irq(NVIC_TIM1_UP_TIM10_IRQ);
	nvic_set_priority(NVIC_TIM1_UP_TIM10_IRQ, 1);
}

static void timer11_setup(void)
{
	rcc_periph_clock_enable(RCC_TIM11);
	timer_set_mode(TIM11, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_counter(TIM11, 1);
	timer_set_prescaler(TIM11, 21000);
	timer_disable_preload(TIM11);
	timer_set_period(TIM11, STRING_SHIFT_PERIOD);
	timer_enable_irq(TIM11, TIM_DIER_UIE);
	timer_enable_counter(TIM11);
}

static void timer10_setup(void)
{
	rcc_periph_clock_enable(RCC_TIM10);
	rcc_set_ppre2(RCC_CFGR_PPRE_DIV_16);
	timer_set_mode(TIM10, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_counter(TIM10, 1);
	timer_set_prescaler(TIM10, 21000);
	timer_disable_preload(TIM10);
	timer_set_period(TIM10, 200);
	timer_enable_irq(TIM10, TIM_DIER_UIE);
	timer_enable_counter(TIM10);
}

void sys_tick_handler(void)
{
	tick_ms_count++;
}

static bool shift_string = false;
static char full_string[] = "GlobalLogic. Embedded Starter Kit.  ";
static char *sp = full_string;

static void fill_lcd_lower_row(char *sp, int n, ...)
{
	va_list arg_list;
	uint8_t _len = strlen(sp);

	if (_len < 16) {
		hd44780_printf_xy(0, 1, "%s", sp);
	} else {
		hd44780_printf_xy(0, 1, "%.16s", sp);
		return;
	}

	va_start(arg_list, n);
	for (int i = 0; i < n; i++) {
		sp = va_arg(arg_list, char*);
		_len += strlen(sp);
		if (_len < 16) {
			hd44780_printf("%s", sp);
		} else {
			hd44780_printf("%.*s", strlen(sp) - _len + 16, sp);
			return;
		}
	}
	va_end(arg_list);
}

void tim1_trg_com_tim11_isr(void)
{
	if (timer_get_flag(TIM11, TIM_SR_UIF)) {
		timer_clear_flag(TIM11, TIM_SR_UIF);
		shift_string = true;
		if (*(++sp) == 0)
			sp = full_string;
	}
}

void tim1_up_tim10_isr(void)
{
	if (timer_get_flag(TIM10, TIM_SR_UIF)) {
		timer_clear_flag(TIM10, TIM_SR_UIF);
		if (reading_user_key && user_key_down) {
			reading_user_key = false;
			user_key_handled = true;
			shift_string = true;
			sp = full_string;
			timer_disable_counter(TIM11);
			gpio_set(GPIOD, GPIO14);
		} else if (reading_user_key) {
			if (key_pressed(&key, 0)) {
				user_key_down = true;
			}
			reading_user_key = false;
		} else if (user_key_down) {
			reading_user_key = true;
		}
	}
}

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	systick_setup();
	gpio_setup();
	nvic_setup();
	timer10_setup();
	hd44780_init(&lcd_bus, 16, false, 2, false);
	keys_setup(&key, 1);
	gpio_clear(GPIOE, GPIO9);
	hd44780_clear();
	timer11_setup();

	while (1) {
		if (key_pressed(&key, 0)) {
			gpio_set(GPIOD, GPIO12);
			if (!reading_user_key && !user_key_down) {
				reading_user_key = true;
				timer_set_counter(TIM10, 0);
			}
		} else if (user_key_down) {
			user_key_down = false;
			if (!user_key_handled) {
				gpio_toggle(GPIOD, GPIO14);
				if (TIM_CR1(TIM11) & TIM_CR1_CEN)
					timer_disable_counter(TIM11);
				else
					timer_enable_counter(TIM11);
			} else {
				user_key_handled = false;
			}
		} else {
			gpio_clear(GPIOD, GPIO12);
		}
		if (shift_string) {
			size_t sp_len;
			if ((sp_len = strlen(sp)) < 16) {
				fill_lcd_lower_row(sp, 1, full_string);
			} else {
				fill_lcd_lower_row(sp, 0);
			}
			shift_string = false;
		}
	}

	return 0;
}
