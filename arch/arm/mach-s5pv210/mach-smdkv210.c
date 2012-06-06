/* linux/arch/arm/mach-s5pv210/mach-smdkv210.c
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
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/serial_core.h>
#include <linux/sysdev.h>
#include <linux/dm9000.h>
#include <linux/fb.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/pwm_backlight.h>
#include <linux/usb/ch9.h>
#include <linux/gpio_keys.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <video/platform_lcd.h>

#include <mach/map.h>
#include <mach/regs-clock.h>
#include <mach/regs-fb.h>

#include <plat/regs-serial.h>
#include <plat/regs-srom.h>
#include <plat/gpio-cfg.h>
#include <plat/s5pv210.h>
#include <plat/devs.h>
#include <plat/cpu.h>
#include <plat/adc.h>
#include <plat/ts.h>
#include <plat/ata.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/pm.h>
#include <plat/fb.h>
#include <plat/s5p-time.h>
#include <plat/sdhci.h>
#include <plat/fimc.h>
#include <plat/jpeg.h>
#include <plat/mfc.h>
#include <plat/clock.h> 
#include <plat/regs-otg.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#include <mach/regs-gpio.h>
#include <mach/gpio.h>
#ifdef CONFIG_ANDROID_PMEM
#include <linux/android_pmem.h>
#include <plat/media.h>
#include <mach/media.h>
#endif
#ifdef CONFIG_S5PV210_POWER_DOMAIN
#include <mach/power-domain.h>
#endif

#include <mach/gpio-smdkc110.h>

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDKV210_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDKV210_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDKV210_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)

static struct s3c2410_uartcfg smdkv210_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDKV210_UCON_DEFAULT,
		.ulcon		= SMDKV210_ULCON_DEFAULT,
		.ufcon		= SMDKV210_UFCON_DEFAULT,
	},
};

static struct s3c_ide_platdata smdkv210_ide_pdata __initdata = {
	.setup_gpio	= s5pv210_ide_setup_gpio,
};

static uint32_t smdkv210_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(0, 3, KEY_ESC), KEY(0, 4, KEY_1), KEY(0, 5, KEY_2),
	KEY(0, 6, KEY_3), KEY(0, 7, KEY_4),
	KEY(1, 4, KEY_5), KEY(1, 5, KEY_6), KEY(1, 6, KEY_7),
	KEY(1, 7, KEY_8)
};

static struct matrix_keymap_data smdkv210_keymap_data __initdata = {
	.keymap		= smdkv210_keymap,
	.keymap_size	= ARRAY_SIZE(smdkv210_keymap),
};

static struct samsung_keypad_platdata smdkv210_keypad_data __initdata = {
	.keymap_data	= &smdkv210_keymap_data,
	.rows		= 8,
	.cols		= 8,
};

static struct resource smdkv210_dm9000_resources[] = {
	[0] = {
		.start	= S5PV210_PA_SROM_BANK5,
		.end	= S5PV210_PA_SROM_BANK5,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= S5PV210_PA_SROM_BANK5 + 2,
		.end	= S5PV210_PA_SROM_BANK5 + 2,
		.flags	= IORESOURCE_MEM,
	},
	[2] = {
		.start	= IRQ_EINT(9),
		.end	= IRQ_EINT(9),
		.flags	= IORESOURCE_IRQ | IORESOURCE_IRQ_HIGHLEVEL,
	},
};

static struct dm9000_plat_data smdkv210_dm9000_platdata = {
	.flags		= DM9000_PLATF_16BITONLY | DM9000_PLATF_NO_EEPROM,
	.dev_addr	= { 0x00, 0x09, 0xc0, 0xff, 0xec, 0x48 },
};

struct platform_device smdkv210_dm9000 = {
	.name		= "dm9000",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(smdkv210_dm9000_resources),
	.resource	= smdkv210_dm9000_resources,
	.dev		= {
		.platform_data	= &smdkv210_dm9000_platdata,
	},
};

//#ifdef CONFIG_FB_S3C_LTE480WV
#define S5PV210_LCD_WIDTH 800
#define S5PV210_LCD_HEIGHT 480
//#endif

#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC0 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC1 (9900 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMC2 (6144 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC0 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_MFC1 (36864 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_FIMD (S5PV210_LCD_WIDTH * \
                                             S5PV210_LCD_HEIGHT * 4 * \
                                             (CONFIG_FB_S3C_NR_BUFFERS + \
                                                 (CONFIG_FB_S3C_NUM_OVLY_WIN * \
                                                  CONFIG_FB_S3C_NUM_BUF_OVLY_WIN))) //sayanta macro values need to be set
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_JPEG (8192 * SZ_1K)

/* 1920 * 1080 * 4 (RGBA)
 * - framesize == 1080p : 1920 * 1080 * 2(16bpp) * 2(double buffer) = 8MB
 * - framesize <  1080p : 1080 *  720 * 4(32bpp) * 2(double buffer) = under 8MB
 **/
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D (8192 * SZ_1K)
#define  S5PV210_VIDEO_SAMSUNG_MEMSIZE_TEXSTREAM (3000 * SZ_1K)
#define  S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1 (3300 * SZ_1K)


