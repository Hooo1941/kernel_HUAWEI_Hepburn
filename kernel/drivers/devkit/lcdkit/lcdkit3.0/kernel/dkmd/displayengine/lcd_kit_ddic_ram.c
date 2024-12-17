/*
 * lcd_kit_ddic_ram.c
 *
 * ddic ram dkmd function in lcd
 *
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd.
 *
 */
#include "lcd_kit_adapt.h"
#include "lcd_kit_common.h"
#include "lcd_kit_disp.h"


#include <linux/fs.h>
#include <securec.h>

#define LCD_KIT_CONFIG_TABLE_MAX_NUM            (2 * PAGE_SIZE)
#define LCD_KIT_HEX_BASE                        ((char)16)
#define PANEL_NAME_DDIC_TYPE                    4
#define WRITE_4_BYTE                            4
#define READ_48_BYTE                            48
#define READ_4_BYTE                             4
#define WRITE_CMD_NUM_ONCE                      12
#define READ_CMD_NUM_ONCE                       2
#define READ_FREAM_LEN                          (READ_48_BYTE * READ_CMD_NUM_ONCE)
#define WRITE_FREAM_LEN                         (WRITE_4_BYTE * WRITE_CMD_NUM_ONCE)

#define HIS_DSI_REG_ADDR_24TO31_OFFSET          50
#define HIS_DSI_REG_ADDR_16TO23_OFFSET          56
#define HIS_DSI_REG_ADDR_00TO07_OFFSET          62
#define HIS_DSI_REG_ADDR_08TO15_OFFSET          110

#define HIS_DSI_REG_VAL_00TO07_OFFSET           116
#define HIS_DSI_REG_VAL_08TO15_OFFSET           122
#define HIS_DSI_REG_VAL_16TO23_OFFSET           128
#define HIS_DSI_REG_VAL_24TO31_OFFSET           134

#define HIS_DSI_RW_BYTES_OFFSET                 104
#define HIS_DTYPE_DCS_LWRITE_OFFSET             68

#define HIS_READ_DSI_TAIL_START_OFFSET          106
#define HIS_WRITE_DSI_TAIL_START_OFFSET         112
#define READ_DDIC_BUF_LEN                       200
#define DEBUG_OK_FLAG                           1
#define MIPI_FLAG_PATH                          "/data/mipi_flag.bin"
#define LCD_KIT_READBIN_FILE_PATH               "/data/ddic.bin"

enum lcd_kit_ddic_type {
	LCD_KIT_NONE_DDIC,
	LCD_KIT_HIS_DDIC,
	LCD_KIT_RAYDIUM_DDIC,
};

struct lcd_kit_dsi_ctrl_hdr {
	char dtype; /* data type */
	char last;  /* last in chain */
	char vc;    /* virtual chan */
	char ack;   /* ask ACK from peripheral */
	char wait;  /* ms */
	char waittype;
	char dlen;  /* 8 bits */
};

static char g_map_itoa[][3] = {"00", "01", "02", "03", "04", "05", "06", "07", "08", "09", "0A", "0B", "0C", "0D", "0E", "0F",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "1A", "1B", "1C", "1D", "1E", "1F",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "2A", "2B", "2C", "2D", "2E", "2F",
	"30", "31", "32", "33", "34", "35", "36", "37", "38", "39", "3A", "3B", "3C", "3D", "3E", "3F",
	"40", "41", "42", "43", "44", "45", "46", "47", "48", "49", "4A", "4B", "4C", "4D", "4E", "4F",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59", "5A", "5B", "5C", "5D", "5E", "5F",
	"60", "61", "62", "63", "64", "65", "66", "67", "68", "69", "6A", "6B", "6C", "6D", "6E", "6F",
	"70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "7A", "7B", "7C", "7D", "7E", "7F",
	"80", "81", "82", "83", "84", "85", "86", "87", "88", "89", "8A", "8B", "8C", "8D", "8E", "8F",
	"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "9A", "9B", "9C", "9D", "9E", "9F",
	"A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9", "AA", "AB", "AC", "AD", "AE", "AF",
	"B0", "B1", "B2", "B3", "B4", "B5", "B6", "B7", "B8", "B9", "BA", "BB", "BC", "BD", "BE", "BF",
	"C0", "C1", "C2", "C3", "C4", "C5", "C6", "C7", "C8", "C9", "CA", "CB", "CC", "CD", "CE", "CF",
	"D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "DA", "DB", "DC", "DD", "DE", "DF",
	"E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7", "E8", "E9", "EA", "EB", "EC", "ED", "EE", "EF",
	"F0", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "FA", "FB", "FC", "FD", "FE", "FF"};

