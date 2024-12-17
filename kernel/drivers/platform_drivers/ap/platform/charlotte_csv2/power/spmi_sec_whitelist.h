#ifndef __SPMI_SEC_WHITELIST_H__
#define __SPMI_SEC_WHITELIST_H__ 
#define SPMI_SLAVEID_EOF 16
struct spmi_sec_reg {
    unsigned char sid;
    unsigned short addr;
};
static struct spmi_sec_reg* spmi_sec_whitelist[SPMI_SLAVEID_EOF] = {
};
#endif
