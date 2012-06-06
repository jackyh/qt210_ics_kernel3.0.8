/* linux/arch/arm/mach-s5pv210/mach-herring.c
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/gpio.h>
#include <linux/gpio_event.h>
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/mfd/max8998.h>

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/usb/ch9.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/clk.h>
#include <linux/usb/ch9.h>

#include <linux/input.h>
#include <linux/irq.h>
#include <linux/skbuff.h>
#include <linux/console.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/system.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/gpio.h>
#include <mach/gpio-smdkc110.h>
#include <mach/regs-gpio.h>
#include <mach/ts-s3c.h>
#include <mach/adc.h>
#include <mach/param.h>
#include <mach/system.h>

#include <linux/usb/gadget.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/wlan_plat.h>

#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#endif
#include <plat/media.h>
#include <mach/media.h>

#ifdef CONFIG_S5PV210_POWER_DOMAIN
#include <mach/power-domain.h>
#endif
#include <mach/cpu-freq-v210.h>

#include <media/s5ka3dfx_platform.h>
#include <media/s5k4ecgx.h>

#include <plat/regs-serial.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/fb.h>
#include <plat/mfc.h>
#include <plat/iic.h>
#include <plat/pm.h>
#include <plat/s5p-time.h>

#include <plat/sdhci.h>
#include <plat/fimc.h>
#include <plat/jpeg.h>
#include <plat/clock.h>
#include <plat/regs-otg.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#include <linux/sec_jack.h>
#include <linux/max17040_battery.h>
#include <linux/mfd/max8998.h>
#include <linux/regulator/max8893.h>
#include <linux/switch.h>


struct class *sec_class;
EXPORT_SYMBOL(sec_class);

struct device *switch_dev;
EXPORT_SYMBOL(switch_dev);

void (*sec_set_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_set_param_value);

void (*sec_get_param_value)(int idx, void *value);
EXPORT_SYMBOL(sec_get_param_value);

#define REBOOT_MODE_FAST_BOOT		7

static void herring_switch_init(void)
{
	sec_class = class_create(THIS_MODULE, "sec");

	if (IS_ERR(sec_class))
		pr_err("Failed to create class(sec)!\n");

	switch_dev = device_create(sec_class, NULL, 0, NULL, "switch");

	if (IS_ERR(switch_dev))
		pr_err("Failed to create device(switch)!\n");
};

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define S5PV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define S5PV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define S5PV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdkc110_uartcfgs[] __initdata = {
	{
		.hwport		= 0,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
	{
		.hwport		= 1,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
#ifndef CONFIG_FIQ_DEBUGGER
	{
		.hwport		= 2,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
#endif
	{
		.hwport		= 3,
		.flags		= 0,
		.ucon		= S5PV210_UCON_DEFAULT,
		.ulcon		= S5PV210_ULCON_DEFAULT,
		.ufcon		= S5PV210_UFCON_DEFAULT,
	},
};

#define S5PV210_LCD_WIDTH 800
#define S5PV210_LCD_HEIGHT 480

static struct s3cfb_lcd s6e63m0 = {
	.width = S5PV210_LCD_WIDTH,
	.height = S5PV210_LCD_HEIGHT,
	.p_width = 52,
	.p_height = 86,
	.bpp = 24,
	.freq = 60,

	.timing = {
		.h_fp = 16,
		.h_bp = 16,
		.h_sw = 2,
		.v_fp = 28,
		.v_fpe = 1,
		.v_bp = 1,
		.v_bpe = 1,
		.v_sw = 2,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 1,
	},
};

static struct s3cfb_lcd nt35580 = {
	.width = 480,
	.height = 800,
	.p_width = 52,
	.p_height = 86,
	.bpp = 24,
	.freq = 60,
	.timing = {
		.h_fp = 10,
		.h_bp = 20,
		.h_sw = 10,
		.v_fp = 6,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw = 2,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 1,
	},
};

static struct s3cfb_lcd r61408 = {
	.width = 480,
	.height = 800,
	.p_width = 52,
	.p_height = 86,
	.bpp = 24,
	.freq = 60,
	.timing = {
		.h_fp = 100,
		.h_bp = 2,
		.h_sw = 2,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 10,
		.v_bpe = 1,
		.v_sw = 2,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1 (9900 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD (S5PV210_LCD_WIDTH * \
					     S5PV210_LCD_HEIGHT * 4 * \
					     (CONFIG_FB_S3C_NR_BUFFERS + \
						 (CONFIG_FB_S3C_NUM_OVLY_WIN * \
						  CONFIG_FB_S3C_NUM_BUF_OVLY_WIN)))
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG (8192 * SZ_1K)

static struct s5p_media_device smdkc110_media_devs[] = {
	[0] = {
		.id = S5P_MDEV_MFC,
		.name = "mfc",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0,
		.paddr = 0,
	},
	[1] = {
		.id = S5P_MDEV_MFC,
		.name = "mfc",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1,
		.paddr = 0,
	},
	[2] = {
		.id = S5P_MDEV_FIMC0,
		.name = "fimc0",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0,
		.paddr = 0,
	},
	[3] = {
		.id = S5P_MDEV_FIMC1,
		.name = "fimc1",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1,
		.paddr = 0,
	},
	[4] = {
		.id = S5P_MDEV_FIMC2,
		.name = "fimc2",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2,
		.paddr = 0,
	},
	[5] = {
		.id = S5P_MDEV_JPEG,
		.name = "jpeg",
		.bank = 0,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG,
		.paddr = 0,
	},
	[6] = {
		.id = S5P_MDEV_FIMD,
		.name = "fimd",
		.bank = 1,
		.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD,
		.paddr = 0,
	},
};

#ifdef CONFIG_CPU_FREQ
static struct s5pv210_cpufreq_voltage smdkc110_cpufreq_volt[] = {
	{
		.freq	= 1000000,
		.varm	= 1275000,
		.vint	= 1100000,
	}, {
		.freq	=  800000,
		.varm	= 1200000,
		.vint	= 1100000,
	}, {
		.freq	=  400000,
		.varm	= 1050000,
		.vint	= 1100000,
	}, {
		.freq	=  200000,
		.varm	=  950000,
		.vint	= 1100000,
	}, {
		.freq	=  100000,
		.varm	=  950000,
		.vint	= 1000000,
	},
};

static struct s5pv210_cpufreq_data smdkc110_cpufreq_plat = {
	.volt	= smdkc110_cpufreq_volt,
	.size	= ARRAY_SIZE(smdkc110_cpufreq_volt),
};
#endif

static struct regulator_consumer_supply ldo3_consumer[] = {
	REGULATOR_SUPPLY("pd_io", "s3c-usbgadget")
};

static struct regulator_consumer_supply ldo7_consumer[] = {
	{	.supply	= "vlcd", },
};

static struct regulator_consumer_supply ldo8_consumer[] = {
	REGULATOR_SUPPLY("pd_core", "s3c-usbgadget")
};

static struct regulator_consumer_supply ldo11_consumer[] = {
	{	.supply	= "cam_af", },
};

static struct regulator_consumer_supply ldo12_consumer[] = {
	{	.supply	= "cam_sensor", },
};

static struct regulator_consumer_supply ldo13_consumer[] = {
	{	.supply	= "vga_vddio", },
};

static struct regulator_consumer_supply ldo14_consumer[] = {
	{	.supply	= "vga_dvdd", },
};

static struct regulator_consumer_supply ldo15_consumer[] = {
	{	.supply	= "cam_isp_host", },
};

static struct regulator_consumer_supply ldo16_consumer[] = {
	{	.supply	= "vga_avdd", },
};

static struct regulator_consumer_supply ldo17_consumer[] = {
	{	.supply	= "vcc_lcd", },
};

static struct regulator_consumer_supply buck1_consumer[] = {
	{	.supply	= "vddarm", },
};

static struct regulator_consumer_supply buck2_consumer[] = {
	{	.supply	= "vddint", },
};

static struct regulator_consumer_supply buck4_consumer[] = {
	{	.supply	= "cam_isp_core", },
};

static struct regulator_init_data smdkc110_ldo2_data = {
	.constraints	= {
		.name		= "VALIVE_1.2V",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.enabled = 1,
		},
	},
};

static struct regulator_init_data smdkc110_ldo3_data = {
	.constraints	= {
		.name		= "VUSB_1.1V",
		.min_uV		= 1100000,
		.max_uV		= 1100000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo3_consumer),
	.consumer_supplies	= ldo3_consumer,
};

static struct regulator_init_data smdkc110_ldo4_data = {
	.constraints	= {
		.name		= "VADC_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
};

static struct regulator_init_data smdkc110_ldo7_data = {
	.constraints	= {
		.name		= "VLCD_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo7_consumer),
	.consumer_supplies	= ldo7_consumer,
};

static struct regulator_init_data smdkc110_ldo8_data = {
	.constraints	= {
		.name		= "VUSB_3.3V",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo8_consumer),
	.consumer_supplies	= ldo8_consumer,
};

static struct regulator_init_data smdkc110_ldo9_data = {
	.constraints	= {
		.name		= "VCC_2.8V_PDA",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.always_on	= 1,
	},
};

static struct regulator_init_data smdkc110_ldo11_data = {
	.constraints	= {
		.name		= "CAM_AF_3.0V",
		.min_uV		= 3000000,
		.max_uV		= 3000000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo11_consumer),
	.consumer_supplies	= ldo11_consumer,
};

static struct regulator_init_data smdkc110_ldo12_data = {
	.constraints	= {
		.name		= "CAM_SENSOR_CORE_1.2V",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo12_consumer),
	.consumer_supplies	= ldo12_consumer,
};

static struct regulator_init_data smdkc110_ldo13_data = {
	.constraints	= {
		.name		= "VGA_VDDIO_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo13_consumer),
	.consumer_supplies	= ldo13_consumer,
};

static struct regulator_init_data smdkc110_ldo14_data = {
	.constraints	= {
		.name		= "VGA_DVDD_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo14_consumer),
	.consumer_supplies	= ldo14_consumer,
};

static struct regulator_init_data smdkc110_ldo15_data = {
	.constraints	= {
		.name		= "CAM_ISP_HOST_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo15_consumer),
	.consumer_supplies	= ldo15_consumer,
};

static struct regulator_init_data smdkc110_ldo16_data = {
	.constraints	= {
		.name		= "VGA_AVDD_2.8V",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo16_consumer),
	.consumer_supplies	= ldo16_consumer,
};

static struct regulator_init_data smdkc110_ldo17_data = {
	.constraints	= {
		.name		= "VCC_3.0V_LCD",
		.min_uV		= 3000000,
		.max_uV		= 3000000,
		.apply_uV	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(ldo17_consumer),
	.consumer_supplies	= ldo17_consumer,
};

static struct regulator_init_data smdkc110_buck1_data = {
	.constraints	= {
		.name		= "VDD_ARM",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1250000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(buck1_consumer),
	.consumer_supplies	= buck1_consumer,
};

static struct regulator_init_data smdkc110_buck2_data = {
	.constraints	= {
		.name		= "VDD_INT",
		.min_uV		= 750000,
		.max_uV		= 1500000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.uV	= 1100000,
			.mode	= REGULATOR_MODE_NORMAL,
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(buck2_consumer),
	.consumer_supplies	= buck2_consumer,
};

static struct regulator_init_data smdkc110_buck3_data = {
	.constraints	= {
		.name		= "VCC_1.8V",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.apply_uV	= 1,
		.always_on	= 1,
	},
};

static struct regulator_init_data smdkc110_buck4_data = {
	.constraints	= {
		.name		= "CAM_ISP_CORE_1.2V",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled = 1,
		},
	},
	.num_consumer_supplies	= ARRAY_SIZE(buck4_consumer),
	.consumer_supplies	= buck4_consumer,
};

static struct max8998_regulator_data herring_regulators[] = {
	{ MAX8998_LDO2,  &smdkc110_ldo2_data },
	{ MAX8998_LDO3,  &smdkc110_ldo3_data },
	{ MAX8998_LDO4,  &smdkc110_ldo4_data },
	{ MAX8998_LDO7,  &smdkc110_ldo7_data },
	{ MAX8998_LDO8,  &smdkc110_ldo8_data },
	{ MAX8998_LDO9,  &smdkc110_ldo9_data },
	{ MAX8998_LDO11, &smdkc110_ldo11_data },
	{ MAX8998_LDO12, &smdkc110_ldo12_data },
	{ MAX8998_LDO13, &smdkc110_ldo13_data },
	{ MAX8998_LDO14, &smdkc110_ldo14_data },
	{ MAX8998_LDO15, &smdkc110_ldo15_data },
	{ MAX8998_LDO16, &smdkc110_ldo16_data },
	{ MAX8998_LDO17, &smdkc110_ldo17_data },
	{ MAX8998_BUCK1, &smdkc110_buck1_data },
	{ MAX8998_BUCK2, &smdkc110_buck2_data },
	{ MAX8998_BUCK3, &smdkc110_buck3_data },
	{ MAX8998_BUCK4, &smdkc110_buck4_data },
};







static void set_adc_table(void)
{
	if (!herring_is_tft_dev()) {
		if (herring_is_cdma_wimax_dev()) {
			herring_charger.adc_table =
				temper_table_cdma_wimax_oled;
			herring_charger.adc_array_size =
				ARRAY_SIZE(temper_table_cdma_wimax_oled);
		} else {
			herring_charger.adc_table = temper_table_oled;
			herring_charger.adc_array_size =
				ARRAY_SIZE(temper_table_oled);
		}
	} else {
		herring_charger.adc_table = temper_table_tft;
		herring_charger.adc_array_size =
			ARRAY_SIZE(temper_table_tft);
	}
}

static struct max8998_platform_data max8998_pdata = {
	.num_regulators = ARRAY_SIZE(herring_regulators),
	.regulators     = herring_regulators,
	.charger        = &herring_charger,
	/* Preloads must be in increasing order of voltage value */
	.buck1_voltage4	= 950000,
	.buck1_voltage3	= 1050000,
	.buck1_voltage2	= 1200000,
	.buck1_voltage1	= 1275000,
	.buck2_voltage2	= 1000000,
	.buck2_voltage1	= 1100000,
	.buck1_set1	= GPIO_BUCK_1_EN_A,
	.buck1_set2	= GPIO_BUCK_1_EN_B,
	.buck2_set3	= GPIO_BUCK_2_EN,
	.buck1_default_idx = 1,
	.buck2_default_idx = 0,
};

