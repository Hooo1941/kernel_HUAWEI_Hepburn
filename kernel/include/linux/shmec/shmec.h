#ifndef SHMEC_H
#define SHMEC_H

#include <linux/types.h>

struct shmec_ep;

enum {
	EC_VM_CMD_CREATE	= 0xEC000000U,
	EC_VM_CMD_DESTROY	= 0xEC000001U,
	EC_VM_CMD_CONNECT	= 0xEC000002U,
	EC_VM_CMD_DISCONNECT	= 0xEC000003U,
	EC_VM_CMD_SEND_EVENT	= 0xEC000004U,
};
/*
 * event channel endpoint's type is defined as follow:
 * bit  0     : has communication source or not
 * bit  1     : has communication destination or not
 * bit  2 - 15: reserved
 * bit 16 -   : communication type
 */
#define EC_EP_SHMEC		(0x0ULL)
#define EC_EP_SRC		(0x1ULL)
#define EC_EP_DST		(0x2ULL)
#define EC_CHN_O_CREAT		(0100)

/*
 * return value for event channel hvc call
 */
#define __HYP_EC_SUCCESS	(0)
#define __HYP_EC_ERR_INVAL	(-1)
#define __HYP_EC_ERR_AGAIN	(-2)
#define __HYP_EC_ERR_DENIED	(-3)
#define __HYP_EC_ERR_OVERFLOW	(-4)
#define __HYP_EC_ERR_FAULT	(-5)
#define __HYP_EC_ERR_INTERNAL	(-6)

/*
* connect: endpoint id, channel name, channel name len
* connect endpoint to channel which is distinguished by channel name
* return: success or not
*/
int shmec_attach(const struct shmec_ep *endpoint, const char *chn, size_t len, int flag, mode_t mode);
/*
* disconnect: endpoint id
* return: success or not
*/
int shmec_detach(const struct shmec_ep *endpoint);
/*
* sent: endpoint id, buff addr, buffsize
* buffsize: more than 8 Bytes
* return: success or not
*/
int shmec_send_event(const struct shmec_ep *endpoint, const void *buf, size_t size);
/*
* recv: endpoint id, buff addr, buffsize
* return: recv buff length or not
*/
int shmec_recv_event(struct shmec_ep *endpoint, void *buf, size_t size);
/*
* create: src/dst type, irq
* return: endpoint id or NULL
*/
struct shmec_ep *shmec_create_endpoint(uint64_t flags, int irq);
/*
* delete: endpoint id
* return: success or not
*/
int shmec_delete_endpoint(struct shmec_ep *endpoint);

#endif
