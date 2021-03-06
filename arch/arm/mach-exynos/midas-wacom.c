/*
 * linux/arch/arm/mach-exynos/midas-wacom.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/i2c.h>

#include <linux/err.h>
#include <linux/gpio.h>

#include <linux/wacom_i2c.h>

#include <linux/regulator/consumer.h>
#include <plat/gpio-cfg.h>

#ifdef CONFIG_CPU_FREQ_GOV_ONDEMAND_FLEXRATE
#include <linux/cpufreq.h>
#endif

static struct wacom_g5_callbacks *wacom_callbacks;

static int wacom_early_suspend_hw(void)
{
	gpio_direction_input(GPIO_PEN_PDCT);
	gpio_set_value(GPIO_PEN_RESET_N, 0);
	return 0;
}

static int wacom_late_resume_hw(void)
{
	gpio_direction_output(GPIO_PEN_PDCT, 1);
	gpio_set_value(GPIO_PEN_RESET_N, 1);
	return 0;
}

static int wacom_suspend_hw(void)
{
	return wacom_early_suspend_hw();
}

static int wacom_resume_hw(void)
{
	return wacom_late_resume_hw();
}

static int wacom_reset_hw(void)
{
	wacom_early_suspend_hw();
	msleep(200);
	wacom_late_resume_hw();

	return 0;
}

static void wacom_register_callbacks(struct wacom_g5_callbacks *cb)
{
	wacom_callbacks = cb;
};


static struct wacom_g5_platform_data wacom_platform_data = {
	.x_invert = 1,
	.y_invert = 0,
	.xy_switch = 1,
	.min_x = 0,
	.max_x = WACOM_POSX_MAX,
	.min_y = 0,
	.max_y = WACOM_POSY_MAX,
	.min_pressure = 0,
	.max_pressure = WACOM_PRESSURE_MAX,
	.gpio_pendct = GPIO_PEN_PDCT,
	/*.init_platform_hw = midas_wacom_init,*/
	/*      .exit_platform_hw =,    */
	.suspend_platform_hw = wacom_suspend_hw,
	.resume_platform_hw = wacom_resume_hw,
	.early_suspend_platform_hw = wacom_early_suspend_hw,
	.late_resume_platform_hw = wacom_late_resume_hw,
	.reset_platform_hw = wacom_reset_hw,
	.register_cb = wacom_register_callbacks,
};

/* I2C2 */
static struct i2c_board_info i2c_devs2[] __initdata = {
	{
		I2C_BOARD_INFO("wacom_g5sp_i2c", 0x56),
		.platform_data = &wacom_platform_data,
	},
};

void __init midas_wacom_init(void)
{
	int gpio;
	int ret;

	gpio = GPIO_PEN_RESET_N;
	ret = gpio_request(gpio, "PEN_RESET");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	gpio_direction_output(gpio, 1);

	gpio = GPIO_PEN_SLP;
	ret = gpio_request(gpio, "PEN_SLP");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0x1));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	gpio_direction_output(gpio, 0);

	gpio = GPIO_PEN_PDCT;
	ret = gpio_request(gpio, "PEN_PDCT");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	s5p_register_gpio_interrupt(gpio);
	gpio_direction_input(gpio);

	irq_set_irq_type(gpio_to_irq(gpio), IRQ_TYPE_EDGE_BOTH);

	gpio = GPIO_PEN_IRQ;
	ret = gpio_request(gpio, "PEN_IRQ");
	s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	s5p_register_gpio_interrupt(gpio);
	gpio_direction_input(gpio);

	i2c_devs2[0].irq = gpio_to_irq(gpio);
	irq_set_irq_type(i2c_devs2[0].irq, IRQ_TYPE_EDGE_RISING);

	s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(0xf));

	gpio = GPIO_WACOM_LDO_EN;
	ret = gpio_request(gpio, "PEN_LDO_EN");
	s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
	gpio_direction_output(gpio, 1);

	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));

	printk(KERN_INFO "[E-PEN] : wacom IC initialized.\n");
}