static bool lcd_kit_is_valid_char(char ch)
{
	if (ch >= '0' && ch <= '9')
		return true;
	if (ch >= 'a' && ch <= 'f')
		return true;
	if (ch >= 'A' && ch <= 'F')
		return true;
	return false;
}

static char lcd_kit_hex_char_to_value(char ch)
{
	switch (ch) {
	case 'a' ... 'f':
		ch = 10 + (ch - 'a');
		break;
	case 'A' ... 'F':
		ch = 10 + (ch - 'A');
		break;
	case '0' ... '9':
		ch = ch - '0';
		break;
	}

	return ch;
}

static int lcd_kit_parse_u8_digit(char *in, char *out, int max)
{
	unsigned char ch = '\0';
	unsigned char last_char = 'Z';
	unsigned char last_ch = 'Z';
	int j = 0;
	int i = 0;
	int len;

	if (!in || !out) {
		LCD_KIT_ERR("in or out is null\n");
		return LCD_KIT_FAIL;
	}
	len = strlen(in);
	LCD_KIT_DEBUG("LEN = %d\n", len);
	while (len--) {
		ch = in[i++];
		if (last_ch == '0' && ((ch == 'x') || (ch == 'X'))) {
			j--;
			last_char = 'Z';
			continue;
		}
		last_ch = ch;
		if (!lcd_kit_is_valid_char(ch)) {
			last_char = 'Z';
			continue;
		}
		if (last_char != 'Z') {
			/*
			 * two char value is possible like F0,
			 * so make it a single char
			 */
			--j;
			if (j >= max) {
				LCD_KIT_ERR("number is too much\n");
				return LCD_KIT_FAIL;
			}
			out[j] = (out[j] * LCD_KIT_HEX_BASE) +
				lcd_kit_hex_char_to_value(ch);
			last_char = 'Z';
		} else {
			if (j >= max) {
				LCD_KIT_ERR("number is too much\n");
				return LCD_KIT_FAIL;
			}
			out[j] = lcd_kit_hex_char_to_value(ch);
			last_char = out[j];
		}
		j++;
	}
	return j;
}

static void lcd_kit_ddic_ram_init_dsi_cmd(struct lcd_kit_dsi_panel_cmds *pcmds,
	struct lcd_kit_dsi_cmd_desc *newcmds, char *buf, int cnt, int blen)
{
	int i;
	struct lcd_kit_dsi_ctrl_hdr *dchdr = NULL;
	int len;
	char *bp = NULL;

	pcmds->cmds = newcmds;
	pcmds->cmd_cnt = cnt;
	pcmds->buf = buf;
	pcmds->blen = blen;
	bp = buf;
	len = blen;
	for (i = 0; i < cnt; i++) {
		dchdr = (struct lcd_kit_dsi_ctrl_hdr *)bp;
		len -= sizeof(*dchdr);
		bp += sizeof(*dchdr);
		pcmds->cmds[i].dtype = dchdr->dtype;
		pcmds->cmds[i].last = dchdr->last;
		pcmds->cmds[i].vc = dchdr->vc;
		pcmds->cmds[i].ack = dchdr->ack;
		pcmds->cmds[i].wait = dchdr->wait;
		pcmds->cmds[i].waittype = dchdr->waittype;
		pcmds->cmds[i].dlen = dchdr->dlen;
		pcmds->cmds[i].payload = bp;
		bp += dchdr->dlen;
		len -= dchdr->dlen;
	}
	return;
}

static int lcd_kit_dbg_parse_cmd(struct lcd_kit_dsi_panel_cmds *pcmds, char *buf,
	int length)
{
	int blen;
	int len;
	char *bp = NULL;
	struct lcd_kit_dsi_ctrl_hdr *dchdr = NULL;
	struct lcd_kit_dsi_cmd_desc *newcmds = NULL;
	int cnt = 0;

	if (!pcmds || !buf) {
		LCD_KIT_ERR("null pointer\n");
		return LCD_KIT_FAIL;
	}
	/* scan dcs commands */
	bp = buf;
	blen = length;
	len = blen;
	while (len > sizeof(*dchdr)) {
		dchdr = (struct lcd_kit_dsi_ctrl_hdr *)bp;
		bp += sizeof(*dchdr);
		len -= sizeof(*dchdr);
		if (dchdr->dlen > len) {
			LCD_KIT_ERR("dtsi cmd=%x error, len=%d, cnt=%d\n",
				dchdr->dtype, dchdr->dlen, cnt);
			return LCD_KIT_FAIL;
		}
		bp += dchdr->dlen;
		len -= dchdr->dlen;
		cnt++;
	}
	if (len != 0) {
		LCD_KIT_ERR("dcs_cmd=%x len=%d error!\n", buf[0], blen);
		return LCD_KIT_FAIL;
	}
	newcmds = kzalloc(cnt * sizeof(*newcmds), GFP_KERNEL);
	if (newcmds == NULL) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}
	// realse last malloce buffer
	if (pcmds->cmds != NULL) {
		kfree(pcmds->cmds);
		pcmds->cmds = NULL;
	}

	lcd_kit_ddic_ram_init_dsi_cmd(pcmds, newcmds, buf, cnt, blen);

	return 0;
}

