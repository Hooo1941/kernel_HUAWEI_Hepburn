/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
 */
// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Simple synchronous userspace interface to SPI devices
 *
 * Copyright (C) 2006 SWAPP
 *	Andrea Paterniani <a.paterniani@swapp-eng.it>
 * Copyright (C) 2007 David Brownell (simplification, cleanup)
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/acpi.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>

#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/delay.h>

#include <huawei_platform/log/hw_log.h>

#include <linux/miscdevice.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include "wszj_gpioinit.h"

#define PMU_LDO_54_STR "2v85ldo54"
#define PMU_LDO_53_STR "simldo53"
#define PMU_LDO_25_STR "1v2ldo25"
#define PMU_LDO_3_STR "rfpower"

#define PMU_32KB_CLK_STR "clk_pmu32kb"

#define HWLOG_TAG			uwb
#define DEV_NAME			"wszj"
#define WAIT_TIMEOUT		(HZ/10)

#define MAX_BUFF_SIZE		2048

#define CHIP_TYPE_BUFF_SIZE	40
#define CHIP_TYPE_FILE_ATTR	0664

#define GPIO_POWER
#define GPIO_RESET
#define GPIO_WAKEUP
#define GPIO_CS
#undef VERBOSE

#define POWER_ON			1
#define POWER_OFF			0
#define UWB_SWITCH_OPEN 	1
#define UWB_SWITCH_CLOSE 	0

#define WAKEUP_ON			0
#define WAKEUP_OFF			1

#define CS_ACTIVE			0
#define CS_INACTIVE			1

#define WAIT_IRQ_ON			0
#define WAIT_IRQ_OFF		1
#define WAIT_IRQ_OTHER		(-1)

HWLOG_REGIST();

/* Read / Write of the SPI device's power field */
#define SPI_IOC_RD_POWER	_IOR(SPI_IOC_MAGIC, 6, __u32)
#define SPI_IOC_WR_POWER	_IOW(SPI_IOC_MAGIC, 6, __u32)

/* Read / Write of the SPI wait status field */
#define SPI_IOC_RD_WAIT		_IOR(SPI_IOC_MAGIC, 7, __u32)
#define SPI_IOC_WR_WAIT		_IOW(SPI_IOC_MAGIC, 7, __u32)

/* Read / Write of the SPI device's reset  field */
#define SPI_IOC_RD_RESET	_IOR(SPI_IOC_MAGIC, 8, __u32)
#define SPI_IOC_WR_RESET	_IOW(SPI_IOC_MAGIC, 8, __u32)

/* Read / Write of the SPI device's reset  field */
#define SPI_IOC_RD_WAKEUP	_IOR(SPI_IOC_MAGIC, 9, __u32)
#define SPI_IOC_WR_WAKEUP	_IOW(SPI_IOC_MAGIC, 9, __u32)

/* Bit masks for spi_device.mode management.  Note that incorrect
 * settings for some settings can cause *lots* of trouble for other
 * devices on a shared bus:
 *
 *  - CS_HIGH ... this device will be active when it shouldn't be
 *  - 3WIRE ... when active, it won't behave as it should
 *  - NO_CS ... there will be no explicit message boundaries; this
 *	is completely incompatible with the shared bus model
 *  - READY ... transfers may proceed when they shouldn't.
 *
 * REVISIT should changing those flags be privileged?
 */
#define SPI_MODE_MASK		(SPI_CPHA | SPI_CPOL | SPI_CS_HIGH \
				| SPI_LSB_FIRST | SPI_3WIRE | SPI_LOOP \
				| SPI_NO_CS | SPI_READY | SPI_TX_DUAL \
				| SPI_TX_QUAD | SPI_TX_OCTAL | SPI_RX_DUAL \
				| SPI_RX_QUAD | SPI_RX_OCTAL)

struct wszj_trans {
	unsigned int		tx_total;
	unsigned int		rx_total;
	u8			*tx_buf;
	u8			*rx_buf;
};

uwb_power_supply g_uwb_pmu = {NULL, NULL, NULL, NULL, NULL, 0, 0};
static char wszj_chip_type[CHIP_TYPE_BUFF_SIZE];
static char wszj_32kclk_name[CHIP_TYPE_BUFF_SIZE];

static unsigned int g_pmusim_id = 0;
static unsigned int g_pmu38m_div_flag = 0;
static unsigned int g_2v85gpio_flag = 0;

static unsigned int wszj_major;
static struct class *wszj_class;
static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static unsigned int bufsiz = MAX_BUFF_SIZE;
module_param(bufsiz, uint, S_IRUGO);
MODULE_PARM_DESC(bufsiz, "data bytes in biggest supported SPI message");

static int wszj_request_gpios(struct wszj_data *wszj, struct spi_device *spi);
static int wszj_free_gpios(struct wszj_data *wszj);
static int wszj_power(struct wszj_data *wszj, int on);
static int wszj_reset(struct wszj_data *wszj);
static int wszj_wakeup(struct wszj_data *wszj);

static ssize_t
wszj_sync(struct wszj_data *wszj, struct spi_message *message)
{
	int status;
	struct spi_device *spi;

	spin_lock_irq(&wszj->spi_lock);
	spi = wszj->spi;
	spin_unlock_irq(&wszj->spi_lock);

	if (spi == NULL) {
		status = -ESHUTDOWN;
	} else {
#ifdef GPIO_CS
		if (wszj->cs_gpio > 0)
			gpio_set_value(wszj->cs_gpio, CS_ACTIVE);
#endif
		status = spi_sync(spi, message);
#ifdef GPIO_CS
		if (wszj->cs_gpio > 0)
			gpio_set_value(wszj->cs_gpio, CS_INACTIVE);
#endif
	}
	if (status == 0)
		status = message->actual_length;
	return status;
}

static ssize_t
wszj_sync_write(struct wszj_data *wszj, size_t len)
{
	struct spi_transfer	t = {
			.tx_buf		= wszj->tx_buffer,
			.len		= len,
			.speed_hz	= wszj->speed_hz,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return wszj_sync(wszj, &m);
}

static ssize_t
wszj_sync_read(struct wszj_data *wszj, size_t len)
{
	struct spi_transfer	t = {
			.rx_buf		= wszj->rx_buffer,
			.len		= len,
			.speed_hz	= wszj->speed_hz,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return wszj_sync(wszj, &m);
}

/* Read-only message with current device setup */
static ssize_t
wszj_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	struct wszj_data	*wszj;
	ssize_t			status;

	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz) {
		return -EMSGSIZE;
	}

	wszj = filp->private_data;

	mutex_lock(&wszj->buf_lock);
	status = wszj_sync_read(wszj, count);
	if (status > 0) {
		unsigned long	missing;

		missing = copy_to_user(buf, wszj->rx_buffer, status);
		if (missing == status) {
			status = -EFAULT;
		} else {
			status = status - missing;
		}
	}
	mutex_unlock(&wszj->buf_lock);

	return status;
}

