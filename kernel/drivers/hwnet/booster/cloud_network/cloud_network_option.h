#ifndef _CLOUD_NETWORK_OPTION_H_
#define _CLOUD_NETWORK_OPTION_H_
#include <linux/skbuff.h>
#include <linux/types.h>
#include <linux/in6.h>
#include <net/udp.h>

#include "cloud_network_honhdr.h"

#define TIMESTAMP_OPTION_LEN 6

struct option_timestamp_struct {
	__u32 timestamp;
};

struct opt_skb {
	u32 type;
	u32 msg_len;
	struct list_head head;
	struct sk_buff *skb;
	u32 timestamp;
};

#pragma pack(1)
struct session_sync {
	__u8 ip_type;
	__u32 sip;
	__u32 dip;
	__u16 sport;
	__u16 dport;
	__u8 proto_type;
};
#pragma pack()
#pragma pack(1)
struct much_session_sync {
	__u8 cnt;
	struct session_sync ss_sync;
};
#pragma pack()
#pragma pack(1)
struct session_sync_v6 {
	__u8 ip_type;
	struct	in6_addr sip;
	struct	in6_addr dip;
	__u16 sport;
	__u16 dport;
	__u8 proto_type;
};
#pragma pack()

struct fid_control {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8	operation : 4,
			fid : 4;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u8	fid : 4,
			operation : 4;
#endif
};

struct cloud_option_tlv {
	__u8 type;
	__u8 length;
	__u8 data[0];
};

void send_reset_option(void);
void send_option_packet(__u8 type, __u8 len, char *data);
void recv_option_ack(u64 timestamp);
void recv_exception_information(char *data);
void send_ack_respond(__u64 timestamp);

struct sk_buff *alloc_option_skb(int option_size);
void option_append(struct sk_buff *skb, __u8 type, __u8 len, char *data);
u32 option_timestamp_append(struct sk_buff *skb, u32 timestamp);
void send_session_synchronize_v4(__u8 ip_type, __u32 sip, __u32 dip, __u16 sport, __u16 dport, __u8 proto_type);
void send_fid_control(__u8 path, __u8 operation, char *data, int length);
void option_handle_dispatch(struct honhdr *honh, u8 *option, u16 tot_len);
__u16 generate_option(char *option, __u16 option_len, __u8 type, __u8 len, char *data);
__u16 generate_timestamp_option(char *option, __u16 option_len);
void cloud_network_option_stop(void);
int cloud_network_option_init(void);
void cloud_network_option_exit(void);
#endif /* _CLOUD_NETWORK_OPTION_H_ */