static int lcd_kit_ddic_ram_mipi_rx(char *par, uint8_t *read_buf, char *dbg_cmds)
{
	int len;
	int panel_id;
	int ret;

	struct lcd_kit_adapt_ops *adapt_ops = NULL;
	struct lcd_kit_dsi_panel_cmds dsi_cmds;

	adapt_ops = lcd_kit_get_adapt_ops();
	if (!adapt_ops) {
		LCD_KIT_ERR("can not register adapt_ops!\n");
		return LCD_KIT_FAIL;
	}

	if (adapt_ops->mipi_rx == NULL) {
		LCD_KIT_ERR("adapt_ops is NULL!\n");
		return LCD_KIT_FAIL;
	}

	(void)memset_s(&dsi_cmds, sizeof(struct lcd_kit_dsi_panel_cmds), 0x00, sizeof(struct lcd_kit_dsi_panel_cmds));
	dsi_cmds.link_state = LCD_KIT_DSI_HS_MODE; // Set default link state to HS Mode

	(void)memset_s(read_buf, READ_DDIC_BUF_LEN * sizeof(uint8_t), 0x00, READ_DDIC_BUF_LEN * sizeof(uint8_t));

	len = lcd_kit_parse_u8_digit(par, dbg_cmds, LCD_KIT_CONFIG_TABLE_MAX_NUM);
	if (len > 0)
		lcd_kit_dbg_parse_cmd(&dsi_cmds, dbg_cmds, len);

	panel_id = lcd_kit_get_active_panel_id();
	ret = adapt_ops->mipi_rx(panel_id, NULL, read_buf, READ_DDIC_BUF_LEN, &dsi_cmds);
	if (ret == LCD_KIT_FAIL) {
		LCD_KIT_ERR("mipi_rx fail!\n");
		return LCD_KIT_FAIL;
	}
	read_buf[READ_DDIC_BUF_LEN - 1] = '\0';

	return LCD_KIT_OK;
}

static void lcd_kit_ddic_ram_reg_addr_to_array(uint32_t *reg_array, uint32_t reg_addr)
{
	reg_array[0] = reg_addr & 0xFF;
	reg_array[1] = (reg_addr >> 8) & 0xFF;
	reg_array[2] = (reg_addr >> 16) & 0xFF;
	reg_array[3] = (reg_addr >> 24) & 0xFF;

	return;
}

static void lcd_kit_ddic_ram_get_read_bin_dsi(char *read_bin_dsi, int32_t base_dsi_len,
	uint32_t read_addr, int32_t cmd_num)
{
	uint32_t reg_array[4];

	lcd_kit_ddic_ram_reg_addr_to_array(reg_array, read_addr);

	read_bin_dsi[HIS_DSI_REG_ADDR_24TO31_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[3]][0];
	read_bin_dsi[HIS_DSI_REG_ADDR_24TO31_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[3]][1];
	read_bin_dsi[HIS_DSI_REG_ADDR_16TO23_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[2]][0];
	read_bin_dsi[HIS_DSI_REG_ADDR_16TO23_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[2]][1];
	read_bin_dsi[HIS_DSI_REG_ADDR_00TO07_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[0]][0];
	read_bin_dsi[HIS_DSI_REG_ADDR_00TO07_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[0]][1];
	read_bin_dsi[HIS_DSI_REG_ADDR_08TO15_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[1]][0];
	read_bin_dsi[HIS_DSI_REG_ADDR_08TO15_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[1]][1];

	return;
}

