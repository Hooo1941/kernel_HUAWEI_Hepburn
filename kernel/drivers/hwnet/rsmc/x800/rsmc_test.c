/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: This file declares module type in rsmc.
 * Author: zhongjilei@huawei.com
 * Create: 2020-10-28
 */

#include "rsmc_test.h"

#include <huawei_platform/log/hw_log.h>
#include <securec.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/kthread.h>
#include <linux/time.h>
#include <linux/timer.h>

#include "rsmc_rx_ctrl.h"
#include "rsmc_spi_ctrl.h"
#include "rsmc_spi_drv.h"
#include "rsmc_tx_ctrl.h"
#include "rsmc_x800_device.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG RSMC_TEST
HWLOG_REGIST();

#ifdef RSMC_DRIVER_TEST
// define Test Case
// SPI_TEST
// SEND_TEST_FOR_SPEC
// SEND_TEST_FOR_PHONE
// TEST_PPS
// ACQ_LOOP_RECV
// RF_TEST

#ifdef SPI_TEST
const struct reg00_sys_cfg0_stru reg00_sys_cfg0 = {
	.data_tx_mode_0 = 2,
	.data_tx_mode_1 = 2,
	.data_tx_mode_2 = 2,
	.data_tx_mode_3 = 2,
	.data_tx_mode_4 = 2,
	.data_tx_mode_5 = 2,
	.data_tx_mode_6 = 2,
	.data_tx_mode_7 = 2,
};

const struct reg01_sys_cfg1_stru reg01_sys_cfg1 = {
	.corr_save_mode_0 = 0,
	.corr_save_mode_1 = 0,
	.corr_save_mode_2 = 0,
	.corr_save_mode_3 = 0,
	.corr_save_mode_4 = 0,
	.corr_save_mode_5 = 0,
	.corr_save_mode_6 = 0,
	.corr_save_mode_7 = 0,
	.pilot_tx_mode_0 = 1,
	.pilot_tx_mode_1 = 1,
	.pilot_tx_mode_2 = 1,
	.pilot_tx_mode_3 = 1,
	.pilot_tx_mode_4 = 1,
	.pilot_tx_mode_5 = 1,
	.pilot_tx_mode_6 = 1,
	.pilot_tx_mode_7 = 1,
	.bit16_23 = 0,
	.bit24_31 = 0
};

const struct reg02_sys_cfg2_stru reg02_sys_cfg2 = {
	.mod_chip_num = 1020,
	.mod_code_type = 2,
	.mod_ctrl_type = 0,
	.data_first = 1,
	.integral_64k = 0,
	.rf_on_polar = 0,
	.lna_on_polar = 1,
	.pa_on_polar = 1,
	.pps_polar = 1,
	.adc_edge = 0,
	.adc_convert_type = 0,
	.swap_iq = 0,
	.bit24_31 = 0
};

const struct reg03_sys_cfg3_stru reg03_sys_cfg3 = {
	.m1_phs_init = 0,
	.bit24_31 = 0
};

const struct reg04_sys_ctrl_stru reg04_sys_ctrl = {
	.mod_start_loc = 0,
	.rst_n_mod_fifo = 1,
	.rst_n_corr_fifo = 1,
	.clear_corr_fifo_err = 0,
	.clear_mod_fifo_err = 0,
	.bit5_23 = 0,
	.bit24_31 = 0
};

const struct reg05_sys_status_stru reg05_sys_status = {
	.mod_empty = 1,
	.mod_full = 0,
	.corr_empty = 1,
	.corr_full = 0,
	.mod_end = 0,
	.corr_err = 0,
	.mod_err = 1,
	.mod_cnt = 0,
	.corr_cnt = 0,
	.bit20_23 = 0,
	.bit24_31 = 0
};

const struct reg06_time_cfg_stru reg06_time_cfg = {
	.timer_pps_en = 1,
	.num_ppxms = 23,
	.timer_param = 0,
	.bit21_23 = 0,
	.bit24_31 = 0
};

const struct reg07_time_param_stru reg07_time_param = {
	.timer_ms = 0,
	.timer_xms = 0,
	.bit14_23 = 0,
	.bit24_31 = 0
};

const struct reg08_mod_ctrl_stru reg08_mod_ctrl = {
	.rf_on = 0,
	.lna_on = 1,
	.pa_on = 0,
	.bit3_7 = 0,
	.timer_param_pps = 0,
	.bit24_31 = 0
};

const struct reg09_mod_code_nco_h8_stru reg09_mod_code_nco_h8 = {
	.mod_code_nco_fw = 0,
	.mod_sync_bit_len = 0,
	.bit24_31 = 0
};

const struct reg0a_mod_code_nco_l24_stru reg0a_mod_code_nco_l24 = {
	.mod_code_nco_fw = 0,
	.bit24_31 = 0
};

const struct reg0b_mod_code_mphs_stru reg0b_mod_code_mphs = {
	.mod_code_mphs = 0,
	.bit24_31 = 0
};

const struct reg0c_mod_info_bit_len_stru reg0c_mod_info_bit_len = {
	.mod_info_bit_len = 0,
	.bit20_23 = 0,
	.bit24_31 = 0
};

const struct reg0d_mod_info_fifo_stru reg0d_mod_info_fifo = {
	.mod_fifo_din = 0,
	.bit24_31 = 0
};

static void read_all_rfreg(void)
{
	u32 i;
	u32 value;

	hwlog_info("%s: in", __func__);
	// rf reg addr 0x60 to 0x74
	for (i = 0x60; i <= 0x74; i++) {
		smc_get_value(i, &value);
		hwlog_err("%s: addr %X value 0x%X", __func__, i, value);
	}
}

static void read_all_dreg(void)
{
	u32 i;
	u32 value;

	hwlog_log("%s: in", __func__);

	for (i = ADDR_00_SYS_CFG_0; i <= ADDR_0F_VERSION; i++) {
		smc_get_value(i, &value);
		hwlog_err("%s: addr %X value 0x%X", __func__, i, value);
	}
}