/* Write-only message with current device setup */
static ssize_t
wszj_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	struct wszj_data	*wszj;
	ssize_t			status;
	unsigned long		missing;

	/* chipselect only toggles at start or end of operation */
	if (count > bufsiz) {
		return -EMSGSIZE;
	}

	wszj = filp->private_data;

	mutex_lock(&wszj->buf_lock);
	missing = copy_from_user(wszj->tx_buffer, buf, count);
	if (missing == 0) {
		status = wszj_sync_write(wszj, count);
	} else {
		status = -EFAULT;
	}
	mutex_unlock(&wszj->buf_lock);

	return status;
}

static int
wszj_message_copy_buff(struct spi_transfer *k_tmp, struct spi_ioc_transfer *u_tmp,
				struct wszj_trans *trans, unsigned int len_aligned)
{
	if (u_tmp->rx_buf) {
		/* this transfer needs space in RX bounce buffer */
		trans->rx_total += len_aligned;
		if (trans->rx_total > bufsiz)
			return -EMSGSIZE;
		k_tmp->rx_buf = trans->rx_buf;
		trans->rx_buf += len_aligned;
	}
	if (u_tmp->tx_buf) {
		/* this transfer needs space in TX bounce buffer */
		trans->tx_total += len_aligned;
		if (trans->tx_total > bufsiz)
			return -EMSGSIZE;
		k_tmp->tx_buf = trans->tx_buf;
		if (copy_from_user(trans->tx_buf, (const u8 __user *)
						(uintptr_t)u_tmp->tx_buf, u_tmp->len))
			return -EFAULT;
		trans->tx_buf += len_aligned;
	}
	return 0;
}

static void
wszj_message_copy_trans(struct wszj_data *wszj, struct spi_transfer *k_tmp,
				struct spi_ioc_transfer *u_tmp)
{
	k_tmp->cs_change = !!u_tmp->cs_change;
	k_tmp->tx_nbits = u_tmp->tx_nbits;
	k_tmp->rx_nbits = u_tmp->rx_nbits;
	k_tmp->bits_per_word = u_tmp->bits_per_word;
	k_tmp->delay.value = u_tmp->delay_usecs;
	k_tmp->delay.unit = SPI_DELAY_UNIT_USECS;
	k_tmp->speed_hz = u_tmp->speed_hz;
	k_tmp->word_delay.value = u_tmp->word_delay_usecs;
	k_tmp->word_delay.unit = SPI_DELAY_UNIT_USECS;
	if (!k_tmp->speed_hz)
		k_tmp->speed_hz = wszj->speed_hz;
}

#ifdef VERBOSE
static void
wszj_message_verbose(struct wszj_data *wszj, struct spi_transfer *k_tmp)
{
	dev_dbg(&wszj->spi->dev,
		"  xfer len %u %s%s%s%dbits %u usec %u usec %uHz\n",
		k_tmp->len,
		k_tmp->rx_buf ? "rx " : "",
		k_tmp->tx_buf ? "tx " : "",
		k_tmp->cs_change ? "cs " : "",
		k_tmp->bits_per_word ? : wszj->spi->bits_per_word,
		k_tmp->delay.value,
		k_tmp->word_delay.value,
		k_tmp->speed_hz ? : wszj->spi->max_speed_hz);
}
#endif

static int
wszj_message_copy_data(struct spi_transfer *k_xfers, struct spi_ioc_transfer *u_xfers,
				unsigned int n_xfers)
{
	struct spi_transfer	*k_tmp;
	struct spi_ioc_transfer *u_tmp;
	unsigned int		n;

	/* copy any rx data out of bounce buffer */
	for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers;	n; n--, k_tmp++, u_tmp++) {
		if (u_tmp->rx_buf) {
			if (copy_to_user((u8 __user *)(uintptr_t)u_tmp->rx_buf,
							k_tmp->rx_buf, u_tmp->len))
				return -EFAULT;
		}
	}
	return 0;
}

static int
wszj_message(struct wszj_data *wszj, struct spi_ioc_transfer *u_xfers, unsigned int n_xfers)
{
	struct spi_message	msg;
	struct spi_transfer	*k_xfers;
	struct spi_transfer	*k_tmp;
	struct spi_ioc_transfer *u_tmp;
	struct wszj_trans		trans;
	unsigned int		n;
	unsigned int		total;
	int			status = -EFAULT;

	spi_message_init(&msg);
	k_xfers = kcalloc(n_xfers, sizeof(*k_tmp), GFP_KERNEL);
	if (k_xfers == NULL)
		return -ENOMEM;

	/* Construct spi_message, copying any tx data to bounce buffer.
	 * We walk the array of user-provided transfers, using each one
	 * to initialize a kernel version of the same transfer.
	 */
	trans.tx_buf = wszj->tx_buffer;
	trans.rx_buf = wszj->rx_buffer;
	total = 0;
	trans.tx_total = 0;
	trans.rx_total = 0;
	for (n = n_xfers, k_tmp = k_xfers, u_tmp = u_xfers; n; n--, k_tmp++, u_tmp++) {
		/* Ensure that also following allocations from rx_buf/tx_buf will meet
		 * DMA alignment requirements.
		 */
		unsigned int len_aligned = ALIGN(u_tmp->len, ARCH_KMALLOC_MINALIGN);

		k_tmp->len = u_tmp->len;

		total += k_tmp->len;
		/* Since the function returns the total length of transfers
		 * on success, restrict the total to positive int values to
		 * avoid the return value looking like an error.  Also check
		 * each transfer length to avoid arithmetic overflow.
		 */
		if (total > INT_MAX || k_tmp->len > INT_MAX) {
			status = -EMSGSIZE;
			goto done;
		}

		if ((status = wszj_message_copy_buff(k_tmp, u_tmp, &trans, len_aligned)) < 0) {
			goto done;
		}
		wszj_message_copy_trans(wszj, k_tmp, u_tmp);
#ifdef VERBOSE
		wszj_message_verbose(wszj, k_tmp);
#endif
		spi_message_add_tail(k_tmp, &msg);
	}

	if ((status = wszj_sync(wszj, &msg)) < 0) {
		goto done;
	}
	/* copy any rx data out of bounce buffer */
	if ((status = wszj_message_copy_data(k_xfers, u_xfers, n_xfers)) < 0) {
		goto done;
	}
	status = total;

done:
	kfree(k_xfers);
	return status;
}

