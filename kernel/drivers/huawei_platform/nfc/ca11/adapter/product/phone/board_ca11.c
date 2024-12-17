
#include <linux/i2c.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/of_platform.h>
#include "oal_os.h"
#include "board_nfc.h"
#include "nfc_dev_q.h"
#include "oal_plat_op.h"
#include "priv_nfc_driver.h"
#include "securec.h"
#include <teek_client_api.h>
#include <teek_client_id.h>
#include <teek_client_constants.h>

#define MAX_CONFG_VALUE_SIZE    32

#define ROOTID 0
/* uuid to SE TA: 78a354fa-7843-487b-a69e-0da325a828aa */
#define UUID_TEE_SERVICE_SEPLAT                        \
{                                                      \
	0x78a354fa, 0x7843, 0x487b,                        \
	{                                                  \
		0xa6, 0x9e, 0x0d, 0xa3, 0x25, 0xa8, 0x28, 0xaa \
	}                                                  \
}

#define TEE_SERVICE_SEPLAT_NAME  "seplat"
#define CA11_SEPLAT_CMD_ID_POWER         0x3U
#define CA11_SEPLAT_CMD_ID_POWER_GPIO    0x4U

#define SEPLAT_POWER_TYPE_BOOT  0
#define SEPLAT_VOTE_TARGET_NFC  0x1U

#define CA_EXEC_RETRY_CNT  3

enum NFC_ESE_TYPE {
	NFC_WITHOUT_ESE = 0,
	NFC_ESE_P61,
	NFC_ESE_HISEE,
	NFC_ESE_SN110
};

enum seplat_power_cmd {
	SEPLAT_POWER_ON_CMD = 0,
	SEPLAT_POWER_OFF_CMD,
	SEPLAT_POWER_RESET_CMD,
	SEPLAT_POWER_MAX_CMD
};

enum seplat_gpio_pin {
	SEPLAT_GPIO_PIN_VEN = 0,
	SEPLAT_GPIO_PIN_DWL,
	SEPLAT_GPIO_PIN_MAX
} ca11_nfc_pin;

enum seplat_gpio_op {
	SEPLAT_GPIO_OP_READ = 0,
	SEPLAT_GPIO_OP_WRITE,
	SEPLAT_GPIO_OP_MAX
};

typedef struct {
	struct i2c_client *client;
	struct wakeup_source *wl;
	int irq_gpio;
	int fwdl_en_gpio;
	int ven_gpio;
	int ssi_data_gpio;
	int ssi_clk_gpio;
	bool irq_enabled;
	spinlock_t irq_enabled_lock;
} board_info_t;

int g_ese_spi_bus = 0;
bool g_is_sn110 = false;
int g_ese_svdd_pwr_req = 0;
int g_nfc_ese_type = NFC_WITHOUT_ESE; /* record ese type wired to nfcc by dts */
int g_nfc_chip_type = NFCTYPE_INVALID;

static board_info_t g_board_info;
TEEC_Session g_ca11_seplat_tee_session;
TEEC_Context g_ca11_seplat_context;

static int nfc_ca_init(void)
{
	const char *package_name = TEE_SERVICE_SEPLAT_NAME;
	u32 root_id = ROOTID;
	TEEC_UUID svc_id = UUID_TEE_SERVICE_SEPLAT;
	TEEC_Operation op = {0};
	TEEC_Result result;
	u32 origin = 0;

	TEEC_Context *context = &g_ca11_seplat_context;
	TEEC_Session *session = &g_ca11_seplat_tee_session;
	/* initialize TEE environment */
	result = TEEK_InitializeContext(NULL, context);
	if (result != TEEC_SUCCESS) {
		PS_PRINT_ERR("InitializeContext failed, ReturnCode=0x%x\n", result);
		goto cleanup_1;
	} else {
		PS_PRINT_DBG("InitializeContext success\n");
	}
	/* operation params create  */
	op.started = 1;
	op.cancel_flag = 0;
	/* open session */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_NONE,
									 TEEC_NONE,
									 TEEC_MEMREF_TEMP_INPUT,
									 TEEC_MEMREF_TEMP_INPUT);

	op.params[2].tmpref.buffer = (void *)&root_id;
	op.params[2].tmpref.size = sizeof(root_id);
	op.params[3].tmpref.buffer = (void *)package_name;
	op.params[3].tmpref.size = (size_t)(strlen(package_name) + 1);

	result = TEEK_OpenSession(context, session, &svc_id,
							  TEEC_LOGIN_IDENTIFY, NULL, &op, &origin);
	if (result != TEEC_SUCCESS) {
		PS_PRINT_ERR("OpenSession fail, RC=0x%x, RO=0x%x\n", result, origin);
		goto cleanup_2;
	} else {
		PS_PRINT_DBG("OpenSession success\n");
	}

	return 0;
