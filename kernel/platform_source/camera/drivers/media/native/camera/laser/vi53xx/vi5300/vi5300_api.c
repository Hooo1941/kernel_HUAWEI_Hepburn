/*
 *  vi5300_module.c - Linux kernel modules for VI5300 FlightSense TOF
 *						 sensor
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/time.h>

#include "vi5300.h"
#include "vi5300_platform.h"
#include "vi5300_firmware.h"
#include "vi5300_api.h"

#define PILEUP_A (9231000)
#define PILEUP_B (4896)
#define PILEUP_C (1922)
#define PILEUP_D (10)

#define STATUS_TOF_CONFIDENT 0
#define STATUS_TOF_SEMI_CONFIDENT 6
#define STATUS_TOF_NOT_CONFIDENT 7
#define STATUS_TOF_NO_OBJECT 255

VI5300_Error VI5300_Interrupt_Enable(VI5300_DEV dev);
VI5300_Error VI5300_Config_XTalk_Parameter(VI5300_DEV dev);

VI5300_Error VI5300_Chip_PowerON(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = gpio_direction_output(dev->xshut_gpio, 0);
	mdelay(5);
	Status = gpio_direction_output(dev->xshut_gpio, 1);
	mdelay(5);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Chip Power ON Failed Status = %d\n", Status);
		return VI5300_ERROR_POWER_ON;
	}

	return Status;
}

VI5300_Error VI5300_Chip_PowerOFF(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = gpio_direction_output(dev->xshut_gpio, 0);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Chip Power OFF Failed Status = %d\n", Status);
		return VI5300_ERROR_POWER_OFF;
	}

	return Status;
}

void VI5300_Chip_Register_Init(VI5300_DEV dev)
{
	vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x00);
	vi5300_write_byte(dev, VI5300_REG_SYS_CFG, 0x0C);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x00);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x01);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x00);
	vi5300_write_byte(dev, VI5300_REG_INTR_MASK, 0x21);
	vi5300_write_byte(dev, VI5300_REG_I2C_IDLE_TIME, 0x0E);
	vi5300_write_byte(dev, VI5300_REG_SPCIAL_PURP, 0x00);
	vi5300_write_byte(dev, VI5300_REG_RCO_AO, 0x80);
	vi5300_write_byte(dev, VI5300_REG_DIGLDO_VREF, 0x30);
	vi5300_write_byte(dev, VI5300_REG_PLLLDO_VREF, 0x00);
	vi5300_write_byte(dev, VI5300_REG_ANALDO_VREF, 0x30);
	 vi5300_write_byte(dev, VI5300_REG_PD_RESET, 0x80);
	vi5300_write_byte(dev, VI5300_REG_I2C_STOP_DELAY, 0x80);
	vi5300_write_byte(dev, VI5300_REG_TRIM_MODE, 0x80);
	vi5300_write_byte(dev, VI5300_REG_GPIO_SINGLE, 0x00);
	vi5300_write_byte(dev, VI5300_REG_ANA_TEST_SINGLE, 0x00);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0E);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0F);
}

static void VI5300_Waiting_For_RCO_Stable(VI5300_DEV dev)
{
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0F);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0E);
	mdelay(4);
}

VI5300_Error VI5300_Wait_For_CPU_Ready(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t stat;
	int retry = 0;

	do {
		mdelay(1);
		vi5300_read_byte(dev, VI5300_REG_DEV_STAT, &stat);
	}while((retry++ < VI5300_MAX_WAIT_RETRY)
		&&(stat & 0x01));
	if(retry >= VI5300_MAX_WAIT_RETRY)
	{
		vi5300_errmsg("CPU Busy stat = %d\n", stat);
		return VI5300_ERROR_CPU_BUSY;
	}

	return Status;
}

void VI5300_Read_ChipVersion(VI5300_DEV dev)
{
	uint8_t chipid[3] = {0};
	uint32_t ChipVersion = 0;

	vi5300_read_multibytes(dev, VI5300_REG_CHIPID_BASE, chipid, 3);
	ChipVersion = (chipid[1] << 16) + (chipid[0] << 8) + chipid[2];
	vi5300_errmsg("VI5300 ChipVersion: 0x%x\n", ChipVersion);
	if(ChipVersion == 0x530004 || ChipVersion == 0x530041)
		dev->chip_version = 0x40;
	else
		dev->chip_version = 0x31;
}

VI5300_Error VI5300_Init_FirmWare(VI5300_DEV dev)
{
	uint8_t  sys_cfg_data = 0;
	uint16_t fw_size = 0;
	uint16_t fw_send = 0;
	uint8_t val;
	VI5300_Error Status = VI5300_ERROR_NONE;

	fw_size = LoadFirmware(dev);
	if(!fw_size)
	{
		vi5300_errmsg("Firmware Load Failed!\n");
		return VI5300_ERROR_FW_FAILURE;
	}
	vi5300_errmsg("Firmware Load begin!!!\n");
	if(dev->chip_version == 0x31)
	{
		VI5300_Waiting_For_RCO_Stable(dev);
	}
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x08);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x0a);
	vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x06);
	vi5300_read_byte(dev, VI5300_REG_SYS_CFG, &sys_cfg_data);
	vi5300_write_byte(dev, VI5300_REG_SYS_CFG, sys_cfg_data | (0x01 << 0));
	vi5300_write_byte(dev, VI5300_REG_CMD, 0x01);
	vi5300_write_byte(dev, VI5300_REG_SIZE, 0x02);
	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x0);
	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, 0x0);
	while(fw_size >= 23)
	{
		vi5300_write_reg_offset(dev, VI5300_REG_CMD, 0, VI5300_WRITEFW_CMD);
		vi5300_write_reg_offset(dev, VI5300_REG_SIZE, 0, 0x17);
		vi5300_write_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, Firmware+fw_send*23, 23);
		udelay(10);
		fw_send += 1;
		fw_size -= 23;
	}
	if(fw_size > 0)
	{
		vi5300_write_reg_offset(dev, VI5300_REG_CMD, 0, VI5300_WRITEFW_CMD);
		vi5300_write_reg_offset(dev, VI5300_REG_SIZE, 0, (uint8_t)fw_size);
		vi5300_write_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, Firmware+fw_send*23, fw_size);
	}
	vi5300_write_byte(dev, VI5300_REG_SYS_CFG, sys_cfg_data & ~(0x01 << 0));
	vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x06);
	vi5300_write_byte(dev, VI5300_REG_PD_RESET, 0xA0);
	vi5300_write_byte(dev, VI5300_REG_PD_RESET, 0x80);
	vi5300_write_byte(dev, VI5300_REG_MCU_CFG, 0x07);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x02);
	vi5300_write_byte(dev, VI5300_REG_PW_CTRL, 0x00);
	vi5300_errmsg("Firmware Load end!!!\n");
	mdelay(5);
	vi5300_read_byte(dev, VI5300_REG_SPCIAL_PURP, &val);
	if(val != 0x66)
	{
		vi5300_errmsg("Download Firmware Failed, value = %d\n", val);
		Status = VI5300_ERROR_FW_FAILURE;
	}

	return Status;
}

__attribute__((unused)) static void VI5300_Integral_Counts_Write(VI5300_DEV dev, uint32_t inte_counts)
{
	union inte_data {
		uint32_t intecnts;
		uint8_t buf[4];
	} intedata;

	intedata.intecnts = inte_counts;
	if(dev->chip_version == 0x40)
	{
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x03);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x14);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, intedata.buf[0]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, intedata.buf[1]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 5, intedata.buf[2]);
	} else if(dev->chip_version == 0x31){
		VI5300_Waiting_For_RCO_Stable(dev);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x03);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, intedata.buf[0]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, intedata.buf[1]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 5, intedata.buf[2]);
	}
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	mdelay(5);
}

__attribute__((unused)) static void VI5300_Delay_Count_Write(VI5300_DEV dev, uint16_t delay_count)
{
	union delay_data {
		uint16_t delay;
		uint8_t buf[2];
	} delaydata;

	delaydata.delay = delay_count;
	if(dev->chip_version == 0x40)
	{
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x17);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, delaydata.buf[0]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, delaydata.buf[1]);
	} else if(dev->chip_version == 0x31){
		VI5300_Waiting_For_RCO_Stable(dev);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x04);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, delaydata.buf[1]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, delaydata.buf[0]);
	}
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	mdelay(5);
}

__attribute__((unused)) static void VI5300_Set_Integralcounts_Frame(VI5300_DEV dev, uint8_t fps, uint32_t intecoutns)
{
	uint32_t inte_time = 0;
	uint32_t fps_time = 0;
	uint32_t delay_time = 0;
	uint16_t delay_counts = 0;

	if(dev->chip_version == 0x40)
	{
		inte_time = intecoutns *1463/10;
		fps_time = 1000000000/fps;
		delay_time = fps_time - inte_time -1400000;
		delay_counts = (uint16_t)(delay_time/3400);
	} else if(dev->chip_version == 0x31){
		inte_time = intecoutns *1463/10;
		fps_time = 1000000000/fps;
		delay_time = fps_time - inte_time -1600000;
		delay_counts = (uint16_t)(delay_time/40900);
	}
	VI5300_Integral_Counts_Write(dev, intecoutns);
	VI5300_Delay_Count_Write(dev, delay_counts);
}

void VI5300_Set_Period(VI5300_DEV dev, uint32_t period)
{
	uint32_t inte_time = 0;
	uint32_t fps_time = 0;
	uint32_t delay_time = 0;
	uint16_t delay_counts = 0;
	union inte_data pdata = {0};
	union delay_data {
		uint16_t delay;
		uint8_t buf[2];
	} delaydata;

	vi5300_infomsg("enter VI5300_Set_Period\n");
	if(dev->chip_version == 0x40)
	{
		if(period == 0)
		{
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x00);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x17);
			vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
			msleep(5);
			vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, delaydata.buf, 2);
		} else {
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x00);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x03);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x14);
			vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
			msleep(5);
			vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, pdata.buf, 3);
			inte_time = pdata.intecnts*1463/10;
			fps_time = 1000000000 / period;
			delay_time = fps_time - inte_time -1260000;
			delay_counts = (uint16_t)(delay_time/3400);
			delaydata.delay = delay_counts;
		}
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x17);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, delaydata.buf[0]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, delaydata.buf[1]);
	}else {
		VI5300_Waiting_For_RCO_Stable(dev);
		if(period == 0)
		{
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x00);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x04);
			vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
			msleep(5);
			vi5300_read_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, &delaydata.buf[1]);
			vi5300_read_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, &delaydata.buf[0]);
		} else {
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x00);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x03);
			vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x01);
			vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
			msleep(5);
			vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, pdata.buf, 3);
			inte_time = pdata.intecnts*1463/10;
			fps_time = 1000000000 / period;
			delay_time = fps_time - inte_time -1600000;
			delay_counts = (uint16_t)(delay_time/40900);
			delaydata.delay = delay_counts;
		}
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 1, 0x02);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 2, 0x04);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 3, delaydata.buf[1]);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 4, delaydata.buf[0]);
	}
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	msleep(5);
}

VI5300_Error VI5300_Read_OTP(VI5300_DEV dev, uint8_t base, uint8_t size, uint8_t* out)

{
	if(size > 32)
		return VI5300_ERROR_OTP_SIZE;
	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x03);
	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, size);
	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x02, base);
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	mdelay(5);
	vi5300_read_multibytes(dev, 0x0F, out, size);
	return 0;
}

static uint8_t VI5300_Get_MA_Window_Data(VI5300_DEV dev)
{
	uint8_t sum_ma = 0;
	int i = 0;
	uint8_t ma_val[8];

	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x00, 0x00);
	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, 0x08);
	if(dev->chip_version == 0x40) {
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x02, 0x1A);
	} else {
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x02, 0x06);
	}
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);

	msleep(5);
	vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, ma_val, 8);
	for(i = 0; i < 8; i++)
	{
		sum_ma += ((ma_val[i] & 0x0F)+((ma_val[i] >> 4) & 0x0F));
	}
	return sum_ma;
}

static uint32_t VI5300_Calculate_Pileup_Bias_V40(VI5300_DEV dev, uint32_t peak, uint32_t noise, uint32_t integral_times)
{
	uint32_t peak_tmp = 0;
	uint16_t xth[] = {0, 172, 420, 680, 1100, 1390, 1600, 1850};
	uint16_t pth[] = {0, 0, 2, 5, 12, 18, 23, 31};
	uint8_t i = 0;
	uint32_t bias = 0;

	if(integral_times == 0)
	{
		return bias;
	}
	peak_tmp = (peak - noise * dev->ma_sum / 8) * 16 / integral_times;
	for(i = 0; i < ARRAY_SIZE(xth) - 1; i++)
	{
		if(peak_tmp < xth[i+1])
		{
			bias = (pth[ i + 1 ] - pth[i])*( peak_tmp -xth[i]) / (xth[i+1] -xth[i]) + pth[i];
			break;
		} else if(peak_tmp >= xth[7])
		{
			bias = (pth[7] -pth[6]) * ( peak_tmp - xth[6]) / (xth[7] -xth[6]) + pth[6];
			break;
		}
	}

	return bias;
}

static uint32_t VI5300_Calculate_Pileup_Bias_V31(VI5300_DEV dev, uint32_t peak, uint32_t noise, uint32_t intetimes)
{
	uint32_t xth[] = {0,8,20,44,79,117};
	uint32_t pth[] = {0,0,7,26,52,79};
	uint8_t i = 0;
	uint8_t bias = 0;
	uint32_t peak_tmp = 0;

	if(intetimes == 0)
	{
		return bias;
	}

	peak_tmp = (peak - noise * 3 / 8) * 16 / intetimes;
	for(i = 0;i < ARRAY_SIZE(xth) - 1;i++)
	{
		if(peak_tmp < xth[i+1])
		{
			bias = (pth[ i + 1 ] - pth[i]) * ( peak_tmp -xth[i])/(xth[i+1]-xth[i])+ pth[i];
			break;
		} else if(peak_tmp >= xth[5])
		{
			bias = (pth[5]-pth[4]) * ( peak_tmp - xth[4])/(xth[5]-xth[4])+pth[4];
			break;
		}
	}

	return bias;
}

static uint32_t VI5300_Calculate_Confidence_V40(VI5300_DEV dev, uint32_t peak, uint32_t noise, uint32_t integral_times)
{
	const uint32_t s_noise[23] = {4, 32, 114, 175, 313, 482, 539, 657, 1472, 2421, 3223, 6777, 7217, 12326, 14946, 20906, 25976, 32287, 41121, 44258, 51439, 56032, 80216};
	const uint32_t s_lower[23] = {4, 7, 16, 22, 34, 49, 54, 66, 136, 211, 279, 566, 600, 1025, 1221, 1682, 2086, 2559, 3236, 3479, 4038, 4396, 6306};
	const uint32_t s_upper[23] = {7, 9, 20, 28, 43, 60, 66, 80, 162, 243, 321, 630, 666, 1138, 1338, 1828, 2266, 2743, 3442, 3695, 4280, 4650, 6671};
	uint32_t noise_r=0;
	uint32_t peak_r=0;
	uint32_t lower=0;
	uint32_t upper=0;
	uint32_t confidence = 0;
	int i;

	if(peak>8000000)
		peak_r=peak*256/integral_times*4;
	else if(peak>4000000)
		peak_r=peak*512/integral_times*2;
	else
		peak_r = peak * 1024 / integral_times;
	peak_r = peak_r * 100;

	if (noise < 32768) {
		noise_r = noise * 131072 / integral_times;
	} else if (noise < 65536) {
		noise_r = noise * 65536 / integral_times * 2;
	} else if (noise < 131072) {
		noise_r = noise * 32768 / integral_times * 4;
	} else {
		noise_r = noise * 16384 / integral_times * 8;
	}

	for (i = 0; i < ARRAY_SIZE(s_noise) - 1; i++) {
		if (noise_r < s_noise[i + 1]) {
			lower = 100 * (s_lower[i + 1] - s_lower[i]) * abs((int32_t)(noise_r - s_noise[i])) /
				(s_noise[i + 1] - s_noise[i]) + 100 * s_lower[i];
			upper = 100 * (s_upper[i + 1] - s_upper[i]) * abs((int32_t)(noise_r - s_noise[i])) /
				(s_noise[i + 1] - s_noise[i]) + 100 * s_upper[i];
			break;
		} else if(noise_r >= s_noise[22]) {
			lower = 100 * (s_lower[22] - s_lower[21]) * abs((int32_t)(noise_r - s_noise[21])) /
				(s_noise[22] - s_noise[21]) + 100 * s_lower[21];
			upper = 100 * (s_upper[22] - s_upper[21]) * abs((int32_t)(noise_r - s_noise[21])) /
				(s_noise[22] - s_noise[21]) + 100 * s_upper[21];
			break;
		}
	}

	if(peak_r<lower){
		confidence = 0;
	} else if(peak_r > upper) {
		confidence = 100;
	} else {
		confidence = 100 * (peak_r - lower) / (upper - lower);
	}
	return confidence;
}

static uint32_t VI5300_Calculate_Confidence_V31(VI5300_DEV dev, uint32_t peak, uint32_t noise, uint32_t intetimes)
{
	const uint32_t s_noise[10] = {19, 83, 163, 691, 1243, 2539, 5091, 10395, 20427, 33235};
	const uint32_t s_lower[10] = {16, 25, 45, 145, 258, 501, 1004, 1980, 3804, 6063};
	const uint32_t s_upper[10] = {26, 35, 61, 173, 303, 574, 1148, 2220, 4230, 6705};
	uint32_t confidence;
	uint32_t noise_r;
	uint32_t peak_r;
	uint32_t lower, upper;
	int i = 0;

	if(peak > 8000000)
		peak_r = peak * 256 / intetimes * 4;
	else if(peak > 4000000)
		peak_r = peak * 512 / intetimes * 2;
	else
		peak_r = peak * 1024 / intetimes;

	noise_r = noise * 65536 / intetimes * 2;

	for(i = 0; i < ARRAY_SIZE(s_noise)-1; i++)
	{
		if(noise_r < s_noise[i + 1])
		{
			lower = (s_lower[i + 1] - s_lower[i]) * abs(noise_r - s_noise[i]) / (s_noise[i + 1] - s_noise[i]) + s_lower[i];
			upper = (s_upper[i + 1] - s_upper[i]) * abs(noise_r - s_noise[i]) / (s_noise[i + 1] - s_noise[i]) + s_upper[i];
			break;
		} else if(noise_r >= s_noise[9]){
			lower = (s_lower[9] - s_lower[8]) * abs(noise_r - s_noise[8]) / (s_noise[9] - s_noise[8]) + s_lower[8];
			upper = (s_upper[9] - s_upper[8]) * abs(noise_r - s_noise[8]) / (s_noise[9] - s_noise[8]) + s_upper[8];
			break;
		}
	}
	if(peak_r < lower) {
		confidence = 0;
	} else if(peak_r > upper) {
		confidence = 100;
	} else {
		confidence = 100 * (peak_r - lower) / (upper - lower);
	}

	return confidence;
}


VI5300_Error VI5300_Single_Measure(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = VI5300_Wait_For_CPU_Ready(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("CPU Abnormal Single Measure!Status = %d\n", Status);
		return VI5300_ERROR_SINGLE_CMD;
	}
	if(dev->chip_version == 0x31)
	{
		VI5300_Waiting_For_RCO_Stable(dev);
	}
	Status = vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_SINGLE_RANGE_CMD);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Single measure Failed Status = %d\n", Status);
		return VI5300_ERROR_SINGLE_CMD;
	}

	return Status;
}

VI5300_Error VI5300_ReDownload_FW(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	if (dev->chip_version == 0x31)
		VI5300_Chip_Register_Init(dev);

	Status = VI5300_Interrupt_Enable(dev);
	if (Status != VI5300_ERROR_NONE)
		return Status;

	Status = VI5300_Init_FirmWare(dev);
	if (Status != VI5300_ERROR_NONE)
		return Status;

	Status = VI5300_Config_XTalk_Parameter(dev);
	if (Status != VI5300_ERROR_NONE)
		return Status;

	VI5300_Set_Period(dev, dev->period);

	return Status;
}

VI5300_Error VI5300_Start_Continuous_Measure(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t value = 0;
	vi5300_infomsg("enter VI5300_Start_Continuous_Measure\n");

	Status = vi5300_read_byte(dev, 0x08, &value);
	if ((value != 0x66) && (value != 0xAA))
	{
		vi5300_infomsg("ReDownload FW\n");
		Status = VI5300_ReDownload_FW(dev);
		if (Status != VI5300_ERROR_NONE)
		{
			vi5300_errmsg("ReDownload FW Failed! Status = %d\n", Status);
			return VI5300_ERROR_CONTINUOUS_CMD;
		}
	}
	Status = VI5300_Wait_For_CPU_Ready(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("CPU Abnormal Continuous Measure!Status = %d\n", Status);
		return VI5300_ERROR_CONTINUOUS_CMD;
	}
	dev->ma_sum = VI5300_Get_MA_Window_Data(dev);
	if (dev->chip_version == 0x31) {
		VI5300_Waiting_For_RCO_Stable(dev);
	}
	Status = vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_CONTINOUS_RANGE_CMD);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Continuous Measure Failed Status = %d\n", Status);
		return VI5300_ERROR_CONTINUOUS_CMD;
	}

	return Status;
}

VI5300_Error VI5300_Stop_Continuous_Measure(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	vi5300_infomsg("enter VI5300_Stop_Continuous_Measure\n");
	Status = vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_STOP_RANGE_CMD);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Stop Measure Failed Status = %d\n", Status);
		return VI5300_ERROR_STOP_CMD;
	}

	// stop twice for solve first stop fail when req is comming
	mdelay(3);
	Status = vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_STOP_RANGE_CMD);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Stop Measure Failed Status = %d\n", Status);
		return VI5300_ERROR_STOP_CMD;
	}

	return Status;
}

VI5300_Error VI5300_Get_Measure_Data(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t buf[32];
	int16_t millimeter = 0;
	int16_t tof_record = 0;
	int16_t nearlimit = 0;
	int16_t farlimit = 0;
	uint32_t noise = 0;
	uint32_t peak1 = 0;
	uint32_t peak2 = 0;
	uint32_t integral_times = 0;
	uint32_t bias = 0;
	uint32_t confidence = 0;
	int32_t ratio;
	struct timespec end_ts = {0},ts_sub = {0};
	uint8_t status;

	Status |= vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, buf, 16);
	Status |= vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE+0x10, buf + 16, 16);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Get Range Data Failed Status = %d\n", Status);
		return VI5300_ERROR_GET_DATA;
	}
	millimeter = *((int16_t *)(buf + 12));
	integral_times = *((uint32_t *)(buf + 22));
	peak1 = *((uint32_t *)(buf + 28));
	peak2 = *((uint32_t *)(buf + 8));
	integral_times = integral_times &0x00ffffff;
	if(dev->chip_version == 0x40)
	{
		nearlimit = *((int16_t *)(buf + 16));
		farlimit = *((int16_t *)(buf + 14));
		noise = *((uint32_t *)(buf + 25));
		noise = noise &0x00ffffff;
	} else if(dev->chip_version == 0x31) {
		nearlimit = *((int16_t *)(buf + 18));
		farlimit = *((int16_t *)(buf + 15));
		noise = *((uint32_t *)(buf + 26));
		noise = noise &0x0000ffff;
	}
	if(dev->chip_version == 0x40)
	{
		bias = VI5300_Calculate_Pileup_Bias_V40(dev, peak1, noise, integral_times);
	} else if(dev->chip_version == 0x31){
		bias = VI5300_Calculate_Pileup_Bias_V31(dev, peak2, noise, integral_times);
	}

	vi5300_infomsg("milimeter(%d), bias(%u), peak2(%u)",
		millimeter,
		bias,
		peak2);
	millimeter = millimeter + (int16_t)bias;
	millimeter = millimeter - dev->offset_config;
	nearlimit = nearlimit + (int16_t)bias - dev->offset_config;
	farlimit = farlimit + (int16_t)bias - dev->offset_config;

	tof_record = millimeter;
	getnstimeofday(&end_ts);
	// ts_sub = timespec_sub(end_ts, dev->start_ts);
	// dev->Rangedata.timeUSec = ts_sub.tv_sec * 1000000 + ts_sub.tv_nsec / 1000;
	dev->Rangedata.milimeter = millimeter;
	// dev->Rangedata.NumberOfObject = 1 << 6;
	// dev->Rangedata.RangeLimit = nearlimit << 16 |(farlimit & 0xFFFF);
	// dev->Rangedata.AmbientRate = (noise) * 300 / (peak2 * 8);

	if(dev->chip_version == 0x40)
	{
		confidence = VI5300_Calculate_Confidence_V40(dev, peak1, noise,integral_times);
	} else if(dev->chip_version == 0x31) {
		confidence = VI5300_Calculate_Confidence_V31(dev, peak1, noise, integral_times);
	}

	ratio = 100 * noise * dev->ma_sum / peak1;
	if (dev->chip_version == 0x40) {
		if (noise > 65000 && buf[3] <= 50 && ratio < 850) {
			confidence = 0;
		}
	} else if (dev->chip_version == 0x31) {
		if (noise > 29000 && buf[3] <= 50 && ratio < 740) {
			confidence = 0;
		}
	}

	if(confidence >= 70) {
		if(millimeter <= 0) {
			millimeter = 25;
			nearlimit = 15;
			farlimit = 35;
			dev->Rangedata.milimeter = millimeter;
			// dev->Rangedata.RangeLimit = nearlimit << 16 |(farlimit & 0xFFFF);
		}
		dev->Rangedata.status = STATUS_TOF_CONFIDENT;
		if(millimeter < 50 && peak1 < 800000) {
			dev->Rangedata.status = STATUS_TOF_NOT_CONFIDENT;
		}
	}
	else if(confidence >= 30) {
		dev->Rangedata.status = STATUS_TOF_SEMI_CONFIDENT;
		if(millimeter < 50 && peak1 < 800000) {
			dev->Rangedata.status = STATUS_TOF_NOT_CONFIDENT;
		}
	}
	else if(confidence > 0) {
		dev->Rangedata.status = STATUS_TOF_NOT_CONFIDENT;
		if(millimeter > 1400) {
			millimeter = 1500;
			nearlimit = 1470;
			farlimit = 1530;
			dev->Rangedata.milimeter = millimeter;
			// dev->Rangedata.RangeLimit = nearlimit << 16 |(farlimit & 0xFFFF);
			dev->Rangedata.status = STATUS_TOF_NO_OBJECT;
		}
	} else {
			// dev->Rangedata.NumberOfObject = 0 << 6;
			millimeter = 1500;
			nearlimit = 1470;
			farlimit = 1530;
			dev->Rangedata.milimeter = millimeter;
			// dev->Rangedata.RangeLimit = nearlimit << 16 |(farlimit & 0xFFFF);
			dev->Rangedata.status = STATUS_TOF_NO_OBJECT;
	}

	status = dev->Rangedata.status;
	if ( status == 255 || status == 0)
		dev->Rangedata.confidence = 1; // 1: confident
	else
		dev->Rangedata.confidence = 0; // 0: not confident

	// vi5300_infomsg("milimeter(%d), status(%u)",
	// 		dev->Rangedata.milimeter,
	// 		dev->Rangedata.status);
	if(dev->enable_debug) {
		vi5300_errmsg("millimeter:%d, status:%u, nearlimit:%d, farlimit:%d,"
			"tof_record:%d, confidence:%d, noise:%d, peak1:%d, integral_times:%d,"
			"millimeter:%d, confidence:%u\n",
			dev->Rangedata.milimeter, dev->Rangedata.status,
			nearlimit, farlimit, tof_record, confidence, noise, peak1,
			integral_times, millimeter, dev->Rangedata.confidence);
	}

	dev->Rangedata.noise = noise;

	return Status;
}

VI5300_Error VI5300_Get_Interrupt_State(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t stat;

	Status = vi5300_read_byte(dev, VI5300_REG_INTR_STAT, &stat);
	if(!(stat & 0x01))
	{
		vi5300_errmsg("Get Interrupt State Failed Status = %d\n", Status);
		return VI5300_ERROR_IRQ_STATE;
	}

	return Status;
}

VI5300_Error VI5300_Interrupt_Enable(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	int loop = 0;
	uint8_t enable = 0;

	do
	{
		vi5300_read_byte(dev, VI5300_REG_INTR_MASK, &enable);
		enable |=  0x01;
		vi5300_write_byte(dev, VI5300_REG_INTR_MASK, enable);
		vi5300_read_byte(dev, VI5300_REG_INTR_MASK, &enable);
		loop++;
	} while((loop < VI5300_MAX_WAIT_RETRY)
		&& (!(enable & 0x01)));
	if(loop >= VI5300_MAX_WAIT_RETRY)
	{
		vi5300_errmsg("Enable interrupt Failed Status = %d\n", Status);
		return VI5300_ERROR_ENABLE_INTR;
	}

	return Status;
}

VI5300_Error VI5300_Chip_Init(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	VI5300_Read_ChipVersion(dev);
	if(dev->chip_version == 0x31)
	{
		VI5300_Chip_Register_Init(dev);
	}
	Status = VI5300_Wait_For_CPU_Ready(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Internal CPU busy!\n");
		return Status;
	}
	Status = VI5300_Interrupt_Enable(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Clear Interrupt Mask failed!\n");
		return Status;
	}
	Status = VI5300_Init_FirmWare(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		laser_dsm_report_dmd(HWLASER_VI5300, DSM_LASER_FW_DOWNLOAD_ERROR_NO);
		vi5300_errmsg("Download Firmware Failed!\n");
		return Status;
	}
	VI5300_Read_FW_Version(dev);

	return Status;
}

VI5300_Error VI5300_Start_XTalk_Calibration(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = VI5300_Wait_For_CPU_Ready(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("CPU Abnormal XTALK Calibrating Status = %d\n", Status);
		return VI5300_ERROR_XTALK_CALIB;
	}
	if(dev->chip_version == 0x31)
	{
		VI5300_Waiting_For_RCO_Stable(dev);
	}
	Status = vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_XTALK_TRIM_CMD);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("XTALK Calibration Failed Status = %d\n", Status);
		return VI5300_ERROR_XTALK_CALIB;
	}

	return Status;
}

VI5300_Error VI5300_Start_Offset_Calibration(VI5300_DEV dev, uint32_t param)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t buf[32];
	int16_t mm = 0;
	uint32_t peak1 = 0;
	uint32_t peak2 = 0;
	uint32_t inte_t = 0;
	uint32_t noise = 0;
	uint32_t bias = 0;
	int16_t total = 0;
	int16_t offset = 0;
	int cnt = 0;
	uint8_t stat = 0;

	Status = VI5300_Start_Continuous_Measure(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Offset Calibtration Start Failed!\n");
		return VI5300_ERROR_OFFSET_CALIB;
	}
	while(1)
	{
		mdelay(35);
		Status = vi5300_read_byte(dev, VI5300_REG_INTR_STAT, &stat);
		if(Status == VI5300_ERROR_NONE)
		{
			if((stat & 0x01) == 0x01)
			{
				Status |= vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, buf, 16);
				Status |= vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE + 0x10, buf + 16, 16);
				if(Status != VI5300_ERROR_NONE)
				{
					vi5300_errmsg("Get Range Data Failed Status = %d\n", Status);
					break;
				}
				mm = *((int16_t *)(buf + 12));
				inte_t = *((uint32_t *)(buf + 22));
				peak1 = *((uint32_t *)(buf + 28));
				peak2 = *((uint32_t *)(buf + 8));
				inte_t = inte_t &0x00ffffff;
				if(dev->chip_version == 0x40)
				{
					noise = *((uint32_t *)(buf + 25));
					noise = noise &0x00ffffff;
					bias = VI5300_Calculate_Pileup_Bias_V40(dev, peak1, noise, inte_t);
				} else if(dev->chip_version == 0x31) {
					noise = *((uint32_t *)(buf + 26));
					noise = noise &0x0000ffff;
					bias = VI5300_Calculate_Pileup_Bias_V31(dev, peak2, noise, inte_t);
				}
				vi5300_infomsg("milimeter(%d), bias(%u), peak2(%u), inte_t(%u), noise(%u), cnt(%d)",
					mm,
					bias,
					peak2,
					inte_t,
					noise,
					cnt);
				mm = mm + (int16_t)bias;
				total += mm;
				++cnt;
			}else
				continue;
		}else {
			vi5300_errmsg("Can't Get irq State!Status = %d\n", Status);
			break;
		}
		if(cnt >= 30)
			break;
	}
	Status = VI5300_Stop_Continuous_Measure(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("Offset Calibtration Stop Failed!\n");
		return VI5300_ERROR_OFFSET_CALIB;
	}
	offset = total / 30;
	dev->OffsetData.offset_cal = offset - param;

	return Status;
}


VI5300_Error VI5300_Get_XTalk_Parameter(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;
	uint8_t val;
	uint8_t cg_buf[5];

	Status = vi5300_read_byte(dev, VI5300_REG_SPCIAL_PURP, &val);
	if(Status == VI5300_ERROR_NONE && val == 0xaa)
	{
		Status = vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, cg_buf, 3);
		if(Status != VI5300_ERROR_NONE)
		{
			vi5300_errmsg("Get XTALK parameter Failed Status = %d\n", Status);
			return VI5300_ERROR_XTALK_CALIB;
		}
		dev->XtalkData.xtalk_cal = *((int8_t *)(cg_buf + 0));
		dev->XtalkData.xtalk_peak = *((uint16_t *)(cg_buf + 1));
	} else {
		vi5300_errmsg("XTALK Calibration Failed Status = %d, val = 0x%02x\n", Status, val);
		return VI5300_ERROR_XTALK_CALIB;
	}

	return Status;
}

VI5300_Error VI5300_Config_XTalk_Parameter(VI5300_DEV dev)
{
	VI5300_Error Status = VI5300_ERROR_NONE;

	Status = VI5300_Wait_For_CPU_Ready(dev);
	if(Status != VI5300_ERROR_NONE)
	{
		vi5300_errmsg("CPU Abnormal Configing XTALK Failed Status = %d\n", Status);
		return VI5300_ERROR_XTALK_CONFIG;
	}
	if(dev->chip_version == 0x40)
	{
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x02, 0x26);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x03, *((uint8_t *)(&dev->xtalk_config)));
		vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	} else if(dev->chip_version == 0x31) {
		VI5300_Waiting_For_RCO_Stable(dev);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x01, 0x01);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x02, 0x00);
		vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0x03, *((uint8_t *)(&dev->xtalk_config)));
		vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	}
	mdelay(5);
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);

	return Status;
}

void VI5300_Read_FW_Version(VI5300_DEV dev)
{
	uint8_t fw_version[4];

	vi5300_write_reg_offset(dev, VI5300_REG_SCRATCH_PAD_BASE, 0, 0x06);
	vi5300_write_byte(dev, VI5300_REG_CMD, VI5300_USER_CFG_CMD);
	mdelay(5);
	vi5300_read_multibytes(dev, VI5300_REG_SCRATCH_PAD_BASE, fw_version, 4);

	vi5300_infomsg("fw version: %#x %#x %#x %#x\n", fw_version[0], fw_version[1], fw_version[2], fw_version[3]);
}
