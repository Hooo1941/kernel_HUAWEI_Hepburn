/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: Log handler for Seplat, including initialization.
 * Create: 2023/10/25
 */

#include <linux/module.h>
#include <linux/timer.h>
#include <linux/of.h>
#include <linux/printk.h>
#include <linux/platform_device.h>
#include <linux/jiffies.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <teek_client_constants.h>
#include <securec.h>
#include <platform_include/basicplatform/linux/dfx_bbox_diaginfo.h>

/* 6 hours */
#define HISES_GET_LOG_EXPIRES       (60 * 60 * 6)
#define HISES_MNTN_WAIT_TIMEOUT_MS  100
#define HISES_MAX_LOG_UPLOAD_CNT    200
#define HISES_SEPLAT_LOG_MIN_SIZE   2
#define HISES_SEPLAT_SW_ERROR       0
#define HISES_SEPLAT_LOG_MAX_LEN    128

#ifndef unused
#define unused(x) (void)(x)
#endif

static TEEC_Context g_context;
static TEEC_Session g_session;

static struct timer_list g_timer;

static struct task_struct *g_log_thread;
static DECLARE_WAIT_QUEUE_HEAD(g_wait);
static bool g_task_start = false;

static struct seq_file *g_seq_file;

#define HISES_ERR_ID_TYPES 4
static unsigned int g_err_id_map[HISES_ERR_ID_TYPES] = {
	DMD_HISES_INVOKE_E_RET,
	DMD_HISES_CPU_FAULT,
	DMD_HISES_HW_ATK,
	DMD_HISES_KEY_REG_DUMP,
};

#ifdef CONFIG_DFX_DEBUG_FS
static struct task_struct *g_cat_thread;
static DECLARE_WAIT_QUEUE_HEAD(g_cat_wait);
static bool g_cat_start = false;
static DECLARE_WAIT_QUEUE_HEAD(g_cat_finish_wait);
static bool g_cat_finish = false;

#define HISES_LOG_BUFF_SIZE \
	(HISES_MAX_LOG_UPLOAD_CNT * HISES_SEPLAT_LOG_MAX_LEN)
static u8* g_log_buff = NULL;
static u16 g_log_buff_offset = 0;
#endif

#define HISES_ASC_ZERO_OFFSET 48
#define HISES_ERR_ID_NOT_EXIST 0
static unsigned int mntn_get_err_id(u8 *buff)
{
	int i;

	if (!buff)
		return HISES_ERR_ID_NOT_EXIST;

	i = buff[0] - HISES_ASC_ZERO_OFFSET;
	if (i < 0 || i >= HISES_ERR_ID_TYPES)
		return HISES_ERR_ID_NOT_EXIST;

	return g_err_id_map[i];
}

#define HISES_STATUS_WORD_HIGH_OFFSET 2
#define HISES_STATUS_WORD_LOW_OFFSET  1
static u16 mntn_get_apdu_sw(u8 *buff, u16 size)
{
	u8 sw1;
	u8 sw2;

	if (size < HISES_SEPLAT_LOG_MIN_SIZE) {
		pr_err("%s: invalid status word size\n", __func__);
		return HISES_SEPLAT_SW_ERROR;
	}
	sw1 = buff[size - HISES_STATUS_WORD_HIGH_OFFSET];
	sw2 = buff[size - HISES_STATUS_WORD_LOW_OFFSET];

	return (u16)((sw1 << 8) | (sw2 & 0xFF));
}

#ifdef CONFIG_DFX_DEBUG_FS
static void mntn_print_log(u8 *buff, u16 size)
{
	if (!g_seq_file || size <= HISES_SEPLAT_LOG_MIN_SIZE ||
		size > HISES_SEPLAT_LOG_MAX_LEN)
		return;

	if (HISES_LOG_BUFF_SIZE < g_log_buff_offset) {
		pr_err("%s: out of buff mem size: %d\n", __func__, g_log_buff_offset);
		g_log_buff_offset = 0;
	}
	if (HISES_LOG_BUFF_SIZE - g_log_buff_offset < size)
		g_log_buff_offset = 0;
	if (memcpy_s(g_log_buff + g_log_buff_offset,
				 HISES_LOG_BUFF_SIZE - g_log_buff_offset,
				 buff, size) != EOK) 
		pr_err("%s: memcpy failed\n", __func__);
	g_log_buff_offset += HISES_SEPLAT_LOG_MAX_LEN;
}
#endif

#define HISES_SEPLAT_SW_SIZE 2
#define HISES_UPLOAD_LOG_INTERVAL 1100
#define HISES_BBOX_MAX_INPUT_SIZE 120
#define HISES_LOG_HEX_SIZE \
	((HISES_SEPLAT_LOG_MAX_LEN - HISES_SEPLAT_SW_SIZE) * 2 + 1)
