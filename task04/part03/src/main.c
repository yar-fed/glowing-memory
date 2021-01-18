#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <tgmath.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/f4/nvic.h>
#include <libopencm3/stm32/f4/i2c.h>
#include "../libopencm3-drivers/include/keys.h"
#include "../libopencm3-drivers/include/hd44780.h"

#define PI 3.14159265

#define I2C_ACC_ADDR 0x6B

#define I2C_ACC_WHOAMI 0x0F

#define I2C_ACC_OUT_X_L 0x28
#define I2C_ACC_OUT_X_H 0x29
#define I2C_ACC_OUT_Y_L 0x2A
#define I2C_ACC_OUT_Y_H 0x2B
#define I2C_ACC_OUT_Z_L 0x2C
#define I2C_ACC_OUT_Z_H 0x2D

#define I2C_ACC_CTRL_R1_G 0x10
#define I2C_ACC_CTRL_R3_G 0x12
#define I2C_ACC_CTRL_R4 0x1E
#define I2C_ACC_CTRL_R5_XL 0x1F
#define I2C_ACC_CTRL_R6_XL 0x20
#define I2C_ACC_CTRL_R7_XL 0x21

#define I2C_ACC_CTRL_R1_G_CONF 0x40
#define I2C_ACC_CTRL_R3_G_CONF 0x41
#define I2C_ACC_CTRL_R4_CONF 0x38
#define I2C_ACC_CTRL_R5_XL_CONF 0x38
#define I2C_ACC_CTRL_R6_XL_CONF 0x38
#define I2C_ACC_CTRL_R7_XL_CONF 0x00

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

static volatile uint32_t tick_ms_count;

void sleep_ms(uint32_t ms)
{
	uint32_t current_ms = tick_ms_count;
	while ((tick_ms_count - current_ms) < ms)
		;
}

static uint16_t wmi;
static void i2c_setup(void)
{
	rcc_periph_clock_enable(RCC_I2C1);
	rcc_periph_clock_enable(RCC_GPIOB);

	i2c_reset(I2C1);
	gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO6 | GPIO9);
	gpio_set_af(GPIOB, GPIO_AF4, GPIO6 | GPIO9);
	i2c_set_clock_frequency(I2C1, I2C_CR2_FREQ_2MHZ);
	i2c_set_speed(I2C1, i2c_speed_sm_100k, 8);
	i2c_set_trise(I2C1, 3);
	i2c_peripheral_enable(I2C1);
}

static void gpio_setup()
{
	rcc_periph_clock_enable(RCC_GPIOA);
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
	// systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(21000 * 8 - 1);
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
	rcc_periph_clock_enable(RCC_TIM10);
	timer_set_mode(TIM10, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE,
		       TIM_CR1_DIR_UP);
	timer_set_counter(TIM10, 1);
	timer_set_prescaler(TIM10, 21000);
	timer_disable_preload(TIM10);
	timer_set_period(TIM10, 1600);
	timer_enable_irq(TIM10, TIM_DIER_UIE);
	timer_enable_counter(TIM10);
}

static void mems_command(uint8_t *command, uint8_t command_size, uint8_t *data,
			 uint8_t data_size);
static void mems_setup(void)
{
	uint8_t command[2];
	uint8_t data;

	command[0] = I2C_ACC_CTRL_R1_G;
	command[1] = I2C_ACC_CTRL_R1_G_CONF;
	mems_command(command, 2, &data, 0);

	command[0] = I2C_ACC_CTRL_R3_G;
	command[1] = I2C_ACC_CTRL_R3_G_CONF;
	mems_command(command, 2, &data, 0);

	command[0] = I2C_ACC_CTRL_R4;
	command[1] = I2C_ACC_CTRL_R4_CONF;
	mems_command(command, 2, &data, 0);

	command[0] = I2C_ACC_CTRL_R5_XL;
	command[1] = I2C_ACC_CTRL_R5_XL_CONF;
	mems_command(command, 2, &data, 0);

	command[0] = I2C_ACC_CTRL_R6_XL;
	command[1] = I2C_ACC_CTRL_R6_XL_CONF;
	mems_command(command, 2, &data, 0);

	command[0] = I2C_ACC_CTRL_R7_XL;
	command[1] = I2C_ACC_CTRL_R7_XL_CONF;
	mems_command(command, 2, &data, 0);
}

