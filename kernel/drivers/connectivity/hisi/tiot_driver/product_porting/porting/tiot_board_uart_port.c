/**
 * Copyright (c) @CompanyNameMagicTag 2023-2023. All rights reserved. \n
 *
 * Description: Demo board UART port\n
 * Author: @CompanyNameTag \n
 * History: \n
 * 2023-03-25, Create file. \n
 */
#include "tiot_board_uart_port.h"
#include "tiot_board_log.h"
#include "securec.h"
#include "tiot_dts_node.h"

#include <linux/tty.h>
#include <linux/platform_device.h>
#include <linux/serial.h>
#include <linux/serial_core.h>

#define TTY_LDISC_MAGIC    0x5403

#define TIOT_TTY_RX_ROOM   1024U

#define HW_STOPPED_WAIT_MS 200U

typedef struct {
    tiot_xmit_callbacks cbs;
} tiot_tty_ctrl;

static tiot_tty_ctrl g_tiot_tty_ctrl = { 0 };

static int32_t tiot_tty_ldisc_open(struct tty_struct *tty);
static void tiot_tty_ldisc_close(struct tty_struct *tty);
static void tiot_tty_ldisc_receive(struct tty_struct *tty, const uint8_t *data, char *tty_flags, int32_t count);

/* 使用内核未使用的LDISC */
#if (!defined(CONFIG_TI_ST))
#define N_HW_TIOT 22 // 优先使用 22
#elif (defined(CONFIG_CONNECTIVITY_HI11XX))
#error "should set N_HW_TIOT that is not used by the system"
#else
#define N_HW_TIOT (NR_LDISCS - 1)
#endif

static struct tty_ldisc_ops g_tiot_ldisc_ops = {
    .magic = TTY_LDISC_MAGIC,
    .name = "n_tiot", /* 必须以n_开头 */
    .open = tiot_tty_ldisc_open,
    .close = tiot_tty_ldisc_close,
    .receive_buf = tiot_tty_ldisc_receive,
    .owner = THIS_MODULE
};

static int32_t tiot_tty_ldisc_open(struct tty_struct *tty)
{
    tiot_print_info("%s enter\n", __func__);
    tty->receive_room = TIOT_TTY_RX_ROOM;

    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);
    return 0;
}

static void tiot_tty_ldisc_close(struct tty_struct *tty)
{
    tiot_print_info("%s: entered\n", __func__);

    if (tty->disc_data == NULL) {
        tiot_print_err("tty or tty->disc_data is NULL\n");
        return;
    }

    /* Flush any pending characters in the driver and discipline. */
    tty_ldisc_flush(tty);
    tty_driver_flush_buffer(tty);

    tiot_print_info("tty close done!\n");
}

static void tiot_tty_ldisc_receive(struct tty_struct *tty, const uint8_t *data, char *tty_flags, int32_t count)
{
    tiot_xmit *xmit = NULL;
    if (data == NULL || count < 0) {
        return;
    }

    xmit = tty->disc_data;
    if (g_tiot_tty_ctrl.cbs.rx_notify) {
        g_tiot_tty_ctrl.cbs.rx_notify(xmit, (char *)data, count);
    }
}

int32_t tiot_board_uart_open(tiot_xmit *xmit, tiot_xmit_callbacks *cb)
{
    struct tty_struct *tty = NULL;
    struct tty_port *port = NULL;
    int ret;
    dev_t dev_no;
    const char *tty_name;

    tiot_print_info("%s\n", __func__);
    tty_name = (const char *)xmit->id;
    /* 获取tty dev_no */
    ret = tty_dev_name_to_number(tty_name, &dev_no);
    if (ret != 0) {
        tiot_print_err("can't found tty:%s ret=%d\n", tty_name, ret);
        return -1;
    }

    /* open tty */
    tty = tty_kopen(dev_no);
    if (!tty || IS_ERR(tty)) {
        tiot_print_err("open tty %x failed ret=%d\n", dev_no, PTR_ERR_OR_ZERO(tty));
        return -ENODEV;
    }

    if (tty->ops->open) {
        ret = tty->ops->open(tty, NULL);
    } else {
        tiot_print_err("tty->ops->open is NULL\n");
        ret = -ENODEV;
    }

    if (ret) {
        tty_unlock(tty);
        xmit->handle = NULL;
        return ret;
    }

    port = tty->port;
    port->closing_wait = (3000 * HZ) / 100;  /* 设为内核默认值. */
    /* 打开后再配置波特率 */
    tty_unlock(tty);
    /* 先注册ldisc再set ldisc */
    tty_register_ldisc(N_HW_TIOT, &g_tiot_ldisc_ops);
    tty_set_ldisc(tty, N_HW_TIOT);
    /* set ldisc前可能tty已设置ldisc，释放旧ldisc时会同时释放disc_data， 必须在set ldisc后再设置tty->disc_data */
    xmit->handle = tty;
    tty->disc_data = xmit;
    if (!g_tiot_tty_ctrl.cbs.rx_notify) {
        g_tiot_tty_ctrl.cbs.rx_notify = cb->rx_notify;
    }
    return 0;
}