static struct spi_ioc_transfer *
wszj_get_ioc_message(unsigned int cmd, struct spi_ioc_transfer __user *u_ioc, unsigned int *n_ioc)
{
	u32 tmp;

	/* Check type, command number and direction */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC ||
		_IOC_NR(cmd) != _IOC_NR(SPI_IOC_MESSAGE(0)) || _IOC_DIR(cmd) != _IOC_WRITE) {
		return ERR_PTR(-ENOTTY);
	}

	tmp = _IOC_SIZE(cmd);
	if ((tmp % sizeof(struct spi_ioc_transfer)) != 0) {
		return ERR_PTR(-EINVAL);
	}
	*n_ioc = tmp / sizeof(struct spi_ioc_transfer);
	if (*n_ioc == 0) {
		return NULL;
	}
	/* copy into scratch area */
	return memdup_user(u_ioc, tmp);
}

static int
wszj_ioctl_rd_wait(struct wszj_data *wszj, unsigned long arg)
{
	int retval;
	int gpio_value = 0;
	unsigned long flags;

	mutex_lock(&wszj->wait_lock);
	if (wszj->irq_gpio > 0)
		gpio_value = gpio_get_value(wszj->irq_gpio);
	spin_lock_irqsave(&wszj->irq_lock, flags);
	switch (wszj->wait_irq) {
	case WAIT_IRQ_ON:
		if (gpio_value) {
			spin_unlock_irqrestore(&wszj->irq_lock, flags);
		} else {
			spin_unlock_irqrestore(&wszj->irq_lock, flags);
			wait_event_interruptible(wszj->wait, wszj->wait_irq);
		}
		retval = put_user(!wszj->wait_irq, (__u32 __user *)arg);
		break;
	case WAIT_IRQ_OFF:
		wszj->wait_irq = WAIT_IRQ_ON;
		spin_unlock_irqrestore(&wszj->irq_lock, flags);
		retval = put_user(!wszj->wait_irq, (__u32 __user *)arg);
		break;
	default:
		spin_unlock_irqrestore(&wszj->irq_lock, flags);
		wait_event_interruptible_timeout(wszj->wait, wszj->wait_irq, WAIT_TIMEOUT);
		retval = put_user(wszj->wait_irq, (__u32 __user *)arg);
		break;
	}
	mutex_unlock(&wszj->wait_lock);
	return retval;
}

static int
wszj_ioctl_wr_mode(struct wszj_data *wszj, struct spi_device *spi,
				unsigned int cmd, unsigned long arg)
{
	int retval;
	u32 tmp;

	if (cmd == SPI_IOC_WR_MODE) {
		retval = get_user(tmp, (u8 __user *)arg);
	} else {
		retval = get_user(tmp, (u32 __user *)arg);
	}
	if (retval == 0) {
		struct spi_controller *ctlr = spi->controller;
		u32 save = spi->mode;

		if (tmp & ~SPI_MODE_MASK)
			return -EINVAL;
		if (ctlr->use_gpio_descriptors && ctlr->cs_gpiods && ctlr->cs_gpiods[spi->chip_select])
			tmp |= SPI_CS_HIGH;
		tmp |= spi->mode & ~SPI_MODE_MASK;
		spi->mode = (u16)tmp;
		retval = spi_setup(spi);
		if (retval < 0) {
			spi->mode = save;
		} else {
			dev_dbg(&spi->dev, "spi mode %x\n", tmp);
		}
	}
	return retval;
}

static int
wszj_ioctl_wr_lsb_first(struct wszj_data *wszj, struct spi_device *spi, unsigned long arg)
{
	int retval;
	u32 tmp;

	retval = get_user(tmp, (__u8 __user *)arg);
	if (retval == 0) {
		u32 save = spi->mode;

		if (tmp) {
			spi->mode |= SPI_LSB_FIRST;
		} else {
			spi->mode &= ~SPI_LSB_FIRST;
		}
		retval = spi_setup(spi);
		if (retval < 0) {
			spi->mode = save;
		} else {
			dev_dbg(&spi->dev, "%csb first\n", tmp ? 'l' : 'm');
		}
	}
	return retval;
}

static int
wszj_ioctl_wr_bits_per_word(struct wszj_data *wszj, struct spi_device *spi, unsigned long arg)
{
	int retval;
	u32 tmp;

	retval = get_user(tmp, (__u8 __user *)arg);
	if (retval == 0) {
		u8 save = spi->bits_per_word;

		spi->bits_per_word = tmp;
		retval = spi_setup(spi);
		if (retval < 0) {
			spi->bits_per_word = save;
		} else {
			dev_dbg(&spi->dev, "%d bits per word\n", tmp);
		}
	}
	return retval;
}

static int
wszj_ioctl_wr_max_speed(struct wszj_data *wszj, struct spi_device *spi, unsigned long arg)
{
	int retval;
	u32 tmp;

	retval = get_user(tmp, (__u32 __user *)arg);
	if (retval == 0) {
		u32 save = spi->max_speed_hz;

		spi->max_speed_hz = tmp;
		retval = spi_setup(spi);
		if (retval == 0) {
			wszj->speed_hz = tmp;
			dev_dbg(&spi->dev, "%d Hz (max)\n", wszj->speed_hz);
		}
		spi->max_speed_hz = save;
	}
	return retval;
}

static int
wszj_ioctl_wr_power(struct wszj_data *wszj, unsigned long arg)
{
	int retval;
	u32 tmp;

	retval = get_user(tmp, (__u32 __user *)arg);
	if (retval == 0)
		wszj_power(wszj, tmp);
	return retval;
}

static int
wszj_ioctl_wr_wait(struct wszj_data *wszj, unsigned long arg)
{
	int retval;
	u32 tmp;
	unsigned long flags;

	mutex_lock(&wszj->wake_lock);
	retval = get_user(tmp, (__u32 __user *)arg);
	if (retval == 0) {
		spin_lock_irqsave(&wszj->irq_lock, flags);
		switch (tmp) {
		case 0:
			wszj->wait_irq = WAIT_IRQ_OFF;
			spin_unlock_irqrestore(&wszj->irq_lock, flags);
			wake_up_interruptible(&wszj->wait);
			break;
		case 1:
			if (wszj->wait_irq == WAIT_IRQ_OFF)
				wszj->wait_irq = WAIT_IRQ_ON;
			spin_unlock_irqrestore(&wszj->irq_lock, flags);
			break;
		default:
			wszj->wait_irq = WAIT_IRQ_OTHER;
			spin_unlock_irqrestore(&wszj->irq_lock, flags);
			wake_up_interruptible(&wszj->wait);
			break;
		}
	}
	mutex_unlock(&wszj->wake_lock);
	return retval;
}

