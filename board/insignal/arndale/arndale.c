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
#include <asm/arch/pmic.h>

unsigned int OmPin;

DECLARE_GLOBAL_DATA_PTR;
extern int nr_dram_banks;
unsigned int second_boot_info = 0xffffffff;

/*
 * Miscellaneous platform dependent initialisations
 */
static void smc9115_pre_init(void)
{
	unsigned int cs1;
	/* gpio configuration */
	writel(0x00220020, 0x11400000 + 0x1A0);
	writel(0x00002222, 0x11400000 + 0x1C0);

	/* 16 Bit bus width */
	writel(0x22222222, 0x11400000 + 0x200);
	writel(0x0000FFFF, 0x11400000 + 0x208);
	writel(0x22222222, 0x11400000 + 0x240);
	writel(0x0000FFFF, 0x11400000 + 0x248);
	writel(0x22222222, 0x11400000 + 0x260);
	writel(0x0000FFFF, 0x11400000 + 0x268);

	/* SROM BANK1 */
	cs1 = SROM_BW_REG & ~(0xF<<4);
	cs1 |= ((1 << 0) |
		(0 << 2) |
		(1 << 3)) << 4;

	SROM_BW_REG = cs1;

	/* set timing for nCS1 suitable for ethernet chip */
	SROM_BC1_REG = ((0x1 << 0) |
		     (0x9 << 4) |
		     (0xc << 8) |
		     (0x1 << 12) |
		     (0x6 << 16) |
		     (0x1 << 24) |
		     (0x1 << 28));
}

int board_init(void)
{
	u8 read_vol_arm;
	u8 read_vol_int;
	u8 read_vol_g3d;
	u8 read_vol_mif;
	u8 read_vol_mem;
	u8 read_vol_apll;
	u8 pmic_id;
	char bl1_version[9] = {0};

	/* display BL1 version */
#ifdef CONFIG_TRUSTZONE
	printf("\nTrustZone Enabled BSP");
	strncpy(&bl1_version[0], (char *)0x0204f810, 8);
#else
	strncpy(&bl1_version[0], (char *)0x02022fc8, 8);
#endif
	printf("\nBL1 version: %s\n", &bl1_version[0]);

#if defined(CONFIG_PM) && !defined(CONFIG_S5M8767)
	/* read ID */
	IIC0_ERead(MAX8997_ADDR, MAX8997_ID, &pmic_id);

	if (pmic_id == 0x77) {
		/* MAX8997 */
		printf("PMIC: MAX8997\n");
		IIC0_ERead(MAX8997_ADDR, MAX8997_BUCK1TV_DVS, &read_vol_arm);
		IIC0_ERead(MAX8997_ADDR, MAX8997_BUCK2TV_DVS, &read_vol_int);
		IIC0_ERead(MAX8997_ADDR, MAX8997_BUCK3TV_DVS, &read_vol_g3d);
		IIC0_ERead(MAX8997_ADDR, MAX8997_BUCK4TV_DVS, &read_vol_mif);
		IIC0_ERead(MAX8997_ADDR, MAX8997_LDO10CTRL, &read_vol_apll);

		printf("ARM: %dmV\t", ((unsigned int)read_vol_arm * 25) + 650);
		printf("INT: %dmV\t", ((unsigned int)read_vol_int * 25) + 650);
		printf("G3D: %dmV\n", ((unsigned int)read_vol_g3d * 50) + 750);
		printf("MIF: %dmV\t", ((unsigned int)read_vol_mif * 25) + 650);
		printf("APLL: %dmV\n",	((unsigned int)(read_vol_apll & 0x3F)
					* 50) + 800);
	} else if (pmic_id >= 0x0 && pmic_id <= 0x5) {
		/* S5M8767 */
		printf("PMIC: S5M8767\n");
	} else {
		/* MAX77686 */
		printf("PMIC: MAX77686\n");
		IIC0_ERead(MAX77686_ADDR, MAX77686_BUCK2TV_DVS1, &read_vol_arm);
		IIC0_ERead(MAX77686_ADDR, MAX77686_BUCK3TV_DVS1, &read_vol_int);
		IIC0_ERead(MAX77686_ADDR, MAX77686_BUCK4TV_DVS1, &read_vol_g3d);
		IIC0_ERead(MAX77686_ADDR, MAX77686_BUCK1OUT, &read_vol_mif);
		IIC0_ERead(MAX77686_ADDR, MAX77686_BUCK5OUT, &read_vol_mem);

		printf("ARM: %dmV\t", ((unsigned int)(read_vol_arm >> 1) * 25) + 600);
		printf("INT: %dmV\t", ((unsigned int)(read_vol_int >> 1) * 25) + 600);
		printf("G3D: %dmV\n", ((unsigned int)(read_vol_g3d >> 1)* 25) + 600);
		printf("MIF: %dmV\t", ((unsigned int)(read_vol_mif & 0x3F) * 50) + 750);
		printf("MEM: %dmV\n", ((unsigned int)(read_vol_mem & 0x3F) * 50) + 750);

	}
#endif

#ifdef CONFIG_SMC911X
	smc9115_pre_init();
#endif

#ifdef CONFIG_CPU_EXYNOS5250_EVT1
	*(unsigned int *)0x10050234 = 0;
#endif

	gd->bd->bi_arch_number = MACH_TYPE;

	gd->bd->bi_boot_params = (PHYS_SDRAM_1+0x100);

	OmPin = INF_REG3_REG;
	printf("\nChecking Boot Mode ...");
	if (OmPin == BOOT_ONENAND) {
		printf(" OneNand\n");
	} else if (OmPin == BOOT_NAND) {
		printf(" NAND\n");
	} else if (OmPin == BOOT_MMCSD) {
		printf(" SDMMC\n");
	} else if (OmPin == BOOT_EMMC) {
		printf(" EMMC4.3\n");
	} else if (OmPin == BOOT_EMMC_4_4) {
		printf(" EMMC4.41\n");
	} else {
		printf(" Please check OM_pin\n");
	}

	return 0;
}