cleanup_2:
	TEEK_FinalizeContext(context);
cleanup_1:
	return -EINVAL;
}

static int nfc_ca_exec_cmd(u32 cmd, u32 pin, u32 pinop, u32 *onoff)
{
	TEEC_Session *session = &g_ca11_seplat_tee_session;
	TEEC_Result result;
	TEEC_Operation op = {0};
	u32 origin = 0;

	if (!session || !onoff)
		return -EINVAL;

	op.started = 1;
	op.cancel_flag = 0;

	switch (cmd) {
	case CA11_SEPLAT_CMD_ID_POWER:
		op.params[0].value.a = (*onoff == 1) ? SEPLAT_POWER_ON_CMD : SEPLAT_POWER_OFF_CMD;
		op.params[1].value.a = SEPLAT_POWER_TYPE_BOOT;
		op.params[2].value.a = SEPLAT_VOTE_TARGET_NFC;
		op.paramTypes = TEEC_PARAM_TYPES(
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_VALUE_INPUT,
			TEEC_NONE);
		break;
	case CA11_SEPLAT_CMD_ID_POWER_GPIO:
		op.params[0].value.a = pin;
		op.params[1].value.a = pinop;
		op.params[2].value.a = 0;
		if (pinop == SEPLAT_GPIO_OP_WRITE)
			op.params[2].value.a = *onoff;
		op.paramTypes = TEEC_PARAM_TYPES(
						TEEC_VALUE_INPUT,
						TEEC_VALUE_INPUT,
						TEEC_VALUE_INOUT,
						TEEC_NONE);
		break;
	default:
		PS_PRINT_ERR("Wrong cmd, 0x%x", cmd);
		return -EFAULT;
	}

	result = TEEK_InvokeCommand(session, cmd, &op, &origin);
	if (result != TEEC_SUCCESS) {
		PS_PRINT_ERR("Invoke CMD fail, RC=0x%x, RO=0x%x\n", result, origin);
		return -EFAULT;
	}
	PS_PRINT_DBG("Exec TEE CMD success.\n");
	if (pinop == SEPLAT_GPIO_OP_READ)
		*onoff = op.params[2].value.a;

	return 0;
}

static int nfc_ca_exec_cmd_retry(u32 cmd, u32 pin, u32 pinop, u32 *onoff)
{
	int ret;
	int retry;
	for (retry = 0; retry < CA_EXEC_RETRY_CNT; retry++) {
		ret = nfc_ca_exec_cmd(cmd, pin, pinop, onoff);
		if (ret == 0) {
			break;
		}
	}
	if (ret != 0)
		PS_PRINT_ERR("CA execute retry 3 times fail\n", ret);

	return 0;
}

int nfc_ca_exec_power_cmd(u32 *onoff)
{
	return nfc_ca_exec_cmd_retry(CA11_SEPLAT_CMD_ID_POWER, SEPLAT_GPIO_PIN_MAX, SEPLAT_GPIO_OP_MAX, onoff);
}

int nfc_ca_exec_power_gpio_cmd(u32 pin, u32 pinop, u32 *onoff)
{
	return nfc_ca_exec_cmd_retry(CA11_SEPLAT_CMD_ID_POWER_GPIO, pin, pinop, onoff);
}