static int lcd_kit_ddic_ram_read_tail_bin_xhis(uint32_t head_addr, uint32_t read_len, char *dbg_cmds, struct file *fp)
{
	int32_t i;
	int32_t ret;
	char read_bin_48byte[] = "0x29, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x00, 0x00, 0x00\r\n"
		"0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x30, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00\r\n";

	uint8_t read_buf[READ_DDIC_BUF_LEN] = {0};
	char mini_dsi[] = "0x00, ";

	int32_t read_time = read_len / READ_48_BYTE;
	int32_t tail = read_len % READ_48_BYTE;
	int32_t base_dsi_len;

	if (read_len == 0)
		return LCD_KIT_OK;

	base_dsi_len = strlen(read_bin_48byte);

	for (i = 0; i < read_time; i++) {
		lcd_kit_ddic_ram_get_read_bin_dsi(read_bin_48byte, base_dsi_len, head_addr + READ_48_BYTE * i, 0);

		ret = lcd_kit_ddic_ram_mipi_rx(read_bin_48byte, read_buf, dbg_cmds);
		if (ret != LCD_KIT_OK)
			return LCD_KIT_FAIL;

		vfs_write(fp, read_buf, READ_48_BYTE, &(fp->f_pos));
	}

	if (tail == 0)
		return LCD_KIT_OK;

	lcd_kit_ddic_ram_get_read_bin_dsi(read_bin_48byte, base_dsi_len, head_addr + READ_48_BYTE * read_time, 0);

	mini_dsi[0] = g_map_itoa[tail][0];
	mini_dsi[1] = g_map_itoa[tail][1];

	read_bin_48byte[HIS_DSI_RW_BYTES_OFFSET] = mini_dsi[0];
	read_bin_48byte[HIS_DSI_RW_BYTES_OFFSET + 1] = mini_dsi[1];
	read_bin_48byte[HIS_READ_DSI_TAIL_START_OFFSET + tail * strlen(mini_dsi)] = '\0';

	ret = lcd_kit_ddic_ram_mipi_rx(read_bin_48byte, read_buf, dbg_cmds);
	if (ret != LCD_KIT_OK)
		return LCD_KIT_FAIL;

	vfs_write(fp, read_buf, tail, &(fp->f_pos));

	return LCD_KIT_OK;
}

static int lcd_ddic_ddic_ram_cycle_read_bin_xhis(uint32_t head_addr, uint32_t read_time, char *dbg_cmds,
	char *read_bin_dsi, int32_t base_dsi_len, struct file *fp)
{
	int32_t i, j;
	int32_t ret;
	uint8_t read_buf[READ_DDIC_BUF_LEN] = {0};
	uint32_t read_addr;

	for (i = 0; i < read_time; i++) {
		for (j = 0; j < READ_CMD_NUM_ONCE; j++) {
			read_addr = head_addr + READ_FREAM_LEN * i + READ_48_BYTE * j;
			lcd_kit_ddic_ram_get_read_bin_dsi(read_bin_dsi, base_dsi_len, read_addr, j);
		}

		ret = lcd_kit_ddic_ram_mipi_rx(read_bin_dsi, read_buf, dbg_cmds);
		if (ret != LCD_KIT_OK)
			return LCD_KIT_FAIL;

		vfs_write(fp, read_buf, READ_FREAM_LEN, &(fp->f_pos));
	}

	return LCD_KIT_OK;
}

static void lcd_kit_ddic_ram_close_file(struct file *fp, mm_segment_t old_fs)
{
	set_fs(old_fs);
	filp_close(fp, NULL);
}

static void lcd_kit_ddic_ram_release_dsi_buf_and_cmd(char *dsi_buf, char *dbg_cmds)
{
	kfree(dsi_buf);
	dsi_buf = NULL;
	kfree(dbg_cmds);
	dbg_cmds = NULL;
}

static int32_t lcd_kit_ddic_ram_create_dsi_buf_and_cmd(char **bin_dsi, int32_t dsi_len,
	char **dbg_cmds, int32_t cmd_len)
{
	char *dsi_buf;
	char *cmd_buf;

	dsi_buf = kzalloc(dsi_len, 0);
	cmd_buf = kzalloc(cmd_len, 0);
	if ((dsi_buf != NULL) && (cmd_buf != NULL)) {
		*bin_dsi = dsi_buf;
		*dbg_cmds = cmd_buf;
		return LCD_KIT_OK;
	}

	if (dsi_buf != NULL) {
		kfree(dsi_buf);
		dsi_buf = NULL;
	}

	if (cmd_buf != NULL) {
		kfree(cmd_buf);
		cmd_buf = NULL;
	}

	LCD_KIT_ERR("kzalloc fail\n");

	return LCD_KIT_FAIL;
}

static int32_t lcd_kit_ddic_ram_read_by_frame(uint32_t head_addr, uint32_t read_len, char *read_bin_dsi,
	char *dbg_cmds, int32_t base_dsi_len, struct file *fp)
{
	int32_t ret;
	uint32_t read_time;
	uint32_t tail;
	uint32_t offset;

	read_time = read_len / READ_FREAM_LEN;
	tail = read_len % READ_FREAM_LEN;

	ret = lcd_ddic_ddic_ram_cycle_read_bin_xhis(head_addr, read_time, dbg_cmds,
			read_bin_dsi, base_dsi_len, fp);
	if (ret != LCD_KIT_OK)
		return LCD_KIT_FAIL;

	offset = head_addr + READ_FREAM_LEN * read_time;
	ret = lcd_kit_ddic_ram_read_tail_bin_xhis(offset, tail, dbg_cmds, fp);
	if (ret != LCD_KIT_OK)
		return LCD_KIT_FAIL;

	return LCD_KIT_OK;
}