static void bb_init(void)
{
	u32 addr;
	u32 value;

	addr = ADDR_00_SYS_CFG_0;
	value = *(u32 *)&reg00_sys_cfg0;
	smc_set_value(addr, value);

	addr = ADDR_01_SYS_CFG_1;
	value = *(u32 *)&reg01_sys_cfg1;
	smc_set_value(addr, value);

	addr = ADDR_02_SYS_CFG_2;
	value = *(u32 *)&reg02_sys_cfg2;
	smc_set_value(addr, value);

	addr = ADDR_03_SYS_CFG_3;
	value = *(u32 *)&reg03_sys_cfg3;
	smc_set_value(addr, value);

	addr = ADDR_04_SYS_CTRL;
	value = *(u32 *)&reg04_sys_ctrl;
	smc_set_value(addr, value);

	addr = ADDR_05_SYS_STATUS;
	value = *(u32 *)&reg05_sys_status;
	smc_set_value(addr, value);

	addr = ADDR_06_TIMER_CFG;
	value = *(u32 *)&reg06_time_cfg;
	smc_set_value(addr, value);

	addr = ADDR_07_TIMER_PARAM;
	value = *(u32 *)&reg07_time_param;
	smc_set_value(addr, value);

	addr = ADDR_08_MOD_CTRL;
	value = *(u32 *)&reg08_mod_ctrl;
	smc_set_value(addr, value);

	addr = ADDR_09_MOD_CODE_NCO_FW_H;
	value = *(u32 *)&reg09_mod_code_nco_h8;
	smc_set_value(addr, value);

	addr = ADDR_0A_MOD_CODE_NCO_FW_L;
	value = *(u32 *)&reg0a_mod_code_nco_l24;
	smc_set_value(addr, value);

	addr = ADDR_0B_MOD_CODE_PHS;
	value = *(u32 *)&reg0b_mod_code_mphs;
	smc_set_value(addr, value);

	addr = ADDR_0C_MOD_INFO_BIT_LEN;
	value = *(u32 *)&reg0c_mod_info_bit_len;
	smc_set_value(addr, value);

	addr = ADDR_0D_MOD_FIFO_DATA;
	value = *(u32 *)&reg0d_mod_info_fifo;
	smc_set_value(addr, value);

	read_all_dreg();
	read_all_rfreg();
}

static void spi_write_read_test(void)
{
	u32 write_value;
	u32 read_value;
	u32 i;
	const u32 test_count = 10;
	const u32 test_value = 0x123456;

	for (i = 0; i < test_count; i++) {
		write_value = test_value + i;
		smc_set_value(ADDR_00_SYS_CFG_0, write_value);
		smc_get_value(ADDR_00_SYS_CFG_0, &read_value);
		hwlog_err("%s: write=0x%x,read=0x%x",
			__func__, write_value, read_value);
		if (read_value == write_value)
			hwlog_err("%s: RSMC chip write_read succ", __func__);
		else
			hwlog_err("%s: RSMC chip write_read ERR", __func__);
	}
}
#endif

#ifdef TEST_PPS
#define NUM_PPXMS 23
const static struct reg06_time_cfg_stru time_cfg_reg6 = {
	.timer_pps_en = 1,
	.num_ppxms = NUM_PPXMS,
	.timer_param = 0,
	.bit21_23 = 0,
	.bit24_31 = 0
};

static void wait_chip_start(void)
{
	const u32 delay_time = 100;
	u32 value;

	hwlog_info("%s: RSMC wait for chip start", __func__);
	while (1) {
		smc_set_value(ADDR_03_SYS_CFG_3, 0x123456);
		smc_get_value(ADDR_03_SYS_CFG_3, &value);
		if (value == 0x123456) {
			hwlog_info("%s: RSMC chip init succ now", __func__);
			break;
		}
		msleep(delay_time);
	}
}

static void test_pps(void)
{
	const u32 fd_est_seconds = 4;
	u32 value;

	hwlog_info("%s: in", __func__);
	wait_chip_start();
	smc_set_value(ADDR_06_TIMER_CFG, *((u32 *)&time_cfg_reg6));
	smc_get_value(ADDR_06_TIMER_CFG, &value);
	hwlog_info("%s: RSMC chip set value:%06x", __func__, value);
	chip_init(0, 0);
	clk_fd_est(fd_est_seconds);
}
#endif

#define DATA_MAX_LEN 1023
static struct spi_transfer *g_spi_transfer;
static void spi_loop_read(void)
{
	u32 *tx = NULL;
	u32 *rx = NULL;
	int cnt;
	struct spi_transfer *tf = NULL;
	int ret;

	hwlog_err("%s: loop read\n", __func__);
	tx = kmalloc_array(DATA_MAX_LEN, sizeof(u32), GFP_ATOMIC);
	if (tx == NULL)
		return;
	for (cnt = 0; cnt < DATA_MAX_LEN; cnt++)
		tx[cnt] = 0x8E8E8E8E; // test reg addr and value
	hwlog_err("%s: tx addr buf:%08x\n", __func__, tx[0]);
	rx = kmalloc_array(DATA_MAX_LEN, sizeof(u32), GFP_ATOMIC);
	if (rx == NULL)
		return;
	tf = kmalloc_array(DATA_MAX_LEN, sizeof(struct spi_transfer), GFP_ATOMIC);
	if (tf == NULL)
		return;
	ret = memset_s(tf, DATA_MAX_LEN * sizeof(struct spi_transfer), 0, DATA_MAX_LEN * sizeof(struct spi_transfer));
	if (ret != EOK) {
		hwlog_err("%s: memset_s failed", __func__);
		return;
	}

	for (cnt = 0; cnt < DATA_MAX_LEN; cnt++) {
		tf[cnt].rx_buf = &rx[cnt];
		tf[cnt].tx_buf = &tx[cnt];
		tf[cnt].len = sizeof(u32);
		tf[cnt].cs_change = 1;
	}
	g_spi_transfer = tf;
	init_unit_test();
}

#ifdef ACQ_LOOP_RECV
static void test_recv(void)
{
	int cnt;
	const u32 limit_count = 100;
	int limit = 0;
	u32 value;

	while (1) {
		if (limit >= limit_count) {
			smc_get_value(ADDR_05_SYS_STATUS, &value);
			cnt = (*(reg05_sys_status_stru *)&value).corr_fifo_cnt;
			hwlog_err("%s: len:%d", __func__, cnt);
			limit = 0;
		}
		rsmc_sync_read_write(g_spi_transfer, DATA_MAX_LEN);
		limit++;
	}
}
#endif

static void spi_test(void)
{
#ifdef SPI_TEST
	wait_chip_ready();
	read_all_dreg();
	read_all_rfreg();
	spi_write_read_test();
	bb_init();
	spi_write_read_test();
	bb_init();
#endif
}