static struct s5p_media_device s5pv210_media_devs[] = {
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
        [7] = {
				.id = S5P_MDEV_TEXSTREAM,
				.name = "texstream",
				.bank = 1,
				.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_TEXSTREAM,
				.paddr = 0,
		},
		[8] = {
				.id = S5P_MDEV_PMEM_GPU1,
				.name = "pmem_gpu1",
				.bank = 0, /* OneDRAM */
				.memsize = S5PV210_ANDROID_PMEM_MEMSIZE_PMEM_GPU1,
				.paddr = 0,
		},
		[9] = {
				.id = S5P_MDEV_G2D,
				.name = "g2d",
				.bank = 0,
				.memsize = S5PV210_VIDEO_SAMSUNG_MEMSIZE_G2D,
				.paddr = 0,
		},
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


static void smdkv210_lte480wv_set_power(struct plat_lcd_data *pd,
					unsigned int power)
{
	if (power) {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(3), "GPD0");
		gpio_direction_output(S5PV210_GPD0(3), 1);
		gpio_free(S5PV210_GPD0(3));
#endif

		/* fire nRESET on power up */
		gpio_request(S5PV210_GPH0(6), "GPH0");

		gpio_direction_output(S5PV210_GPH0(6), 1);

		gpio_set_value(S5PV210_GPH0(6), 0);
		mdelay(10);

		gpio_set_value(S5PV210_GPH0(6), 1);
		mdelay(10);

		gpio_free(S5PV210_GPH0(6));
	} else {
#if !defined(CONFIG_BACKLIGHT_PWM)
		gpio_request(S5PV210_GPD0(3), "GPD0");
		gpio_direction_output(S5PV210_GPD0(3), 0);
		gpio_free(S5PV210_GPD0(3));
#endif
	}
}

static struct plat_lcd_data smdkv210_lcd_lte480wv_data = {
	.set_power	= smdkv210_lte480wv_set_power,
};

static struct platform_device smdkv210_lcd_lte480wv = {
	.name			= "platform-lcd",
	.dev.parent		= &s3c_device_fb.dev,
	.dev.platform_data	= &smdkv210_lcd_lte480wv_data,
};

#if 0
static struct s3c_fb_pd_win smdkv210_fb_win0 = {
	.win_mode = {
		.left_margin	= 13,
		.right_margin	= 8,
		.upper_margin	= 7,
		.lower_margin	= 5,
		.hsync_len	= 3,
		.vsync_len	= 1,
		.xres		= 800,
		.yres		= 480,
	},
	.max_bpp	= 32,
	.default_bpp	= 24,
};

static struct s3c_fb_platdata smdkv210_lcd0_pdata __initdata = {
	.win[0]		= &smdkv210_fb_win0,
	.vidcon0	= VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
	.vidcon1	= VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
	.setup_gpio	= s5pv210_fb_gpio_setup_24bpp,
};
#endif 
#define S5PV210_LCD_WIDTH  800
#define S5PV210_LCD_HEIGHT 480
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
        writel(0x2, S5P_MDNIE_SEL);