static int
wszj_ioctl_wr_reset(struct wszj_data *wszj, unsigned long arg)
{
	int retval;
	u32 tmp;

	retval = get_user(tmp, (__u32 __user *)arg);
	if (retval == 0 && tmp)
		wszj_reset(wszj);
	return retval;
}

static int
wszj_ioctl_wr_wakeup(struct wszj_data *wszj, unsigned long arg)
{
	int retval;
	u32 tmp;

	retval = get_user(tmp, (__u32 __user *)arg);
	if (retval == 0 && tmp)
		wszj_wakeup(wszj);
	return retval;
}

static int
wszj_ioctl_default(struct wszj_data *wszj, struct spi_device *spi,
				unsigned int cmd, unsigned long arg)
{
	int retval;
	unsigned int n_ioc;
	struct spi_ioc_transfer *ioc;

	/* segmented and/or full-duplex I/O request */
	/* Check message and copy into scratch area */
	ioc = wszj_get_ioc_message(cmd, (struct spi_ioc_transfer __user *)arg, &n_ioc);
	if (IS_ERR(ioc)) {
		retval = PTR_ERR(ioc);
		return retval;
	}
	if (!ioc)
		return 0;  /* nIoc is also 0 */
	/* translate to spi_message, execute */
	retval = wszj_message(wszj, ioc, n_ioc);
	kfree(ioc);
	return retval;
}

static int
wszj_ioctl_switch_extend(struct wszj_data *wszj, struct spi_device *spi,
				unsigned int cmd, unsigned long arg)
{
	int retval;
	switch (cmd) {
	case SPI_IOC_RD_POWER:
		retval = put_user(wszj->power, (__u32 __user *)arg);
		break;
	case SPI_IOC_RD_RESET:
#ifdef GPIO_RESET
		if (wszj->reset_gpio > 0) {
			retval = put_user(gpio_get_value(wszj->reset_gpio), (__u32 __user *)arg);
		} else
#endif
		retval = put_user(0, (__u32 __user *)arg);
		break;
	case SPI_IOC_RD_WAKEUP:
#ifdef GPIO_WAKEUP
		if (wszj->wakeup_gpio > 0) {
			retval = put_user(gpio_get_value(wszj->wakeup_gpio), (__u32 __user *)arg);
		} else
#endif
		retval = put_user(1, (__u32 __user *)arg);
		break;
	case SPI_IOC_WR_POWER:
		retval = wszj_ioctl_wr_power(wszj, arg);
		break;
	case SPI_IOC_WR_RESET:
		retval = wszj_ioctl_wr_reset(wszj, arg);
		break;
	case SPI_IOC_WR_WAKEUP:
		retval = wszj_ioctl_wr_wakeup(wszj, arg);
		break;
	default:
		/* translate to spi_message, execute */
		retval = wszj_ioctl_default(wszj, spi, cmd, arg);
		break;
	}
	return retval;
}