static char g_hex_nums_hises[] = \
	{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
#define hises_get_high_bit_hex(x) (g_hex_nums_hises[((x) & 0xf0) >> 4])
#define hises_get_low_bit_hex(x) (g_hex_nums_hises[(x) & 0x0f])
static void mntn_upload_log(u8 *buff, u16 size)
{
	u8 idx;
	u32 err;
	u32 last_idx;
	u8 log_hex[HISES_LOG_HEX_SIZE] = {0};

	if (size <= HISES_SEPLAT_LOG_MIN_SIZE || size > HISES_SEPLAT_LOG_MAX_LEN) {
		pr_err("%s: invalid log struct size", __func__);
		return;
	}
	for (idx = 0; idx < size - HISES_SEPLAT_SW_SIZE; ++idx) {
		log_hex[idx * 2] = hises_get_high_bit_hex(buff[idx]);
		log_hex[idx * 2 + 1] = hises_get_low_bit_hex(buff[idx]);
	}
	/* set the end pos of log */
	last_idx = (size - HISES_SEPLAT_SW_SIZE) * 2;
	if (last_idx > HISES_BBOX_MAX_INPUT_SIZE)
		last_idx = HISES_BBOX_MAX_INPUT_SIZE;
	log_hex[last_idx] = 0;
	err = bbox_diaginfo_record(mntn_get_err_id(buff), NULL, "%s", log_hex);
	if (err != BBOX_DIAGINFO_OK) {
		pr_err("%s: bbox record failed, ret: 0x%x", __func__, err);	
		return;
	}
	msleep(HISES_UPLOAD_LOG_INTERVAL);
}

#define HISES_APDU_CMD_SW_LOG_END 0x6A83
#define HISES_APDU_CMD_SW_LOG_OK  0x9000
#define HISES_SEPLAT_GET_LOG_CMD_ID    12U
static TEEC_Result mntn_get_log(void)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	u32 origin;
	u8 cnt = 0;
	u8 buff[HISES_SEPLAT_LOG_MAX_LEN] = {0};
	u16 size = 0;
	u16 sw = 0;

	operation.started = 1;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INOUT,
											TEEC_MEMREF_TEMP_INOUT,
											TEEC_NONE,
											TEEC_NONE);
	operation.params[0].tmpref.buffer = (void *)(buff);
	operation.params[0].tmpref.size = sizeof(buff);
	operation.params[1].tmpref.buffer = (void *)(&size);
	operation.params[1].tmpref.size = sizeof(size);

	while (cnt < HISES_MAX_LOG_UPLOAD_CNT) {
		result = TEEK_InvokeCommand(&g_session, HISES_SEPLAT_GET_LOG_CMD_ID, &operation, &origin);
		sw = mntn_get_apdu_sw(buff, size);
		if ((result != TEEC_SUCCESS) ||
			(sw != HISES_APDU_CMD_SW_LOG_OK && sw != HISES_APDU_CMD_SW_LOG_END)) {
			pr_err("%s: invoke failed, ret 0x%x, sw 0x%x\n", __func__, result, sw);
			return result;
		}

		/* trans finish */
		if (sw == HISES_APDU_CMD_SW_LOG_END)
			break;

		/* if invoked by timer, g_seq_file must be NULL */
		if (!g_seq_file) {
			mntn_upload_log(buff, size);			
			++cnt;
		}

#ifdef CONFIG_DFX_DEBUG_FS
		mntn_print_log(buff, size);
#endif
		memset_s(buff, sizeof(buff), 0, sizeof(buff));
	}

	return result;
}

/* uuid to SE TA: 78a354fa-7843-487b-a69e-0da325a828aa */
#define HISES_UUID_TEE_SERVICE_SEPLAT                      \
	{                                                      \
		0x78a354fa, 0x7843, 0x487b,                        \
		{                                                  \
			0xa6, 0x9e, 0x0d, 0xa3, 0x25, 0xa8, 0x28, 0xaa \
		}                                                  \
	}