void tiot_board_uart_close(tiot_xmit *xmit)
{
    /* close tty */
    struct tty_struct *tty =  (struct tty_struct *)xmit->handle;
    struct uart_state *state = tty->driver_data;
    struct uart_port *uport = state->uart_port;
    struct tty_port *port = tty->port;

    tiot_print_info("%s\n", __func__);

    /* close tty */
    if (tty == NULL) {
        tiot_print_err("tty is null, ignore\n");
        return;
    }

    tty_lock(tty);

    if (uport->hw_stopped) {
        tiot_print_warning("uart close when uart is hw_stopped, ignore pending data.");
        tty_wait_until_sent(tty, msecs_to_jiffies(HW_STOPPED_WAIT_MS));  /* 被流控拉住时，尝试等待200ms流控恢复. */
        tiot_print_warning("wait hw_stopped done.");
        port->closing_wait = ASYNC_CLOSING_WAIT_NONE;    /* 被流控拉住时，直接关闭uart，不再等待阻塞数据发完. */
    }

    if (tty->ops->close) {
        tty->ops->close(tty, NULL);
    } else {
        tiot_print_warning("tty->ops->close is null\n");
    }
    tty_unlock(tty);
    tty_kclose(tty);
    tty_unregister_ldisc(N_HW_TIOT);
    (void)memset_s(&g_tiot_tty_ctrl, sizeof(tiot_tty_ctrl), 0, sizeof(tiot_tty_ctrl));
}

int32_t tiot_board_uart_write(tiot_xmit *xmit, const uint8_t *buff, uint32_t len)
{
    /* 异步接口，但是设置波特率前会自动flush */
    struct tty_struct *tty = (struct tty_struct *)xmit->handle;
    return tty->ops->write(tty, buff, len);
}

int32_t tiot_board_uart_read(tiot_xmit *xmit, uint8_t *buff, uint32_t len)
{
    /* 内核tty无同步读取接口，只通过回调接收 */
    return -1;
}

int32_t tiot_board_uart_set_config(tiot_xmit *xmit, tiot_uart_config *config)
{
    struct tty_struct *tty = (struct tty_struct *)xmit->handle;
    tiot_uart_config *uart_config = config;
    uint32_t baudrate = uart_config->baudrate;
    struct ktermios ktermios;

    if (baudrate == 0) {
        baudrate = bsxx_board_get_baudrate();
    }

    ktermios = tty->termios;
    /* close soft flowctrl */
    ktermios.c_iflag &= ~IXON;

    /* set uart cts/rts flowctrl */
    ktermios.c_cflag &= ~CRTSCTS;
    /* on to ignore modem status change */
    ktermios.c_cflag |= CLOCAL;
    if (uart_config->attr.flow_ctrl == TIOT_UART_ATTR_FLOW_CTRL_ENABLE) {
        ktermios.c_cflag |= CRTSCTS;
        /* off to respect modem status change */
        ktermios.c_cflag &= ~CLOCAL;
    }

    /* set csize */
    ktermios.c_cflag &= ~(CSIZE);
    ktermios.c_cflag |= CS8;

    /* set uart baudrate */
    ktermios.c_cflag &= ~CBAUD;
    ktermios.c_cflag |= BOTHER;

    /* Ignore break & parity to ignore overrun, when using cpu to receive. */
    ktermios.c_iflag |= IGNBRK;
    ktermios.c_iflag |= IGNPAR;

    /* close parity */
    ktermios.c_iflag &= ~INPCK;
    ktermios.c_cflag &= ~PARENB;
    ktermios.c_cflag &= ~PARODD;
    tty_termios_encode_baud_rate(&ktermios, baudrate, baudrate);

    /* UART硬件fifo清空后才能切换UART配置，通过tty_wait_until_sent等待 */
    tty_wait_until_sent(tty, 0);
    tty_set_termios(tty, &ktermios);
    return 0;
}
