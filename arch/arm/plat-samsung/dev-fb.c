/* linux/arch/arm/plat-s3c/dev-fb.c
 *
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * S3C series device definition for framebuffer device
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/gfp.h>

#include <mach/irqs.h>
#include <mach/map.h>

#include <plat/fb.h>
#include <plat/devs.h>
#include <plat/media.h>
#include <plat/cpu.h>
#include <../../../drivers/video/samsung/s3cfb.h>
#include <mach/media.h>
#if 0
static struct resource s3c_fb_resource[] = {
	[0] = {
		.start = S3C_PA_FB,
		.end   = S3C_PA_FB + SZ_16K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = IRQ_LCD_VSYNC,
		.end   = IRQ_LCD_VSYNC,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
		.start = IRQ_LCD_FIFO,
		.end   = IRQ_LCD_FIFO,
		.flags = IORESOURCE_IRQ,
	},
	[3] = {
		.start = IRQ_LCD_SYSTEM,
		.end   = IRQ_LCD_SYSTEM,
		.flags = IORESOURCE_IRQ,
	},
};

struct platform_device s3c_device_fb = {
	.name		  = "s3c-fb",
	.id		  = -1,
	.num_resources	  = ARRAY_SIZE(s3c_fb_resource),
	.resource	  = s3c_fb_resource,
	.dev.dma_mask	  = &s3c_device_fb.dev.coherent_dma_mask,
	.dev.coherent_dma_mask = 0xffffffffUL,
};

void __init s3c_fb_set_platdata(struct s3c_fb_platdata *pd)
{
	struct s3c_fb_platdata *npd;

	if (!pd) {
		printk(KERN_ERR "%s: no platform data\n", __func__);
		return;
	}

	npd = kmemdup(pd, sizeof(struct s3c_fb_platdata), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);

	s3c_device_fb.dev.platform_data = npd;
}

static struct resource s3cfb_resource[] = {
        [0] = {
                .start = S5P_PA_LCD,
                .end   = S5P_PA_LCD + S5P_SZ_LCD - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_LCD1,
                .end   = IRQ_LCD1,
                .flags = IORESOURCE_IRQ,
        },
        [2] = {
                .start = IRQ_LCD0,
                .end   = IRQ_LCD0,
                .flags = IORESOURCE_IRQ,
        },
};

static u64 fb_dma_mask = 0xffffffffUL;

struct platform_device s3c_device_fb = {
        .name             = "s3cfb",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3cfb_resource),
        .resource         = s3cfb_resource,
        .dev              = {
                .dma_mask               = &fb_dma_mask,
                .coherent_dma_mask      = 0xffffffffUL
        }
};
#endif
static struct resource s3cfb_resource[] = {
        [0] = {
                .start = S5P_PA_LCD,
                .end   = S5P_PA_LCD + S5P_SZ_LCD - 1,
                .flags = IORESOURCE_MEM,
        },
        [1] = {
                .start = IRQ_LCD1,
                .end   = IRQ_LCD1,
                .flags = IORESOURCE_IRQ,
        },
        [2] = {
                .start = IRQ_LCD0,
                .end   = IRQ_LCD0,
                .flags = IORESOURCE_IRQ,
        },
};

static u64 fb_dma_mask = 0xffffffffUL;

struct platform_device s3c_device_fb = {
        .name             = "s3cfb",
        .id               = -1,
        .num_resources    = ARRAY_SIZE(s3cfb_resource),
        .resource         = s3cfb_resource,
        .dev              = {
                .dma_mask               = &fb_dma_mask,
                .coherent_dma_mask      = 0xffffffffUL
        }
};

static struct s3c_platform_fb default_fb_data __initdata = {
#if defined(CONFIG_CPU_S5PV210_EVT0)
        .hw_ver = 0x60,
#else
        .hw_ver = 0x62,
#endif
        .nr_wins = 5,
        .default_win = CONFIG_FB_S3C_DEFAULT_WINDOW,
        .swap = FB_SWAP_WORD | FB_SWAP_HWORD,
};
void __init s3c_fb_set_platdata(struct s3c_platform_fb *pd)
{
        struct s3c_platform_fb *npd;
        struct s3cfb_lcd *lcd;
        phys_addr_t pmem_start;
        int i, default_win, num_overlay_win;
        int frame_size;
        if (!pd)
                pd = &default_fb_data;

        npd = kmemdup(pd, sizeof(struct s3c_platform_fb), GFP_KERNEL);
        if (!npd)
                printk(KERN_ERR "%s: no memory for platform data\n", __func__);
        else {
                for (i = 0; i < npd->nr_wins; i++)
                        npd->nr_buffers[i] = 1;

                default_win = npd->default_win;
                num_overlay_win = CONFIG_FB_S3C_NUM_OVLY_WIN;

                if (num_overlay_win >= default_win) {
                        printk(KERN_WARNING "%s: NUM_OVLY_WIN should be less than default \
                                        window number. set to 0.\n", __func__);
                        num_overlay_win = 0;
                }
                for (i = 0; i < num_overlay_win; i++)
                        npd->nr_buffers[i] = CONFIG_FB_S3C_NUM_BUF_OVLY_WIN;
                npd->nr_buffers[default_win] = CONFIG_FB_S3C_NR_BUFFERS;

                lcd = (struct s3cfb_lcd *)npd->lcd;
                frame_size = (lcd->width * lcd->height * 4);

                s3cfb_get_clk_name(npd->clk_name);
                npd->backlight_onoff = NULL;
                npd->clk_on = s3cfb_clk_on;
                npd->clk_off = s3cfb_clk_off;

                /* set starting physical address & size of memory region for overlay
 *                  * window */
		printk("pmem_start is 0x%x\n",pmem_start);
                pmem_start = s5p_get_media_memory_bank(S5P_MDEV_FIMD, 1);
		printk("---pmem_start is 0x%x\n",pmem_start);
                for (i = 0; i < num_overlay_win; i++) {
                        npd->pmem_start[i] = pmem_start;
                        npd->pmem_size[i] = frame_size * npd->nr_buffers[i];
                        pmem_start +=npd->pmem_size[i];
                }

                /* set starting physical address & size of memory region for default
 *                  * window */
                npd->pmem_start[default_win] = pmem_start;
                npd->pmem_size[default_win] = frame_size * npd->nr_buffers[default_win];

                s3c_device_fb.dev.platform_data = npd;
        }
}
