/*
 * (C) Copyright 2011 Samsung Electronics Co. Ltd
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
 
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/gpio.h>

unsigned int OmPin;

DECLARE_GLOBAL_DATA_PTR;
extern int nr_dram_banks;
unsigned int second_boot_info = 0xffffffff;

int board_init(void)
{
	/* check half synchronizer for asynchronous bridge */
	if(*(unsigned int *)(0x10010350) == 0x1)
		printf("Using half synchronizer for asynchronous bridge\n");

	gd->bd->bi_arch_number = MACH_TYPE;
	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

   	OmPin = INF_REG3_REG;
	printf("\n\nChecking Boot Mode ...");
	if(OmPin == BOOT_ONENAND) {
		printf(" OneNand\n");
	} else if (OmPin == BOOT_NAND) {
		printf(" NAND\n");
	} else if (OmPin == BOOT_MMCSD) {
		printf(" SDMMC\n");
	} else if (OmPin == BOOT_EMMC) {
		printf(" EMMC4.3\n");
	} else if (OmPin == BOOT_EMMC_4_4) {
		printf(" EMMC4.41\n");
	}

	return 0;
}

int dram_init(void)
{
	//gd->ram_size = get_ram_size((long *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	
	return 0;
}

void dram_init_banksize(void)
{
	nr_dram_banks = CONFIG_NR_DRAM_BANKS;
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;
	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
	gd->bd->bi_dram[2].start = PHYS_SDRAM_3;
	gd->bd->bi_dram[2].size = PHYS_SDRAM_3_SIZE;
	gd->bd->bi_dram[3].start = PHYS_SDRAM_4;
	gd->bd->bi_dram[3].size = PHYS_SDRAM_4_SIZE;

#ifdef CONFIG_RESERVED_DRAM
	gd->bd->bi_dram[nr_dram_banks - 1].size -= CONFIG_RESERVED_DRAM;
#endif
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;
#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("Board:\tORIGEN\n");
	
	return 0;

}
#endif

int board_late_init (void)
{
	char *var;
#ifdef CONFIG_BOOTCMD_NORMAL
	setenv("bootcmd_normal", CONFIG_BOOTCMD_NORMAL);
#endif
#ifdef CONFIG_BOOTCMD_EXTEND
	setenv("bootcmd_extend", CONFIG_BOOTCMD_EXTEND);
#endif

#ifdef CONFIG_CPU_EXYNOS4X12
	if(INF_REG4_REG == 0xf)
		run_command(CONFIG_FACTORYRESET, NULL);
#endif

	if (second_boot_info == 1) {
		second_boot_info = 0;
		printf("Booted with secondary booting device\n");
		run_command("emmc open 0", NULL);
		run_command("movi r fwbl1 1 50000000; movi w z fwbl1 0 50000000", NULL);
		run_command("movi r bl2 1 50000000; movi w z bl2 0 50000000", NULL);
		run_command("movi r u-boot 1 50000000; movi w z u-boot 0 50000000", NULL);
		run_command("emmc close 0", NULL);
		run_command("reset", NULL);
	}
	return 0;
}

int board_mmc_init(bd_t *bis)
{
#ifdef CONFIG_S3C_HSMMC
	setup_hsmmc_clock();
	setup_hsmmc_cfg_gpio();
#ifdef USE_MMC4
	if (OmPin == BOOT_EMMC_4_4 || OmPin == BOOT_EMMC) {
		smdk_s5p_mshc_init();
		smdk_s3c_hsmmc_init();
	} else {
		smdk_s3c_hsmmc_init();
		smdk_s5p_mshc_init();
	}
#else
	smdk_s3c_hsmmc_init();
#endif
#endif
	return 0;
}

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_s5pv310(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xe0000000))
		return (addr - 0xc0000000 + 0x40000000);

	return addr;
}
#endif