static int
wszj_ioctl_switch(struct wszj_data *wszj, struct spi_device *spi,
				unsigned int cmd, unsigned long arg)
{
	int retval;

	/* use the buffer lock here for triple duty:
	 * - prevent I/O (from us) so calling spi_setup() is safe;
	 * - prevent concurrent SPI_IOC_WR_* from morphing
	 * data fields while SPI_IOC_RD_* reads them;
	 * - SPI_IOC_MESSAGE needs the buffer locked "normally".
	 */
	mutex_lock(&wszj->buf_lock);
	switch (cmd) {
	case SPI_IOC_RD_MODE:
		retval = put_user(spi->mode & SPI_MODE_MASK, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_MODE32:
		retval = put_user(spi->mode & SPI_MODE_MASK, (__u32 __user *)arg);
		break;
	case SPI_IOC_RD_LSB_FIRST:
		retval = put_user((spi->mode & SPI_LSB_FIRST) ? 1 : 0, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_BITS_PER_WORD:
		retval = put_user(spi->bits_per_word, (__u8 __user *)arg);
		break;
	case SPI_IOC_RD_MAX_SPEED_HZ:
		retval = put_user(wszj->speed_hz, (__u32 __user *)arg);
		break;
	case SPI_IOC_WR_MODE:
	case SPI_IOC_WR_MODE32:
		retval = wszj_ioctl_wr_mode(wszj, spi, cmd, arg);
		break;
	case SPI_IOC_WR_LSB_FIRST:
		retval = wszj_ioctl_wr_lsb_first(wszj, spi, arg);
		break;
	case SPI_IOC_WR_BITS_PER_WORD:
		retval = wszj_ioctl_wr_bits_per_word(wszj, spi, arg);
		break;
	case SPI_IOC_WR_MAX_SPEED_HZ:
		retval = wszj_ioctl_wr_max_speed(wszj, spi, arg);
		break;
	default:
		retval = wszj_ioctl_switch_extend(wszj, spi, cmd, arg);
		break;
	}
	mutex_unlock(&wszj->buf_lock);
	return retval;
}

static long wszj_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int			retval;
	struct wszj_data	*wszj;
	struct spi_device	*spi;

	/* Check type and command number */
	if (_IOC_TYPE(cmd) != SPI_IOC_MAGIC)
		return -ENOTTY;

	/* guard against device removal before, or while,
	 * we issue this ioctl.
	 */
	wszj = filp->private_data;
	if (cmd == SPI_IOC_RD_WAIT)
		return wszj_ioctl_rd_wait(wszj, arg);
	else if (cmd == SPI_IOC_WR_WAIT)
		return wszj_ioctl_wr_wait(wszj, arg);
	spin_lock_irq(&wszj->spi_lock);
	spi = spi_dev_get(wszj->spi);
	spin_unlock_irq(&wszj->spi_lock);

	if (spi == NULL)
		return -ESHUTDOWN;
	retval = wszj_ioctl_switch(wszj, spi, cmd, arg);
	spi_dev_put(spi);
	return retval;
}

static int wszj_open(struct inode *inode, struct file *filp)
{
	struct wszj_data *wszj;
	int status = -ENXIO;
	mutex_lock(&device_list_lock);
	list_for_each_entry(wszj, &device_list, device_entry) {
		if (wszj->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}
	if (status) {
		pr_debug("wszj: nothing for minor %d\n", iminor(inode));
		goto err_find_dev;
	}
	if (!wszj->tx_buffer) {
		wszj->tx_buffer = kmalloc(bufsiz, GFP_KERNEL);
		if (!wszj->tx_buffer) {
			dev_dbg(&wszj->spi->dev, "open/ENOMEM\n");
			status = -ENOMEM;
			goto err_find_dev;
		}
	}
	if (!wszj->rx_buffer) {
		wszj->rx_buffer = kmalloc(bufsiz, GFP_KERNEL);
		if (!wszj->rx_buffer) {
			dev_dbg(&wszj->spi->dev, "open/ENOMEM\n");
			status = -ENOMEM;
			goto err_alloc_rx_buf;
		}
	}
	wszj->users++;
	filp->private_data = wszj;
	stream_open(inode, filp);
	mutex_unlock(&device_list_lock);
	return 0;
err_alloc_rx_buf:
	kfree(wszj->tx_buffer);
	wszj->tx_buffer = NULL;
err_find_dev:
	mutex_unlock(&device_list_lock);
	return status;
}

static int wszj_release(struct inode *inode, struct file *filp)
{
	struct wszj_data	*wszj;
	int			dofree;

	mutex_lock(&device_list_lock);
	wszj = filp->private_data;
	filp->private_data = NULL;

	spin_lock_irq(&wszj->spi_lock);
	/* ... after we unbound from the underlying device? */
	dofree = (wszj->spi == NULL);
	spin_unlock_irq(&wszj->spi_lock);

	/* last close? */
	wszj->users--;
	if (!wszj->users) {
		kfree(wszj->tx_buffer);
		wszj->tx_buffer = NULL;

		kfree(wszj->rx_buffer);
		wszj->rx_buffer = NULL;

		if (dofree) {
			kfree(wszj);
		} else {
			wszj->speed_hz = wszj->spi->max_speed_hz;
		}
	}
#ifdef CONFIG_SPI_SLAVE
	if (!dofree)
		spi_slave_abort(wszj->spi);
#endif
	mutex_unlock(&device_list_lock);

	return 0;
}

static const struct file_operations wszj_fops = {
	.owner =	THIS_MODULE,
	/* REVISIT switch to aio primitives, so that userspace
	 * gets more complete API coverage.  It'll simplify things
	 * too, except for the locking.
	 */
	.write =	wszj_write,
	.read =		wszj_read,
	.unlocked_ioctl = wszj_ioctl,
	.open =		wszj_open,
	.release =	wszj_release,
	.llseek =	no_llseek,
};

static irqreturn_t wszj_interrupt(int irq, void *dev_id)
{
	struct wszj_data	*wszj = dev_id;
	unsigned long		flags;

	spin_lock_irqsave(&wszj->irq_lock, flags);
	if (wszj->wait_irq == WAIT_IRQ_ON)
		wszj->wait_irq = WAIT_IRQ_OFF;
	spin_unlock_irqrestore(&wszj->irq_lock, flags);
	wake_up_interruptible(&wszj->wait);
	return IRQ_HANDLED;
}

static int wszj_request_gpio_power(struct wszj_data *wszj)
{
	int status = -1;
	hwlog_info("%s: wszj_request_gpios: %d\n", __func__, g_2v85gpio_flag);
	if (g_2v85gpio_flag == 0) {
		hwlog_info("wszj_request_gpio_power: no 2v85_gpio");
		return 0;
	}
	/* power on/off gpio */
	wszj->power_gpio = of_get_named_gpio(wszj->spi->dev.of_node, "2v85sw_gpio", 0);
	if (wszj->power_gpio > 0) {
		status = gpio_request(wszj->power_gpio, "wszj-pwr");
		if (status < 0) {
			wszj->power_gpio = 0;
			wszj_free_gpios(wszj);
			return status;
		}
		gpio_direction_output(wszj->power_gpio, POWER_OFF);
	}
	return status;
}

static int wszj_request_gpio_reset(struct wszj_data *wszj)
{
	int status = -1;
	/* reset device gpio */
	wszj->reset_gpio = of_get_named_gpio(wszj->spi->dev.of_node, "rst_gpio", 0);
	if (wszj->reset_gpio > 0) {
		status = gpio_request(wszj->reset_gpio, "wszj-rst");
		if (status < 0) {
			wszj->reset_gpio = 0;
			wszj_free_gpios(wszj);
			return status;
		}
		gpio_direction_output(wszj->reset_gpio, RESET_ON);
	}
	return status;
}

static int wszj_request_gpio_wakeup(struct wszj_data *wszj)
{
	int status = -1;
	/* wakeup device gpio */
	wszj->wakeup_gpio = of_get_named_gpio(wszj->spi->dev.of_node, "wke_gpio", 0);
	if (wszj->wakeup_gpio > 0) {
		status = gpio_request(wszj->wakeup_gpio, "wszj-wake");
		if (status < 0) {
			wszj->wakeup_gpio = 0;
			wszj_free_gpios(wszj);
			return status;
		}
		gpio_direction_output(wszj->wakeup_gpio, WAKEUP_ON);
	}
	return status;
}

static int wszj_request_gpio_cs(struct wszj_data *wszj)
{
	int status = -1;
	/* chip select gpio */
	wszj->cs_gpio = of_get_named_gpio(wszj->spi->dev.of_node, "cs_gpio", 0);
	if (wszj->cs_gpio > 0) {
		status = gpio_request(wszj->cs_gpio, "wszj-cs");
		if (status < 0) {
			wszj->cs_gpio = 0;
			wszj_free_gpios(wszj);
			return status;
		}
		gpio_direction_output(wszj->cs_gpio, CS_INACTIVE);
	}
	return status;
}

static int wszj_request_gpio_irp(struct wszj_data *wszj)
{
	int status = -1;
	/* device irq gpio */
	wszj->irq_gpio = of_get_named_gpio(wszj->spi->dev.of_node, "irq_gpio", 0);
	if (wszj->irq_gpio > 0) {
		status = gpio_request(wszj->irq_gpio, "wszj-irq");
		if (status < 0) {
			wszj->irq_gpio = 0;
			wszj_free_gpios(wszj);
			return status;
		}
		gpio_direction_input(wszj->irq_gpio);
	}
	return status;
}

static int wszj_request_gpios(struct wszj_data *wszj, struct spi_device *spi)
{
	int status = 0;

#ifdef GPIO_POWER
	status = wszj_request_gpio_power(wszj);
	if (status < 0) {
		hwlog_err("wszj_request_gpios: request gpio_power failed\n");
		return status;
	}
#endif
#ifdef GPIO_RESET
	status = wszj_request_gpio_reset(wszj);
	if (status < 0) {
		hwlog_err("wszj_request_gpios: request gpio_reset failed\n");
		return status;
	}
#endif
#ifdef GPIO_WAKEUP
	status = wszj_request_gpio_wakeup(wszj);
	if (status < 0) {
		hwlog_err("wszj_request_gpios: request gpio_wakeup failed\n");
		return status;
	}
#endif
#ifdef GPIO_CS
	status = wszj_request_gpio_cs(wszj);
	if (status < 0) {
		hwlog_err("wszj_request_gpios: request gpio_cs failed\n");
		return status;
	}
#endif
	status = wszj_request_gpio_irp(wszj);
	if (status < 0) {
		hwlog_err("wszj_request_gpios: request gpio_irp failed\n");
		return status;
	}
	return status;
}

static int wszj_request_irq(struct wszj_data *wszj, struct spi_device *spi)
{
	int status = 0;
	unsigned long flags;
	/* register wszj irq */
	wszj->irq = gpio_to_irq(wszj->irq_gpio);
	if (wszj->irq < 0) {
		hwlog_err("%s: gpio to irq failed %d<=>%d\n", __func__,
				wszj->irq_gpio, wszj->irq);
		status = wszj->irq;
	} else {
		spin_lock_irqsave(&wszj->irq_lock, flags);
		wszj->wait_irq = WAIT_IRQ_ON;
		spin_unlock_irqrestore(&wszj->irq_lock, flags);
		status = devm_request_irq(&spi->dev, wszj->irq,
				wszj_interrupt, IRQF_TRIGGER_RISING, DEV_NAME, wszj);
		if (status < 0) {
			hwlog_err("%s: devm_request_irq %d failed!\n", __func__,
					wszj->irq);
			wszj->irq = 0;
		}
	}
	return status;
}

static int wszj_free_gpios(struct wszj_data *wszj)
{
	if (g_2v85gpio_flag == 1) {
#ifdef GPIO_POWER
		if (wszj->power_gpio > 0) {
			gpio_direction_output(wszj->power_gpio, POWER_OFF);
			gpio_free(wszj->power_gpio);
			wszj->power_gpio = 0;
		}
#endif
	}
#ifdef GPIO_RESET
	if (wszj->reset_gpio > 0) {
		gpio_direction_output(wszj->reset_gpio, RESET_ON);
		gpio_free(wszj->reset_gpio);
		wszj->reset_gpio = 0;
	}
#endif
#ifdef GPIO_WAKEUP
	if (wszj->wakeup_gpio > 0) {
		gpio_direction_output(wszj->wakeup_gpio, WAKEUP_ON);
		gpio_free(wszj->wakeup_gpio);
		wszj->wakeup_gpio = 0;
	}
#endif
#ifdef GPIO_CS
	if (wszj->cs_gpio > 0) {
		gpio_direction_output(wszj->cs_gpio, CS_INACTIVE);
		gpio_free(wszj->cs_gpio);
		wszj->cs_gpio = 0;
	}
#endif
	if (wszj->irq_gpio > 0) {
		gpio_free(wszj->irq_gpio);
		wszj->irq_gpio = 0;
	}

	return 0;
}

static int wszj_wakeup(struct wszj_data *wszj)
{
#ifdef GPIO_WAKEUP
	if (wszj->wakeup_gpio > 0) {
		gpio_set_value(wszj->wakeup_gpio, WAKEUP_OFF);
		msleep(WAKEUP_DELAY);
		gpio_set_value(wszj->wakeup_gpio, WAKEUP_ON);
	}
#endif

	return 0;
}

static int
wszj_get_dts_config(const char *node_name, const char *prop_name, char *out_str, int out_str_len)
{
	struct device_node *np = NULL;
	const char *out_value;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_string(np, prop_name, (const char **)&out_value);
		if (ret != 0) {
			hwlog_err("%s: cann't get prop values with prop_name: %s\n", __func__, prop_name);
			continue;
		}
		if (out_value == NULL) {
			hwlog_err("%s: error out_value = NULL\n", __func__);
			ret = -1;
		} else if (strlen(out_value) >= out_str_len) {
			hwlog_err("%s: error out_value:%d >= out_str_len:%d\n", __func__,
					strlen(out_value), out_str_len);
			ret = -1;
		} else {
			hwlog_info("%s: =%s\n", __func__, out_str);
			if (strncpy_s(out_str, out_str_len, out_value, strlen(out_value)) != EOK)
				hwlog_err("wszj_get_dts_config strncpy fail\n");
		}
	}

	return ret;
}

static int wszj_get_dts_config_u32(const char *node_name, const char *prop_name, u32 *pvalue)
{
	struct device_node *np = NULL;
	int ret = -1;

	for_each_node_with_property(np, node_name) {
		ret = of_property_read_u32(np, prop_name, pvalue);
		if (ret != 0) {
			hwlog_err("%s: can not get prop values with prop_name: %s\n", __func__, prop_name);
		} else {
			hwlog_info("%s: %s=%d\n", __func__, prop_name, *pvalue);
		}
	}
	return ret;
}

static ssize_t
wszj_chip_type_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return snprintf_s(buf, sizeof(wszj_chip_type), sizeof(wszj_chip_type) - 1, "%s", wszj_chip_type);
}