void nfc_ca_deinit(void)
{
	TEEC_Context *context = &g_ca11_seplat_context;
	TEEC_Session *session = &g_ca11_seplat_tee_session;
	TEEK_CloseSession(session);
	TEEK_FinalizeContext(context);
}

int nfc_ven_set(bool onoff)
{
	u32 pin_onff = (onoff == true) ? 1 : 0;
	return nfc_ca_exec_power_cmd(&pin_onff);
}
int nfc_ven_force_set(bool onoff)
{
	u32 pin_onff = (onoff == true) ? 1 : 0;
	return nfc_ca_exec_power_gpio_cmd(SEPLAT_GPIO_PIN_VEN, SEPLAT_GPIO_OP_WRITE, &pin_onff);
}

int nfc_ven_get(void)
{
	int ret;
	u32 pin_onoff;
	ret = nfc_ca_exec_power_gpio_cmd(SEPLAT_GPIO_PIN_VEN, SEPLAT_GPIO_OP_READ, &pin_onoff);
	if (ret != 0)
		return -1;

	return pin_onoff;
}

int nfc_dwl_req_set(bool onoff)
{
	u32 pin_onff = onoff == true ? 1 : 0;
	return nfc_ca_exec_power_gpio_cmd(SEPLAT_GPIO_PIN_DWL, SEPLAT_GPIO_OP_WRITE, &pin_onff);
}

int nfc_dwl_req_get(void)
{
	int ret;
	u32 pin_onoff;
	ret = nfc_ca_exec_power_gpio_cmd(SEPLAT_GPIO_PIN_DWL, SEPLAT_GPIO_OP_READ, &pin_onoff);
	if (ret != 0)
		return -1;

	return pin_onoff;
}

static int nfc_get_dts_config_string(const char *node_name,
	const char *prop_name,
	char *out_string,
	int out_string_len)
{
	struct device_node *np = NULL;
	const char *out_value = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_string(np, prop_name, (const char **)&out_value);
		if (ret != 0) {
			PS_PRINT_ERR("can not get prop values with prop_name: %s\n", prop_name);
			continue;
		}
		if (out_value == NULL) {
			PS_PRINT_ERR("error out_value = NULL\n");
			ret = -1;
		} else if (strlen(out_value) >= out_string_len) {
			PS_PRINT_ERR("error out_value len :%d >= out_string_len:%d\n",
						(int)strlen(out_value), (int)out_string_len);
			ret = -1;
		} else {
			if (strcpy_s(out_string, out_string_len, out_value) == EOK) {
				PS_PRINT_INFO(": =%s\n", out_string);
			} else {
				ret = -1;
			}
		}
	}
	return ret;
}

static void get_nfc_wired_ese_type(void)
{
	int ret;
	char nfc_on_str[MAX_CONFG_VALUE_SIZE] = {0};

	ret = nfc_get_dts_config_string("nfc_ese_type", "nfc_ese_type",
									nfc_on_str, sizeof(nfc_on_str));
	if (ret != 0) {
		g_nfc_ese_type = NFC_WITHOUT_ESE;
		PS_PRINT_ERR("can't find nfc_ese_type node\n");
		return;
	}
	if (!strncasecmp(nfc_on_str, "hisee", strlen("hisee"))) {
		g_nfc_ese_type = NFC_ESE_HISEE;
	} else if (!strncasecmp(nfc_on_str, "p61", strlen("p61"))) {
		g_nfc_ese_type = NFC_ESE_P61;
	} else {
		g_nfc_ese_type = NFC_WITHOUT_ESE;
	}
	PS_PRINT_INFO(" g_nfc_ese_type:%d\n", g_nfc_ese_type);
	return;
}

/* FUNCTION: caxx_parse_dt
 * DESCRIPTION: caxx_parse_dt, get gpio configuration from device tree system
 * Parameters
 * struct device *dev:device data
 * board_info_t *pdata:i2c data
 * RETURN VALUE
 * int: result
 */