#ifdef CONFIG_FB_S3C_LTE480WV
static struct s3cfb_lcd lte480wv = {
	.width = S5PV210_LCD_WIDTH,
	.height = S5PV210_LCD_HEIGHT,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 120,
		.h_bp = 13,
		.h_sw = 3,
		.v_fp = 5,
		.v_fpe = 1,
		.v_bp = 7,
		.v_bpe = 1,
		.v_sw = 1,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static void lte480wv_cfg_gpio(struct platform_device *pdev)
{
	int i;

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF0(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF0(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF1(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF1(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 8; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF2(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF2(i), S3C_GPIO_PULL_NONE);
	}

	for (i = 0; i < 6; i++) {
		s3c_gpio_cfgpin(S5PV210_GPF3(i), S3C_GPIO_SFN(2));
		s3c_gpio_setpull(S5PV210_GPF3(i), S3C_GPIO_PULL_NONE);
	}

	/* mDNIe SEL: why we shall write 0x2 ? */
#ifdef CONFIG_FB_S3C_MDNIE
	writel(0x1, S5P_MDNIE_SEL);
#else
	writel(0x2, S5P_MDNIE_SEL);
#endif

	/* drive strength to max */
	writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
	writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
	writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
}

#define S5PV210_GPD_0_0_TOUT_0  (0x2)
#define S5PV210_GPD_0_1_TOUT_1  (0x2 << 4)
#define S5PV210_GPD_0_2_TOUT_2  (0x2 << 8)
#define S5PV210_GPD_0_3_TOUT_3  (0x2 << 12)
static int lte480wv_backlight_on(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV210_GPD0(3), "GPD0");

	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
			"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPD0(3), 1);

	s3c_gpio_cfgpin(S5PV210_GPD0(3), S5PV210_GPD_0_3_TOUT_3);

	gpio_free(S5PV210_GPD0(3));
	return 0;
}

static int lte480wv_backlight_off(struct platform_device *pdev, int onoff)
{
	int err;

	err = gpio_request(S5PV210_GPD0(3), "GPD0");

	if (err) {
		printk(KERN_ERR "failed to request GPD0 for "
				"lcd backlight control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPD0(3), 0);
	gpio_free(S5PV210_GPD0(3));
	return 0;
}

static int lte480wv_reset_lcd(struct platform_device *pdev)
{
	int err;

	err = gpio_request(S5PV210_GPH0(6), "GPH0");
	if (err) {
		printk(KERN_ERR "failed to request GPH0 for "
				"lcd reset control\n");
		return err;
	}

	gpio_direction_output(S5PV210_GPH0(6), 1);
	mdelay(100);

	gpio_set_value(S5PV210_GPH0(6), 0);
	mdelay(10);

	gpio_set_value(S5PV210_GPH0(6), 1);
	mdelay(10);

	gpio_free(S5PV210_GPH0(6));

	return 0;
}

static struct s3c_platform_fb lte480wv_fb_data __initdata = {
	.hw_ver	= 0x62,
	.nr_wins = 5,
	.default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
	.swap = FB_SWAP_WORD | FB_SWAP_HWORD,

	.lcd = &lte480wv,
	.cfg_gpio	= lte480wv_cfg_gpio,
	.backlight_on	= lte480wv_backlight_on,
	.backlight_onoff    = lte480wv_backlight_off,
	.reset_lcd	= lte480wv_reset_lcd,
};
#endif



static struct i2c_gpio_platform_data herring_i2c4_platdata = {
	.sda_pin		= GPIO_AP_SDA_18V,
	.scl_pin		= GPIO_AP_SCL_18V,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c4_device = {
	.name			= "i2c-gpio",
	.id			= 4,
	.dev.platform_data	= &herring_i2c4_platdata,
};

static struct i2c_gpio_platform_data herring_i2c5_platdata = {
	.sda_pin		= GPIO_AP_SDA_28V,
	.scl_pin		= GPIO_AP_SCL_28V,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c5_device = {
	.name			= "i2c-gpio",
	.id			= 5,
	.dev.platform_data	= &herring_i2c5_platdata,
};

static struct i2c_gpio_platform_data herring_i2c6_platdata = {
	.sda_pin		= GPIO_AP_PMIC_SDA,
	.scl_pin		= GPIO_AP_PMIC_SCL,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c6_device = {
	.name			= "i2c-gpio",
	.id			= 6,
	.dev.platform_data	= &herring_i2c6_platdata,
};

static struct i2c_gpio_platform_data herring_i2c7_platdata = {
	.sda_pin		= GPIO_USB_SDA_28V,
	.scl_pin		= GPIO_USB_SCL_28V,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c7_device = {
	.name			= "i2c-gpio",
	.id			= 7,
	.dev.platform_data	= &herring_i2c7_platdata,
};

static struct i2c_gpio_platform_data herring_i2c8_platdata = {
	.sda_pin		= GYRO_SDA_28V,
	.scl_pin		= GYRO_SCL_28V,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c8_device = {
	.name			= "i2c-gpio",
	.id			= 8,
	.dev.platform_data	= &herring_i2c8_platdata,
};

static struct i2c_gpio_platform_data herring_i2c9_platdata = {
	.sda_pin		= FUEL_SDA_18V,
	.scl_pin		= FUEL_SCL_18V,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c9_device = {
	.name			= "i2c-gpio",
	.id			= 9,
	.dev.platform_data	= &herring_i2c9_platdata,
};

static struct i2c_gpio_platform_data herring_i2c10_platdata = {
	.sda_pin		= _3_TOUCH_SDA_28V,
	.scl_pin		= _3_TOUCH_SCL_28V,
	.udelay			= 0,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c10_device = {
	.name			= "i2c-gpio",
	.id			= 10,
	.dev.platform_data	= &herring_i2c10_platdata,
};

static struct i2c_gpio_platform_data herring_i2c11_platdata = {
	.sda_pin		= GPIO_ALS_SDA_28V,
	.scl_pin		= GPIO_ALS_SCL_28V,
	.udelay			= 2,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c11_device = {
	.name			= "i2c-gpio",
	.id			= 11,
	.dev.platform_data	= &herring_i2c11_platdata,
};

static struct i2c_gpio_platform_data herring_i2c12_platdata = {
	.sda_pin		= GPIO_MSENSE_SDA_28V,
	.scl_pin		= GPIO_MSENSE_SCL_28V,
	.udelay			= 0,    /* 250KHz */
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c12_device = {
	.name			= "i2c-gpio",
	.id			= 12,
	.dev.platform_data	= &herring_i2c12_platdata,
};

static struct i2c_gpio_platform_data herring_i2c14_platdata = {
	.sda_pin		= NFC_SDA_18V,
	.scl_pin		= NFC_SCL_18V,
	.udelay			= 2,
	.sda_is_open_drain	= 0,
	.scl_is_open_drain	= 0,
	.scl_is_output_only	= 0,
};

static struct platform_device herring_i2c14_device = {
	.name			= "i2c-gpio",
	.id			= 14,
	.dev.platform_data	= &herring_i2c14_platdata,
};

/* max8893 wimax PMIC */
static struct i2c_gpio_platform_data herring_i2c15_platdata = {
	.sda_pin		= GPIO_WIMAX_PM_SDA,
	.scl_pin		= GPIO_WIMAX_PM_SCL,
};

static struct platform_device herring_i2c15_device = {
	.name			= "i2c-gpio",
	.id			= 15,
	.dev.platform_data	= &herring_i2c15_platdata,
};

static struct regulator_init_data herring_max8893_buck_data = {
	.constraints	= {
		.name		= "max8893_buck",
		.min_uV		= 1200000,
		.max_uV		= 1200000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data herring_max8893_ldo1_data = {
	.constraints	= {
		.name		= "max8893_ldo1",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data herring_max8893_ldo2_data = {
	.constraints	= {
		.name		= "max8893_ldo2",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data herring_max8893_ldo3_data = {
	.constraints	= {
		.name		= "max8893_ldo3",
		.min_uV		= 3300000,
		.max_uV		= 3300000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data herring_max8893_ldo4_data = {
	.constraints	= {
		.name		= "max8893_ldo4",
		.min_uV		= 2900000,
		.max_uV		= 2900000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
};

static struct regulator_init_data herring_max8893_ldo5_data = {
	.constraints	= {
		.name		= "max8893_ldo5",
		.min_uV		= 2800000,
		.max_uV		= 2800000,
		.apply_uV	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
	},
};

static struct max8893_subdev_data herring_max8893_subdev_data[] = {
	{
		.id = MAX8893_BUCK,
		.initdata = &herring_max8893_buck_data,
	},
	{
		.id = MAX8893_LDO1,
		.initdata = &herring_max8893_ldo1_data,
	},
	{
		.id = MAX8893_LDO2,
		.initdata = &herring_max8893_ldo2_data,
	},
	{
		.id = MAX8893_LDO3,
		.initdata = &herring_max8893_ldo3_data,
	},
	{
		.id = MAX8893_LDO4,
		.initdata = &herring_max8893_ldo4_data,
	},
	{
		.id = MAX8893_LDO5,
		.initdata = &herring_max8893_ldo5_data,
	},
};

static struct max8893_platform_data herring_max8893_pdata = {
	.num_subdevs = ARRAY_SIZE(herring_max8893_subdev_data),
	.subdevs = herring_max8893_subdev_data,
};

static struct i2c_board_info i2c_devs15[] __initdata = {
	{
		I2C_BOARD_INFO("max8893", 0x3E),
		.platform_data	= &herring_max8893_pdata,
	},
};





static void touch_keypad_gpio_init(void)
{
	int ret = 0;

	ret = gpio_request(_3_GPIO_TOUCH_EN, "TOUCH_EN");
	if (ret)
		printk(KERN_ERR "Failed to request gpio touch_en.\n");
}

static void touch_keypad_onoff(int onoff)
{
	gpio_direction_output(_3_GPIO_TOUCH_EN, onoff);

	if (onoff == TOUCHKEY_OFF)
		msleep(30);
	else
		msleep(50);
}

static const int touch_keypad_code[] = {
	KEY_MENU,
	KEY_HOME,
	KEY_BACK,
	KEY_SEARCH
};

static struct touchkey_platform_data touchkey_data = {
	.keycode_cnt = ARRAY_SIZE(touch_keypad_code),
	.keycode = touch_keypad_code,
	.touchkey_onoff = touch_keypad_onoff,
	.fw_name = "cypress-touchkey.bin",
	.scl_pin = _3_TOUCH_SCL_28V,
	.sda_pin = _3_TOUCH_SDA_28V,
	.en_pin = _3_GPIO_TOUCH_EN,
};

static struct gpio_event_direct_entry herring_keypad_key_map[] = {
	{
		.gpio	= S5PV210_GPH2(6),
		.code	= KEY_POWER,
	},
	{
		.gpio	= S5PV210_GPH3(1),
		.code	= KEY_VOLUMEDOWN,
	},
	{
		.gpio	= S5PV210_GPH3(2),
		.code	= KEY_VOLUMEUP,
	}
};

static struct gpio_event_input_info herring_keypad_key_info = {
	.info.func = gpio_event_input_func,
	.info.no_suspend = true,
	.debounce_time.tv64 = 5 * NSEC_PER_MSEC,
	.type = EV_KEY,
	.keymap = herring_keypad_key_map,
	.keymap_size = ARRAY_SIZE(herring_keypad_key_map)
};

static struct gpio_event_info *herring_input_info[] = {
	&herring_keypad_key_info.info,
};


static struct gpio_event_platform_data herring_input_data = {
	.names = {
		"herring-keypad",
		NULL,
	},
	.info = herring_input_info,
	.info_count = ARRAY_SIZE(herring_input_info),
};

static struct platform_device herring_input_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev = {
		.platform_data = &herring_input_data,
	},
};

#ifdef CONFIG_S5P_ADC
static struct s3c_adc_mach_info s3c_adc_platform __initdata = {
	/* s5pc110 support 12-bit resolution */
	.delay  = 10000,
	.presc  = 65,
	.resolution = 12,
};
#endif



/*************************add by xiang guangchao for ov3640 camera*********************8*/
/* 3.2M Cameras */
#ifdef CONFIG_VIDEO_OV3640
static int ov3640_power_en(int onoff)
{
	printk("ov3640: power %s\n", onoff ? "ON" : "Off");
	return 0;
}

static struct ov3640_platform_data ov3640_plat = {
	.default_width = 2048,
	.default_height = 1536,
	.pixelformat = V4L2_PIX_FMT_YUYV,
	.freq = 24000000,
	.is_mipi = 1,
};

static struct i2c_board_info  ov3640_i2c_info = {
	I2C_BOARD_INFO("OV3640", 0x3c),   //0x78 >> 1
	.platform_data = &ov3640_plat,
};
static struct s3c_platform_camera ov3640 = {
#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
#else
	.id		= CAMERA_PAR_B,
#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_YCBYCR,
	.i2c_busnum	= 1,
	.info		= &ov3640_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_YUYV,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam1",
	/* .clk_name	= "sclk_cam1", */
	.clk_rate	= 24000000,
	.line_length	= 1920,
	.width		= 640,
	.height		= 480,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 640,
		.height	= 480,
	},

	/* Polarity */
	.inv_pclk = 0,
	.inv_vsync = 1,
	.inv_href = 0,
	.inv_hsync = 0,

	.initialized	= 0,
	.cam_power	= ov3640_power_en,
};
#endif
/*****************************add end 20111226*********************************/


/* Interface setting */
static struct s3c_platform_fimc fimc_plat_lsi = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc",
	.lclk_name	= "fimc",
	.clk_rate	= 166750000,
	.default_cam	= CAMERA_PAR_A,
	.camera		= {
		
//xiang guangchao add for QT210 Camera
#ifdef CONFIG_VIDEO_OV3640
			&ov3640,
#endif
////xiang guangchao add end
	},
	.hw_ver		= 0x43,
};

#ifdef CONFIG_VIDEO_JPEG_V2
static struct s3c_platform_jpeg jpeg_plat __initdata = {
	.max_main_width	= 800,
	.max_main_height	= 480,
	.max_thumb_width	= 320,
	.max_thumb_height	= 240,
};
#endif


/* I2C0 */
static struct i2c_board_info i2c_devs0[] __initdata = {
#ifdef CONFIG_SND_SOC_WM8580
	{
		I2C_BOARD_INFO("wm8580", 0x1b),
	},
#endif
/* hcj needs modification */
#ifdef	CONFIG_TOUCHSCREEN_GOODIX

	{ 
		I2C_BOARD_INFO("Goodix-TS", 0x55),
        .irq = IRQ_EINT(4), 
    },
#endif


};

/* I2C1 */
static struct i2c_board_info i2c_devs1[] __initdata = {
#ifdef CONFIG_VIDEO_TV20
	{
		I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),
	},
#endif
};

/* I2C2 */
static struct i2c_board_info i2c_devs2[] __initdata = {
	{
#if defined(CONFIG_SMDKC110_REV03) || defined(CONFIG_SMDKV210_REV02)
		/* The address is 0xCC used since SRAD = 0 */
		I2C_BOARD_INFO("max8998", (0xCC >> 1)),
		.platform_data	= &max8998_pdata,
		.irq		= IRQ_EINT7,
	}, {
		I2C_BOARD_INFO("rtc_max8998", (0x0D >> 1)),
	},
#endif

	},
#ifdef CONFIG_SENSORS_BMA150
	{
		I2C_BOARD_INFO("max17040", (0x6D >> 1)),
		.platform_data = &max17040_pdata,
	},
#endif
};

static void gp2a_gpio_init(void)
{
	int ret = gpio_request(GPIO_PS_ON, "gp2a_power_supply_on");
	if (ret)
		printk(KERN_ERR "Failed to request gpio gp2a power supply.\n");
}

static int gp2a_power(bool on)
{
	/* this controls the power supply rail to the gp2a IC */
	gpio_direction_output(GPIO_PS_ON, on);
	return 0;
}

static int gp2a_light_adc_value(void)
{
	return s3c_adc_get_adc_data(9);
}

static struct gp2a_platform_data gp2a_pdata = {
	.power = gp2a_power,
	.p_out = GPIO_PS_VOUT,
	.light_adc_value = gp2a_light_adc_value,
	.light_adc_max = 4095,
	.light_adc_fuzz = 64,
};

static struct i2c_board_info i2c_devs11[] __initdata = {
	{
		I2C_BOARD_INFO("gp2a", (0x88 >> 1)),
		.platform_data = &gp2a_pdata,
	},
};

static struct i2c_board_info i2c_devs12[] __initdata = {
	{
		I2C_BOARD_INFO("ak8973", 0x1c),
		.platform_data = &akm8973_pdata,
	},
};

static struct resource ram_console_resource[] = {
	{
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources = ARRAY_SIZE(ram_console_resource),
	.resource = ram_console_resource,
};

#ifdef CONFIG_ANDROID_PMEM
static struct android_pmem_platform_data pmem_pdata = {
	.name = "pmem",
	.no_allocator = 1,
	.cached = 1,
	.start = 0,
	.size = 0,
};

static struct android_pmem_platform_data pmem_gpu1_pdata = {
	.name = "pmem_gpu1",
	.no_allocator = 1,
	.cached = 1,
	.buffered = 1,
	.start = 0,
	.size = 0,
};

static struct android_pmem_platform_data pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.no_allocator = 1,
	.cached = 1,
	.buffered = 1,
	.start = 0,
	.size = 0,
};

static struct platform_device pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &pmem_pdata },
};

static struct platform_device pmem_gpu1_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &pmem_gpu1_pdata },
};

static struct platform_device pmem_adsp_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &pmem_adsp_pdata },
};

static void __init android_pmem_set_platdata(void)
{
	pmem_pdata.start = (u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM, 0);
	pmem_pdata.size = (u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM, 0);

	pmem_gpu1_pdata.start =
		(u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_GPU1, 0);
	pmem_gpu1_pdata.size =
		(u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_GPU1, 0);

	pmem_adsp_pdata.start =
		(u32)s5p_get_media_memory_bank(S5P_MDEV_PMEM_ADSP, 0);
	pmem_adsp_pdata.size =
		(u32)s5p_get_media_memsize_bank(S5P_MDEV_PMEM_ADSP, 0);
}
#endif

struct platform_device sec_device_battery = {
	.name	= "sec-battery",
	.id	= -1,
};




#define S3C_GPIO_SETPIN_ZERO         0
#define S3C_GPIO_SETPIN_ONE          1
#define S3C_GPIO_SETPIN_NONE	     2

struct gpio_init_data {
	uint num;
	uint cfg;
	uint val;
	uint pud;
	uint drv;
};

static struct gpio_init_data herring_init_gpios[] = {
	{
		.num	= S5PV210_GPB(0),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPB(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,

	}, {
		.num	= S5PV210_GPB(2),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPB(3),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPB(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPB(5),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPB(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPB(7),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPC0(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC0(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC0(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC0(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC0(4),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPC1(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC1(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC1(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC1(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPC1(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPD0(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD0(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD0(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD0(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPD1(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD1(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD1(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD1(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD1(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPD1(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPE0(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE0(7),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},


	{
		.num	= S5PV210_GPE1(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE1(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE1(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE1(3),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPE1(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPF3(4),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPF3(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPG0(0),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG0(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG0(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG0(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG0(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG0(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG0(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPG1(0),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG1(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG1(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG1(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG1(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG1(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG1(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPG2(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG2(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG2(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG2(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG2(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG2(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG2(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPG3(0),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG3(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG3(2),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG3(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG3(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG3(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPG3(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPH0(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH0(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH0(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH0(3),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH0(4),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH0(5),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* GPIO_DET_35 - 3.5" ear jack */
		.num	= S5PV210_GPH0(6),
		.cfg	= S3C_GPIO_SFN(GPIO_DET_35_AF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH0(7),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPH1(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH1(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH1(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH1(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* NFC_IRQ */
		.num	= S5PV210_GPH1(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* NFC_EN */
		.num	= S5PV210_GPH1(5),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* NFC_FIRM */
		.num	= S5PV210_GPH1(6),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH1(7),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPH2(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(2),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(3),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(5),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(6),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH2(7),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPH3(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_UP,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH3(1),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH3(2),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH3(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH3(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH3(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* GPIO_EAR_SEND_END */
		.num	= S5PV210_GPH3(6),
		.cfg	= S3C_GPIO_SFN(GPIO_EAR_SEND_END_AF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPH3(7),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPI(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPI(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPI(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPI(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPI(4),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPI(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPI(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPJ0(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(5),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(6),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ0(7),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPJ1(0),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ1(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ1(2),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ1(3),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ1(4),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ1(5),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPJ2(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(1),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(3),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ2(7),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPJ3(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ3(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ3(2),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* GPIO_EAR_ADC_SEL */
		.num	= S5PV210_GPJ3(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ3(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ3(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ3(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ3(7),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_GPJ4(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ4(1),
		.cfg	= S3C_GPIO_SFN(0xF),
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ4(2),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ4(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_GPJ4(4),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_MP01(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP01(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP01(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_MP02(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP02(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP02(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_MP03(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP03(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP03(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP03(7),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_MP04(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP04(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* NFC_SCL_18V - has external pull up resistor */
		.num	= S5PV210_MP04(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, { /* NFC_SDA_18V - has external pull up resistor */
		.num	= S5PV210_MP04(5),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP04(6),
		.cfg	= S3C_GPIO_OUTPUT,
		.val	= S3C_GPIO_SETPIN_ZERO,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP04(7),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},

	{
		.num	= S5PV210_MP05(0),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP05(1),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP05(2),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP05(3),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_NONE,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP05(4),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	}, {
		.num	= S5PV210_MP05(6),
		.cfg	= S3C_GPIO_INPUT,
		.val	= S3C_GPIO_SETPIN_NONE,
		.pud	= S3C_GPIO_PULL_DOWN,
		.drv	= S3C_GPIO_DRVSTR_1X,
	},
};

void s3c_config_gpio_table(void)
{
	u32 i, gpio;

	for (i = 0; i < ARRAY_SIZE(herring_init_gpios); i++) {
		gpio = herring_init_gpios[i].num;
		if (system_rev <= 0x07 && gpio == S5PV210_GPJ3(3)) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
			gpio_set_value(gpio, S3C_GPIO_SETPIN_ONE);
		} else if (gpio <= S5PV210_MP05(7)) {
			s3c_gpio_cfgpin(gpio, herring_init_gpios[i].cfg);
			s3c_gpio_setpull(gpio, herring_init_gpios[i].pud);

			if (herring_init_gpios[i].val != S3C_GPIO_SETPIN_NONE)
				gpio_set_value(gpio, herring_init_gpios[i].val);

			s3c_gpio_set_drvstrength(gpio,
					herring_init_gpios[i].drv);
		}
	}

	if (herring_is_cdma_wimax_dev()) {
		/* WiMAX_I2C_CON */
		gpio = S5PV210_GPC1(1);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_ONE);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_4X);

		gpio = S5PV210_GPC1(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_NONE);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_4X);

		gpio = S5PV210_GPC1(4);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_NONE);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_4X);

		gpio = S5PV210_GPG2(0);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio = S5PV210_GPG2(1);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio = S5PV210_GPG2(3);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio = S5PV210_GPG2(4);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio = S5PV210_GPG2(5);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio = S5PV210_GPG2(6);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);

		/* WIMAX_EN */
		gpio = S5PV210_GPH1(0);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_ZERO);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_1X);

		/* GPIO_CP_RST, CDMA modem specific setting */
		gpio = S5PV210_GPH3(7);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_DOWN);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_ZERO);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_4X);

		gpio = S5PV210_MP05(2);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_NONE);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_4X);

		gpio = S5PV210_MP05(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		gpio_set_value(gpio, S3C_GPIO_SETPIN_NONE);
		s3c_gpio_set_drvstrength(gpio, S3C_GPIO_DRVSTR_4X);
	}
}

#define S5PV210_PS_HOLD_CONTROL_REG (S3C_VA_SYS+0xE81C)
static void herring_power_off(void)
{
	int phone_wait_cnt = 0;

	if (herring_is_cdma_wimax_dev()) {
		/* confirm phone is powered-off  */
		while (1) {
			if (gpio_get_value(GPIO_PHONE_ACTIVE)) {
				pr_info("%s: Try to Turn Phone Off by CP_RST\n",
					__func__);
				gpio_set_value(GPIO_CP_RST, 0);
				if (phone_wait_cnt > 1) {
					pr_info("%s: PHONE OFF Fail\n",
					__func__);
					break;
				}
				phone_wait_cnt++;
				mdelay(100);
			} else {
				pr_info("%s: PHONE OFF Success\n", __func__);
				break;
			}
		}
	}

	while (1) {
		/* Check reboot charging */
		if (set_cable_status) {
			/* watchdog reset */
			pr_info("%s: charger connected, rebooting\n", __func__);
			writel(3, S5P_INFORM6);
			arch_reset('r', NULL);
			pr_crit("%s: waiting for reset!\n", __func__);
			while (1);
		}

		/* wait for power button release */
		if (gpio_get_value(GPIO_nPOWER)) {
			pr_info("%s: set PS_HOLD low\n", __func__);

			/* PS_HOLD high  PS_HOLD_CONTROL, R/W, 0xE010_E81C */
			writel(readl(S5PV210_PS_HOLD_CONTROL_REG) & 0xFFFFFEFF,
			       S5PV210_PS_HOLD_CONTROL_REG);

			pr_crit("%s: should not reach here!\n", __func__);
		}

		/* if power button is not released, wait and check TA again */
		pr_info("%s: PowerButton is not released.\n", __func__);
		mdelay(1000);
	}
}

/* this table only for B4 board */
static unsigned int herring_sleep_gpio_table[][3] = {
	{ S5PV210_GPA0(0), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(1), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(2), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(3), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA0(5), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA0(7), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPA1(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA1(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA1(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA1(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPB(0),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(1),  S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(2),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(3),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(5),  S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPB(7),  S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPC0(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(4), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPC1(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC1(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC1(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC1(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC1(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPD0(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPD0(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD0(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPD0(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPD1(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPD1(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPE0(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPE1(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE1(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE1(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE1(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPE1(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPF0(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(2), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(4), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(5), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(6), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(7), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPF1(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(2), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(4), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(5), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(6), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(7), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPF2(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(2), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(4), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(5), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(6), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(7), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPF3(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(2), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(4), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPG0(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG0(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPG1(0), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG1(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPG2(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPG3(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(1), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(2), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(3), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(4), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(5), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(6), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},

	/* Alive part ending and off part start*/
	{ S5PV210_GPI(0),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(1),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(2),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(3),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(4),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(5),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(6),  S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPJ0(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ0(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ0(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ0(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(6), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ0(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPJ1(0), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(2), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(4), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(5), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPJ2(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(3), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ2(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(7), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},

	{ S5PV210_GPJ3(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(2), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(3), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPJ4(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ4(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ4(2), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ4(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ4(4), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},

	/* memory part */
	{ S5PV210_MP01(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP01(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(4), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP01(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP02(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP02(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP02(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP02(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP03(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(2), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP03(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP03(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP03(5), S3C_GPIO_SLP_OUT1,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP03(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP04(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP04(1), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP04(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP04(3), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP04(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP04(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP04(6), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP04(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP05(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP05(5), S3C_GPIO_SLP_OUT0,	S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP05(7), S3C_GPIO_SLP_PREV,	S3C_GPIO_PULL_NONE},

	{ S5PV210_MP06(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP07(0), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(1), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(2), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(3), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(4), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(5), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(6), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(7), S3C_GPIO_SLP_INPUT,	S3C_GPIO_PULL_DOWN},

	/* Memory part ending and off part ending */
};

static unsigned int herring_cdma_wimax_sleep_gpio_table[][3] = {
	{ S5PV210_GPA0(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(1), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(3), S3C_GPIO_SLP_OUT1,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA0(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA0(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA0(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA0(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPA1(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA1(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPA1(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPA1(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPB(0),  S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(1),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(2),  S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(3),  S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(4),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(5),  S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPB(6),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPB(7),  S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPC0(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC0(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	/*WIMAX PMIC SDA*/
	{ S5PV210_GPC1(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	/*Wimax eeprom switch*/
	{ S5PV210_GPC1(1), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	/*WIMAX PMIC SCL*/
	{ S5PV210_GPC1(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	/*WIMAX EEPROM I2C LINES*/
	{ S5PV210_GPC1(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPC1(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	/*WIMAX DBGEN*/
	{ S5PV210_GPD0(0), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPD0(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD0(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	/*WIMAX RESET_N*/
	{ S5PV210_GPD0(3), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPD1(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPD1(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPD1(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPE0(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE0(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},


	{ S5PV210_GPE1(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE1(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE1(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPE1(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPE1(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPF0(0), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(2), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(4), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(5), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(6), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF0(7), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPF1(0), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(2), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(4), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(5), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(6), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF1(7), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},


	{ S5PV210_GPF2(0), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(2), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(4), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(5), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(6), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF2(7), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPF3(0), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(2), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(4), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPF3(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},


	{ S5PV210_GPG0(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG0(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG0(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG0(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	{ S5PV210_GPG1(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG1(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG1(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG1(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG1(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	/*wimax SDIO pins*/
	{ S5PV210_GPG2(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG2(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG2(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPG2(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG2(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG2(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG2(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	{ S5PV210_GPG3(0), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(2), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPG3(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	/*WIMAX*/
	{ S5PV210_GPH1(0), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPH1(2), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPH2(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPH3(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_UP},
	{ S5PV210_GPH3(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	/* Alive part ending and off part start*/
	{ S5PV210_GPI(0),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(1),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(2),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(3),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(4),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(5),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPI(6),  S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPJ0(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ0(6), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ0(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPJ1(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(2), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(4), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ1(5), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPJ2(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(4), S3C_GPIO_SLP_PREV, 	S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPJ2(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ2(7), S3C_GPIO_SLP_OUT1,   S3C_GPIO_PULL_NONE},

	{ S5PV210_GPJ3(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(2), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ3(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ3(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_GPJ4(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ4(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_GPJ4(2), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ4(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_GPJ4(4), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	 /* memory part */
	{ S5PV210_MP01(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	/*WIMAX*/
	{ S5PV210_MP01(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	{ S5PV210_MP01(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(4), S3C_GPIO_SLP_OUT1,   S3C_GPIO_PULL_NONE},
	{ S5PV210_MP01(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP01(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP02(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP02(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP02(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_MP02(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP03(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(2), S3C_GPIO_SLP_OUT1,   S3C_GPIO_PULL_NONE},
	{ S5PV210_MP03(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_MP03(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP03(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	/*WIMAX*/
	{ S5PV210_MP04(0), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_MP04(1), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	/*WIMAX*/
	{ S5PV210_MP04(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP04(3), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},
	{ S5PV210_MP04(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP04(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP04(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	/*WIMAX*/
	{ S5PV210_MP04(7), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_MP05(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},
	{ S5PV210_MP05(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_NONE},

	/*WIMAX*/
	{ S5PV210_MP05(4), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_MP05(5), S3C_GPIO_SLP_OUT0,   S3C_GPIO_PULL_NONE},

	/*WIMAX*/
	{ S5PV210_MP05(6), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_MP05(7), S3C_GPIO_SLP_PREV,   S3C_GPIO_PULL_NONE},

	{ S5PV210_MP06(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP06(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	{ S5PV210_MP07(0), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(1), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(2), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(3), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(4), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(5), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(6), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},
	{ S5PV210_MP07(7), S3C_GPIO_SLP_INPUT,  S3C_GPIO_PULL_DOWN},

	/* Memory part ending and off part ending */
};

void s3c_config_sleep_gpio_table(int array_size, unsigned int (*gpio_table)[3])
{
	u32 i, gpio;

	for (i = 0; i < array_size; i++) {
		gpio = gpio_table[i][0];
		s3c_gpio_slp_cfgpin(gpio, gpio_table[i][1]);
		s3c_gpio_slp_setpull_updown(gpio, gpio_table[i][2]);
	}
}

void s3c_config_cdma_wimax_sleep_gpio(void)
{
	s3c_gpio_cfgpin(S5PV210_GPH0(0), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH0(0), S3C_GPIO_PULL_DOWN);

	s3c_gpio_cfgpin(S5PV210_GPH0(1), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH0(1), S3C_GPIO_PULL_DOWN);

	s3c_gpio_cfgpin(S5PV210_GPH0(2), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH0(2), S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(S5PV210_GPH0(3), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH0(3), S3C_GPIO_PULL_NONE);
	gpio_set_value(S5PV210_GPH0(3), 0);

	s3c_gpio_cfgpin(S5PV210_GPH0(4), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH0(4), S3C_GPIO_PULL_NONE);
	gpio_set_value(S5PV210_GPH0(4), 0);

	s3c_gpio_cfgpin(S5PV210_GPH0(5), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH0(5), S3C_GPIO_PULL_NONE);
	gpio_set_value(S5PV210_GPH0(5), 0);

	s3c_gpio_cfgpin(S5PV210_GPH1(0), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH1(0), S3C_GPIO_PULL_NONE);
	gpio_set_value(S5PV210_GPH1(0), 0);

	s3c_gpio_cfgpin(S5PV210_GPH1(1), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH1(1), S3C_GPIO_PULL_DOWN);
	gpio_set_value(S5PV210_GPH1(1), 0);

	s3c_gpio_cfgpin(S5PV210_GPH1(2), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH1(2), S3C_GPIO_PULL_UP);
	gpio_set_value(S5PV210_GPH1(2), 0);

	s3c_gpio_cfgpin(S5PV210_GPH1(4), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH1(4), S3C_GPIO_PULL_DOWN);

	s3c_gpio_cfgpin(S5PV210_GPH1(5), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH1(5), S3C_GPIO_PULL_DOWN);
	gpio_set_value(S5PV210_GPH1(5), 0);

	s3c_gpio_cfgpin(S5PV210_GPH1(6), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH1(6), S3C_GPIO_PULL_DOWN);

	s3c_gpio_cfgpin(S5PV210_GPH1(7), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH1(7), S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(S5PV210_GPH2(0), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH2(0), S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(S5PV210_GPH2(2), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH2(2), S3C_GPIO_PULL_NONE);
	gpio_set_value(S5PV210_GPH2(2), 0);

	s3c_gpio_cfgpin(S5PV210_GPH2(6), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH2(6), S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(S5PV210_GPH2(3), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH2(3), S3C_GPIO_PULL_NONE);
	gpio_set_value(S5PV210_GPH2(3), 0);

	s3c_gpio_cfgpin(S5PV210_GPH3(0), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH3(0), S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(S5PV210_GPH3(3), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH3(3), S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(S5PV210_GPH3(4), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH3(4), S3C_GPIO_PULL_UP);

	s3c_gpio_cfgpin(S5PV210_GPH3(5), S3C_GPIO_INPUT);
	s3c_gpio_setpull(S5PV210_GPH3(5), S3C_GPIO_PULL_DOWN);

	s3c_gpio_cfgpin(S5PV210_GPH3(7), S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(S5PV210_GPH3(7), S3C_GPIO_PULL_UP);
	gpio_set_value(S5PV210_GPH3(7), 1);

}




static struct platform_device *herring_devices[] __initdata = {
	&watchdog_device,
#ifdef CONFIG_FIQ_DEBUGGER
	&s5pv210_device_fiqdbg_uart2,
#endif
	&s5p_device_onenand,
#ifdef CONFIG_RTC_DRV_S3C
	&s5p_device_rtc,
#endif
	&herring_input_device,

	&s5pv210_device_iis0,
	&s3c_device_wdt,

#ifdef CONFIG_FB_S3C
	&s3c_device_fb,
#endif

#ifdef CONFIG_VIDEO_MFC50
	&s3c_device_mfc,
#endif
#ifdef	CONFIG_S5P_ADC
	&s3c_device_adc,
#endif
#ifdef CONFIG_VIDEO_FIMC
	&s3c_device_fimc0,
	&s3c_device_fimc1,
	&s3c_device_fimc2,
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif

	&s3c_device_g3d,
	&s3c_device_lcd,

#ifdef CONFIG_FB_S3C_TL2796
	&s3c_device_spi_gpio,
#endif
	&sec_device_jack,

	&s3c_device_i2c0,
#if defined(CONFIG_S3C_DEV_I2C1)
	&s3c_device_i2c1,
#endif

#if defined(CONFIG_S3C_DEV_I2C2)
	&s3c_device_i2c2,
#endif
	&herring_i2c4_device,
	&herring_i2c6_device,
	&herring_i2c7_device,
	&herring_i2c8_device,  /* gyro sensor */
	&herring_i2c9_device,  /* max1704x:fuel_guage */
	&herring_i2c11_device, /* optical sensor */
	&herring_i2c12_device, /* magnetic sensor */
	&herring_i2c14_device, /* nfc sensor */
#if defined CONFIG_USB_S3C_OTG_HOST
 	&s3c_device_usb_otghcd,
#endif
#if defined CONFIG_USB_DWC_OTG
	&s3c_device_usb_dwcotg,
#endif
#ifdef CONFIG_USB_GADGET
	&s3c_device_usbgadget,
#endif
#ifdef CONFIG_USB_ANDROID
	&s3c_device_android_usb,
#ifdef CONFIG_USB_ANDROID_MASS_STORAGE
	&s3c_device_usb_mass_storage,
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
	&s3c_device_rndis,
#endif
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
	&s3c_device_hsmmc0,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	&s3c_device_hsmmc1,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	&s3c_device_hsmmc2,
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	&s3c_device_hsmmc3,
#endif

	&sec_device_battery,
	&herring_i2c10_device,

#ifdef CONFIG_S5PV210_POWER_DOMAIN
	&s5pv210_pd_audio,
	&s5pv210_pd_cam,
	&s5pv210_pd_tv,
	&s5pv210_pd_lcd,
	&s5pv210_pd_g3d,
	&s5pv210_pd_mfc,
#endif

#ifdef CONFIG_ANDROID_PMEM
	&pmem_device,
	&pmem_gpu1_device,
	&pmem_adsp_device,
#endif

#ifdef CONFIG_HAVE_PWM
	&s3c_device_timer[0],
	&s3c_device_timer[1],
	&s3c_device_timer[2],
	&s3c_device_timer[3],
#endif

#ifdef CONFIG_CPU_FREQ
	&s5pv210_device_cpufreq,
#endif

	&sec_device_rfkill,
	&sec_device_btsleep,
	&ram_console_device,
	&sec_device_wifi,
	&samsung_asoc_dma,
};

unsigned int HWREV;
EXPORT_SYMBOL(HWREV);

static void __init herring_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(herring_uartcfgs, ARRAY_SIZE(herring_uartcfgs));
#ifndef CONFIG_S5P_HIGH_RES_TIMERS
	s5p_set_timer_source(S5P_PWM3, S5P_PWM4);
#endif
	s5p_reserve_bootmem(herring_media_devs,
			ARRAY_SIZE(herring_media_devs), S5P_RANGE_MFC);
#ifdef CONFIG_MTD_ONENAND
	s5p_device_onenand.name = "s5pc110-onenand";
#endif
}

unsigned int pm_debug_scratchpad;

static unsigned int ram_console_start;
static unsigned int ram_console_size;

#if 0
static void __init herring_fixup(struct machine_desc *desc,
		struct tag *tags, char **cmdline,
		struct meminfo *mi)
{
	mi->bank[0].start = 0x30000000;
	mi->bank[0].size = 80 * SZ_1M;

	mi->bank[1].start = 0x40000000;
	mi->bank[1].size = 256 * SZ_1M;

	mi->bank[2].start = 0x50000000;
	/* 1M for ram_console buffer */
	mi->bank[2].size = 127 * SZ_1M;
	mi->nr_banks = 3;

	ram_console_start = mi->bank[2].start + mi->bank[2].size;
	ram_console_size = SZ_1M - SZ_4K;

	pm_debug_scratchpad = ram_console_start + ram_console_size;
}
#else
static void __init herring_fixup(struct machine_desc *desc,
                struct tag *tags, char **cmdline,
                struct meminfo *mi)
{
        mi->bank[0].start = 0x20000000;
        mi->bank[0].size = 256 * SZ_1M;

        mi->bank[1].start = 0x30000000;
        mi->bank[1].size = 256 * SZ_1M;

        //mi->bank[2].start = 0x50000000;
        /* 1M for ram_console buffer */
        //mi->bank[2].size = 127 * SZ_1M;
        mi->nr_banks = 2;

        //ram_console_start = mi->bank[2].start + mi->bank[2].size;
        //ram_console_size = SZ_1M - SZ_4K;

        //pm_debug_scratchpad = ram_console_start + ram_console_size;
}
#endif


/* this function are used to detect s5pc110 chip version temporally */
int s5pc110_version ;

void _hw_version_check(void)
{
	void __iomem *phy_address ;
	int temp;

	phy_address = ioremap(0x40, 1);

	temp = __raw_readl(phy_address);

	if (temp == 0xE59F010C)
		s5pc110_version = 0;
	else
		s5pc110_version = 1;

	printk(KERN_INFO "S5PC110 Hardware version : EVT%d\n",
				s5pc110_version);

	iounmap(phy_address);
}

/*
 * Temporally used
 * return value 0 -> EVT 0
 * value 1 -> evt 1
 */

int hw_version_check(void)
{
	return s5pc110_version ;
}
EXPORT_SYMBOL(hw_version_check);

static void herring_init_gpio(void)
{
	s3c_config_gpio_table();
	s3c_config_sleep_gpio_table(ARRAY_SIZE(herring_sleep_gpio_table),
			herring_sleep_gpio_table);
	if (herring_is_cdma_wimax_dev())
		s3c_config_sleep_gpio_table(
				ARRAY_SIZE(herring_cdma_wimax_sleep_gpio_table),
				herring_cdma_wimax_sleep_gpio_table);

}

static void __init fsa9480_gpio_init(void)
{
	if (herring_is_cdma_wimax_dev()) {
		s3c_gpio_cfgpin(GPIO_USB_HS_SEL, S3C_GPIO_OUTPUT);
		gpio_set_value(GPIO_USB_HS_SEL, 1);
	} else {
		s3c_gpio_cfgpin(GPIO_USB_SEL, S3C_GPIO_OUTPUT);
		s3c_gpio_setpull(GPIO_USB_SEL, S3C_GPIO_PULL_NONE);
	}

	s3c_gpio_cfgpin(GPIO_UART_SEL, S3C_GPIO_OUTPUT);
	s3c_gpio_setpull(GPIO_UART_SEL, S3C_GPIO_PULL_NONE);

	s3c_gpio_cfgpin(GPIO_JACK_nINT, S3C_GPIO_SFN(0xf));
	s3c_gpio_setpull(GPIO_JACK_nINT, S3C_GPIO_PULL_NONE);
}

static void __init setup_ram_console_mem(void)
{
	ram_console_resource[0].start = ram_console_start;
	ram_console_resource[0].end = ram_console_start + ram_console_size - 1;
}

static void __init sound_init(void)
{
	u32 reg;

	reg = __raw_readl(S5P_OTHERS);
	reg &= ~(0x3 << 8);
	reg |= 3 << 8;
	__raw_writel(reg, S5P_OTHERS);

	reg = __raw_readl(S5P_CLK_OUT);
	reg &= ~(0x1f << 12);
	reg |= 19 << 12;
	__raw_writel(reg, S5P_CLK_OUT);

	reg = __raw_readl(S5P_CLK_OUT);
	reg &= ~0x1;
	reg |= 0x1;
	__raw_writel(reg, S5P_CLK_OUT);

	gpio_request(GPIO_MICBIAS_EN, "micbias_enable");
}

static s8 accel_rotation_wimax_rev0[9] = {
	0, -1, 0,
	-1, 0, 0,
	0, 0, -1,
};

static void __init accel_init(void)
{
	if (herring_is_cdma_wimax_rev0())
		kr3dm_data.rotation = accel_rotation_wimax_rev0;
}

static bool console_flushed;

static void flush_console(void)
{
	if (console_flushed)
		return;

	console_flushed = true;

	printk("\n");
	pr_emerg("Restarting %s\n", linux_banner);
	if (!is_console_locked())
		return;

	mdelay(50);

	local_irq_disable();
	if (!console_trylock())
		pr_emerg("flush_console: console was locked! busting!\n");
	else
		pr_emerg("flush_console: console was locked!\n");
	console_unlock();
}

static void herring_pm_restart(char mode, const char *cmd)
{
	flush_console();

	/* On a normal reboot, INFORM6 will contain a small integer
	 * reason code from the notifier hook.  On a panic, it will
	 * contain the 0xee we set at boot.  Write 0xbb to differentiate
	 * a watchdog-timeout-and-reboot (0xee) from a controlled reboot
	 * (0xbb)
	 */
	if (__raw_readl(S5P_INFORM6) == 0xee)
		__raw_writel(0xbb, S5P_INFORM6);

	arm_machine_restart(mode, cmd);
}

static void __init herring_machine_init(void)
{
	arm_pm_restart = herring_pm_restart;

	setup_ram_console_mem();
	platform_add_devices(herring_devices, ARRAY_SIZE(herring_devices));
	if (!herring_is_tft_dev())
		platform_device_register(&herring_i2c5_device);

	/* Find out S5PC110 chip version */
	_hw_version_check();

	pm_power_off = herring_power_off ;

	s3c_gpio_cfgpin(GPIO_HWREV_MODE0, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_HWREV_MODE0, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_HWREV_MODE1, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_HWREV_MODE1, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_HWREV_MODE2, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_HWREV_MODE2, S3C_GPIO_PULL_NONE);
	HWREV = gpio_get_value(GPIO_HWREV_MODE0);
	HWREV = HWREV | (gpio_get_value(GPIO_HWREV_MODE1) << 1);
	HWREV = HWREV | (gpio_get_value(GPIO_HWREV_MODE2) << 2);
	s3c_gpio_cfgpin(GPIO_HWREV_MODE3, S3C_GPIO_INPUT);
	s3c_gpio_setpull(GPIO_HWREV_MODE3, S3C_GPIO_PULL_NONE);
	HWREV = HWREV | (gpio_get_value(GPIO_HWREV_MODE3) << 3);
	printk(KERN_INFO "HWREV is 0x%x\n", HWREV);

	/*initialise the gpio's*/
	herring_init_gpio();

#ifdef CONFIG_ANDROID_PMEM
	android_pmem_set_platdata();
#endif

	/* headset/earjack detection */
	if (system_rev >= 0x09)
		gpio_request(GPIO_EAR_MICBIAS_EN, "ear_micbias_enable");

	gpio_request(GPIO_TOUCH_EN, "touch en");

	/* i2c */
	s3c_i2c0_set_platdata(NULL);
#ifdef CONFIG_S3C_DEV_I2C1
	s3c_i2c1_set_platdata(NULL);
#endif

#ifdef CONFIG_S3C_DEV_I2C2
	s3c_i2c2_set_platdata(NULL);
#endif
	k3g_irq_init();
	set_adc_table();
	accel_init();
	/* H/W I2C lines */
	if (system_rev >= 0x05) {
		/* gyro sensor */
		if (herring_is_cdma_wimax_dev() && herring_is_cdma_wimax_rev0())
			i2c_register_board_info(5, i2c_devs0,
							ARRAY_SIZE(i2c_devs0));
		else
			i2c_register_board_info(0, i2c_devs0,
							ARRAY_SIZE(i2c_devs0));
		/* magnetic and accel sensor */
		i2c_register_board_info(1, i2c_devs1, ARRAY_SIZE(i2c_devs1));
	}
	mxt224_init();
	i2c_register_board_info(2, i2c_devs2, ARRAY_SIZE(i2c_devs2));

	/* wm8994 codec */
	sound_init();
	i2c_register_board_info(4, i2c_devs4, ARRAY_SIZE(i2c_devs4));
	/* accel sensor for rev04 */
	if (system_rev == 0x04)
		i2c_register_board_info(5, i2c_devs5, ARRAY_SIZE(i2c_devs5));

	if (herring_is_cdma_wimax_dev()) {
		struct max8998_platform_data *pdata =
			(struct max8998_platform_data *)&max8998_pdata;
		pdata->num_regulators =
			ARRAY_SIZE(herring_cdma_wimax_regulators);
		pdata->regulators = herring_cdma_wimax_regulators;
	}

	i2c_register_board_info(6, i2c_devs6, ARRAY_SIZE(i2c_devs6));
	if (!herring_is_tft_dev()) {
		/* Touch Key */
		touch_keypad_gpio_init();
		i2c_register_board_info(10, i2c_devs10, ARRAY_SIZE(i2c_devs10));
	} else {
		herring_virtual_keys_init();
	}
	/* FSA9480 */
	fsa9480_gpio_init();
	i2c_register_board_info(7, i2c_devs7, ARRAY_SIZE(i2c_devs7));

	/* gyro sensor for rev04 */
	if (system_rev == 0x04)
		i2c_register_board_info(8, i2c_devs8, ARRAY_SIZE(i2c_devs8));

	i2c_register_board_info(9, i2c_devs9, ARRAY_SIZE(i2c_devs9));
	/* optical sensor */
	gp2a_gpio_init();
	i2c_register_board_info(11, i2c_devs11, ARRAY_SIZE(i2c_devs11));
	/* magnetic sensor for rev04 */
	if (system_rev == 0x04)
		i2c_register_board_info(12, i2c_devs12, ARRAY_SIZE(i2c_devs12));

       /* nfc sensor */
	i2c_register_board_info(14, i2c_devs14, ARRAY_SIZE(i2c_devs14));

	/* max8893 wimax PMIC */
	if (herring_is_cdma_wimax_dev()) {
		platform_device_register(&herring_i2c15_device);
		i2c_register_board_info(15, i2c_devs15, ARRAY_SIZE(i2c_devs15));
	}

	if (!herring_is_tft_dev()) {
		spi_register_board_info(spi_board_info,
					ARRAY_SIZE(spi_board_info));
		s3cfb_set_platdata(&tl2796_data);
	} else {
		switch (lcd_type) {
		case 1:
			spi_register_board_info(spi_board_info_hydis,
					ARRAY_SIZE(spi_board_info_hydis));
			s3cfb_set_platdata(&nt35580_data);
			break;
		case 2:
			spi_register_board_info(spi_board_info_hitachi,
					ARRAY_SIZE(spi_board_info_hitachi));
			s3cfb_set_platdata(&r61408_data);
			break;
		default:
			spi_register_board_info(spi_board_info_sony,
					ARRAY_SIZE(spi_board_info_sony));
			s3cfb_set_platdata(&nt35580_data);
			break;
		}
	}

#if defined(CONFIG_S5P_ADC)
	s3c_adc_set_platdata(&s3c_adc_platform);
#endif

#if defined(CONFIG_PM)
	s3c_pm_init();
#endif

	s5ka3dfx_request_gpio();

	s5k4ecgx_init();

#ifdef CONFIG_VIDEO_FIMC
	/* fimc */
	s3c_fimc0_set_platdata(&fimc_plat_lsi);
	s3c_fimc1_set_platdata(&fimc_plat_lsi);
	s3c_fimc2_set_platdata(&fimc_plat_lsi);
#endif

#ifdef CONFIG_VIDEO_JPEG_V2
	s3c_jpeg_set_platdata(&jpeg_plat);
#endif

#ifdef CONFIG_VIDEO_MFC50
	/* mfc */
	s3c_mfc_set_platdata(NULL);
#endif

#ifdef CONFIG_S3C_DEV_HSMMC
	s5pv210_default_sdhci0();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC1
	s5pv210_default_sdhci1();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC2
	s5pv210_default_sdhci2();
#endif
#ifdef CONFIG_S3C_DEV_HSMMC3
	s5pv210_default_sdhci3();
#endif
#ifdef CONFIG_S5PV210_SETUP_SDHCI
	s3c_sdhci_set_platdata();
#endif

#ifdef CONFIG_CPU_FREQ
	s5pv210_cpufreq_set_platdata(&smdkc110_cpufreq_plat);
#endif

	regulator_has_full_constraints();

	register_reboot_notifier(&herring_reboot_notifier);

	herring_switch_init();

	gps_gpio_init();

	uart_switch_init();

	herring_init_wifi_mem();

	if (herring_is_cdma_wimax_dev())
		platform_device_register(&s3c_device_cmc732);

	/* write something into the INFORM6 register that we can use to
	 * differentiate an unclear reboot from a clean reboot (which
	 * writes a small integer code to INFORM6).
	 */
	__raw_writel(0xee, S5P_INFORM6);
}

#ifdef CONFIG_USB_SUPPORT
/* Initializes OTG Phy. */
void otg_phy_init(void)
{
	/* USB PHY0 Enable */
	writel(readl(S5P_USB_PHY_CONTROL) | (0x1<<0),
			S5P_USB_PHY_CONTROL);
	writel((readl(S3C_USBOTG_PHYPWR) & ~(0x3<<3) & ~(0x1<<0)) | (0x1<<5),
			S3C_USBOTG_PHYPWR);
	writel((readl(S3C_USBOTG_PHYCLK) & ~(0x5<<2)) | (0x3<<0),
			S3C_USBOTG_PHYCLK);
	writel((readl(S3C_USBOTG_RSTCON) & ~(0x3<<1)) | (0x1<<0),
			S3C_USBOTG_RSTCON);
	mdelay(1);
	writel(readl(S3C_USBOTG_RSTCON) & ~(0x7<<0),
			S3C_USBOTG_RSTCON);
	mdelay(1);

	/* rising/falling time */
	writel(readl(S3C_USBOTG_PHYTUNE) | (0x1<<20),
			S3C_USBOTG_PHYTUNE);

	/* set DC level as 6 (6%) */
	writel((readl(S3C_USBOTG_PHYTUNE) & ~(0xf)) | (0x1<<2) | (0x1<<1),
			S3C_USBOTG_PHYTUNE);
}
EXPORT_SYMBOL(otg_phy_init);

/* USB Control request data struct must be located here for DMA transfer */
struct usb_ctrlrequest usb_ctrl __attribute__((aligned(64)));

/* OTG PHY Power Off */
void otg_phy_off(void)
{
	writel(readl(S3C_USBOTG_PHYPWR) | (0x3<<3),
			S3C_USBOTG_PHYPWR);
	writel(readl(S5P_USB_PHY_CONTROL) & ~(1<<0),
			S5P_USB_PHY_CONTROL);
}
EXPORT_SYMBOL(otg_phy_off);

void usb_host_phy_init(void)
{
	struct clk *otg_clk;

	otg_clk = clk_get(NULL, "otg");
	clk_enable(otg_clk);

	if (readl(S5P_USB_PHY_CONTROL) & (0x1<<1))
		return;

	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL) | (0x1<<1),
			S5P_USB_PHY_CONTROL);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
			& ~(0x1<<7) & ~(0x1<<6)) | (0x1<<8) | (0x1<<5),
			S3C_USBOTG_PHYPWR);
	__raw_writel((__raw_readl(S3C_USBOTG_PHYCLK) & ~(0x1<<7)) | (0x3<<0),
			S3C_USBOTG_PHYCLK);
	__raw_writel((__raw_readl(S3C_USBOTG_RSTCON)) | (0x1<<4) | (0x1<<3),
			S3C_USBOTG_RSTCON);
	__raw_writel(__raw_readl(S3C_USBOTG_RSTCON) & ~(0x1<<4) & ~(0x1<<3),
			S3C_USBOTG_RSTCON);
}
EXPORT_SYMBOL(usb_host_phy_init);

void usb_host_phy_off(void)
{
	__raw_writel(__raw_readl(S3C_USBOTG_PHYPWR) | (0x1<<7)|(0x1<<6),
			S3C_USBOTG_PHYPWR);
	__raw_writel(__raw_readl(S5P_USB_PHY_CONTROL) & ~(1<<1),
			S5P_USB_PHY_CONTROL);
}
EXPORT_SYMBOL(usb_host_phy_off);
#endif

#if defined CONFIG_USB_S3C_OTG_HOST || defined CONFIG_USB_DWC_OTG

/* Initializes OTG Phy */
void otg_host_phy_init(void)
{
       __raw_writel(__raw_readl(S5P_USB_PHY_CONTROL)
               |(0x1<<0), S5P_USB_PHY_CONTROL); /*USB PHY0 Enable */
// from galaxy tab otg host:
       __raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
             &~(0x3<<3)&~(0x1<<0))|(0x1<<5), S3C_USBOTG_PHYPWR);
// from galaxy s2 otg host:
//     __raw_writel((__raw_readl(S3C_USBOTG_PHYPWR)
//           &~(0x7<<3)&~(0x1<<0)), S3C_USBOTG_PHYPWR);
       __raw_writel((__raw_readl(S3C_USBOTG_PHYCLK)
               &~(0x1<<4))|(0x7<<0), S3C_USBOTG_PHYCLK);

       __raw_writel((__raw_readl(S3C_USBOTG_RSTCON)
               &~(0x3<<1))|(0x1<<0), S3C_USBOTG_RSTCON);
       mdelay(1);
       __raw_writel((__raw_readl(S3C_USBOTG_RSTCON)
               &~(0x7<<0)), S3C_USBOTG_RSTCON);
       mdelay(1);

       __raw_writel((__raw_readl(S3C_UDC_OTG_GUSBCFG)
               |(0x3<<8)), S3C_UDC_OTG_GUSBCFG);

//     smb136_set_otg_mode(1);

       printk("otg_host_phy_int : USBPHYCTL=0x%x,PHYPWR=0x%x,PHYCLK=0x%x,USBCFG=0x%x\n",
               readl(S5P_USB_PHY_CONTROL),
               readl(S3C_USBOTG_PHYPWR),
               readl(S3C_USBOTG_PHYCLK),
               readl(S3C_UDC_OTG_GUSBCFG)
               );
}
EXPORT_SYMBOL(otg_host_phy_init);
#endif

MACHINE_START(HERRING, "herring")
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.fixup		= herring_fixup,
	.init_irq	= s5pv210_init_irq,
	.map_io		= herring_map_io,
	.init_machine	= herring_machine_init,
#ifdef CONFIG_S5P_HIGH_RES_TIMERS
	.timer		= &s5p_systimer,
#else
	.timer		= &s5p_timer,
#endif
MACHINE_END

void s3c_setup_uart_cfg_gpio(unsigned char port)
{
	switch (port) {
	case 0:
		s3c_gpio_cfgpin(GPIO_BT_RXD, S3C_GPIO_SFN(GPIO_BT_RXD_AF));
		s3c_gpio_setpull(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_TXD, S3C_GPIO_SFN(GPIO_BT_TXD_AF));
		s3c_gpio_setpull(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_CTS, S3C_GPIO_SFN(GPIO_BT_CTS_AF));
		s3c_gpio_setpull(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_BT_RTS, S3C_GPIO_SFN(GPIO_BT_RTS_AF));
		s3c_gpio_setpull(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_RXD, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_TXD, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_CTS, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_slp_cfgpin(GPIO_BT_RTS, S3C_GPIO_SLP_PREV);
		s3c_gpio_slp_setpull_updown(GPIO_BT_RTS, S3C_GPIO_PULL_NONE);
		break;
	case 1:
		s3c_gpio_cfgpin(GPIO_GPS_RXD, S3C_GPIO_SFN(GPIO_GPS_RXD_AF));
		s3c_gpio_setpull(GPIO_GPS_RXD, S3C_GPIO_PULL_UP);
		s3c_gpio_cfgpin(GPIO_GPS_TXD, S3C_GPIO_SFN(GPIO_GPS_TXD_AF));
		s3c_gpio_setpull(GPIO_GPS_TXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_GPS_CTS, S3C_GPIO_SFN(GPIO_GPS_CTS_AF));
		s3c_gpio_setpull(GPIO_GPS_CTS, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_GPS_RTS, S3C_GPIO_SFN(GPIO_GPS_RTS_AF));
		s3c_gpio_setpull(GPIO_GPS_RTS, S3C_GPIO_PULL_NONE);
		break;
	case 2:
		s3c_gpio_cfgpin(GPIO_AP_RXD, S3C_GPIO_SFN(GPIO_AP_RXD_AF));
		s3c_gpio_setpull(GPIO_AP_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_AP_TXD, S3C_GPIO_SFN(GPIO_AP_TXD_AF));
		s3c_gpio_setpull(GPIO_AP_TXD, S3C_GPIO_PULL_NONE);
		break;
	case 3:
		s3c_gpio_cfgpin(GPIO_FLM_RXD, S3C_GPIO_SFN(GPIO_FLM_RXD_AF));
		s3c_gpio_setpull(GPIO_FLM_RXD, S3C_GPIO_PULL_NONE);
		s3c_gpio_cfgpin(GPIO_FLM_TXD, S3C_GPIO_SFN(GPIO_FLM_TXD_AF));
		s3c_gpio_setpull(GPIO_FLM_TXD, S3C_GPIO_PULL_NONE);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(s3c_setup_uart_cfg_gpio);