static ssize_t
wszj_chip_type_store(struct device *dev, struct device_attribute *attr,
				const char *buf, size_t count)
{
	hwlog_info("%s: %s count=%lu\n", __func__, buf, count);
	if (strncpy_s(wszj_chip_type, CHIP_TYPE_BUFF_SIZE, buf, sizeof(wszj_chip_type) - 1) != EOK)
		hwlog_err("wszj_chip_type_store strncpy fail\n");
	return (ssize_t)count;
}

static struct device_attribute wszj_driver_attr[] = {
	__ATTR(uwb_chip_type, CHIP_TYPE_FILE_ATTR, wszj_chip_type_show, wszj_chip_type_store),
};

static int wszj_create_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(wszj_driver_attr); i++) {
		if (device_create_file(dev, &wszj_driver_attr[i])) {
			goto error;
		}
	}
	return 0;
error:
	for (; i >= 0; i--)
		device_remove_file(dev, &wszj_driver_attr[i]);
	hwlog_err("%s: unable to create sysfs interface\n", __func__);
	return -1;
}

static int wszj_remove_sysfs_interfaces(struct device *dev)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(wszj_driver_attr); i++)
		device_remove_file(dev, &wszj_driver_attr[i]);
	return 0;
}

static void wszj_set_chip_type_name(void)
{
	memset_s(wszj_chip_type, sizeof(wszj_chip_type), 0, sizeof(wszj_chip_type));
	if (wszj_get_dts_config("uwb_chip_type", "uwb_chip_type",
					wszj_chip_type, sizeof(wszj_chip_type)) < 0) {
		memset_s(wszj_chip_type, sizeof(wszj_chip_type), 0, sizeof(wszj_chip_type));
		strcpy_s(wszj_chip_type, CHIP_TYPE_BUFF_SIZE, DEV_NAME);
		hwlog_err("%s: can't get uwb_chip_type, default %s\n", __func__, DEV_NAME);
	}
	hwlog_info("%s: chip type name: %s\n", __func__, wszj_chip_type);
}