const static u32 info_dat_spec_32k[5000] = {
// sync code  9ms  32k
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
	0x00000000,
// code
	0x0000ffff,
	0x0000ffff,
	0xffff0000,
	0xffffffff,
	0x00000000,
	0xffffffff,
	0x000f38b3, 0xc43470f4, 0x80333f83, 0x3bc4b873,
	0x7084cc77, 0x7c4838c3, 0x743f0f20, 0x841ef314,
	0xc845922f, 0x21083de4, 0x1012ab24, 0xf6c0007b,
	0xc8210606, 0x0cbd921a, 0x2790420f, 0x79495124,
	0x1f270084, 0x1ef20972, 0x197f4dd1, 0x383de410,
	0x82c07e93, 0x08b84bc8, 0x2107bd78, 0x6655748a,
	0x50420f79, 0x055602eb, 0x3531041e, 0xf2084025,
	0xc23f6186, 0x3de41083, 0xdf1e744b, 0xa7480821,
	0x07bc8218, 0x961ad735, 0x120f7904, 0x21bcbf0e,
	0xc20f5ef2, 0x0841ef7e, 0x5c910926, 0x441083de,
	0x418b00ba, 0xe76e2107, 0xbc821174, 0x25db8864,
	0x8f790420, 0xf7bb35ba, 0x497b0208, 0x41ef20db,
	0x6487e1dd, 0x0083de41, 0x09190dc9, 0x32ab27bc,
	0x82107a8b, 0x8635070c, 0x790420f7, 0x910ec2ae,
	0x99526841, 0xef208634, 0x1937e71a, 0x63de4108,
	0x3f784fce, 0xb2d73c82, 0x107bc86a, 0x8c61b963,
	0x0420f790, 0x42da6352, 0xe42001ef, 0x20841d97,
	0xb0cc01c7, 0x07bc8210, 0x7a527a53, 0xf6c67904,
	0x20f791d6, 0xad452674, 0x0841ef20, 0x854bcf19,
	0xfbb183de, 0x41083d34, 0x11776891, 0xbc82107b,
	0xc963fe81, 0x77b00420, 0xf7904277, 0x8a4d60d6,
	0x41ef2084, 0x1f049ed5, 0xfde0de41, 0x083de535,
	0xa373c337, 0x82107bc8, 0x2117f6c7, 0x7aac20f7,
	0x90420e6f, 0x2eff7a0e, 0xef20841e, 0xf34dffe4,
	0x0da94108, 0x3de411dd, 0xe03b5ab4, 0x107bc821,
	0x079463b1, 0x3e78f790, 0x420f786d, 0xc8545a64,
	0x20841ef2, 0x08c5b8a4, 0x9ffa083d, 0xe41082db,
	0x6b95fcab, 0x7bc82107, 0xbcc67ea9, 0x077a9042,
	0x0f79057f, 0xda2c5e85, 0x841ef208, 0x40a00cec,
	0x4b9e3de4, 0x1083dfbb, 0x78bf9cb3, 0xc82107bc,
	0x80b17e6c, 0x72fd420f, 0x790422fe, 0x52cd7508,
	0x1ef20841, 0xeff4cfff, 0x448be410, 0x83de4017,
	0xd4831f00, 0x2107bc82, 0x10ec022e, 0x42e4841e,
	0xf20840da, 0x4155386f, 0x3de41083, 0xdea2027a,
	0x552dc821, 0x07bc8250, 0xa1ee4df7, 0x420f7904,
	0x214b5639, 0xc6d81ef2, 0x0841eeed, 0x6788b515,
	0xe41083de, 0x404f99e0, 0x208c2107, 0xbc821172,
	0x81450a5e, 0x0f790420, 0xf6222a34, 0x1749f208,
	0x41ef2110, 0x3c6f9239, 0x1083de41, 0x097aff86,
	0x519e07bc, 0x82107bea, 0x08f76d51, 0x790420f7,
	0x90314ed2, 0x22810841, 0xef2084d9, 0xb10407c7,
	0x83de4108, 0x3dc0a887, 0x275bbc82, 0x107bc811,
	0x5f4ea58e, 0x0420f790, 0x43b497cb, 0xbced41ef,
	0x20841e6e, 0xc67cda15, 0xde41083d, 0xe486d39e,
	0x08288210, 0x7bc821b6, 0x295150f4, 0x20f79042,
	0x0efa0a29, 0xeb5def20, 0x841ef211, 0x4393bc73,
	0x41083de4, 0x13858dfa, 0x4538107b, 0xc821070e,
	0xb0db63c3, 0xf790420f, 0x7b693e67, 0x800b2084,
	0x1ef20a39, 0x9a44002a, 0x82107bc8, 0x21d5dc9a,
	0xc3df20f7, 0x90420f1b, 0x97871de5, 0xef20841e,
	0xf22e0a2a, 0xced34108, 0x3de41036, 0xd73d8627,
	0x107bc821, 0x07ab2b48, 0x5c8df790, 0x420f7874,
	0x90b19971, 0x20841ef2, 0x09342322, 0xf0e2083d,
	0xe410836e, 0xcdc16fd8, 0x7bc82107, 0xbc8bc68f,
	0xe2f09042, 0x0f79050f, 0x376dc38a, 0x841ef208,
	0x412e9ad6, 0x56223de4, 0x1083df3d, 0x848c6676,
	0xc82107bc, 0x83584988, 0xa92d420f, 0x79042031,
	0xbbfad35d, 0x1ef20841, 0xee33b4e7, 0xadade410,
	0x83de4141, 0xed735ac3, 0x2107bc82, 0x11daa2e1,
	0x94dd0f79, 0x0420f6cf, 0x49813915, 0xf20841ef,
	0x21930663, 0x2b4f1083, 0xde410886, 0xc454be77,
	0x07bc8210, 0x7addad79, 0xab6f7904, 0x20f793da,
	0x59fc7eb3, 0x0841ef20, 0x87e2f9a8, 0x702683de,
	0x41083c79, 0x5040cc44, 0xbc82107b, 0xc9f0c49c,
	0xda83f208, 0x41ef21a5, 0xa3bdac24, 0x1083de41,
	0x096f73d8, 0xe06707bc, 0x82107af2, 0xe4918ddb,
	0x790420f7, 0x90e90311, 0x1d050841, 0xef208513,
	0x362a5a3e, 0x83de4108, 0x3d6cf6be, 0xf715bc82,
	0x107bc9f8, 0x38ee6a4d, 0x0420f790, 0x43f9d654,
	0x983b41ef, 0x20841fb8, 0xb9353662, 0xde41083d,
	0xe478ca6e, 0x6deb8210, 0x7bc820d1, 0x9ccac38a,
	0x20f79042, 0x0f311d27, 0x376fef20, 0x841ef26f,
	0x1b2f8bc3, 0x41083de4, 0x1036f7b7, 0x2c07107b,
	0xc82106aa, 0x2a190d8d, 0xf790420f, 0x795c3abb,
	0x317a2084, 0x1ef20965, 0x3776e1a6, 0x083de410,
	0x82cc4d63, 0xef737bc8, 0x2107bcdf, 0xd7dfa3f4,
	0x90420f79, 0x042d97c5, 0x4b03841e, 0xf208417a,
	0x8bd24733, 0x3de41083, 0xdf9dac8c, 0x44fcc821,
	0x07bc8008, 0x1dddec6a, 0x420f7904, 0x2119bb7a,
	0x73f51ef2, 0x0841ec63, 0xb1e3e8e8, 0x7bc82107,
	0xbc6a185d, 0xd8f89042, 0x0f79049a, 0x3131f0db,
	0x841ef208, 0x413337b4, 0xb7f83de4, 0x1083dee7,
	0xebed73fa, 0xc82107bc, 0x83d78eb3, 0xe591420f,
	0x7904219f, 0x4f49ab9a, 0x1ef20841, 0xee8ec202,
	0x672ee410, 0x83de408c, 0xa44ef416, 0x2107bc82,
	0x101888ec, 0x7dfa0f79, 0x0420f699, 0x52717cf6,
	0xf20841ef, 0x20e1f2ec, 0xf8351083, 0xde41086f,
	0xf15a48c5, 0x07bc8210, 0x7af6b185, 0x8d9f7904,
	0x20f79149, 0x29b13daf, 0x0841ef20, 0x8542272e,
	0x1b3f83de, 0x41083c44, 0x7cbcd71f, 0xbc82107b,
	0xc9bc3dfe, 0x3a190420, 0xf790425b, 0x56f43039,
	0x41ef2084, 0x1ef9b820, 0x2336de41, 0x083de4da,
	0xe0446d61, 0x82107bc8, 0x2084988f, 0xc6ca20f7,
	0x90420f13, 0x9fa5bdee, 0xef20841e, 0xf26f4b7e,
	0x8e964108, 0x3de4133c, 0xd53f8c8f, 0x107bc821,
	0x04be2e5c, 0x199c41ef, 0x20841f27, 0x7a0b6ece,
	0xde41083d, 0xe556965f, 0xb5078210, 0x7bc8206d,
	0xeb2f4d1d, 0x20f79042, 0x0ffe7e3a, 0x9b12ef20,
	0x841ef3b9, 0x707263e5, 0x41083de4, 0x10e0c4ed,
	0x63ef107b, 0xc82107d9, 0x8a928fa6, 0xf790420f,
	0x781d0f1d, 0xc7e92084, 0x1ef2085b, 0x7f8e9747,
	0x083de410, 0x837f35a6, 0x066d7bc8, 0x2107bd6a,
	0x5918c9ad, 0x90420f79, 0x049a1193, 0xdaf3841e,
	0xf2084033, 0x67e0e6fd, 0x3de41083, 0xde6f6165,
	0xd35ac821, 0x07bc82c2, 0xdee3b5d4, 0x420f7904,
	0x21bfe7e9, 0x89111ef2, 0x0841efce, 0xc712327b,
	0xe41083de, 0x408ca4e4, 0xd49f2107, 0xbc821149,
	0x88b828fa, 0x0f790420, 0xf71352f1, 0x745ff208,
	0x41ef21b4, 0xa6fcec70, 0x1083de41, 0x08cf59da,
	0xcac407bc, 0x82107be3, 0xa095dcce, 0x790420f7,
	0x92e989bb, 0x9d8c0841, 0xef208502, 0x332e5e3d,
	0x2107bc82, 0x1059d8fd, 0x39ab0f79, 0x0420f61b,
	0x78f15e5c, 0xf20841ef, 0x21a0f2bc, 0xed301083,
	0xde4109ef, 0x7b70c26f, 0x07bc8210, 0x7ab3f180,
	0xcc8a7904, 0x20f7906b, 0x8bbbb7a7, 0x0841ef20,
	0x8512222f, 0x0f7b83de, 0x41083dce, 0x74347d1d,
	0xbc82107b, 0xc9fd7def, 0x7e4c0420, 0xf7904351,
	0xf6763a93, 0x41ef2084, 0x1facac74, 0x3767de41,
	0x083de472, 0x6a6ee7e3, 0x82107bc8, 0x2095dd9e,
	0x838a20f7, 0x90420f13, 0x158f37ef, 0xef20841e,
	0xf26e5e6e, 0x8bc24108, 0x3de4109c, 0x75b72c2f,
	0x107bc821, 0x06aa7e58, 0x08d9f790, 0x420f797e,
	0x981393f8, 0x20841ef2, 0x09702772, 0xe0b7083d,
	0xe410826e, 0xcd49cfdb, 0x7bc82107, 0xbcde829e,
	0xe3a49042, 0x0f790505, 0x1fed4b01, 0x841ef208,
	0x406bcb82, 0x53723de4, 0x1083de9f, 0xa40e4ef6,
	0xc82107bc, 0x801c49c9, 0xb83e2084, 0x1ef2091a,
	0x2adad312, 0x083de410, 0x82751d0c, 0xae467bc8,
	0x2107bc3e, 0x4d4d8ced, 0x90420f79, 0x04189119,
	0x7af3841e, 0xf2084167, 0x67a1f7f8, 0x3de41083,
	0xdeefe967, 0x59f8c821, 0x07bc8383, 0x9ff2b480,
	0x420f7904, 0x2197c569, 0xab921ef2, 0x0841eedf,
	0x8317333f, 0xe41083de, 0x40acac6c, 0xdc942107,
	0xbc821059, 0x8ded7dbe, 0x0f790420, 0xf6317ad1,
	0x7ed4f208, 0x41ef20f0, 0xe2e8ec70, 0x1083de41,
	0x08cf51d2, 0xe84f07bc, 0x82107af7, 0xa095c89b,
	0x790420f7, 0x91e98193, 0x1f2d0841, 0xef208512,
	0x772f1a3e, 0x83de4108, 0x3c467cb6, 0xd797bc82,
	0x107bc9e9, 0x38af3b1d, 0x0420f790, 0x42f176fe,
	0x921941ef, 0x20841eb9, 0xf8356677, 0xde41083d,
	0xe55ae8c4, 0xc7e18210, 0x7bc82180, 0xc8de978e,
	0x20f79042, 0x0e19bfa5, 0x15eeef20, 0x841ef32f,
	0x5e6a9ac3, 0xbc82107b, 0xc8cb8920, 0xad330420,
	0xf7904390, 0xc9dacc09, 0x41ef2084, 0x1ec2b19d,
	0x0597de41, 0x083de543, 0x98ab8e75, 0x82107bc8,
	0x2034520d, 0xa98320f7, 0x90420f84, 0xb359a69c,
	0xef20841e, 0xf262e74d, 0x6b0d4108, 0x3de411ee,
	0xb2fe99aa, 0x107bc821, 0x0731b867, 0x5534f790,
	0x420f7852, 0x46a8e39d, 0x20841ef2, 0x089c54d2,
	0x3e30083d, 0xe4108221, 0x0ef46b85, 0x7bc82107,
	0xbc1cecc6, 0x4fc79042, 0x0f7904f1, 0x261f04c3,
	0x841ef208, 0x401d7e09, 0x90183de4, 0x1083de7e,
	0x13001ace, 0xc82107bc, 0x82365061, 0xde9d420f,
	0x79042002, 0x6b9fb8c8, 0x1ef20841, 0xef967f30,
	0x92a0e410, 0x83de41f4, 0x49a74331, 0x2107bc82,
	0x10830f83, 0x34060f79, 0x0420f415, 0x0e688cb9,
	0xf20841ef, 0x221c9109, 0x22b61083, 0xde410b08,
	0x3a676e19, 0x07bc8210, 0x78248a9d, 0x64fc1ef2,
	0x0841ee51, 0x416c2f86, 0xe41083de, 0x40aa587f,
	0x2cdb2107, 0xbc8210e4, 0xaa5de26d, 0x0f790420,
	0xf6f611c6, 0xd28af208, 0x41ef2023, 0x99b01147,
	0x1083de41, 0x08994208, 0x0d0507bc, 0x82107a85,
	0x501b4ff1, 0x790420f7, 0x90221ebf, 0xcb3d0841,
	0xef208569, 0x3e876cce, 0x83de4108, 0x3c7d2c79,
	0xb6aabc82, 0x107bc818, 0xe6791540, 0x0420f790,
	0x42e6f280, 0xab6341ef, 0x20841fb0, 0x440782fd,
	0xde41083d, 0xe4002787, 0xf8458210, 0x7bc8201f,
	0x1be4ca62, 0x20f79042, 0x0f95e91e, 0xc709ef20,
	0x841ef2c6, 0x38ce1551, 0x41083de4, 0x11f39e00,
	0x0ad1107b, 0xc821076c, 0x5010f0ea, 0xf790420f,
	0x78a8a1c3, 0xde332084, 0x1ef20a07, 0xd7a8768f,
	0x083de410, 0x81a7f8e7, 0x3b4a7bc8, 0x2107bda5,
	0x8f778455, 0x90420f79, 0x05b64728, 0x0217841e,
	0xf208428b, 0x0415682e, 0x107bc821, 0x06034cb8,
	0x825bf790, 0x420f789b, 0x53a6bd07, 0x20841ef2,
	0x08a74d7e, 0x08d4083d, 0xe4108238, 0x56b1a2bb,
	0x7bc82107, 0xbde82640, 0x318a9042, 0x0f7904e4,
	0x224b9711, 0x841ef208, 0x4100d62f, 0x71823de4,
	0x1083de04, 0xf4690f42, 0xc82107bc, 0x82add20b,
	0xd221420f, 0x790421a6, 0x1d064a07, 0x1ef20841,
	0xef3b0c90, 0x4d22e410, 0x83de419b, 0x2890c565,
	0x2107bc82, 0x111164da, 0xc8210f79, 0x0420f7c9,
	0x17b84b7a, 0xf20841ef, 0x213e2093, 0xb49d1083,
	0xde4109e3, 0x2fcb1881, 0x07bc8210, 0x7b4ad674,
	0x175c7904, 0x20f79084, 0x400431fb, 0x0841ef20,
	0x85910962, 0xa21d83de, 0x41083dba, 0x6766127f,
	0xbc82107b, 0xc9da9920, 0xb9230420, 0xf7904210,
	0x49784c80, 0x41ef2084, 0x1e83e0dc, 0x1183de41,
	0x083de4c1, 0xb2a306fc, 0x82107bc8, 0x23705608,
	0xa9d50841, 0xef20840f, 0x8b5dbaf0, 0x83de4108,
	0x3db693dd, 0x4232bc82, 0x107bc963, 0xfa8066e0,
	0x0420f790, 0x43758247, 0x6af441ef, 0x20841f00,
	0xda94ecf4, 0xde41083d, 0xe5b7a179, 0xeb978210,
	0x7bc82016, 0xb7967fb9, 0x20f79042, 0x0f4daeff,
	0xf08cef20, 0x841ef31d, 0xefb51ca8, 0x41083de4,
	0x11f7c019, 0xda35107b, 0xc8210691, 0x27e07a2d,
	0xf790420f, 0x79ef6a74, 0xfae62084, 0x1ef20981,
	0xfde0defb, 0x083de410, 0x83f9e9b5, 0xf6ab7bc8,
	0x2107bdc3, 0x6ee8036a, 0x90420f79, 0x05d7f004,
	0x56a7841e, 0xf20841f5, 0x09bc4bce, 0x3de41083,
	0xdf3bf29f, 0x9cb8c821, 0x07bc82a0, 0x6b6c26ef,
	0x420f7904, 0x21de5245, 0xf5881ef2, 0x0841efb0,
	0x9beb018b, 0xe41083de, 0x41b7d483, 0x9fa02107,
	0xbc8210a9, 0x176f07a3, 0x0f790420, 0xf5a45ca5,
	0xef85f208, 0x41ef21b9, 0x4b9f18bd, 0xc82107bd,
	0xe2d4d06d, 0x083de413, 0xe7c26ab1, 0xef20841e,
	0xb1ffcab9, 0x0420f7de, 0xea458f21, 0x07bc8372,
	0x2e56283d, 0xe4108823, 0xf57e7f20, 0x841ef703,
	0x71c9c420, 0xf7908b8e, 0x48ee07bc, 0x8212d2ef,
	0x7501e410, 0x83da3bf9, 0x1670841e, 0xf237c211,
	0x70a0f790, 0x42559f90, 0xa8bc8210, 0x785e0fe7,
	0x081083de, 0x492a6877, 0xf41ef208, 0x51693774,
	0x3790420f, 0x1dbaa45c, 0x82107bcb, 0xd7a84fe4,
	0x83de4105, 0x122a16fe, 0xf20841d9, 0x78973410,
	0x420f7895, 0xf8f1d910, 0x7bc821e7, 0x0c57d3de,
	0x41083063, 0xaad14208, 0x41ef3fa8, 0x063a083d,
	0xe413204d, 0xe257bc82, 0x1009b81a, 0x9e41ef20,
	0x84ccfc94, 0xfb790420, 0xfbb4a2fd, 0xf82107bc,
	0xad354feb, 0xc83de410, 0x2f7f4849, 0xef20841f,
	0xf042e4e5, 0x0420f792, 0x13372b01, 0x07bc823b,
	0xb178517d, 0xe410839b, 0x979bf620, 0x841ef096,
	0xd73f2820, 0xf7904eac, 0xb92127bc, 0x82106dc9,
	0x0119e410, 0x83de1d18, 0xc9e8841e, 0xf2084645,
	0xeb6cf790, 0x42022f1f, 0x7e8c8210, 0x7bd0fb7e,
	0xd45083de, 0x41cba7b1, 0xc41ef208, 0x43bd2e0e,
	0xc390420f, 0x79316766, 0xf2107bc8, 0x69191f07,
	0x43de4109, 0x0cad7dfe, 0xf20841e9, 0xc9e7fbf0,
	0x420f791e, 0xf73d2c81, 0xef2084a2, 0xb195c9e4,
	0x1083dc98, 0x9913c210, 0x7bc84673, 0x7a480000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const static u32 info_dat_phone_32k[5000] = {
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x0FFF0F00, 0x00F00F0F, 0x0FF0F00F, 0x00F0000F,
	0xF00FF000, 0x0F0000FF, 0x000F0F0F, 0x0F00F0F0,
	0x00F0F0F0, 0xF00000F0, 0xFF0F00F0, 0x00F0FF0F,
	0x0000FFF0, 0x000F0000, 0x0F00FFF0, 0x000F00FF,
	0xF0FF0F0F, 0xFF000FFF, 0x00F00000, 0xF000FFF0,
	0xFF0F0FFF, 0x00FFFF0F, 0xF0F0F0F0, 0x0F0000F0,
	0xF0000FF0, 0xFFF00FF0, 0x000FF00F, 0x0F0000FF,
	0x0FFFF0F0, 0x0F000F00, 0x00F000F0, 0x0000F000,
	0x0F00F000, 0x0F000000, 0xF000F000, 0x000F00F0,
	0x00FFF00F, 0xF0F00F00, 0x00FFF000, 0x0F00FFF0,
	0xF0FF0FF0, 0x00F0000F, 0x00000000, 0x00FF00F0,
	0x00000FF0, 0x0FFF0FFF, 0x00F0FF00, 0xF0F000FF,
	0x00F0000F, 0x000FF0F0, 0xFF000FF0, 0xF0F0000F,
	0x00F0F0F0, 0x0FF0F0FF, 0x0F0F0F00, 0xFFFF000F,
	0x00000F00, 0x0FF000FF, 0x0F0000FF, 0xF00F00F0,
	0x00F00F00, 0xF0FF0F0F, 0xF0F00000, 0x0F00F00F,
	0xFF0FFFF0, 0xFFF000FF, 0xF0F0FF00, 0xFF00F00F,
	0x0FF0F00F, 0x000FF0F0, 0x00000FFF, 0x0000F000,
	0x0000FFFF, 0x00F000FF, 0x0F0F0F00, 0xFF00000F,
	0x00000F00, 0x0FF000FF, 0x0F0000FF, 0xF00F0F00,
	0x000F000F, 0x00FF000F, 0xF0F000FF, 0x0FF0F0FF,
	0xFF0F00F0, 0xFFF00000, 0x0000FF0F, 0xFFF0FF00,
	0x00000000, 0x0000FFF0, 0xFFF000F0, 0x00F0FF00,
	0xF000000F, 0x0000F0FF, 0x0F0F0000, 0xFFF00F0F,
	0xF0FFF000, 0x00F00F00, 0x0F0FF0FF, 0x0F0F0FF0,
	0xF0FF00F0, 0x00F0F00F, 0x0FF00000, 0x0000F000,
	0xFF00FF00, 0x0F000F0F, 0x0000000F, 0xFFF00F0F,
	0x0F0F0000, 0xFF00F000, 0x000F00F0, 0x0000F00F,
	0xFF000FFF, 0x0F0FF0FF, 0xF00F0FF0, 0xFFF00F00,
	0x00FF0F00, 0x0F000FFF, 0x0F0000FF, 0xF00F0FF0,
	0x00F00F00, 0xF000F000, 0x00F00F0F, 0x000000FF,
	0xF00F00F0, 0x00F000FF, 0xF000F00F, 0xFFF0F0F0,
	0x0FFF0000, 0x0F00F000, 0x0000000F, 0x000FFF0F,
	0x0F00F0F0, 0x0000F0FF, 0xF00F0F00, 0xFFFF0000,
	0xF0FF0F00, 0x0FF0FFFF, 0x0F0000FF, 0xF00F0F00,
	0xF0FF0FF0, 0x000FFF00, 0xF0000F0F, 0x0FFFF000,
	0x0F00F0F0, 0xF0FF0FFF, 0x0000F00F, 0xF0000000,
	0x00FFF000, 0x0F00F000, 0xFF000000, 0xF0FF0F00,
	0x0FF0FFFF, 0x0F0000FF, 0xF00F0F00, 0xFF000000,
	0xF0FF0F00, 0x0FF0FFFF, 0x0F0000FF, 0xF00F00F0,
	0x000000F0, 0xF0FF0F00, 0xF0F0FF0F, 0x0F0F0000,
	0x0F0000F0, 0x00FF0F0F, 0xF0000F00, 0xF000FFF0,
	0x0F0FF0FF, 0x0F0FF000, 0x0F0F00FF, 0x0000000F,
	0x00F00F0F, 0x000FF0F0, 0x0F00F0F0, 0xF00F00F0,
	0xF00FF00F, 0xFFF0FF00, 0x00F0F0F0, 0xF000F0F0,
	0xF0F0000F, 0xF0F0FF00, 0x0FF0FFF0, 0x00FF00FF,
	0x0F0FFF00, 0xF00F0000, 0xF000F000, 0xF0000FFF,
	0x0000F0FF, 0xF00FFFF0, 0xF0F000FF, 0x00F00F00,
	0x0FF000FF, 0x0F00F0F0, 0xF000FF00, 0xF00F00FF,
	0x000F0F0F, 0xF000FFFF, 0x00FF00F0, 0x0F00F0F0,
	0xFF000000, 0xF0FF0000, 0xFF000F00, 0x0F0000FF,
	0xFF0FFF00, 0x0000000F, 0xF0FFFF0F, 0x0FF00FFF,
	0x0F0F0000, 0xFF000000, 0x0F000FFF, 0xF000FF0F,
	0x0000000F, 0x0FF0000F, 0x0F0F0000, 0x0FFF0000,
	0x00F0F000, 0x0F00F000, 0x0F00F00F, 0xF00F0FF0,
	0x0F0F00F0, 0x00F00F00, 0x0FF00F00, 0x00FFF000,
	0x00000F00, 0xF00F0F0F, 0x00FFFF0F, 0x0000FF0F,
	0x000FF000, 0xFF000000, 0xF0000FF0, 0x00F00F00,
	0x00F00F00, 0x00F0F00F, 0x0F0F0000, 0x0F000000,
	0x00F0F000, 0x0F00F000, 0x0F00F00F, 0xF00F0000,
	0x0FF00FFF, 0xF0F00000, 0x0FF00FFF, 0x000FF0F0,
	0x0000F000, 0xF00F0FF0, 0xF00FFF00, 0x00F0F000,
	0x0FFF000F, 0xFF0F0F00, 0x0FF000FF, 0x00000000,
	0xF0F0F0F0, 0x0000000F, 0x0F0F0F00, 0x0FF00F00,
	0xF00F0F00, 0x0000FFFF, 0x0F0F000F, 0x0F0F00F0,
	0xFF000F00, 0xF0FFF000, 0xF0F00F00, 0x0FFFF00F,
	0x000F0FF0, 0x00FF00FF, 0xF00F0000, 0x00F0000F,
	0x00F0000F, 0x000F00F0, 0xF00F00FF, 0x00F00F0F,
	0xFFF0FF00, 0x0F0F000F, 0xF00F00F0, 0x0FF00F0F,
	0x000FF000, 0x0FF0FF00, 0x0F00F00F, 0xF00F00F0,
	0x0F0F00F0, 0x000FF00F, 0xFFF0000F, 0x0FFF00F0,
	0x0F00F000, 0x0F0F0F0F, 0x000FF000, 0x00F0FFF0,
	0x0000000F, 0xF00F00F0, 0xF0000000, 0x00FF000F,
	0x0FF0000F, 0x0000000F, 0xF00F0000, 0x0FFF000F,
	0xF00FF000, 0x0F000F00, 0x0F00F00F, 0xF00F0000,
	0xFF000000, 0xF000FF0F, 0x0F00000F, 0x0000F00F,
	0xF00F0000, 0xFF00000F, 0xF00FF000, 0x0F000F00,
	0x0F00F00F, 0xF00F00F0, 0x0F00000F, 0xF00FF000,
	0x0F000F00, 0x0F00F00F, 0xF00F0000, 0x0F00000F,
	0xF00FF000, 0x0F000F00, 0x0F00F00F, 0xF00F0FF0
};
static u8 datainfo[5000] = {0};
static u32 info_dat_phone_8k[5000] = {0};

const char *polar_256 = "00000000000000000000000000000000000000000000000000000000000000000000000000000000742569219843154a2a82d22d0e104e13b5c7208ed73daa4286e619437a4422084840881239a4384eb621003206772ca3211ac6a12a6b54f10463439224b5a049dee3acc9691a07080f2354c1046343941131a36bd2e00dec000ee22c810b50e5b8245b56b2296008cc4501e550c81209c75b96e43447439624882503922389ea7048011d4a0b94f0b46f4394b61c85784ab709803848c0b46f4394c0b46f439202b4ad504235848e5b585301251a4a9299ec2a8aa1ac6e335c9088870b9ea324634a8c93158f324ac0b0c443dc01bd6750c0478d01615070284849965224643804953d0d18c08624242950402848499067a0671a08969c2871d46300aa015464940f5152c4b8a4791633902121129325ec519265186c49925219e1724855182e019280316101907198444990c08d410990c19844499241984449904198444996";

#define NIBBLES_A_BYTE 2
#define HIGH_FOUR_BITS 0xf0
#define LOW_FOUR_BITS 0x0f
#define RADIX_DEC 10
#define RADIX_HEX 16
#define SHIFT_HALF_BYTE 4

static int ascii2hex(const char *ascii, unsigned char *hex, int ascii_len)
{
	unsigned char nibble[NIBBLES_A_BYTE];
	int hex_len;
	int id;
	int cnt;

	if (ascii == NULL)
		return -EINVAL;
	if (hex == NULL)
		return -EINVAL;
	if ((ascii_len % NIBBLES_A_BYTE) == 1)
		return -EINVAL;
	hex_len = ascii_len / NIBBLES_A_BYTE;
	for (id = 0; id < hex_len; ++id) {
		nibble[0] = *(ascii++);
		nibble[1] = *(ascii++);
		for (cnt = 0; cnt < NIBBLES_A_BYTE; ++cnt) {
			if (nibble[cnt] >= 'A' && nibble[cnt] <= 'F')
				nibble[cnt] = nibble[cnt] - 'A' + RADIX_DEC;
			else if (nibble[cnt] >= 'a' && nibble[cnt] <= 'f')
				nibble[cnt] = nibble[cnt] - 'a' + RADIX_DEC;
			else if (nibble[cnt] >= '0' && nibble[cnt] <= '9')
				nibble[cnt] -= '0';
			else
				return -EINVAL;
		}
		hex[id] = nibble[0] << SHIFT_HALF_BYTE; // Set the high nibble
		hex[id] |= nibble[1];       // Set the low nibble
	}
	return 0;
}

static void to_int_array(unsigned char *hex, unsigned int *array, int len)
{
	const u32 int2byte = 4;
	const u32 byte1_bit = 24;
	const u32 byte2_bit = 16;
	const u32 byte3_bit = 8;
	int index;

	for (index = 0; index < len / int2byte; index++) {
		array[index] = 0;
		array[index] += hex[index * int2byte] << byte1_bit;
		array[index] += hex[index * int2byte + IDX_1] << byte2_bit;
		array[index] += hex[index * int2byte + IDX_2] << byte3_bit;
		array[index] += hex[index * int2byte + IDX_3];
	}
}

static int send_data_for_phone_8k(void)
{
	const u32 default_m_phs = 0xFFFFFF;
	const u32 default_m2_phs = 0xFFFFFF;
	const u32 default_code_nco = 0x1B333333;
	const u32 default_sync_len = 8 * 40;
	const u32 default_info_len = 8 * 700;
	int ret;

	struct tx_data_msg *msg = kmalloc(
		sizeof(struct tx_data_msg) + sizeof(info_dat_phone_8k),
		GFP_ATOMIC);
	if (msg == NULL)
		return -EINVAL;

	u32 *frame = (u32 *)(msg + 1);

	msg->freq_point = 1;
	msg->m_phs       = default_m_phs;
	msg->m2_phs = default_m2_phs;
	msg->code_nco    = default_code_nco;
	msg->sync_len    = default_sync_len;
	msg->info_len    = default_info_len;
	msg->frame_len = sizeof(info_dat_phone_8k);

	ret = ascii2hex(polar_256, datainfo, strlen(polar_256));
	to_int_array(datainfo, info_dat_phone_8k, sizeof(polar_256) / TIMES_2);

	ret = memcpy_s(frame, sizeof(struct tx_data_msg) + sizeof(info_dat_phone_8k),
		info_dat_phone_8k, sizeof(info_dat_phone_8k));
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s failed", __func__);
		return -EINVAL;
	}

	return send_data(msg);
}