#ifndef CONFIG_FB_S3C_TL2796
        /* drive strength to max */
        writel(0xffffffff, S5PV210_GPF0_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF1_BASE + 0xc);
        writel(0xffffffff, S5PV210_GPF2_BASE + 0xc);
        writel(0x000000ff, S5PV210_GPF3_BASE + 0xc);
#else
        writel(0xC0, S5PV210_GPF0_BASE + 0xc);
#endif
}



#define S5PV210_GPD_0_0_TOUT_0  (0x2)
#define S5PV210_GPD_0_1_TOUT_1  (0x2 << 4)
#define S5PV210_GPD_0_2_TOUT_2  (0x2 << 8)
#define S5PV210_GPD_0_3_TOUT_3  (0x2 << 12)
static int lte480wv_backlight_on(struct platform_device *pdev)
{
        int err;
#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(4), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(4) for "
                "LVDS PWDN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(4), 1);
        gpio_set_value(S5PV210_GPB(4), 1);
        gpio_free(S5PV210_GPB(4));
        mdelay(100);
#endif
        err = gpio_request(S5PV210_GPD0(3), "GPD0");

        if (err) {
                printk(KERN_ERR "failed to request GPD0 for "
                        "lcd backlight control\n");
                return err;
        }

        gpio_direction_output(S5PV210_GPD0(3), 1);

        s3c_gpio_cfgpin(S5PV210_GPD0(3), S5PV210_GPD_0_3_TOUT_3);

        gpio_free(S5PV210_GPD0(3));

#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(5), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(5) for "
                "LED_EN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(5), 1);
        gpio_set_value(S5PV210_GPB(5), 1);
        gpio_free(S5PV210_GPB(5));
#endif
        return 0;
}


static int lte480wv_backlight_off(struct platform_device *pdev, int onoff)
{
        int err;

#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(5), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(5) for "
                "LED_EN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(5), 0);
        gpio_set_value(S5PV210_GPB(5), 0);
        gpio_free(S5PV210_GPB(5));
#endif

        err = gpio_request(S5PV210_GPD0(3), "GPD0");

        if (err) {
                printk(KERN_ERR "failed to request GPD0 for "
                                "lcd backlight control\n");
                return err;
        }

        gpio_direction_output(S5PV210_GPD0(3), 0);
        gpio_free(S5PV210_GPD0(3));

#if defined (CONFIG_FB_S3C_TL2796)
        err = gpio_request(S5PV210_GPB(4), "GPB");
        if (err) {
                printk(KERN_ERR "failed to request GPB(4) for "
                "LVDS PWDN pin\n");
                return err;
        }
        gpio_direction_output(S5PV210_GPB(4), 0);
        gpio_set_value(S5PV210_GPB(4), 0);
        gpio_free(S5PV210_GPB(4));
#endif
        return 0;
}


static int lte480wv_reset_lcd(struct platform_device *pdev)
{
#ifndef CONFIG_FB_S3C_TL2796
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
#endif
        return 0;
}

static struct s3c_platform_fb lte480wv_fb_data __initdata = {
        .hw_ver = 0x62,
        .clk_name       = "sclk_fimd",
        .nr_wins = 5,
        .default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
        .swap = FB_SWAP_WORD | FB_SWAP_HWORD,

        .lcd = &lte480wv,
        .cfg_gpio       = lte480wv_cfg_gpio,
        .backlight_on   = lte480wv_backlight_on,
        .backlight_onoff    = lte480wv_backlight_off,
        .reset_lcd      = lte480wv_reset_lcd,
};
static int smdkv210_backlight_init(struct device *dev)
{
	int ret;
	//need to check the calling function for this function and remove the call.
	return 0;

	ret = gpio_request(S5PV210_GPD0(3), "Backlight");
	if (ret) {
		printk(KERN_ERR "failed to request GPD for PWM-OUT 3\n");
		return ret;
	}

	/* Configure GPIO pin with S5PV210_GPD_0_3_TOUT_3 */
	s3c_gpio_cfgpin(S5PV210_GPD0(3), S3C_GPIO_SFN(2));

	return 0;
}