static void wszj_set_sim_id(void)
{
	int ret = -1;

	ret = wszj_get_dts_config_u32("levelshift_simid", "levelshift_simid", &g_pmusim_id);
	if (ret != 0) {
		g_pmusim_id = 0;
		hwlog_err("%s: can't levelshift_simid\n", __func__);
	}
	hwlog_info("%s: levelshift_simid:%d\n", __func__, g_pmusim_id);
	return;
}

static void wszj_set_pmu38m_div_flag(void)
{
	int ret = -1;

	ret = wszj_get_dts_config_u32("pmu_38mclk", "pmu_38mclk", &g_pmu38m_div_flag);
	if (ret != 0) {
		g_pmu38m_div_flag = 0;
		hwlog_err("%s: can't pmu38m_div_flag\n", __func__);
	}
	hwlog_info("%s: pmu38m_div_flag:%d\n", __func__, g_pmu38m_div_flag);
	return;
}

static void wszj_set_2v85gpio_flag(void)
{
	int ret = -1;

	ret = wszj_get_dts_config_u32("2v85gpio_enbale", "2v85gpio_enbale", &g_2v85gpio_flag);
	if (ret != 0) {
		g_2v85gpio_flag = 0;
		hwlog_err("%s: can't 2v85gpio_enbale\n", __func__);
	}
	hwlog_info("%s: 2v85gpio_enbale:%d\n", __func__, g_2v85gpio_flag);
	return;
}

static void wszj_set_pmu_32k_clk_name(void)
{
	memset_s(wszj_32kclk_name, sizeof(wszj_32kclk_name), 0, sizeof(wszj_32kclk_name));
	if (wszj_get_dts_config("clock-32k", "clock-32k",
					wszj_32kclk_name, sizeof(wszj_32kclk_name)) < 0) {
		memset_s(wszj_32kclk_name, sizeof(wszj_32kclk_name), 0, sizeof(wszj_32kclk_name));
		strcpy_s(wszj_32kclk_name, CHIP_TYPE_BUFF_SIZE, DEV_NAME);
		hwlog_err("%s: can't get 32k_clk, default %s\n", __func__, DEV_NAME);
	}
	hwlog_info("%s: default 32k_clk: %s\n", __func__, wszj_32kclk_name);
}

static int wszj_create_device_attr(struct wszj_data *wszj)
{
	int status = UWB_FAIL;
	wszj_set_chip_type_name();
	wszj_set_sim_id();
	wszj_set_pmu38m_div_flag();
	wszj_set_2v85gpio_flag();
	wszj_set_pmu_32k_clk_name();

	status = wszj_create_sysfs_interfaces(wszj->device);
	if (status < 0) {
		hwlog_err("%s: failed to wszj_create_sysfs_interfaces\n", __func__);
		return UWB_FAIL;
	}
	if (sysfs_create_link(NULL, &wszj->device->kobj, "uwb") < 0) {
		wszj_remove_sysfs_interfaces(wszj->device);
		hwlog_err("%s: failed to sysfs_create_link\n", __func__);
		return UWB_FAIL;
	}
	return UWB_SUCC;
}

static void wszj_data_init(struct wszj_data *wszj, struct spi_device *spi)
{
	wszj->spi = spi;
	wszj->speed_hz = spi->max_speed_hz;
	spin_lock_init(&wszj->spi_lock);
	spin_lock_init(&wszj->irq_lock);
	init_waitqueue_head(&wszj->wait);
	mutex_init(&wszj->buf_lock);
	mutex_init(&wszj->wait_lock);
	mutex_init(&wszj->wake_lock);

	INIT_LIST_HEAD(&wszj->device_entry);
}

static int wszj_register_device(struct wszj_data *wszj)
{
	int status;

	/* If we can allocate a minor number, hook up this device.
	 * Reusing minors is fine so long as udev or mdev is working.
	 */
	mutex_lock(&device_list_lock);
	if (wszj_major) {
		wszj->devt = MKDEV(wszj_major, 0);
		status = register_chrdev_region(wszj->devt, 1, DEV_NAME);
	} else {
		status = alloc_chrdev_region(&wszj->devt, 0, 1, DEV_NAME);
		wszj_major = MAJOR(wszj->devt);
	}
	if (status == 0) {
		cdev_init(&wszj->cdev, &wszj_fops);
		status = cdev_add(&wszj->cdev, wszj->devt, 1);
		if (status == 0) {
			wszj->device = device_create(wszj_class, NULL, wszj->devt, NULL, DEV_NAME);
			if (IS_ERR(wszj->device)) {
				cdev_del(&wszj->cdev);
				unregister_chrdev_region(wszj->devt, 1);
				status = PTR_ERR(wszj->device);
				hwlog_err("%s: create device failed\n", __func__);
			}
		} else {
			unregister_chrdev_region(wszj->devt, 1);
			hwlog_err("%s: add cdev failed\n", __func__);
		}
	} else {
		hwlog_err("%s: register chrdev region failed\n", __func__);
	}
	if (status == 0)
		list_add(&wszj->device_entry, &device_list);
	mutex_unlock(&device_list_lock);

	return status;
}

static int wszj_free(struct wszj_data *wszj)
{
	wszj_free_gpios(wszj);
	/* prevent new opens */
	mutex_lock(&device_list_lock);
	/* make sure ops on existing fds can abort cleanly */
	spin_lock_irq(&wszj->spi_lock);
	wszj->spi = NULL;
	spin_unlock_irq(&wszj->spi_lock);

	list_del(&wszj->device_entry);
	device_destroy(wszj_class, wszj->devt);
	cdev_del(&wszj->cdev);
	if (wszj->users == 0)
		kfree(wszj);
	mutex_unlock(&device_list_lock);

	return 0;
}

static int uwb_power_manage(struct wszj_data *spidev, uwb_power_supply uwbldo, char sw)
{
	int ret = 0;
	uwbldo.gpio_enable2v85 = g_2v85gpio_flag;
	uwbldo.freq2div38m = g_pmu38m_div_flag;
	switch (sw) {
	case UWB_SWITCH_OPEN:
		ret = uwb_open_init(spidev, uwbldo);
		break;
	case UWB_SWITCH_CLOSE:
		ret = uwb_close_deinit(spidev, uwbldo);
		break;
	default:
		ret = -1;
		hwlog_info("%s: invalid input: uwb switch[%d]\n", __func__, sw);
	}
	return ret;
}