static int send_data_for_phone_32k(void)
{
	const u32 default_m_phs = 0xFFFFFF;
	const u32 default_m2_phs = 0xFFFFFF;
	const u32 default_code_nco = 0x1B333333;
	const u32 default_sync_len = 32 * 40;  // 40ms sync code
	const u32 default_info_len = 32 * 700; // 660ms data
	int ret;

	struct tx_data_msg *msg = kmalloc(
		sizeof(struct tx_data_msg) + sizeof(info_dat_phone_32k),
		GFP_ATOMIC);
	if (msg == NULL)
		return;

	u32 *frame = (u32 *)(msg + 1);

	msg->freq_point = 1;
	msg->m_phs = default_m_phs;
	msg->m2_phs = default_m2_phs;
	msg->code_nco = default_code_nco;
	msg->sync_len = default_sync_len;
	msg->info_len = default_info_len;
	msg->frame_len = sizeof(info_dat_phone_32k);

	ret = memcpy_s(frame, sizeof(struct tx_data_msg) + sizeof(info_dat_phone_32k),
		info_dat_phone_32k, sizeof(info_dat_phone_32k));
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s failed", __func__);
		return -EINVAL;
	}

	return send_data(msg);
}

static int send_data_for_spec_32k(void)
{
	const u32 default_m_phs = 0xFFFFFF;
	const u32 default_m2_phs = 0xFFFFFF;
	const u32 default_code_nco = 0x1B333333;
	// 32KHz * (sync code len + Walsh code len)ms + 52bit * 32ksps/8kbps
	const u32 default_sync_len = 32 * (9 + 6) + 52 * 4;
	const u32 default_info_len = 1000 * 32;
	int ret;

	struct tx_data_msg *msg = kmalloc(
		sizeof(struct tx_data_msg) + sizeof(info_dat_spec_32k),
		GFP_ATOMIC);
	if (msg == NULL)
		return -EINVAL;

	u32 *frame = (u32 *)(msg + 1);

	msg->freq_point = 1;
	msg->m_phs = default_m_phs;
	msg->m2_phs = default_m2_phs;
	msg->code_nco = default_code_nco;
	msg->sync_len = default_sync_len;
	msg->info_len = default_info_len;
	msg->frame_len = sizeof(info_dat_spec_32k);

	ret = memcpy_s(frame, sizeof(struct tx_data_msg) + sizeof(info_dat_spec_32k),
		info_dat_spec_32k, sizeof(info_dat_spec_32k));
	if (ret != EOK) {
		hwlog_err("%s: memcpy_s failed", __func__);
		return -EINVA;
	}

	return send_data(msg);
}