static int caxx_parse_dt(struct device *dev,
						board_info_t *pdata)
{
	struct device_node *np = dev->of_node;

	/* int gpio */
	pdata->irq_gpio = of_get_named_gpio_flags(np, "caxx,gpio_wakeup_host", 0, NULL);
	if (pdata->irq_gpio < 0) {
		PS_PRINT_ERR("failed to get \"caxx,gpio_wakeup_host\"\n");
		goto err;
	}

	if (g_nfc_chip_type != NFCTYPE_CA11) {
		/* nfc_fm_dload gpio */
		pdata->fwdl_en_gpio = of_get_named_gpio_flags(np, "caxx,gpio_wakeup_device", 0, NULL);
		if (pdata->fwdl_en_gpio < 0) {
			PS_PRINT_ERR("failed to get \"caxx,gpio_wakeup_device\"\n");
			goto err;
		}

		/* nfc_ven gpio */
		pdata->ven_gpio = of_get_named_gpio_flags(np, "caxx,gpio_poweron", 0, NULL);
		if (pdata->ven_gpio < 0) {
			PS_PRINT_ERR("failed to get \"caxx,gpio_poweron\"\n");
			goto err;
		}
	}

	PS_PRINT_INFO("irq_gpio=%d, fwdl_en_gpio=%d, ven_gpio=%d \n",
				pdata->irq_gpio, pdata->fwdl_en_gpio, pdata->ven_gpio);
	return 0;
err:
	return -1;
}

/* FUNCTION: caxx_gpio_request
 * DESCRIPTION: caxx_gpio_request, nfc gpio configuration
 * Parameters
 * struct device *dev:device data
 * board_info_t *pdata:i2c data
 * RETURN VALUE
 * int: result
 */
static int caxx_gpio_request(struct device *dev,
							board_info_t *pdata)
{
	int ret;
	unused(dev);

	PS_PRINT_INFO("caxx_gpio_request enter\n");

	// NFC_INT
	ret = gpio_request(pdata->irq_gpio, "nfc_int");
	if (ret != 0) {
		goto err_irq;
	}
	ret = gpio_direction_input(pdata->irq_gpio);
	if (ret != 0) {
		goto err_fwdl_en;
	}

	if (g_nfc_chip_type != NFCTYPE_CA11) {
		// NFC_FWDL
		ret = gpio_request(pdata->fwdl_en_gpio, "nfc_wake");
		if (ret != 0) {
			goto err_fwdl_en;
		}
		ret = gpio_direction_output(pdata->fwdl_en_gpio, 0);
		if (ret != 0) {
			goto err_ven;
		}

		// NFC_VEN
		ret = gpio_request(pdata->ven_gpio, "nfc_ven");
		if (ret != 0) {
			goto err_ven;
		}
		ret = gpio_direction_output(pdata->ven_gpio, 0);
		if (ret != 0) {
			goto err_clk_req;
		}
	}

	return 0;

err_clk_req:
	gpio_free(pdata->ven_gpio);
err_ven:
	gpio_free(pdata->fwdl_en_gpio);
err_fwdl_en:
	gpio_free(pdata->irq_gpio);
err_irq:

	PS_PRINT_ERR("gpio request err %d\n", ret);
	return ret;
}

static void get_nfc_chip_type(void)
{
	int ret;
	char nfc_on_str[MAX_CONFG_VALUE_SIZE] = {0};
	char ca11_str[] = "ca11";

	ret = nfc_get_dts_config_string("nfc_chip_type", "nfc_chip_type",
									nfc_on_str, sizeof(nfc_on_str));
	if (ret != 0) {
		PS_PRINT_ERR("can't find nfc_chip_type node\n");
		return;
	}

	if (!strncasecmp(nfc_on_str, ca11_str, MAX_CONFG_VALUE_SIZE)) {
		g_nfc_chip_type = NFCTYPE_CA11;
	} else {
		g_nfc_chip_type = NFCTYPE_CA01;
	}

	g_is_sn110 = false;
	PS_PRINT_INFO("nfc_chip_type:%d, %s\n", g_is_sn110, nfc_on_str);
	return;
}

