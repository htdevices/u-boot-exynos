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
#include <asm/arch/pmic.h>
#include <asm/arch/cpu.h>

void Delay(void)
{
	unsigned long i;
	for(i=0;i<DELAY;i++);
}

void IIC0_SCLH_SDAH(void)
{
	IIC0_ESCL_Hi;
	IIC0_ESDA_Hi;
	Delay();
}

void IIC0_SCLH_SDAL(void)
{
	IIC0_ESCL_Hi;
	IIC0_ESDA_Lo;
	Delay();
}

void IIC0_SCLL_SDAH(void)
{
	IIC0_ESCL_Lo;
	IIC0_ESDA_Hi;
	Delay();
}

void IIC0_SCLL_SDAL(void)
{
	IIC0_ESCL_Lo;
	IIC0_ESDA_Lo;
	Delay();
}

void IIC0_ELow(void)
{
	IIC0_SCLL_SDAL();
	IIC0_SCLH_SDAL();
	IIC0_SCLH_SDAL();
	IIC0_SCLL_SDAL();
}

void IIC0_EHigh(void)
{
	IIC0_SCLL_SDAH();
	IIC0_SCLH_SDAH();
	IIC0_SCLH_SDAH();
	IIC0_SCLL_SDAH();
}

void IIC0_EStart(void)
{
	IIC0_SCLH_SDAH();
	IIC0_SCLH_SDAL();
	Delay();
	IIC0_SCLL_SDAL();
}

void IIC0_EEnd(void)
{
	IIC0_SCLL_SDAL();
	IIC0_SCLH_SDAL();
	Delay();
	IIC0_SCLH_SDAH();
}

void IIC0_EAck_write(void)
{
	unsigned long ack;

	IIC0_ESDA_INP;			// Function <- Input

	IIC0_ESCL_Lo;
	Delay();
	IIC0_ESCL_Hi;
	Delay();
	ack = GPD1DAT;
	IIC0_ESCL_Hi;
	Delay();
	IIC0_ESCL_Hi;
	Delay();

	IIC0_ESDA_OUTP;			// Function <- Output (SDA)

	ack = (ack>>0)&0x1;
	//	while(ack!=0);

	IIC0_SCLL_SDAL();
}

void IIC0_EAck_read(void)
{
	IIC0_ESDA_OUTP;			// Function <- Output

	IIC0_ESCL_Lo;
	IIC0_ESCL_Lo;
	IIC0_ESDA_Hi;
	IIC0_ESCL_Hi;
	IIC0_ESCL_Hi;

	IIC0_ESDA_INP;			// Function <- Input (SDA)

	IIC0_SCLL_SDAL();
}

void IIC0_ESetport(void)
{
	GPD1PUD &= ~(0xf<<0);	// Pull Up/Down Disable	SCL, SDA

	IIC0_ESCL_Hi;
	IIC0_ESDA_Hi;

	IIC0_ESCL_OUTP;		// Function <- Output (SCL)
	IIC0_ESDA_OUTP;		// Function <- Output (SDA)

	Delay();
}