static void spec_send_test(void)
{
#ifdef SEND_TEST_FOR_SPEC
	const u32 delay_time_ms = 2000;
	int loop = 5;
	int ret;
	struct tx_data_msg data_msg;

	set_chip_mode(MODE_TX);
	while (loop > 0) {
		ret = send_data_for_spec_32k();
		msleep(delay_time_ms);
		loop--;
	}
	set_chip_mode(MODE_RX);
#endif
}

static void phone_send_test(void)
{
#ifdef SEND_TEST_FOR_PHONE
	const u32 delay_time_ms = 2000;
	const u32 tx_power = 5;
	const int freq_offset = -700;
	int loop = 5;
	int ret;
	struct tx_data_msg data_msg;

	set_chip_mode(MODE_TX);
	while (loop > 0) {
		update_tx_power(tx_power);
		update_tx_freq(freq_offset);
		ret = send_data_for_phone_8k();
		msleep(delay_time_ms);
		loop--;
	}
	set_chip_mode(MODE_RX);
	ret = send_data_for_phone_32k();
#endif
}

static void pps_test(void)
{
#ifdef TEST_PPS
	test_pps();
#endif
}

static void recv_test(void)
{
#ifdef ACQ_LOOP_RECV
	test_recv();
#endif
}

static void rf_test(void)
{
#ifdef RF_TEST
	const u32 tx_power6 = 6;
	const u32 tx_power15 = 15;
	const int rf_test = 3;
	const int rf_testcase1 = 1;
	const int rf_testcase2 = 2;
	const int rf_testcase3 = 3;
	const u32 delay_time_ms = 5000;
	struct freq_off_est_entry g_freq_est_entry;

	g_freq_est_entry.enable = 1;
	g_freq_est_entry.frequency = 1;
	g_freq_est_entry.power = tx_power6;

	wait_chip_ready();
	bb_init();
	set_chip_mode(MODE_RX);
	while (1) {
		if (rf_test == rf_testcase1) {
			update_tx_power(tx_power6);
			set_chip_mode(MODE_TX);
			send_data_for_phone_8k();
			set_chip_mode(MODE_RX);
		} else if (rf_test == rf_testcase2) {
			update_tx_power(tx_power15);
			set_chip_mode(MODE_TX);
			send_data_for_phone_8k();
			set_chip_mode(MODE_RX);
		} else if (rf_test == rf_testcase3) {
			set_chip_mode(MODE_TX);
			channel_freq_offset_proc(&g_freq_est_entry);
			set_chip_mode(MODE_RX);
		} else {
			spi_test();
		}
		msleep(delay_time_ms);
		read_all_dreg();
		read_all_rfreg();
	}
#endif
}

static int rsmc_driver_test_thread(void *data)
{
	hwlog_err("%s: in", __func__);
	spi_test();
	spec_send_test();
	phone_send_test();
	pps_test();
	recv_test();
	rf_test();
	hwlog_info("%s: out", __func__);
	return 1;
}
#endif

void init_unit_test(void)
{
#ifdef RSMC_DRIVER_TEST
	kthread_run(rsmc_driver_test_thread, NULL, "rsmc_driver_test_thread");
#endif
}