int dram_init(void)
{
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
		gd->bd->bi_dram[4].start = PHYS_SDRAM_5;
		gd->bd->bi_dram[4].size = PHYS_SDRAM_5_SIZE;
		gd->bd->bi_dram[5].start = PHYS_SDRAM_6;
		gd->bd->bi_dram[5].size = PHYS_SDRAM_6_SIZE;
		gd->bd->bi_dram[6].start = PHYS_SDRAM_7;
		gd->bd->bi_dram[6].size = PHYS_SDRAM_7_SIZE;
		gd->bd->bi_dram[7].start = PHYS_SDRAM_8;
		gd->bd->bi_dram[7].size = PHYS_SDRAM_8_SIZE;

#ifdef CONFIG_TRUSTZONE
	gd->bd->bi_dram[nr_dram_banks - 1].size -= CONFIG_TRUSTZONE_RESERVED_DRAM;
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
	printf("Board:\tSMDK5250\n");

	return 0;

}
#endif

int board_late_init(void)
{
#ifdef CONFIG_BOOTCMD_NORMAL
	setenv("bootcmd_normal", CONFIG_BOOTCMD_NORMAL);
#endif
#ifdef CONFIG_BOOTCMD_EXTEND
	setenv("bootcmd_extend", CONFIG_BOOTCMD_EXTEND);
#endif

	if (second_boot_info == 1) {
		printf("###Secondary Boot###\n");
		run_command(CONFIG_BOOTCOMMAND_ERASE, NULL);
		run_command(CONFIG_BOOTCOMMAND_FUSE_BOOT, NULL);
		//run_command(CONFIG_BOOTCOMMAND_PARTITION, NULL);
	}

	int *GPX1DAT = 0x11400C24;
	if ((*GPX1DAT & 0xF0) == 0x60) {
		setenv("bootcmd", CONFIG_BOOTCMD_EXTEND);
	}

	if(INF_REG4_REG == CONFIG_FACTORY_RESET_MODE)
		setenv ("bootcmd", CONFIG_FACTORY_RESET_BOOTCOMMAND);

	return 0;
}

int board_mmc_init(bd_t *bis)
{
#ifdef CONFIG_S3C_HSMMC
	setup_hsmmc_clock();
	setup_hsmmc_cfg_gpio();
#if defined(CONFIG_CPU_EXYNOS5250_EVT1)
	smdk_s5p_mshc_init();
#else
	if (OmPin == BOOT_EMMC_4_4 || OmPin == BOOT_EMMC) {
		smdk_s5p_mshc_init();
		smdk_s3c_hsmmc_init();
	} else {
		smdk_s3c_hsmmc_init();
		smdk_s5p_mshc_init();
	}
#endif
#endif
	return 0;
}

#ifdef CONFIG_ENABLE_MMU
ulong virt_to_phy_exynos5250(ulong addr)
{
	if ((0xc0000000 <= addr) && (addr < 0xe0000000))
		return (ulong)(addr - 0xc0000000 + 0x40000000);

	return addr;
}
#endif