void IIC0_EWrite (unsigned char ChipId, unsigned char IicAddr,
	unsigned char IicData) {
	unsigned long i;

	IIC0_EStart();

	////////////////// write chip id //////////////////
	for(i = 7; i>0; i--) {
		if((ChipId >> i) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_ELow();	// write

	IIC0_EAck_write();	// ACK

	////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--) {
		if((IicAddr >> (i-1)) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EAck_write();	// ACK

	////////////////// write reg. data. //////////////////
	for(i = 8; i>0; i--) {
		if((IicData >> (i-1)) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EAck_write();	// ACK

	IIC0_EEnd();
}

void IIC0_ERead (unsigned char ChipId, unsigned char IicAddr,
	unsigned char *IicData) {

	unsigned long i, reg;
	unsigned char data = 0;

	IIC0_EStart();

	////////////////// write chip id //////////////////
	for(i = 7; i>0; i--) {
		if((ChipId >> i) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_ELow();	// write

	IIC0_EAck_write();	// ACK

	////////////////// write reg. addr. //////////////////
	for(i = 8; i>0; i--) {
		if((IicAddr >> (i-1)) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EAck_write();	// ACK

	IIC0_EStart();

	////////////////// write chip id //////////////////
	for(i = 7; i>0; i--) {
		if((ChipId >> i) & 0x0001)
			IIC0_EHigh();
		else
			IIC0_ELow();
	}

	IIC0_EHigh();	// read

	IIC0_EAck_write();	// ACK

	////////////////// read reg. data. //////////////////
	IIC0_ESDA_INP;

	IIC0_ESCL_Lo;
	IIC0_ESCL_Lo;
	Delay();

	for(i = 8; i>0; i--) {
		IIC0_ESCL_Lo;
		IIC0_ESCL_Lo;
		Delay();
		IIC0_ESCL_Hi;
		IIC0_ESCL_Hi;
		Delay();
		reg = GPD1DAT;
		IIC0_ESCL_Hi;
		IIC0_ESCL_Hi;
		Delay();
		IIC0_ESCL_Lo;
		IIC0_ESCL_Lo;
		Delay();

		reg = (reg >> 0) & 0x1;

		data |= reg << (i-1);
	}

	IIC0_EAck_read();	// ACK
	IIC0_ESDA_OUTP;

	IIC0_EEnd();

	*IicData = data;
}

#define CALC_S5M8767_BUCK234_VOLT(x)	( (x<600) ? 0 : ((x-600)/6.25) )
#define CALC_S5M8767_BUCK156_VOLT(x)	( (x<650) ? 0 : ((x-650)/6.25) )
#define CALC_S5M8767_LDO1267815_VOLT(x)	( (x<800) ? 0 : ((x-800)/25) )
#define CALC_S5M8767_ALL_LDO_VOLT(x)	( (x<800) ? 0 : ((x-800)/50) )

void I2C_S5M8767_VolSetting(PMIC_RegNum eRegNum, unsigned char ucVolLevel,
	unsigned char ucEnable) {
	unsigned char reg_addr, reg_bitpos, reg_bitmask, vol_level;

	reg_bitpos = 0;
	reg_bitmask = 0xFF;
	if(eRegNum == 0)  // BUCK1
	{
		reg_addr = 0x33;
	}
	else if(eRegNum == 1)  // BUCK2
	{
		reg_addr = 0x35;
	}
	else if(eRegNum == 2)  //  BUCK3
	{
		reg_addr = 0x3E;
	}
	else if(eRegNum == 3)  // BUCK4
	{
		reg_addr = 0x47;
	}
	else if(eRegNum == 4)  // LDO2
	{
		reg_addr = 0x5D;
		reg_bitmask = 0x3F;
	}
	else if(eRegNum == 5)  // LDO3
	{
		reg_addr = 0x61;
		reg_bitmask = 0x3F;
	}
	else if(eRegNum == 6)  // LDO5
	{
		reg_addr = 0x63;
		reg_bitmask = 0x3F;
	}
	else if(eRegNum == 7)  // LDO10
	{
		reg_addr = 0x68;
		reg_bitmask = 0x3F;
	}
	else if(eRegNum == 8)  // LDO7
	{
		reg_addr = 0x65;
		reg_bitmask = 0x3F;
	}
	else if(eRegNum == 9) // BUCK5
	{
		reg_addr = 0x50;
	}
	else
		while(1);

	vol_level = ucVolLevel & reg_bitmask | (ucEnable << 6);
	IIC0_EWrite(S5M8767_ADDR, reg_addr, vol_level);

}

void pmic8767_init(void)
{
#if defined(CONFIG_CPU_EXYNOS5250_EVT1)
	u8 vdd_arm, vdd_int, vdd_g3d;
	u8 vdd_mif;
	u8 vdd_mem, vdd_mem_set = 0;
	u8 vddq_m1_m2, vddq_m1_m2_set = 0;
	u8 vdd_apll;

	if (PRO_PKGINFO == POP_TYPE) {

	} else {
		vdd_arm = CALC_S5M8767_BUCK234_VOLT(CONFIG_SCP_PM_VDD_ARM);
		vdd_int = CALC_S5M8767_BUCK234_VOLT(CONFIG_SCP_PM_VDD_INT);
		vdd_g3d = CALC_S5M8767_BUCK234_VOLT(CONFIG_SCP_PM_VDD_G3D);
		vdd_mif = CALC_S5M8767_BUCK156_VOLT(CONFIG_SCP_PM_VDD_MIF);
#if defined(CONFIG_SCP_PM_VDDQ_M1_M2)
		vddq_m1_m2 = CALC_S5M8767_LDO1267815_VOLT(CONFIG_SCP_PM_VDDQ_M1_M2);
		vddq_m1_m2_set = 1;
#endif
	}

	I2C_S5M8767_VolSetting(PMIC_BUCK1, vdd_mif, 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK2, vdd_arm, 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK3, vdd_int, 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK4, vdd_g3d, 1);
	if (vddq_m1_m2_set)
		I2C_S5M8767_VolSetting(0x04, vddq_m1_m2, 3);
	/* LDO4: VDD_eMMC_1V8_PM */
	IIC0_EWrite(S5M8767_ADDR, 0x62, 0x14);

#else
	float vdd_arm, vdd_int, vdd_g3d;
	float vdd_mif;

	vdd_arm = CONFIG_PM_VDD_ARM;
	vdd_int = CONFIG_PM_VDD_INT;
	vdd_g3d = CONFIG_PM_VDD_G3D;
	vdd_mif = CONFIG_PM_VDD_MIF;

#ifdef CONFIG_DDR3
	I2C_S5M8767_VolSetting(PMIC_BUCK1, CALC_S5M8767_BUCK156_VOLT(vdd_mif), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK2, CALC_S5M8767_BUCK234_VOLT(vdd_arm), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK3, CALC_S5M8767_BUCK234_VOLT(vdd_int), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK4, CALC_S5M8767_BUCK234_VOLT(vdd_g3d), 1);

	IIC0_EWrite(S5M8767_ADDR, 0x59, 0x4C); // BUCK8 1.4 -> 1.7V
	IIC0_EWrite(S5M8767_ADDR, 0x5D, 0xDC); // LDO2 1.2 -> 1.5V

	IIC0_EWrite(S5M8767_ADDR, 0x34, 0x78);
	IIC0_EWrite(S5M8767_ADDR, 0x3d, 0x58);
	IIC0_EWrite(S5M8767_ADDR, 0x46, 0x78);

	IIC0_EWrite(S5M8767_ADDR, 0x5A, 0x58);

	IIC0_EWrite(S5M8767_ADDR, 0x65, 0xCE); // LDO7 1.0 -> 1.2V
#else
	I2C_S5M8767_VolSetting(PMIC_BUCK1, CALC_S5M8767_BUCK156_VOLT(vdd_mif), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK2, CALC_S5M8767_BUCK234_VOLT(vdd_arm), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK3, CALC_S5M8767_BUCK234_VOLT(vdd_int), 1);
	I2C_S5M8767_VolSetting(PMIC_BUCK4, CALC_S5M8767_BUCK234_VOLT(vdd_g3d), 1);
	IIC0_EWrite(S5M8767_ADDR, 0x34, 0x78);
	IIC0_EWrite(S5M8767_ADDR, 0x3d, 0x58);
	IIC0_EWrite(S5M8767_ADDR, 0x46, 0x78);

	IIC0_EWrite(S5M8767_ADDR, 0x5A, 0x58);

	IIC0_EWrite(S5M8767_ADDR, 0x63, 0xE8); // LDO5
	IIC0_EWrite(S5M8767_ADDR, 0x64, 0xD0); // LDO6
	IIC0_EWrite(S5M8767_ADDR, 0x65, 0xD0); // LDO7

#endif
#endif /* CONFIG_CPU_EXYNOS5250_EVT1 */
	IIC0_EWrite(S5M8767_ADDR, 0x67, 0xEC); // LDO9
	IIC0_EWrite(S5M8767_ADDR, 0x78, 0xD4); // LDO26
}

void pmic_init(void)
{
	u8 read_data;

	IIC0_ESetport();

	/* read ID */
	IIC0_ERead(S5M8767_ADDR, 0, &read_data);

	if (read_data >= 0x0 && read_data <= 0x5) {
		pmic8767_init();
	}
}