static void smdkv210_backlight_exit(struct device *dev)
{
	s3c_gpio_cfgpin(S5PV210_GPD0(3), S3C_GPIO_OUTPUT);
	gpio_free(S5PV210_GPD0(3));
}

static struct platform_pwm_backlight_data smdkv210_backlight_data = {
	.pwm_id		= 3,
	.max_brightness	= 255,
	.dft_brightness	= 255,
	.pwm_period_ns	= 2500000,
	.init		= smdkv210_backlight_init,
	.exit		= smdkv210_backlight_exit,
};

static struct platform_device smdkv210_backlight_device = {
	.name		= "pwm-backlight",
	.dev		= {
		.parent		= &s3c_device_timer[3].dev,
		.platform_data	= &smdkv210_backlight_data,
	},
};
static struct gpio_keys_button origen_gpio_keys_table[] = {
        {
                .code = KEY_MENU,
                .gpio = S5PV210_GPH0(4),
                .desc = "gpio-keys: KEY_MENU",
                .type = EV_KEY,
                .active_low = 1,
                .wakeup = 1,
                .debounce_interval = 1,
        },
#if 1
        {
                .code = 26,
                .gpio = S5PV210_GPH3(7),
                .desc = "gpio-keys: KEY_MENU",
                .type = EV_KEY,
                .active_low = 1,
                .wakeup = 1,
                .debounce_interval = 1,
        },
#endif
};

static struct gpio_keys_platform_data origen_gpio_keys_data = {
        .buttons        = origen_gpio_keys_table,
        .nbuttons       = ARRAY_SIZE(origen_gpio_keys_table),
};

static struct platform_device origen_device_gpiokeys = {
        .name = "gpio-keys",
        .dev = {
                .platform_data = &origen_gpio_keys_data,
        },
};
struct s3c_adc_mach_info {
        /* if you need to use some platform data, add in here*/
        int delay;
        int presc;
        int resolution;
};

static struct platform_device *smdkv210_devices[] __initdata = {
	&s3c_device_adc,
	&s3c_device_cfcon,
	&s3c_device_fb,
#ifdef CONFIG_VIDEO_JPEG_V2
	&s3c_device_jpeg,
#endif
#ifdef CONFIG_VIDEO_G2D
	&s3c_device_g2d,
#endif
    &s3c_device_g3d,
	&s3c_device_hsmmc0,
	&s3c_device_hsmmc1,
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_rtc,
	&s3c_device_ts,
	&s3c_device_wdt,
	&s5pv210_device_ac97,
#ifdef CONFIG_MTD_NAND
	&s3c_device_nand,
#endif
	&s5pv210_device_iis0,
	&s5pv210_device_spdif,
	&samsung_asoc_dma,
//	&s3c_device_keypad,

	&samsung_device_keypad,