static int uwb_perip_component_init(struct wszj_data *spidev, struct spi_device *spi)
{
	int status = UWB_FAIL;

	if (spidev == NULL || spi == NULL) {
		hwlog_err("fr8000: UwbPowerInit input Invalid!\n");
		return UWB_FAIL;
	}
	g_uwb_pmu.ldo_reg53 = devm_regulator_get(&spi->dev, PMU_LDO_53_STR);
	if (IS_ERR(g_uwb_pmu.ldo_reg53)) {
		hwlog_err("fr8000: %s: devm_regulator_get ldoReg53 failed !\n", __func__);
		return UWB_FAIL;
	}
	g_uwb_pmu.ldo_reg54 = devm_regulator_get(&spi->dev, PMU_LDO_54_STR);
	if (IS_ERR(g_uwb_pmu.ldo_reg54)) {
		hwlog_err("fr8000: %s: devm_regulator_get ldoReg54 failed !\n", __func__);
		return UWB_FAIL;
	}
	g_uwb_pmu.ldo_reg25 = devm_regulator_get(&spi->dev, PMU_LDO_25_STR);
	if (IS_ERR(g_uwb_pmu.ldo_reg25)) {
		hwlog_err("fr8000: %s: devm_regulator_get ldoReg25 failed !\n", __func__);
		return UWB_FAIL;
	}
	g_uwb_pmu.ldo_reg3 = devm_regulator_get(&spi->dev, PMU_LDO_3_STR);
	if (IS_ERR(g_uwb_pmu.ldo_reg3)) {
		hwlog_err("fr8000: %s: devm_regulator_get ldoReg3 failed !\n", __func__);
		return UWB_FAIL;
	}
	g_uwb_pmu.clk32kb = devm_clk_get(&spi->dev, wszj_32kclk_name);
	if (IS_ERR(g_uwb_pmu.clk32kb)) {
		hwlog_err("fr8000:%s:devm_clk_get clk32kb fail",  __func__);
		return UWB_FAIL;
	}
	status = uwb_sim_levelshift_init(g_pmusim_id);
	if (status != UWB_SUCC) {
		hwlog_err("fr8000: uwb_sim_levelshift_init failed!\n");
		return UWB_FAIL;
	}
	/* test func end */
	return UWB_SUCC;
}

static int wszj_power(struct wszj_data *wszj, int on)
{
	if ((on != POWER_OFF) && (on != POWER_ON)) {
		hwlog_err("%s: invalid op\n", __func__);
		return UWB_FAIL;
	}
	if (on == POWER_OFF && wszj->power == POWER_OFF) {
		hwlog_info("%s: already poweroff\n", __func__);
		return UWB_SUCC;
	}
	if (on == POWER_ON && wszj->power == POWER_ON) {
		hwlog_info("%s: already poweron\n", __func__);
		return UWB_SUCC;
	}
	if (on == POWER_OFF) {
		uwb_power_manage(wszj, g_uwb_pmu, UWB_SWITCH_CLOSE);
		wszj->power = POWER_OFF;
	}
	if (on == POWER_ON) {
		uwb_power_manage(wszj, g_uwb_pmu, UWB_SWITCH_OPEN);
		wszj->power = POWER_ON;
	}
	return UWB_SUCC;
}

static int wszj_reset(struct wszj_data *wszj)
{
	uwb_hardware_reset(wszj);
	return UWB_SUCC;
}

static int wszj_probe(struct spi_device *spi)
{
	struct wszj_data *wszj;
	int status;

	/*
	 * wszj should never be referenced in DT without a specific
	 * compatible string, it is a Linux implementation thing
	 * rather than a description of the hardware.
	 */
	WARN(spi->dev.of_node && of_device_is_compatible(spi->dev.of_node, DEV_NAME),
		 "%pOF: buggy DT: wszj listed directly in DT\n", spi->dev.of_node);

	/* Allocate driver data */
	wszj = kzalloc(sizeof(struct wszj_data), GFP_KERNEL);
	if (!wszj)
		return -ENOMEM;
	/* Initialize the driver data */
	wszj_data_init(wszj, spi);
	status = wszj_register_device(wszj);
	if (status < 0) {
		hwlog_err("%s: cann't wszj_register_device \n", __func__);
		wszj_free(wszj);
		return status;
	}
	status = wszj_create_device_attr(wszj);
	if (status < 0) {
		hwlog_err("%s: cann't wszj_create_device_attr \n", __func__);
		wszj_free(wszj);
		return status;
	}
	status = wszj_request_gpios(wszj, spi);
	if (status < 0) {
		hwlog_err("%s: cann't request gpios\n", __func__);
		wszj_free(wszj);
		return status;
	}
	status = wszj_request_irq(wszj, spi);
	if (status < 0) {
		hwlog_err("%s: cann't wszj_request_irq\n", __func__);
		wszj_free(wszj);
		return status;
	}
	status = uwb_perip_component_init(wszj, spi);
	if (status < 0) {
		hwlog_err("%s: cann't ComponentInit\n", __func__);
		wszj_free(wszj);
		return status;
	}
	if (status == 0) {
		spi_set_drvdata(spi, wszj);
	} else {
		wszj_free(wszj);
	}
	return status;
}

static int wszj_remove(struct spi_device *spi)
{
	struct wszj_data	*wszj = spi_get_drvdata(spi);

	wszj_free(wszj);

	return 0;
}

static const struct spi_device_id wszj_spi_ids[] = {
	{ .name = "wszj" },
	{},
};
MODULE_DEVICE_TABLE(spi, wszj_spi_ids);

static const struct of_device_id wszj_of_ids[] = {
	{ .compatible = "huawei,wszj" },
	{},
};
MODULE_DEVICE_TABLE(of, wszj_of_ids);

static struct spi_driver wszj_spi_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = DEV_NAME,
		.of_match_table = of_match_ptr(wszj_of_ids),
	},
	.probe =	wszj_probe,
	.remove =	wszj_remove,
	.id_table =	wszj_spi_ids,

	/* NOTE:  suspend/resume methods are not necessary here.
	 * We don't do anything except pass the requests to/from
	 * the underlying controller.  The refrigerator handles
	 * most issues; the controller driver handles the rest.
	 */
};

static int __init wszj_init(void)
{
	int status;

	wszj_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(wszj_class)) {
		return PTR_ERR(wszj_class);
	}

	status = spi_register_driver(&wszj_spi_driver);
	if (status < 0) {
		class_destroy(wszj_class);
	}
	return status;
}
module_init(wszj_init);

static void __exit wszj_exit(void)
{
	spi_unregister_driver(&wszj_spi_driver);
	class_destroy(wszj_class);
}
module_exit(wszj_exit);

MODULE_AUTHOR("wszj, <wszj@huawei.com>");
MODULE_DESCRIPTION("Huawei WSZJ UWB SPI driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("spi:wszj");