static int32_t lcd_kit_ddic_ram_read_bin_xhis(uint32_t head_addr, uint32_t read_len)
{
	uint32_t i;
	int32_t ret;
	char *read_bin_dsi = NULL;
	char *dbg_cmds = NULL;
	int32_t base_dsi_len;
	char read_bin_48byte[] = "0x29, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x00, 0x00, 0x00\r\n"
		"0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x30, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, "
		"0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00\r\n";

	struct file *fp;
	mm_segment_t old_fs = get_fs();

	base_dsi_len = strlen(read_bin_48byte);

	LCD_KIT_INFO("base_dsi_len = %d\n", base_dsi_len);

	ret = lcd_kit_ddic_ram_create_dsi_buf_and_cmd(&read_bin_dsi, READ_CMD_NUM_ONCE * sizeof(read_bin_48byte),
		&dbg_cmds, LCD_KIT_CONFIG_TABLE_MAX_NUM);
	if (ret != LCD_KIT_OK)
		return LCD_KIT_FAIL;

	for (i = 0; i < READ_CMD_NUM_ONCE; i++) {
		ret = memcpy_s(read_bin_dsi + base_dsi_len * i, base_dsi_len, read_bin_48byte, base_dsi_len);
		if (ret != LCD_KIT_OK) {
			lcd_kit_ddic_ram_release_dsi_buf_and_cmd(read_bin_dsi, dbg_cmds);
			LCD_KIT_ERR("memcpy fail\n");
			return LCD_KIT_FAIL;
		}
	}

	fp = filp_open(LCD_KIT_READBIN_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	LCD_KIT_INFO("read bin begin\n");
	ret = lcd_kit_ddic_ram_read_by_frame(head_addr, read_len, read_bin_dsi,
		dbg_cmds, base_dsi_len, fp);
	lcd_kit_ddic_ram_release_dsi_buf_and_cmd(read_bin_dsi, dbg_cmds);
	lcd_kit_ddic_ram_close_file(fp, old_fs);
	LCD_KIT_INFO("read bin end\n");

	return ret;
}

static void lcd_kit_ddic_ram_get_write_bin_dsi(char *write_bin_dsi, int32_t base_dsi_len,
	uint32_t reg_addr, uint32_t write_val, int32_t cmd_num)
{
	uint32_t reg_array[4];
	uint32_t val_array[4];

	lcd_kit_ddic_ram_reg_addr_to_array(reg_array, reg_addr);
	lcd_kit_ddic_ram_reg_addr_to_array(val_array, write_val);

	write_bin_dsi[HIS_DSI_REG_ADDR_24TO31_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[3]][0];
	write_bin_dsi[HIS_DSI_REG_ADDR_24TO31_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[3]][1];
	write_bin_dsi[HIS_DSI_REG_ADDR_16TO23_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[2]][0];
	write_bin_dsi[HIS_DSI_REG_ADDR_16TO23_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[2]][1];
	write_bin_dsi[HIS_DSI_REG_ADDR_00TO07_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[0]][0];
	write_bin_dsi[HIS_DSI_REG_ADDR_00TO07_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[0]][1];
	write_bin_dsi[HIS_DSI_REG_ADDR_08TO15_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[reg_array[1]][0];
	write_bin_dsi[HIS_DSI_REG_ADDR_08TO15_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[reg_array[1]][1];

	write_bin_dsi[HIS_DSI_REG_VAL_00TO07_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[val_array[0]][0];
	write_bin_dsi[HIS_DSI_REG_VAL_00TO07_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[val_array[0]][1];
	write_bin_dsi[HIS_DSI_REG_VAL_08TO15_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[val_array[1]][0];
	write_bin_dsi[HIS_DSI_REG_VAL_08TO15_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[val_array[1]][1];
	write_bin_dsi[HIS_DSI_REG_VAL_16TO23_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[val_array[2]][0];
	write_bin_dsi[HIS_DSI_REG_VAL_16TO23_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[val_array[2]][1];
	write_bin_dsi[HIS_DSI_REG_VAL_24TO31_OFFSET + base_dsi_len * cmd_num] = g_map_itoa[val_array[3]][0];
	write_bin_dsi[HIS_DSI_REG_VAL_24TO31_OFFSET + 1 + base_dsi_len * cmd_num] = g_map_itoa[val_array[3]][1];

	if ((reg_array[1] == 0x2C) || (reg_array[1] == 0x3C) || (reg_array[1] == 0x6D)) {
		write_bin_dsi[HIS_DTYPE_DCS_LWRITE_OFFSET + base_dsi_len * cmd_num] = '2'; // long write 0x29
	} else {
		write_bin_dsi[HIS_DTYPE_DCS_LWRITE_OFFSET + base_dsi_len * cmd_num] = '3'; // long write 0x39
	}

	return;
}

static ssize_t lcd_kit_ddic_ram_create_write_reg_dsi(char *write_base_dsi, uint32_t reg_addr,
	uint32_t reg_value, uint32_t write_len)
{
	uint32_t reg_array[4];
	uint32_t val_array[4];
	char mini_dsi[] = "0x00, ";

	if (write_len == 0 || write_len > WRITE_4_BYTE)
	    return LCD_KIT_FAIL;

	lcd_kit_ddic_ram_reg_addr_to_array(reg_array, reg_addr);
	lcd_kit_ddic_ram_reg_addr_to_array(val_array, reg_value);

	write_base_dsi[HIS_DSI_REG_ADDR_24TO31_OFFSET] = g_map_itoa[reg_array[3]][0];
	write_base_dsi[HIS_DSI_REG_ADDR_24TO31_OFFSET + 1] = g_map_itoa[reg_array[3]][1];
	write_base_dsi[HIS_DSI_REG_ADDR_16TO23_OFFSET] = g_map_itoa[reg_array[2]][0];
	write_base_dsi[HIS_DSI_REG_ADDR_16TO23_OFFSET + 1] = g_map_itoa[reg_array[2]][1];
	write_base_dsi[HIS_DSI_REG_ADDR_00TO07_OFFSET] = g_map_itoa[reg_array[0]][0];
	write_base_dsi[HIS_DSI_REG_ADDR_00TO07_OFFSET + 1] = g_map_itoa[reg_array[0]][1];
	write_base_dsi[HIS_DSI_REG_ADDR_08TO15_OFFSET] = g_map_itoa[reg_array[1]][0];
	write_base_dsi[HIS_DSI_REG_ADDR_08TO15_OFFSET + 1] = g_map_itoa[reg_array[1]][1];

	write_base_dsi[HIS_DSI_REG_VAL_00TO07_OFFSET] = g_map_itoa[val_array[0]][0];
	write_base_dsi[HIS_DSI_REG_VAL_00TO07_OFFSET + 1] = g_map_itoa[val_array[0]][1];
	write_base_dsi[HIS_DSI_REG_VAL_08TO15_OFFSET] = g_map_itoa[val_array[1]][0];
	write_base_dsi[HIS_DSI_REG_VAL_08TO15_OFFSET + 1] = g_map_itoa[val_array[1]][1];
	write_base_dsi[HIS_DSI_REG_VAL_16TO23_OFFSET] = g_map_itoa[val_array[2]][0];
	write_base_dsi[HIS_DSI_REG_VAL_16TO23_OFFSET + 1] = g_map_itoa[val_array[2]][1];
	write_base_dsi[HIS_DSI_REG_VAL_24TO31_OFFSET] = g_map_itoa[val_array[3]][0];
	write_base_dsi[HIS_DSI_REG_VAL_24TO31_OFFSET + 1] = g_map_itoa[val_array[3]][1];

	if ((reg_array[1] == 0x2C) || (reg_array[1] == 0x3C) || (reg_array[1] == 0x6D)) {
		write_base_dsi[HIS_DTYPE_DCS_LWRITE_OFFSET] = '2';
	} else {
		write_base_dsi[HIS_DTYPE_DCS_LWRITE_OFFSET] = '3';
	}

	mini_dsi[0] = g_map_itoa[write_len + 1][0]; // 1: dsi_len = write_len + 1
	mini_dsi[1] = g_map_itoa[write_len + 1][1]; // 1: dsi_len = write_len + 1

	write_base_dsi[HIS_DSI_RW_BYTES_OFFSET] = mini_dsi[0];
	write_base_dsi[HIS_DSI_RW_BYTES_OFFSET + 1] = mini_dsi[1];
	write_base_dsi[HIS_WRITE_DSI_TAIL_START_OFFSET + write_len * strlen(mini_dsi)] = '\0';

	 return LCD_KIT_OK;
}

static ssize_t lcd_kit_ddic_ram_write_reg(uint32_t reg_addr, uint32_t reg_value, uint32_t write_len)
{
	int32_t ret;
	uint8_t read_buf[READ_DDIC_BUF_LEN] = {0};
	char *dbg_cmds;
	char write_base_dsi[] = "0x29, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x00, 0x00, 0x00\r\n"
		"0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00\r\n";

	ret = lcd_kit_ddic_ram_create_write_reg_dsi(write_base_dsi, reg_addr, reg_value, write_len);
	if (ret == LCD_KIT_FAIL)
		return LCD_KIT_FAIL;

	dbg_cmds = kzalloc(LCD_KIT_CONFIG_TABLE_MAX_NUM, 0);
	if (!dbg_cmds) {
		LCD_KIT_ERR("kzalloc fail\n");
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_ddic_ram_mipi_rx(write_base_dsi, read_buf, dbg_cmds);
	kfree(dbg_cmds);
	dbg_cmds = NULL;

	return ret;
}

static ssize_t lcd_kit_ddic_ram_write_bin_base_4byte(uint32_t head_addr, uint32_t write_len, uint8_t *bin_data)
{
	int32_t write_time = write_len / WRITE_4_BYTE;
	int32_t tail = write_len % WRITE_4_BYTE;
	int32_t i;
	int32_t j;
	int32_t ret;

	uint32_t write_addr;
	uint32_t write_val;
	uint32_t temp;

	for (i = 0; i < write_time; i++) {
		write_addr = head_addr + WRITE_4_BYTE * i;
		write_val = 0;
		for (j = 0; j < WRITE_4_BYTE; j++) {
			temp = (bin_data[WRITE_4_BYTE * i + j] & 0xFF);
			write_val += temp << (8 * j);
		}
		ret = lcd_kit_ddic_ram_write_reg(write_addr, write_val, WRITE_4_BYTE);
		if (ret != LCD_KIT_OK)
			return LCD_KIT_FAIL;
	}

	write_addr = head_addr + WRITE_4_BYTE * write_time;
	if (tail > 0) {
		write_val = 0;
		for (j = 0; j < tail; j++) {
			temp = (bin_data[WRITE_4_BYTE * write_time + j] & 0xFF);
			write_val += temp << (8 * j);
		}
		ret = lcd_kit_ddic_ram_write_reg(write_addr, write_val, tail);
		if (ret != LCD_KIT_OK)
			return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

static int lcd_kit_ddic_ram_cycle_write_bin_xhis(uint32_t head_addr, uint32_t write_time, char *dbg_cmds,
	char *write_bin_dsi, int32_t base_dsi_len, struct file *fp)
{
	int32_t i, j, k, ret;
	uint8_t read_buf[READ_DDIC_BUF_LEN] = {0};
	uint8_t bin_data[WRITE_FREAM_LEN];

	uint32_t write_val;

	for (i = 0; i < write_time; i++) {
		vfs_read(fp, bin_data, WRITE_FREAM_LEN, &(fp->f_pos));

		for (j = 0; j < WRITE_CMD_NUM_ONCE; j++) {
			write_val = 0;
			for (k = 0; k < WRITE_4_BYTE; k++)
				write_val += ((uint32_t)(bin_data[WRITE_4_BYTE * j + k] & 0xFF)) << (8 * k);

			lcd_kit_ddic_ram_get_write_bin_dsi(write_bin_dsi, base_dsi_len,
				head_addr + WRITE_FREAM_LEN * i + WRITE_4_BYTE * j, write_val, j);
		}

		ret = lcd_kit_ddic_ram_mipi_rx(write_bin_dsi, read_buf, dbg_cmds);
		if (ret != LCD_KIT_OK)
			return LCD_KIT_FAIL;
	}

	return LCD_KIT_OK;
}

static int lcd_kit_ddic_ram_write_tail_bin_xhis(uint32_t head_addr, uint32_t write_len, uint32_t tail,
	struct file *fp, mm_segment_t old_fs)
{
	uint8_t bin_data[WRITE_FREAM_LEN];
	int32_t offset;

	if (tail == 0)
		return LCD_KIT_OK;

	vfs_read(fp, bin_data, tail, &(fp->f_pos));
	offset = WRITE_FREAM_LEN * (write_len / WRITE_FREAM_LEN);

	return lcd_kit_ddic_ram_write_bin_base_4byte(head_addr + offset, tail, bin_data);
}

static int32_t lcd_kit_ddic_ram_write_bin_xhis(uint32_t head_addr, uint32_t write_len)
{
	int32_t i;
	int ret;
	char write_base_dsi[] = "0x29, 0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0xFF, 0x00, 0x00, 0x00\r\n"
		"0x39, 0x01, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00\r\n";
	char *write_bin_dsi = NULL;
	char *dbg_cmds = NULL;
	struct file *fp;
	mm_segment_t old_fs;
	int32_t base_dsi_len = strlen(write_base_dsi);

	LCD_KIT_INFO("base_dsi_len = %d\n", base_dsi_len);

	ret = lcd_kit_ddic_ram_create_dsi_buf_and_cmd(&write_bin_dsi, WRITE_CMD_NUM_ONCE * sizeof(write_base_dsi),
		&dbg_cmds, LCD_KIT_CONFIG_TABLE_MAX_NUM);
	if (ret != LCD_KIT_OK)
		return LCD_KIT_FAIL;

	for (i = 0; i < WRITE_CMD_NUM_ONCE; i++) {
		ret = memcpy_s(write_bin_dsi + base_dsi_len * i, base_dsi_len, write_base_dsi, base_dsi_len);
		if (ret != LCD_KIT_OK) {
			lcd_kit_ddic_ram_release_dsi_buf_and_cmd(write_bin_dsi, dbg_cmds);
			return LCD_KIT_FAIL;
		}
	}

	fp = filp_open(LCD_KIT_READBIN_FILE_PATH, O_RDONLY, 0644);
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	LCD_KIT_INFO("write bin begin\n");

	ret = lcd_kit_ddic_ram_cycle_write_bin_xhis(head_addr, (write_len / WRITE_FREAM_LEN), dbg_cmds,
		write_bin_dsi, base_dsi_len, fp);
	lcd_kit_ddic_ram_release_dsi_buf_and_cmd(write_bin_dsi, dbg_cmds);
	if (ret != LCD_KIT_OK) {
		lcd_kit_ddic_ram_close_file(fp, old_fs);
		return LCD_KIT_FAIL;
	}

	ret = lcd_kit_ddic_ram_write_tail_bin_xhis(head_addr, write_len, write_len % WRITE_FREAM_LEN, fp, old_fs);
	if (ret != LCD_KIT_OK) {
		lcd_kit_ddic_ram_close_file(fp, old_fs);
		return LCD_KIT_FAIL;
	}

	lcd_kit_ddic_ram_close_file(fp, old_fs);
	LCD_KIT_INFO("write bin end\n");

	return LCD_KIT_OK;
}

static void lcd_kit_ddic_ram_get_panel_name(char *panel_name)
{
	int panel_id = lcd_kit_get_active_panel_id();
	common_ops->get_panel_name(panel_id, panel_name);
	return;
}

static enum lcd_kit_ddic_type lcd_kit_ddic_ram_get_type(void)
{
	char panel_name[READ_DDIC_BUF_LEN] = {0};

	lcd_kit_ddic_ram_get_panel_name(panel_name);
	if (strlen(panel_name) != 0) {
	    LCD_KIT_INFO("panel_name is %s\n", panel_name);
	} else {
		LCD_KIT_ERR("get lcd_name fail\n");
		return LCD_KIT_NONE_DDIC;
	}

	if (panel_name[PANEL_NAME_DDIC_TYPE] == 'H' || panel_name[PANEL_NAME_DDIC_TYPE] == 'h') {
		return LCD_KIT_HIS_DDIC;
	} else if (panel_name[PANEL_NAME_DDIC_TYPE] == 'C' || panel_name[PANEL_NAME_DDIC_TYPE] == 'c') {
		return LCD_KIT_RAYDIUM_DDIC;
	}

	return LCD_KIT_NONE_DDIC;
}

int32_t lcd_kit_ddic_ram_read_bin(uint32_t head_addr, uint32_t read_len)
{
	enum lcd_kit_ddic_type ic_type;
	int32_t ret = LCD_KIT_OK;
	ic_type = lcd_kit_ddic_ram_get_type();

	switch (ic_type) {
	case LCD_KIT_HIS_DDIC:
		ret = lcd_kit_ddic_ram_read_bin_xhis(head_addr, read_len);
		break;
	case LCD_KIT_RAYDIUM_DDIC:
		break;
	default:
		LCD_KIT_ERR("invaild ic type\n");
		return LCD_KIT_FAIL;
	}

	if (ret == LCD_KIT_FAIL)
		LCD_KIT_ERR("ddic ram read fail\n");

	return ret;
}

int32_t lcd_kit_ddic_ram_write_bin(uint32_t head_addr, uint32_t read_len)
{
	enum lcd_kit_ddic_type ic_type;
	int32_t ret = LCD_KIT_OK;
	ic_type = lcd_kit_ddic_ram_get_type();

	switch (ic_type) {
	case LCD_KIT_HIS_DDIC:
		ret = lcd_kit_ddic_ram_write_bin_xhis(head_addr, read_len);
		break;
	case LCD_KIT_RAYDIUM_DDIC:
		break;
	default:
		LCD_KIT_ERR("invaild ic type\n");
		return LCD_KIT_FAIL;
	}

	if (ret == LCD_KIT_FAIL)
		LCD_KIT_ERR("ddic ram write fail\n");

	return ret;
}