	&smdkv210_dm9000,
#ifdef CONFIG_VIDEO_MFC50
	&s3c_device_mfc,
#endif
	&smdkv210_lcd_lte480wv,
	&s3c_device_timer[3],
	&smdkv210_backlight_device,

#ifdef CONFIG_USB_EHCI_HCD
	&s3c_device_usb_ehci,
#endif
#ifdef CONFIG_USB_OHCI_HCD
	&s3c_device_usb_ohci,
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

#ifdef CONFIG_VIDEO_FIMC
    &s3c_device_fimc0,
    &s3c_device_fimc1,
    &s3c_device_fimc2,
#endif
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
};
#ifdef CAM_ITU_CH_A
static int smdkv210_cam0_power(int onoff)
{
	int err;
	/* Camera A */
	err = gpio_request(GPIO_PS_VOUT, "GPH0");
	if (err)
		printk(KERN_ERR "failed to request GPH0 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_PS_VOUT, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_PS_VOUT, 0);
	gpio_direction_output(GPIO_PS_VOUT, 1);
	gpio_free(GPIO_PS_VOUT);

	return 0;
}
#else
static int smdkv210_cam1_power(int onoff)
{
	int err;

	/* S/W workaround for the SMDK_CAM4_type board
	 * When SMDK_CAM4 type module is initialized at power reset,
	 * it needs the cam_mclk.
	 *
	 * Now cam_mclk is set as below, need only set the gpio mux.
	 * CAM_SRC1 = 0x0006000, CLK_DIV1 = 0x00070400.
	 * cam_mclk source is SCLKMPLL, and divider value is 8.
	*/

	/* gpio mux select the cam_mclk */
	err = gpio_request(GPIO_PS_ON, "GPJ1");
	if (err)
		printk(KERN_ERR "failed to request GPJ1 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_PS_ON, S3C_GPIO_PULL_NONE);
	s3c_gpio_cfgpin(GPIO_PS_ON, (0x3<<16));


	/* Camera B */
	err = gpio_request(GPIO_BUCK_1_EN_A, "GPH0");
	if (err)
		printk(KERN_ERR "failed to request GPH0 for CAM_2V8\n");

	s3c_gpio_setpull(GPIO_BUCK_1_EN_A, S3C_GPIO_PULL_NONE);
	gpio_direction_output(GPIO_BUCK_1_EN_A, 0);
	gpio_direction_output(GPIO_BUCK_1_EN_A, 1);

	udelay(1000);

	gpio_free(GPIO_PS_ON);
	gpio_free(GPIO_BUCK_1_EN_A);

	return 0;
}
#endif
static int s5k5ba_power_en(int onoff)
{
	if (onoff) {
#ifdef CAM_ITU_CH_A
		smdkv210_cam0_power(onoff);
#else
		smdkv210_cam1_power(onoff);
#endif
	} else {
#ifdef CAM_ITU_CH_A
		smdkv210_cam0_power(onoff);
#else
		smdkv210_cam1_power(onoff);
#endif
	}

	return 0;
}

#ifdef CONFIG_VIDEO_S5K4EA
/* Set for MIPI-CSI Camera module Power Enable */
static int smdkv210_mipi_cam_pwr_en(int enabled)
{
	int err;

	err = gpio_request(S5PV210_GPH1(2), "GPH1");
	if (err)
		printk(KERN_ERR "#### failed to request(GPH1)for CAM_2V8\n");

	s3c_gpio_setpull(S5PV210_GPH1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPH1(2), enabled);
	gpio_free(S5PV210_GPH1(2));

	return 0;
}

/* Set for MIPI-CSI Camera module Reset */
static int smdkv210_mipi_cam_rstn(int enabled)
{
	int err;

	err = gpio_request(S5PV210_GPH0(3), "GPH0");
	if (err)
		printk(KERN_ERR "#### failed to reset(GPH0) for MIPI CAM\n");

	s3c_gpio_setpull(S5PV210_GPH0(3), S3C_GPIO_PULL_NONE);
	gpio_direction_output(S5PV210_GPH0(3), enabled);
	gpio_free(S5PV210_GPH0(3));

	return 0;
}

/* MIPI-CSI Camera module Power up/down sequence */
static int smdkv210_mipi_cam_power(int on)
{
	if (on) {
		smdkv210_mipi_cam_pwr_en(1);
		mdelay(5);
		smdkv210_mipi_cam_rstn(1);
	} else {
		smdkv210_mipi_cam_rstn(0);
		mdelay(5);
		smdkv210_mipi_cam_pwr_en(0);
	}
	return 0;
}
#endif

#ifdef CONFIG_VIDEO_S5K4BA
static struct s5k4ba_platform_data s5k4ba_plat = {
	.default_width = 640,
	.default_height = 480,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 44000000,
	.is_mipi = 0,
};

static struct i2c_board_info  s5k4ba_i2c_info = {
	I2C_BOARD_INFO("S5K4BA", 0x2d),
	.platform_data = &s5k4ba_plat,
};

static struct s3c_platform_camera s5k4ba = {
	#ifdef CAM_ITU_CH_A
	.id		= CAMERA_PAR_A,
	#else
	.id		= CAMERA_PAR_B,
	#endif
	.type		= CAM_TYPE_ITU,
	.fmt		= ITU_601_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &s5k4ba_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	/* .srclk_name	= "xusbxti", */
	.clk_name	= "sclk_cam1",
	.clk_rate	= 44000000,
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
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= s5k5ba_power_en,
};
#endif

/* 2 MIPI Cameras */
#ifdef CONFIG_VIDEO_S5K4EA
static struct s5k4ea_platform_data s5k4ea_plat = {
	.default_width = 1920,
	.default_height = 1080,
	.pixelformat = V4L2_PIX_FMT_UYVY,
	.freq = 24000000,
	.is_mipi = 1,
};

static struct i2c_board_info  s5k4ea_i2c_info = {
	I2C_BOARD_INFO("S5K4EA", 0x2d),
	.platform_data = &s5k4ea_plat,
};

static struct s3c_platform_camera s5k4ea = {
	.id		= CAMERA_CSI_C,
	.type		= CAM_TYPE_MIPI,
	.fmt		= MIPI_CSI_YCBCR422_8BIT,
	.order422	= CAM_ORDER422_8BIT_CBYCRY,
	.i2c_busnum	= 0,
	.info		= &s5k4ea_i2c_info,
	.pixelformat	= V4L2_PIX_FMT_UYVY,
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_cam0",
	/* .clk_name	= "sclk_cam1", */
	.clk_rate	= 48000000,
	.line_length	= 1920,
	.width		= 1920,
	.height		= 1080,
	.window		= {
		.left	= 0,
		.top	= 0,
		.width	= 1920,
		.height	= 1080,
	},

	.mipi_lanes	= 2,
	.mipi_settle	= 12,
	.mipi_align	= 32,

	/* Polarity */
	.inv_pclk	= 0,
	.inv_vsync	= 1,
	.inv_href	= 0,
	.inv_hsync	= 0,

	.initialized	= 0,
	.cam_power	= smdkv210_mipi_cam_power,
};
#endif

/* Interface setting */
static struct s3c_platform_fimc fimc_plat_lsi = {
	.srclk_name	= "mout_mpll",
	.clk_name	= "sclk_fimc",
	.lclk_name	= "fimc",
	.clk_rate	= 166750000,
#if defined(CONFIG_VIDEO_S5K4EA)
	.default_cam	= CAMERA_CSI_C,
#else
#ifdef CAM_ITU_CH_A
	.default_cam	= CAMERA_PAR_A,
#else
	.default_cam	= CAMERA_PAR_B,
#endif
#endif
	.camera		= {

#ifdef CONFIG_VIDEO_S5K4BA
			&s5k4ba,
#endif
#ifdef CONFIG_VIDEO_S5K4EA
			&s5k4ea,
#endif
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

static void __init smdkv210_dm9000_init(void)
{
	unsigned int tmp;

	gpio_request(S5PV210_MP01(5), "nCS5");
	s3c_gpio_cfgpin(S5PV210_MP01(5), S3C_GPIO_SFN(2));
	gpio_free(S5PV210_MP01(5));

	tmp = (5 << S5P_SROM_BCX__TACC__SHIFT);
	__raw_writel(tmp, S5P_SROM_BC5);

	tmp = __raw_readl(S5P_SROM_BW);
	tmp &= (S5P_SROM_BW__CS_MASK << S5P_SROM_BW__NCS5__SHIFT);
	tmp |= (1 << S5P_SROM_BW__NCS5__SHIFT);
	__raw_writel(tmp, S5P_SROM_BW);
}

static struct i2c_board_info smdkv210_i2c_devs0[] __initdata = {
	{ I2C_BOARD_INFO("24c08", 0x50), },     /* Samsung S524AD0XD1 */
	{ I2C_BOARD_INFO("wm8580", 0x1b), },
	/* hcj needs modification */
#ifdef	CONFIG_TOUCHSCREEN_GOODIX

	{ 
		I2C_BOARD_INFO("Goodix-TS", 0x55),
        .irq = IRQ_EINT(4), 
    },
#endif
};

static struct i2c_board_info smdkv210_i2c_devs1[] __initdata = {
	/* To Be Updated */
#ifdef CONFIG_VIDEO_TV20
	{
		I2C_BOARD_INFO("s5p_ddc", (0x74>>1)),
	},
#endif
};

static struct i2c_board_info smdkv210_i2c_devs2[] __initdata = {

#ifdef CONFIG_SENSORS_BMA150
	{
		I2C_BOARD_INFO("bma150", 0x38),
	},
#endif
	/* To Be Updated */
};

static struct s3c2410_ts_mach_info s3c_ts_platform __initdata = {
	.delay			= 10000,
	.presc			= 49,
	.oversampling_shift	= 2,
	.cal_x_max              = 800,
        .cal_y_max              = 480,
        .cal_param              = {
                -13357, -85, 53858048, -95, -8493, 32809514, 65536
        },

};


static void __init smdkv210_map_io(void)
{
	s5p_init_io(NULL, 0, S5P_VA_CHIPID);
	s3c24xx_init_clocks(24000000);
	s5pv210_gpiolib_init();
	s3c24xx_init_uarts(smdkv210_uartcfgs, ARRAY_SIZE(smdkv210_uartcfgs));
	s5p_set_timer_source(S5P_PWM2, S5P_PWM4);
	/* hcj: change from S5P_RANGE_MFC to 0 */
	s5p_reserve_bootmem(s5pv210_media_devs,
                        ARRAY_SIZE(s5pv210_media_devs), 0);
#ifdef CONFIG_MTD_NAND
	s3c_device_nand.name = "s5pv210-nand";
#endif
}

unsigned int pm_debug_scratchpad;

static void __init sound_init(void)
{
	u32 reg;

	reg = __raw_readl(S5P_CLK_OUT);
	reg &= ~(0x1f << 12);
	reg &= ~(0xf << 20);
	reg |= 0x12 << 12;
	reg |= 0x1  << 20;
	__raw_writel(reg, S5P_CLK_OUT);

	reg = __raw_readl(S5P_OTHERS);
	reg &= ~(0x3 << 8);
	reg |= 0x0 << 8;
	__raw_writel(reg, S5P_OTHERS);
}


#ifdef CONFIG_S3C_DEV_HSMMC
static struct s3c_sdhci_platdata smdkc110_hsmmc0_pdata __initdata = {
        .cd_type                = S3C_SDHCI_CD_INTERNAL,
        //.wp_gpio                = S5PV210_GPH0(7),
        //.has_wp_gpio    = true,
#if defined(CONFIG_S5PV210_SD_CH0_8BIT)
        .max_width              = 8,
        .host_caps              = MMC_CAP_8_BIT_DATA,
#endif
};      
#endif
 
static void __init smdkc110_setup_clocks(void)
{
        struct clk *pclk;
        struct clk *clk;

        /* set MMC0 clock */
        clk = clk_get(&s3c_device_hsmmc0.dev, "sclk_mmc");
        pclk = clk_get(NULL, "mout_mpll");
        clk_set_parent(clk, pclk);
        clk_set_rate(clk, 50*MHZ);

        pr_info("%s: %s: source is %s, rate is %ld\n",
                                __func__, clk->name, clk->parent->name,
                                clk_get_rate(clk));
}

#if defined(CONFIG_KEYPAD_S3C) || defined(CONFIG_KEYPAD_S3C_MODULE) || defined(CONFIG_KEYBOARD_SAMSUNG)
#if defined(CONFIG_KEYPAD_S3C_MSM)
void s3c_setup_keypad_cfg_gpio(void)
{
	unsigned int gpio;
	unsigned int end;

	/* gpio setting for KP_COL0 */
	s3c_gpio_cfgpin(S5PV210_GPJ1(5), S3C_GPIO_SFN(3));
	s3c_gpio_setpull(S5PV210_GPJ1(5), S3C_GPIO_PULL_NONE);

	/* gpio setting for KP_COL1 ~ KP_COL7 and KP_ROW0 */
	end = S5PV210_GPJ2(8);
	for (gpio = S5PV210_GPJ2(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	/* gpio setting for KP_ROW1 ~ KP_ROW8 */
	end = S5PV210_GPJ3(8);
	for (gpio = S5PV210_GPJ3(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}

	/* gpio setting for KP_ROW9 ~ KP_ROW13 */
	end = S5PV210_GPJ4(5);
	for (gpio = S5PV210_GPJ4(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}
#else
void s3c_setup_keypad_cfg_gpio(int rows, int columns)
{
	unsigned int gpio;
	unsigned int end;

	end = S5PV210_GPH3(rows);

	/* Set all the necessary GPH2 pins to special-function 0 */
	for (gpio = S5PV210_GPH3(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
	}

	end = S5PV210_GPH2(columns);

	/* Set all the necessary GPK pins to special-function 0 */
	for (gpio = S5PV210_GPH2(0); gpio < end; gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
	}
}
#endif /* if defined(CONFIG_KEYPAD_S3C_MSM)*/
EXPORT_SYMBOL(s3c_setup_keypad_cfg_gpio);
#endif
/* hcj for mem setup */
static void __init smdkv210_fixup(struct machine_desc *desc,
		struct tag *tags, char **cmdline,
		struct meminfo *mi)
{
	mi->bank[0].start = 0x20000000;
	mi->bank[0].size = 512 * SZ_1M;
	
    mi->nr_banks = 1;
}

//extern void s3c_fb_set_platdata(struct s3c_platform_fb *fimd);

static void __init smdkv210_machine_init(void)
{
	s3c_pm_init();

	smdkv210_dm9000_init();
	
#ifdef CONFIG_ANDROID_PMEM
    android_pmem_set_platdata();
#endif

	samsung_keypad_set_platdata(&smdkv210_keypad_data);
	s3c24xx_ts_set_platdata(&s3c_ts_platform);

	s3c_i2c0_set_platdata(NULL);
	s3c_i2c1_set_platdata(NULL);
	s3c_i2c2_set_platdata(NULL);
	i2c_register_board_info(0, smdkv210_i2c_devs0,
			ARRAY_SIZE(smdkv210_i2c_devs0));
	i2c_register_board_info(1, smdkv210_i2c_devs1,
			ARRAY_SIZE(smdkv210_i2c_devs1));
	i2c_register_board_info(2, smdkv210_i2c_devs2,
			ARRAY_SIZE(smdkv210_i2c_devs2));

	s3c_ide_set_platdata(&smdkv210_ide_pdata);

//	s3c_fb_set_platdata(&smdkv210_lcd0_pdata);
	s3c_fb_set_platdata(&lte480wv_fb_data);
    platform_add_devices(smdkv210_devices, ARRAY_SIZE(smdkv210_devices));
    sound_init();

#ifdef CONFIG_S3C_DEV_HSMMC
        s3c_sdhci0_set_platdata(&smdkc110_hsmmc0_pdata);
#endif

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



	 smdkc110_setup_clocks(); 
}

#ifdef CONFIG_USB_SUPPORT
/* Initializes OTG Phy. */
void otg_host_phy_init(void)
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
	msleep(1);
	writel(readl(S3C_USBOTG_RSTCON) & ~(0x7<<0),
			S3C_USBOTG_RSTCON);
	msleep(1);

	/* rising/falling time */
	writel(readl(S3C_USBOTG_PHYTUNE) | (0x1<<20),
			S3C_USBOTG_PHYTUNE);

	/* set DC level as 6 (6%) */
	writel((readl(S3C_USBOTG_PHYTUNE) & ~(0xf)) | (0x1<<2) | (0x1<<1),
			S3C_USBOTG_PHYTUNE);
}
EXPORT_SYMBOL(otg_host_phy_init);

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

MACHINE_START(SMDKV210, "SMDKV210")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.boot_params	= S5P_PA_SDRAM + 0x100,
	.fixup         = smdkv210_fixup,
	.init_irq	= s5pv210_init_irq,
	.map_io		= smdkv210_map_io,
	.init_machine	= smdkv210_machine_init,
	.timer		= &s5p_timer,
MACHINE_END