static void mems_command(uint8_t *command, uint8_t command_size, uint8_t *data,
			 uint8_t data_size)
{
	i2c_transfer7(I2C1, I2C_ACC_ADDR, command, command_size, data,
		      data_size);
	// uint16_t return_value;
	// uint16_t ignore;
	//
	// gpio_clear(GPIOE, GPIO3);
	// spi_send(SPI1, command);
	// ignore = spi_read(SPI1);
	//(void)ignore;
	// spi_send(SPI1, data);
	// return_value = spi_read(SPI1);
	// gpio_set(GPIOE, GPIO3);
	// gpio_set(GPIOD, GPIO12);
	// while (SPI_SR(SPI1) & SPI_SR_BSY)
	//;
	// gpio_clear(GPIOD, GPIO12);
	// return (uint8_t)return_value;
}

static uint8_t mems_read_reg(uint8_t reg)
{
	uint16_t command;
	uint8_t data;

	mems_command(&reg, 1, &data, 1);
	return data;
}

static void mems_read_values(int16_t x[3], int16_t y[3], int16_t z[3], int i)
{
	x[i] = mems_read_reg(I2C_ACC_OUT_X_L)
	       | (uint16_t)(mems_read_reg(I2C_ACC_OUT_X_H) << 8);
	y[i] = mems_read_reg(I2C_ACC_OUT_Y_L)
	       | (uint16_t)(mems_read_reg(I2C_ACC_OUT_Y_H) << 8);
	z[i] = mems_read_reg(I2C_ACC_OUT_Z_L)
	       | (uint16_t)(mems_read_reg(I2C_ACC_OUT_Z_H) << 8);
}

static int16_t median_filter(int16_t values[3])
{
	int16_t return_value;
	if (values[0] < values[1]) {
		if (values[1] <= values[2])
			return values[1];
		else if (values[0] <= values[2])
			return values[2];
		else
			return values[0];
	} else {
		if (values[0] <= values[2])
			return values[0];
		else if (values[1] <= values[2])
			return values[2];
		else
			return values[1];
	}
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

int main(void)
{
	rcc_clock_setup_pll(&rcc_hse_8mhz_3v3[RCC_CLOCK_3V3_168MHZ]);
	systick_setup();
	gpio_setup();
	nvic_setup();
	timer_setup();
	hd44780_init(&lcd_bus, 16, false, 2, false);
	keys_setup(&key, 1);
	gpio_clear(GPIOE, GPIO9);
	i2c_setup();
	mems_setup();

	uint8_t whoami;
	whoami = mems_read_reg(I2C_ACC_WHOAMI);
	// Turn on green led if the right accelerometer is found,
	// else turn on red led
	gpio_set(GPIOD, (whoami == 0x68) ? GPIO12 : GPIO14);

	int16_t ax[3], ay[3], az[3];
	int16_t a_filtered[3];
	float default_angle = 0;
	float pitch;
	int i;

	for (int i = 0; i < 3; ++i) {
		mems_read_values(ax, ay, az, i);
	}
	a_filtered[0] = median_filter(ax);
	a_filtered[1] = median_filter(ay);
	a_filtered[2] = median_filter(az);
	pitch = atan2(-a_filtered[0], sqrt(a_filtered[1] * a_filtered[1]
					   + a_filtered[2] * a_filtered[2]))
		* 180 / PI;
	default_angle -= pitch;

	i = 0;
	while (1) {
		if (key_pressed(&key, 0)) {
			if (!reading_user_key && !user_key_down) {
				reading_user_key = true;
				timer_set_counter(TIM10, 0);
			}
		} else if (user_key_down) {
			user_key_down = false;
			default_angle -= pitch;
		}

		mems_read_values(ax, ay, az, i);
		a_filtered[0] = median_filter(ax);
		a_filtered[1] = median_filter(ay);
		a_filtered[2] = median_filter(az);
		// Since output to lcd takes such a long time compered to
		// reading key_press and reading data from accelerometer that it
		// makes sense to read accelerometer 3 times before output
		// making visual latency lower. Then it is also not needed to
		// calculate pitch every cycle
		if (i == 0) {
			pitch = atan2(-a_filtered[0],
				      sqrt(a_filtered[1] * a_filtered[1]
					   + a_filtered[2] * a_filtered[2]))
					* 180 / PI
				+ default_angle;
			hd44780_clear();
			hd44780_printf_xy(16 - 4, 1, "%+4.0f", pitch);
			hd44780_printf_xy(7 + (int)(pitch / 90 * 7), 0, "VV");
			hd44780_printf_xy(0, 1, "%hX", whoami);
		}
		i = (i + 1) % 3;
	}

	return 0;
}