static int nfc_get_dts_config_u32(const char *node_name, const char *prop_name, u32 *pvalue)
{
	struct device_node *np = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_u32(np, prop_name, pvalue);
		if (ret != 0) {
			PS_PRINT_DBG("can not get prop values with prop_name: %s\n", prop_name);
		} else {
			PS_PRINT_INFO("%s=%d\n", prop_name, *pvalue);
		}
	}
	return ret;
}

static int nfc_get_ese_spi_bus(void)
{
	int ret;
	ret = nfc_get_dts_config_u32("ese_config_spi_bus", "ese_config_spi_bus", &g_ese_spi_bus);
	if (ret != 0) {
		PS_PRINT_ERR("can't get nfc spi_bus config node!\n");
		return -1;
	}
	PS_PRINT_INFO("g_ese_spi_bus = %d\n", g_ese_spi_bus);
	return 0;
}

static int nfc_get_ese_pwr_req_gpio(void)
{
	int ret;
	ret = nfc_get_dts_config_u32("gpio_svdd_pwr_req", "gpio_svdd_pwr_req", &g_ese_svdd_pwr_req);
	if (ret != 0) {
		PS_PRINT_ERR("can't get nfc gpio_svdd_pwr_req config node!\n");
		return -1;
	}
	PS_PRINT_INFO("g_ese_svdd_pwr_req = %d\n", g_ese_svdd_pwr_req);
	return 0;
}

static void nfc_write_conf_to_tee(void)
{
}

/* FUNCTION: caxx_dev_irq_handler
 * DESCRIPTION: irq handler, jump here when receive an irq request from NFC chip
 * Parameters
 * int irq: irq number
 * void *dev_id:device structure
 * RETURN VALUE
 * irqreturn_t: irq handle result
 */
static irqreturn_t caxx_dev_irq_handler(int irq, void *dev_id)
{
	const unsigned int wait_time = 3; // 3ms
	unused(irq);
	unused(dev_id);
	board_wkup_host_gpio_int_enable(NFC_DISABLE);
	__pm_wakeup_event(g_board_info.wl, jiffies_to_msecs(wait_time * HZ));
	nfc_wakeup_host_gpio_isr();

	return IRQ_HANDLED;
}

static int nfc_ven_low_beforepwd(struct notifier_block *this, unsigned long code,
								void *unused)
{
	unused(this);
	unused(code);
	unused(unused);
	PS_PRINT_INFO("enter!\n");
	msleep(10); // 拉低后延时10ms，避免上层应用立即又拉高
	return 0;
}

static struct notifier_block nfc_ven_low_notifier = {
	.notifier_call = nfc_ven_low_beforepwd,
};

static int board_gpio_config(void)
{
	int ret;
	ret = caxx_parse_dt(&g_board_info.client->dev, &g_board_info);
	if (ret < 0) {
		dev_err(&g_board_info.client->dev, "failed to parse device tree: %d\n", ret);
		return BOARD_FAIL;
	}
	/* config nfc gpio */
	ret = caxx_gpio_request(&g_board_info.client->dev, &g_board_info);
	if (ret != 0) {
		dev_err(&g_board_info.client->dev, "failed to request gpio\n");
		return BOARD_FAIL;
	}
	gpio_set_value(g_board_info.fwdl_en_gpio, 0);
	return BOARD_SUCC;
}

/* FUNCTION: caxx_gpio_release
 * DESCRIPTION: caxx_gpio_release, release nfc gpio
 * Parameters
 * RETURN VALUE
 * none
 */
static void caxx_gpio_release(void)
{
	gpio_free(g_board_info.irq_gpio);
	if (g_nfc_chip_type != NFCTYPE_CA11) {
		gpio_free(g_board_info.ven_gpio);
		gpio_free(g_board_info.fwdl_en_gpio);
	}
	return;
}