#define HISES_TEE_SERVICE_NAME "seplat"
static TEEC_Result mntn_open_con(void)
{
	TEEC_Result result;
	TEEC_Operation operation = {0};
	TEEC_UUID uuid_id = HISES_UUID_TEE_SERVICE_SEPLAT;
	u32 root_id = 0;
	const char pack_name[] = HISES_TEE_SERVICE_NAME;

	result = TEEK_InitializeContext(NULL, &g_context);
	if (result != TEEC_SUCCESS) {
		pr_err("%s, TEEC init failed.\n", __func__);
		goto exit;
	}

	operation.started = 1;
	operation.cancel_flag = 0;
	operation.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
											TEEC_NONE,
											TEEC_MEMREF_TEMP_INPUT,
											TEEC_MEMREF_TEMP_INPUT);
	operation.params[2].tmpref.buffer = (void *)(&root_id);
	operation.params[2].tmpref.size = sizeof(root_id);
	operation.params[3].tmpref.buffer = (void *)(pack_name);
	operation.params[3].tmpref.size = sizeof(pack_name);

	result = TEEK_OpenSession(&g_context, &g_session, &uuid_id,
							  TEEC_LOGIN_IDENTIFY, NULL, &operation, NULL);
	if (result != TEEC_SUCCESS) {
		pr_err("%s: open session failed, 0x%x\n", __func__, result);
		TEEK_FinalizeContext(&g_context);
	}

exit:
	return result;
}

static void mntn_close_con(void)
{
	TEEK_CloseSession(&g_session);
	TEEK_FinalizeContext(&g_context);
}

static void mntn_reset_timer(void)
{
	g_timer.expires = jiffies + HISES_GET_LOG_EXPIRES * HZ;
	add_timer(&g_timer);
}

static int mntn_get_log_thread(void *arg)
{
	TEEC_Result result;

	unused(arg);

	while (1) {
		wait_event_interruptible(g_wait, g_task_start || kthread_should_stop());
		g_task_start = false;

		if (kthread_should_stop())
			break;

		result = mntn_open_con();
		if (result != TEEC_SUCCESS)
			continue;

		result = mntn_get_log();
		if (result != TEEC_SUCCESS)
			pr_err("%s: get log failed\n", __func__);

		mntn_close_con();
	}
	return 0;
}

static void mntn_do_get_log(struct timer_list *arg)
{
	unused(arg);

	g_task_start = true;
	wake_up_interruptible(&g_wait);

	mntn_reset_timer();
}

static void mntn_add_log_timer(void)
{
	g_timer.function = &mntn_do_get_log;
	mntn_reset_timer();
}

#ifdef CONFIG_DFX_DEBUG_FS
static int mntn_cat_log_thread(void *arg)
{
	TEEC_Result result;

	unused(arg);

	while (1) {
		wait_event_interruptible(g_cat_wait, g_cat_start || kthread_should_stop());
		g_cat_start = false;

		if (kthread_should_stop())
			break;

		result = mntn_open_con();
		if (result != TEEC_SUCCESS)
			goto out;

		result = mntn_get_log();
		if (result != TEEC_SUCCESS)
			pr_err("%s: get log failed\n", __func__);

		mntn_close_con();
	out:
		g_cat_finish = true;
		wake_up_interruptible(&g_cat_finish_wait);
	}
	return 0;
}

static int mntn_debug_show(struct seq_file *s, void *d)
{
	u16 i;

	unused(d);

	g_seq_file = s;
	g_cat_start = true;
	wake_up_interruptible(&g_cat_wait);

	wait_event_interruptible(g_cat_finish_wait, g_cat_finish);
	g_cat_finish = false;
	g_seq_file = NULL;

	for (i = 0; i < HISES_LOG_BUFF_SIZE; ++i)
		seq_printf(s, "%02x", g_log_buff[i]);
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(mntn_debug);
#endif /* ifdef CONFIG_DFX_DEBUG_FS */

#define HISES_FILE_RW 0666
#define HISES_LOG_DIR "HiSES_mntn_log"
#define HISES_LOG_FILENAME "HiSES_log"
int seplat_mntn_init(void)
{
#ifdef CONFIG_DFX_DEBUG_FS
	struct dentry *root;

	g_log_buff = kmalloc(HISES_LOG_BUFF_SIZE, GFP_KERNEL);
	if (g_log_buff == NULL) {
		pr_err("%s: kmalloc failed, g_log_buff is NULL\n", __func__);
		return -ENOMEM;
	}
	root = debugfs_create_dir(HISES_LOG_DIR, NULL);
	if (!root) {
		pr_err("failed to create mntn dir\n");
		return -EFAULT;
	}

	if (!debugfs_create_file(HISES_LOG_FILENAME, HISES_FILE_RW,
							 root, NULL, &mntn_debug_fops)) {
		pr_err("failed to create mntn file\n");
		return -EFAULT;
	}
	g_cat_thread = kthread_run(mntn_cat_log_thread, NULL, "mntn_cat_log_thread");
	if (IS_ERR(g_cat_thread)) {
		pr_err("cat kthread_create failed\n");
		return -EFAULT;
	}
#endif
	g_log_thread = kthread_run(mntn_get_log_thread, NULL, "mntn_get_log_thread");
	if (IS_ERR(g_log_thread)) {
		pr_err("log kthread_create failed\n");
		return -EFAULT;
	}
	mntn_add_log_timer();
	return 0;
}