static int caxx_irq_request(void)
{
	int ret;
	dev_info(&g_board_info.client->dev, "%s : requesting IRQ %d\n",
		__func__, g_board_info.client->irq);
	g_board_info.irq_enabled = true;
	ret = request_irq(g_board_info.client->irq, caxx_dev_irq_handler,
					IRQF_TRIGGER_HIGH,
					g_board_info.client->name, NULL);
	if (ret != 0)
		dev_err(&g_board_info.client->dev, "request_irq failed\n");
	return ret;
}

int board_init(void *param)
{
	int ret;

	if (param == NULL) {
		PS_PRINT_ERR("caxx device is null\n");
		return BOARD_FAIL;
	}
	g_board_info.client = (struct i2c_client *)param;

	get_nfc_wired_ese_type();
	get_nfc_chip_type();

	/* get gpio config */
	ret = board_gpio_config();
	if (ret != BOARD_SUCC)
		return BOARD_FAIL;

	g_board_info.client->irq = gpio_to_irq(g_board_info.irq_gpio);
	ret = irq_set_irq_wake(g_board_info.client->irq, 1);
	if (ret != 0) {
		PS_PRINT_ERR("failed: ret=%d\n", ret);
		goto board_init_gpio_err;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	g_board_info.wl = wakeup_source_register(&g_board_info.client->dev, "nfc_locker");
#else
	g_board_info.wl = wakeup_source_register("nfc_locker");
#endif
	g_board_info.ssi_clk_gpio = g_board_info.fwdl_en_gpio;
	g_board_info.ssi_data_gpio = g_board_info.irq_gpio;

	/* notifier for supply shutdown */
	PS_PRINT_INFO("register_reboot_notifier\n");
	register_reboot_notifier(&nfc_ven_low_notifier);

	/* Initialise mutex and work queue */
	spin_lock_init(&g_board_info.irq_enabled_lock);
	nfc_driver_init(NULL);

	/* request irq.  the irq is set whenever the chip has data available
	 * for reading.  it is cleared when all data has been read.
	 */
	if (caxx_irq_request() != BOARD_SUCC)
		goto board_init_irq_err;

	board_wkup_host_gpio_int_enable(NFC_DISABLE);
	enable_irq_wake(g_board_info.client->irq);

	/* get and save configure name */
	if ((nfc_get_ese_spi_bus() == 0) && (nfc_get_ese_pwr_req_gpio() == 0))
		nfc_write_conf_to_tee();

	if (g_nfc_chip_type == NFCTYPE_CA11) {
		if (nfc_ca_init() != BOARD_SUCC) {
			goto board_init_irq_err;
		}
	}

	PS_PRINT_INFO("success.\n");
	return BOARD_SUCC;

board_init_irq_err:
	nfc_driver_deinit();
	wakeup_source_unregister(g_board_info.wl);
board_init_gpio_err:
	caxx_gpio_release();
	return BOARD_FAIL;
}

void board_deinit(void)
{
	irq_set_irq_wake(g_board_info.client->irq, 0);
	wakeup_source_unregister(g_board_info.wl);
	nfc_ven_set(false);
	free_irq(g_board_info.client->irq, NULL);

	caxx_gpio_release();
	if (g_nfc_chip_type == NFCTYPE_CA11)
		nfc_ca_deinit();

	return;
}

void board_pwn_ctrl(nfc_enable_ctrl_t enable)
{
	if (enable == NFC_ENABLE) {
		/* power on */
		nfc_dwl_req_set(false);
		nfc_ven_force_set(true);
	} else {
		/* power off */
		nfc_dwl_req_set(false);
		nfc_ven_force_set(false);
	}
}

void board_wkup_host_gpio_int_enable(nfc_enable_ctrl_t en)
{
	unsigned long flags;

	spin_lock_irqsave(&g_board_info.irq_enabled_lock, flags);
	if (en == NFC_ENABLE) {
		if (!g_board_info.irq_enabled) {
			g_board_info.irq_enabled = true;
			enable_irq(g_board_info.client->irq);
		}
	} else {
		if (g_board_info.irq_enabled) {
			disable_irq_nosync(g_board_info.client->irq);
			g_board_info.irq_enabled = false;
		}
	}
	spin_unlock_irqrestore(&g_board_info.irq_enabled_lock, flags);
}

int board_get_dev_wkup_host_status(void)
{
	return gpio_get_value(g_board_info.irq_gpio);
}

int board_get_host_wkup_dev_status(void)
{
	return nfc_dwl_req_get();
}

void board_set_host_wkup_dev_value(int value)
{
	bool onoff = (value == 1) ? true : false;
	nfc_dwl_req_set(onoff);
}

const char *board_get_fw_cfg_path(void)
{
	return NULL;
}

int board_i2c_write(char *buf, int count)
{
	return (int)i2c_master_send(g_board_info.client, buf, count);
}

int board_i2c_read(char *buf, int count)
{
	return (int)i2c_master_recv(g_board_info.client, buf, count);
}

int board_get_chip_type(void)
{
	return g_nfc_chip_type;
}

void request_irq_failed(void)
{
	nfc_driver_deinit();
	caxx_gpio_release();
}

#ifdef PLATFORM_DEBUG_ENABLE
int board_get_ssi_data_gpio_value(void)
{
	gpio_direction_input(g_board_info.ssi_data_gpio); // 设置为输入
	return gpio_get_value(g_board_info.ssi_data_gpio);
}
void board_set_ssi_data_gpio_value(int value)
{
	if (value) {
		gpio_direction_output(g_board_info.ssi_data_gpio, 1);
	} else {
		gpio_direction_output(g_board_info.ssi_data_gpio, 0);
	}
}

void board_set_ssi_clk_gpio_value(int value)
{
	if (value) {
		gpio_direction_output(g_board_info.ssi_clk_gpio, 1);
	} else {
		gpio_direction_output(g_board_info.ssi_clk_gpio, 0);
	}
}

void board_ssi_gpio_init(void)
{
	int ret;
	const unsigned short wait_after_init_ms = 100;

	free_irq(g_board_info.client->irq, NULL);

	ret = gpio_direction_output(g_board_info.ssi_clk_gpio, 0);
	if (ret < 0)
		PS_PRINT_ERR("fail set ssi_clk_gpio gpio as out, gpio:%d ret:%d\n", g_board_info.ssi_clk_gpio, ret);

	ret = gpio_direction_output(g_board_info.ssi_data_gpio, 0);
	if (ret < 0)
		PS_PRINT_ERR("fail set ssi_data_gpio gpio as out, gpio:%d ret:%d\n", g_board_info.ssi_data_gpio, ret);

	mdelay(wait_after_init_ms);
}

void board_ssi_gpio_restore(void)
{
	int ret;
	const unsigned short wait_after_set_ms = 100;

	ret = gpio_get_value(g_board_info.ssi_data_gpio);
	if (ret != 0) {
		PS_PRINT_ERR("ssi data gpio_get_value ret:%d\n", ret);
		gpio_set_value(g_board_info.ssi_data_gpio, 0);
		mdelay(wait_after_set_ms);
	}

	ret = gpio_direction_input(g_board_info.ssi_data_gpio);
	if (ret < 0)
		PS_PRINT_ERR("restore fail set ssi_data_gpio gpio as in failed ret=%d\n", ret);

	g_board_info.client->irq = gpio_to_irq(g_board_info.ssi_data_gpio);
	ret = request_irq(g_board_info.client->irq, caxx_dev_irq_handler,
					IRQF_TRIGGER_HIGH | IRQF_NO_SUSPEND, "nfc_wkup_gpio_irq", NULL);
	if (ret < 0)
		PS_PRINT_ERR("irq request fail, irq:%d, ret:%d\n", g_board_info.client->irq, ret);

	board_wkup_host_gpio_int_enable(NFC_DISABLE);
	board_wkup_host_gpio_int_enable(NFC_ENABLE);
	PS_PRINT_ERR("restore plat_dev->irq_enable : %d\n", g_board_info.irq_enabled);
}
#endif

