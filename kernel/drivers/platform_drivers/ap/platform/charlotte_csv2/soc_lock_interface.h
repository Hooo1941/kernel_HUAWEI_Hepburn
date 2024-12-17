#ifndef __SOC_LOCK_INTERFACE_H__
#define __SOC_LOCK_INTERFACE_H__ 
#ifdef __cplusplus
    #if __cplusplus
        extern "C" {
    #endif
#endif
#ifndef __SOC_H_FOR_ASM__
#define SOC_LOCK_RESOURCE0_LOCK_ADDR(base) ((base) + (0x000UL))
#define SOC_LOCK_RESOURCE0_UNLOCK_ADDR(base) ((base) + (0x004UL))
#define SOC_LOCK_RESOURCE0_LOCK_ST_ADDR(base) ((base) + (0x008UL))
#define SOC_LOCK_RESOURCE1_LOCK_ADDR(base) ((base) + (0x00CUL))
#define SOC_LOCK_RESOURCE1_UNLOCK_ADDR(base) ((base) + (0x010UL))
#define SOC_LOCK_RESOURCE1_LOCK_ST_ADDR(base) ((base) + (0x014UL))
#define SOC_LOCK_RESOURCE2_LOCK_ADDR(base) ((base) + (0x018UL))
#define SOC_LOCK_RESOURCE2_UNLOCK_ADDR(base) ((base) + (0x01CUL))
#define SOC_LOCK_RESOURCE2_LOCK_ST_ADDR(base) ((base) + (0x020UL))
#define SOC_LOCK_RESOURCE3_LOCK_ADDR(base) ((base) + (0x024UL))
#define SOC_LOCK_RESOURCE3_UNLOCK_ADDR(base) ((base) + (0x028UL))
#define SOC_LOCK_RESOURCE3_LOCK_ST_ADDR(base) ((base) + (0x02CUL))
#define SOC_LOCK_RESOURCE4_LOCK_ADDR(base) ((base) + (0x400UL))
#define SOC_LOCK_RESOURCE4_UNLOCK_ADDR(base) ((base) + (0x404UL))
#define SOC_LOCK_RESOURCE4_LOCK_ST_ADDR(base) ((base) + (0x408UL))
#define SOC_LOCK_RESOURCE5_LOCK_ADDR(base) ((base) + (0x40CUL))
#define SOC_LOCK_RESOURCE5_UNLOCK_ADDR(base) ((base) + (0x410UL))
#define SOC_LOCK_RESOURCE5_LOCK_ST_ADDR(base) ((base) + (0x414UL))
#define SOC_LOCK_RESOURCE6_LOCK_ADDR(base) ((base) + (0x418UL))
#define SOC_LOCK_RESOURCE6_UNLOCK_ADDR(base) ((base) + (0x41CUL))
#define SOC_LOCK_RESOURCE6_LOCK_ST_ADDR(base) ((base) + (0x420UL))
#define SOC_LOCK_RESOURCE7_LOCK_ADDR(base) ((base) + (0x424UL))
#define SOC_LOCK_RESOURCE7_UNLOCK_ADDR(base) ((base) + (0x428UL))
#define SOC_LOCK_RESOURCE7_LOCK_ST_ADDR(base) ((base) + (0x42CUL))
#else
#define SOC_LOCK_RESOURCE0_LOCK_ADDR(base) ((base) + (0x000))
#define SOC_LOCK_RESOURCE0_UNLOCK_ADDR(base) ((base) + (0x004))
#define SOC_LOCK_RESOURCE0_LOCK_ST_ADDR(base) ((base) + (0x008))
#define SOC_LOCK_RESOURCE1_LOCK_ADDR(base) ((base) + (0x00C))
#define SOC_LOCK_RESOURCE1_UNLOCK_ADDR(base) ((base) + (0x010))
#define SOC_LOCK_RESOURCE1_LOCK_ST_ADDR(base) ((base) + (0x014))
#define SOC_LOCK_RESOURCE2_LOCK_ADDR(base) ((base) + (0x018))
#define SOC_LOCK_RESOURCE2_UNLOCK_ADDR(base) ((base) + (0x01C))
#define SOC_LOCK_RESOURCE2_LOCK_ST_ADDR(base) ((base) + (0x020))
#define SOC_LOCK_RESOURCE3_LOCK_ADDR(base) ((base) + (0x024))
#define SOC_LOCK_RESOURCE3_UNLOCK_ADDR(base) ((base) + (0x028))
#define SOC_LOCK_RESOURCE3_LOCK_ST_ADDR(base) ((base) + (0x02C))
#define SOC_LOCK_RESOURCE4_LOCK_ADDR(base) ((base) + (0x400))
#define SOC_LOCK_RESOURCE4_UNLOCK_ADDR(base) ((base) + (0x404))
#define SOC_LOCK_RESOURCE4_LOCK_ST_ADDR(base) ((base) + (0x408))
#define SOC_LOCK_RESOURCE5_LOCK_ADDR(base) ((base) + (0x40C))
#define SOC_LOCK_RESOURCE5_UNLOCK_ADDR(base) ((base) + (0x410))
#define SOC_LOCK_RESOURCE5_LOCK_ST_ADDR(base) ((base) + (0x414))
#define SOC_LOCK_RESOURCE6_LOCK_ADDR(base) ((base) + (0x418))
#define SOC_LOCK_RESOURCE6_UNLOCK_ADDR(base) ((base) + (0x41C))
#define SOC_LOCK_RESOURCE6_LOCK_ST_ADDR(base) ((base) + (0x420))
#define SOC_LOCK_RESOURCE7_LOCK_ADDR(base) ((base) + (0x424))
#define SOC_LOCK_RESOURCE7_UNLOCK_ADDR(base) ((base) + (0x428))
#define SOC_LOCK_RESOURCE7_LOCK_ST_ADDR(base) ((base) + (0x42C))
#endif
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_cmd0 : 1;
        unsigned int resource0lock_id0 : 3;
        unsigned int resource0lock_cmd1 : 1;
        unsigned int resource0lock_id1 : 3;
        unsigned int resource0lock_cmd2 : 1;
        unsigned int resource0lock_id2 : 3;
        unsigned int resource0lock_cmd3 : 1;
        unsigned int resource0lock_id3 : 3;
        unsigned int resource0lock_cmd4 : 1;
        unsigned int resource0lock_id4 : 3;
        unsigned int resource0lock_cmd5 : 1;
        unsigned int resource0lock_id5 : 3;
        unsigned int resource0lock_cmd6 : 1;
        unsigned int resource0lock_id6 : 3;
        unsigned int resource0lock_cmd7 : 1;
        unsigned int resource0lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE0_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id0_START (1)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id0_END (3)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id1_START (5)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id1_END (7)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id2_START (9)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id2_END (11)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id3_START (13)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id3_END (15)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id4_START (17)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id4_END (19)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id5_START (21)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id5_END (23)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id6_START (25)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id6_END (27)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id7_START (29)
#define SOC_LOCK_RESOURCE0_LOCK_resource0lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0unlock_cmd0 : 1;
        unsigned int resource0unlock_id0 : 3;
        unsigned int resource0unlock_cmd1 : 1;
        unsigned int resource0unlock_id1 : 3;
        unsigned int resource0unlock_cmd2 : 1;
        unsigned int resource0unlock_id2 : 3;
        unsigned int resource0unlock_cmd3 : 1;
        unsigned int resource0unlock_id3 : 3;
        unsigned int resource0unlock_cmd4 : 1;
        unsigned int resource0unlock_id4 : 3;
        unsigned int resource0unlock_cmd5 : 1;
        unsigned int resource0unlock_id5 : 3;
        unsigned int resource0unlock_cmd6 : 1;
        unsigned int resource0unlock_id6 : 3;
        unsigned int resource0unlock_cmd7 : 1;
        unsigned int resource0unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE0_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id0_START (1)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id0_END (3)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id1_START (5)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id1_END (7)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id2_START (9)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id2_END (11)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id3_START (13)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id3_END (15)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id4_START (17)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id4_END (19)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id5_START (21)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id5_END (23)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id6_START (25)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id6_END (27)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id7_START (29)
#define SOC_LOCK_RESOURCE0_UNLOCK_resource0unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource0lock_st0 : 1;
        unsigned int resource0lock_st_id0 : 3;
        unsigned int resource0lock_st1 : 1;
        unsigned int resource0lock_st_id1 : 3;
        unsigned int resource0lock_st2 : 1;
        unsigned int resource0lock_st_id2 : 3;
        unsigned int resource0lock_st3 : 1;
        unsigned int resource0lock_st_id3 : 3;
        unsigned int resource0lock_st4 : 1;
        unsigned int resource0lock_st_id4 : 3;
        unsigned int resource0lock_st5 : 1;
        unsigned int resource0lock_st_id5 : 3;
        unsigned int resource0lock_st6 : 1;
        unsigned int resource0lock_st_id6 : 3;
        unsigned int resource0lock_st7 : 1;
        unsigned int resource0lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE0_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st0_START (0)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st0_END (0)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st1_START (4)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st1_END (4)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st2_START (8)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st2_END (8)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st3_START (12)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st3_END (12)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st4_START (16)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st4_END (16)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st5_START (20)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st5_END (20)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st6_START (24)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st6_END (24)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st7_START (28)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st7_END (28)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE0_LOCK_ST_resource0lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource1lock_cmd0 : 1;
        unsigned int resource1lock_id0 : 3;
        unsigned int resource1lock_cmd1 : 1;
        unsigned int resource1lock_id1 : 3;
        unsigned int resource1lock_cmd2 : 1;
        unsigned int resource1lock_id2 : 3;
        unsigned int resource1lock_cmd3 : 1;
        unsigned int resource1lock_id3 : 3;
        unsigned int resource1lock_cmd4 : 1;
        unsigned int resource1lock_id4 : 3;
        unsigned int resource1lock_cmd5 : 1;
        unsigned int resource1lock_id5 : 3;
        unsigned int resource1lock_cmd6 : 1;
        unsigned int resource1lock_id6 : 3;
        unsigned int resource1lock_cmd7 : 1;
        unsigned int resource1lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE1_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id0_START (1)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id0_END (3)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id1_START (5)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id1_END (7)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id2_START (9)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id2_END (11)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id3_START (13)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id3_END (15)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id4_START (17)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id4_END (19)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id5_START (21)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id5_END (23)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id6_START (25)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id6_END (27)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id7_START (29)
#define SOC_LOCK_RESOURCE1_LOCK_resource1lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource1unlock_cmd0 : 1;
        unsigned int resource1unlock_id0 : 3;
        unsigned int resource1unlock_cmd1 : 1;
        unsigned int resource1unlock_id1 : 3;
        unsigned int resource1unlock_cmd2 : 1;
        unsigned int resource1unlock_id2 : 3;
        unsigned int resource1unlock_cmd3 : 1;
        unsigned int resource1unlock_id3 : 3;
        unsigned int resource1unlock_cmd4 : 1;
        unsigned int resource1unlock_id4 : 3;
        unsigned int resource1unlock_cmd5 : 1;
        unsigned int resource1unlock_id5 : 3;
        unsigned int resource1unlock_cmd6 : 1;
        unsigned int resource1unlock_id6 : 3;
        unsigned int resource1unlock_cmd7 : 1;
        unsigned int resource1unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE1_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id0_START (1)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id0_END (3)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id1_START (5)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id1_END (7)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id2_START (9)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id2_END (11)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id3_START (13)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id3_END (15)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id4_START (17)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id4_END (19)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id5_START (21)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id5_END (23)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id6_START (25)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id6_END (27)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id7_START (29)
#define SOC_LOCK_RESOURCE1_UNLOCK_resource1unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource1lock_st0 : 1;
        unsigned int resource1lock_st_id0 : 3;
        unsigned int resource1lock_st1 : 1;
        unsigned int resource1lock_st_id1 : 3;
        unsigned int resource1lock_st2 : 1;
        unsigned int resource1lock_st_id2 : 3;
        unsigned int resource1lock_st3 : 1;
        unsigned int resource1lock_st_id3 : 3;
        unsigned int resource1lock_st4 : 1;
        unsigned int resource1lock_st_id4 : 3;
        unsigned int resource1lock_st5 : 1;
        unsigned int resource1lock_st_id5 : 3;
        unsigned int resource1lock_st6 : 1;
        unsigned int resource1lock_st_id6 : 3;
        unsigned int resource1lock_st7 : 1;
        unsigned int resource1lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE1_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st0_START (0)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st0_END (0)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st1_START (4)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st1_END (4)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st2_START (8)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st2_END (8)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st3_START (12)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st3_END (12)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st4_START (16)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st4_END (16)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st5_START (20)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st5_END (20)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st6_START (24)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st6_END (24)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st7_START (28)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st7_END (28)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE1_LOCK_ST_resource1lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource2lock_cmd0 : 1;
        unsigned int resource2lock_id0 : 3;
        unsigned int resource2lock_cmd1 : 1;
        unsigned int resource2lock_id1 : 3;
        unsigned int resource2lock_cmd2 : 1;
        unsigned int resource2lock_id2 : 3;
        unsigned int resource2lock_cmd3 : 1;
        unsigned int resource2lock_id3 : 3;
        unsigned int resource2lock_cmd4 : 1;
        unsigned int resource2lock_id4 : 3;
        unsigned int resource2lock_cmd5 : 1;
        unsigned int resource2lock_id5 : 3;
        unsigned int resource2lock_cmd6 : 1;
        unsigned int resource2lock_id6 : 3;
        unsigned int resource2lock_cmd7 : 1;
        unsigned int resource2lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE2_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id0_START (1)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id0_END (3)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id1_START (5)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id1_END (7)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id2_START (9)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id2_END (11)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id3_START (13)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id3_END (15)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id4_START (17)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id4_END (19)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id5_START (21)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id5_END (23)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id6_START (25)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id6_END (27)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id7_START (29)
#define SOC_LOCK_RESOURCE2_LOCK_resource2lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource2unlock_cmd0 : 1;
        unsigned int resource2unlock_id0 : 3;
        unsigned int resource2unlock_cmd1 : 1;
        unsigned int resource2unlock_id1 : 3;
        unsigned int resource2unlock_cmd2 : 1;
        unsigned int resource2unlock_id2 : 3;
        unsigned int resource2unlock_cmd3 : 1;
        unsigned int resource2unlock_id3 : 3;
        unsigned int resource2unlock_cmd4 : 1;
        unsigned int resource2unlock_id4 : 3;
        unsigned int resource2unlock_cmd5 : 1;
        unsigned int resource2unlock_id5 : 3;
        unsigned int resource2unlock_cmd6 : 1;
        unsigned int resource2unlock_id6 : 3;
        unsigned int resource2unlock_cmd7 : 1;
        unsigned int resource2unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE2_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id0_START (1)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id0_END (3)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id1_START (5)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id1_END (7)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id2_START (9)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id2_END (11)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id3_START (13)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id3_END (15)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id4_START (17)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id4_END (19)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id5_START (21)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id5_END (23)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id6_START (25)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id6_END (27)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id7_START (29)
#define SOC_LOCK_RESOURCE2_UNLOCK_resource2unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource2lock_st0 : 1;
        unsigned int resource2lock_st_id0 : 3;
        unsigned int resource2lock_st1 : 1;
        unsigned int resource2lock_st_id1 : 3;
        unsigned int resource2lock_st2 : 1;
        unsigned int resource2lock_st_id2 : 3;
        unsigned int resource2lock_st3 : 1;
        unsigned int resource2lock_st_id3 : 3;
        unsigned int resource2lock_st4 : 1;
        unsigned int resource2lock_st_id4 : 3;
        unsigned int resource2lock_st5 : 1;
        unsigned int resource2lock_st_id5 : 3;
        unsigned int resource2lock_st6 : 1;
        unsigned int resource2lock_st_id6 : 3;
        unsigned int resource2lock_st7 : 1;
        unsigned int resource2lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE2_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st0_START (0)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st0_END (0)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st1_START (4)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st1_END (4)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st2_START (8)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st2_END (8)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st3_START (12)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st3_END (12)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st4_START (16)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st4_END (16)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st5_START (20)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st5_END (20)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st6_START (24)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st6_END (24)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st7_START (28)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st7_END (28)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE2_LOCK_ST_resource2lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource3lock_cmd0 : 1;
        unsigned int resource3lock_id0 : 3;
        unsigned int resource3lock_cmd1 : 1;
        unsigned int resource3lock_id1 : 3;
        unsigned int resource3lock_cmd2 : 1;
        unsigned int resource3lock_id2 : 3;
        unsigned int resource3lock_cmd3 : 1;
        unsigned int resource3lock_id3 : 3;
        unsigned int resource3lock_cmd4 : 1;
        unsigned int resource3lock_id4 : 3;
        unsigned int resource3lock_cmd5 : 1;
        unsigned int resource3lock_id5 : 3;
        unsigned int resource3lock_cmd6 : 1;
        unsigned int resource3lock_id6 : 3;
        unsigned int resource3lock_cmd7 : 1;
        unsigned int resource3lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE3_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id0_START (1)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id0_END (3)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id1_START (5)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id1_END (7)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id2_START (9)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id2_END (11)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id3_START (13)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id3_END (15)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id4_START (17)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id4_END (19)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id5_START (21)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id5_END (23)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id6_START (25)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id6_END (27)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id7_START (29)
#define SOC_LOCK_RESOURCE3_LOCK_resource3lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource3unlock_cmd0 : 1;
        unsigned int resource3unlock_id0 : 3;
        unsigned int resource3unlock_cmd1 : 1;
        unsigned int resource3unlock_id1 : 3;
        unsigned int resource3unlock_cmd2 : 1;
        unsigned int resource3unlock_id2 : 3;
        unsigned int resource3unlock_cmd3 : 1;
        unsigned int resource3unlock_id3 : 3;
        unsigned int resource3unlock_cmd4 : 1;
        unsigned int resource3unlock_id4 : 3;
        unsigned int resource3unlock_cmd5 : 1;
        unsigned int resource3unlock_id5 : 3;
        unsigned int resource3unlock_cmd6 : 1;
        unsigned int resource3unlock_id6 : 3;
        unsigned int resource3unlock_cmd7 : 1;
        unsigned int resource3unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE3_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id0_START (1)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id0_END (3)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id1_START (5)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id1_END (7)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id2_START (9)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id2_END (11)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id3_START (13)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id3_END (15)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id4_START (17)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id4_END (19)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id5_START (21)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id5_END (23)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id6_START (25)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id6_END (27)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id7_START (29)
#define SOC_LOCK_RESOURCE3_UNLOCK_resource3unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource3lock_st0 : 1;
        unsigned int resource3lock_st_id0 : 3;
        unsigned int resource3lock_st1 : 1;
        unsigned int resource3lock_st_id1 : 3;
        unsigned int resource3lock_st2 : 1;
        unsigned int resource3lock_st_id2 : 3;
        unsigned int resource3lock_st3 : 1;
        unsigned int resource3lock_st_id3 : 3;
        unsigned int resource3lock_st4 : 1;
        unsigned int resource3lock_st_id4 : 3;
        unsigned int resource3lock_st5 : 1;
        unsigned int resource3lock_st_id5 : 3;
        unsigned int resource3lock_st6 : 1;
        unsigned int resource3lock_st_id6 : 3;
        unsigned int resource3lock_st7 : 1;
        unsigned int resource3lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE3_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st0_START (0)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st0_END (0)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st1_START (4)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st1_END (4)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st2_START (8)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st2_END (8)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st3_START (12)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st3_END (12)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st4_START (16)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st4_END (16)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st5_START (20)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st5_END (20)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st6_START (24)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st6_END (24)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st7_START (28)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st7_END (28)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE3_LOCK_ST_resource3lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource4lock_cmd0 : 1;
        unsigned int resource4lock_id0 : 3;
        unsigned int resource4lock_cmd1 : 1;
        unsigned int resource4lock_id1 : 3;
        unsigned int resource4lock_cmd2 : 1;
        unsigned int resource4lock_id2 : 3;
        unsigned int resource4lock_cmd3 : 1;
        unsigned int resource4lock_id3 : 3;
        unsigned int resource4lock_cmd4 : 1;
        unsigned int resource4lock_id4 : 3;
        unsigned int resource4lock_cmd5 : 1;
        unsigned int resource4lock_id5 : 3;
        unsigned int resource4lock_cmd6 : 1;
        unsigned int resource4lock_id6 : 3;
        unsigned int resource4lock_cmd7 : 1;
        unsigned int resource4lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE4_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id0_START (1)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id0_END (3)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id1_START (5)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id1_END (7)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id2_START (9)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id2_END (11)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id3_START (13)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id3_END (15)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id4_START (17)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id4_END (19)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id5_START (21)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id5_END (23)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id6_START (25)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id6_END (27)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id7_START (29)
#define SOC_LOCK_RESOURCE4_LOCK_resource4lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource4unlock_cmd0 : 1;
        unsigned int resource4unlock_id0 : 3;
        unsigned int resource4unlock_cmd1 : 1;
        unsigned int resource4unlock_id1 : 3;
        unsigned int resource4unlock_cmd2 : 1;
        unsigned int resource4unlock_id2 : 3;
        unsigned int resource4unlock_cmd3 : 1;
        unsigned int resource4unlock_id3 : 3;
        unsigned int resource4unlock_cmd4 : 1;
        unsigned int resource4unlock_id4 : 3;
        unsigned int resource4unlock_cmd5 : 1;
        unsigned int resource4unlock_id5 : 3;
        unsigned int resource4unlock_cmd6 : 1;
        unsigned int resource4unlock_id6 : 3;
        unsigned int resource4unlock_cmd7 : 1;
        unsigned int resource4unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE4_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id0_START (1)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id0_END (3)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id1_START (5)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id1_END (7)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id2_START (9)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id2_END (11)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id3_START (13)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id3_END (15)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id4_START (17)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id4_END (19)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id5_START (21)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id5_END (23)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id6_START (25)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id6_END (27)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id7_START (29)
#define SOC_LOCK_RESOURCE4_UNLOCK_resource4unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource4lock_st0 : 1;
        unsigned int resource4lock_st_id0 : 3;
        unsigned int resource4lock_st1 : 1;
        unsigned int resource4lock_st_id1 : 3;
        unsigned int resource4lock_st2 : 1;
        unsigned int resource4lock_st_id2 : 3;
        unsigned int resource4lock_st3 : 1;
        unsigned int resource4lock_st_id3 : 3;
        unsigned int resource4lock_st4 : 1;
        unsigned int resource4lock_st_id4 : 3;
        unsigned int resource4lock_st5 : 1;
        unsigned int resource4lock_st_id5 : 3;
        unsigned int resource4lock_st6 : 1;
        unsigned int resource4lock_st_id6 : 3;
        unsigned int resource4lock_st7 : 1;
        unsigned int resource4lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE4_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st0_START (0)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st0_END (0)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st1_START (4)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st1_END (4)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st2_START (8)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st2_END (8)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st3_START (12)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st3_END (12)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st4_START (16)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st4_END (16)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st5_START (20)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st5_END (20)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st6_START (24)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st6_END (24)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st7_START (28)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st7_END (28)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE4_LOCK_ST_resource4lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource5lock_cmd0 : 1;
        unsigned int resource5lock_id0 : 3;
        unsigned int resource5lock_cmd1 : 1;
        unsigned int resource5lock_id1 : 3;
        unsigned int resource5lock_cmd2 : 1;
        unsigned int resource5lock_id2 : 3;
        unsigned int resource5lock_cmd3 : 1;
        unsigned int resource5lock_id3 : 3;
        unsigned int resource5lock_cmd4 : 1;
        unsigned int resource5lock_id4 : 3;
        unsigned int resource5lock_cmd5 : 1;
        unsigned int resource5lock_id5 : 3;
        unsigned int resource5lock_cmd6 : 1;
        unsigned int resource5lock_id6 : 3;
        unsigned int resource5lock_cmd7 : 1;
        unsigned int resource5lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE5_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id0_START (1)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id0_END (3)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id1_START (5)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id1_END (7)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id2_START (9)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id2_END (11)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id3_START (13)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id3_END (15)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id4_START (17)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id4_END (19)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id5_START (21)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id5_END (23)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id6_START (25)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id6_END (27)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id7_START (29)
#define SOC_LOCK_RESOURCE5_LOCK_resource5lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource5unlock_cmd0 : 1;
        unsigned int resource5unlock_id0 : 3;
        unsigned int resource5unlock_cmd1 : 1;
        unsigned int resource5unlock_id1 : 3;
        unsigned int resource5unlock_cmd2 : 1;
        unsigned int resource5unlock_id2 : 3;
        unsigned int resource5unlock_cmd3 : 1;
        unsigned int resource5unlock_id3 : 3;
        unsigned int resource5unlock_cmd4 : 1;
        unsigned int resource5unlock_id4 : 3;
        unsigned int resource5unlock_cmd5 : 1;
        unsigned int resource5unlock_id5 : 3;
        unsigned int resource5unlock_cmd6 : 1;
        unsigned int resource5unlock_id6 : 3;
        unsigned int resource5unlock_cmd7 : 1;
        unsigned int resource5unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE5_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id0_START (1)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id0_END (3)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id1_START (5)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id1_END (7)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id2_START (9)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id2_END (11)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id3_START (13)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id3_END (15)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id4_START (17)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id4_END (19)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id5_START (21)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id5_END (23)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id6_START (25)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id6_END (27)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id7_START (29)
#define SOC_LOCK_RESOURCE5_UNLOCK_resource5unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource5lock_st0 : 1;
        unsigned int resource5lock_st_id0 : 3;
        unsigned int resource5lock_st1 : 1;
        unsigned int resource5lock_st_id1 : 3;
        unsigned int resource5lock_st2 : 1;
        unsigned int resource5lock_st_id2 : 3;
        unsigned int resource5lock_st3 : 1;
        unsigned int resource5lock_st_id3 : 3;
        unsigned int resource5lock_st4 : 1;
        unsigned int resource5lock_st_id4 : 3;
        unsigned int resource5lock_st5 : 1;
        unsigned int resource5lock_st_id5 : 3;
        unsigned int resource5lock_st6 : 1;
        unsigned int resource5lock_st_id6 : 3;
        unsigned int resource5lock_st7 : 1;
        unsigned int resource5lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE5_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st0_START (0)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st0_END (0)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st1_START (4)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st1_END (4)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st2_START (8)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st2_END (8)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st3_START (12)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st3_END (12)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st4_START (16)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st4_END (16)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st5_START (20)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st5_END (20)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st6_START (24)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st6_END (24)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st7_START (28)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st7_END (28)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE5_LOCK_ST_resource5lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource6lock_cmd0 : 1;
        unsigned int resource6lock_id0 : 3;
        unsigned int resource6lock_cmd1 : 1;
        unsigned int resource6lock_id1 : 3;
        unsigned int resource6lock_cmd2 : 1;
        unsigned int resource6lock_id2 : 3;
        unsigned int resource6lock_cmd3 : 1;
        unsigned int resource6lock_id3 : 3;
        unsigned int resource6lock_cmd4 : 1;
        unsigned int resource6lock_id4 : 3;
        unsigned int resource6lock_cmd5 : 1;
        unsigned int resource6lock_id5 : 3;
        unsigned int resource6lock_cmd6 : 1;
        unsigned int resource6lock_id6 : 3;
        unsigned int resource6lock_cmd7 : 1;
        unsigned int resource6lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE6_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id0_START (1)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id0_END (3)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id1_START (5)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id1_END (7)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id2_START (9)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id2_END (11)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id3_START (13)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id3_END (15)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id4_START (17)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id4_END (19)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id5_START (21)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id5_END (23)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id6_START (25)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id6_END (27)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id7_START (29)
#define SOC_LOCK_RESOURCE6_LOCK_resource6lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource6unlock_cmd0 : 1;
        unsigned int resource6unlock_id0 : 3;
        unsigned int resource6unlock_cmd1 : 1;
        unsigned int resource6unlock_id1 : 3;
        unsigned int resource6unlock_cmd2 : 1;
        unsigned int resource6unlock_id2 : 3;
        unsigned int resource6unlock_cmd3 : 1;
        unsigned int resource6unlock_id3 : 3;
        unsigned int resource6unlock_cmd4 : 1;
        unsigned int resource6unlock_id4 : 3;
        unsigned int resource6unlock_cmd5 : 1;
        unsigned int resource6unlock_id5 : 3;
        unsigned int resource6unlock_cmd6 : 1;
        unsigned int resource6unlock_id6 : 3;
        unsigned int resource6unlock_cmd7 : 1;
        unsigned int resource6unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE6_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id0_START (1)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id0_END (3)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id1_START (5)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id1_END (7)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id2_START (9)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id2_END (11)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id3_START (13)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id3_END (15)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id4_START (17)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id4_END (19)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id5_START (21)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id5_END (23)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id6_START (25)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id6_END (27)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id7_START (29)
#define SOC_LOCK_RESOURCE6_UNLOCK_resource6unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource6lock_st0 : 1;
        unsigned int resource6lock_st_id0 : 3;
        unsigned int resource6lock_st1 : 1;
        unsigned int resource6lock_st_id1 : 3;
        unsigned int resource6lock_st2 : 1;
        unsigned int resource6lock_st_id2 : 3;
        unsigned int resource6lock_st3 : 1;
        unsigned int resource6lock_st_id3 : 3;
        unsigned int resource6lock_st4 : 1;
        unsigned int resource6lock_st_id4 : 3;
        unsigned int resource6lock_st5 : 1;
        unsigned int resource6lock_st_id5 : 3;
        unsigned int resource6lock_st6 : 1;
        unsigned int resource6lock_st_id6 : 3;
        unsigned int resource6lock_st7 : 1;
        unsigned int resource6lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE6_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st0_START (0)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st0_END (0)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st1_START (4)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st1_END (4)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st2_START (8)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st2_END (8)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st3_START (12)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st3_END (12)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st4_START (16)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st4_END (16)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st5_START (20)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st5_END (20)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st6_START (24)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st6_END (24)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st7_START (28)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st7_END (28)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE6_LOCK_ST_resource6lock_st_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource7lock_cmd0 : 1;
        unsigned int resource7lock_id0 : 3;
        unsigned int resource7lock_cmd1 : 1;
        unsigned int resource7lock_id1 : 3;
        unsigned int resource7lock_cmd2 : 1;
        unsigned int resource7lock_id2 : 3;
        unsigned int resource7lock_cmd3 : 1;
        unsigned int resource7lock_id3 : 3;
        unsigned int resource7lock_cmd4 : 1;
        unsigned int resource7lock_id4 : 3;
        unsigned int resource7lock_cmd5 : 1;
        unsigned int resource7lock_id5 : 3;
        unsigned int resource7lock_cmd6 : 1;
        unsigned int resource7lock_id6 : 3;
        unsigned int resource7lock_cmd7 : 1;
        unsigned int resource7lock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE7_LOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd0_START (0)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd0_END (0)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id0_START (1)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id0_END (3)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd1_START (4)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd1_END (4)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id1_START (5)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id1_END (7)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd2_START (8)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd2_END (8)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id2_START (9)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id2_END (11)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd3_START (12)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd3_END (12)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id3_START (13)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id3_END (15)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd4_START (16)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd4_END (16)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id4_START (17)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id4_END (19)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd5_START (20)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd5_END (20)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id5_START (21)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id5_END (23)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd6_START (24)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd6_END (24)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id6_START (25)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id6_END (27)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd7_START (28)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_cmd7_END (28)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id7_START (29)
#define SOC_LOCK_RESOURCE7_LOCK_resource7lock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource7unlock_cmd0 : 1;
        unsigned int resource7unlock_id0 : 3;
        unsigned int resource7unlock_cmd1 : 1;
        unsigned int resource7unlock_id1 : 3;
        unsigned int resource7unlock_cmd2 : 1;
        unsigned int resource7unlock_id2 : 3;
        unsigned int resource7unlock_cmd3 : 1;
        unsigned int resource7unlock_id3 : 3;
        unsigned int resource7unlock_cmd4 : 1;
        unsigned int resource7unlock_id4 : 3;
        unsigned int resource7unlock_cmd5 : 1;
        unsigned int resource7unlock_id5 : 3;
        unsigned int resource7unlock_cmd6 : 1;
        unsigned int resource7unlock_id6 : 3;
        unsigned int resource7unlock_cmd7 : 1;
        unsigned int resource7unlock_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE7_UNLOCK_UNION;
#endif
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd0_START (0)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd0_END (0)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id0_START (1)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id0_END (3)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd1_START (4)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd1_END (4)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id1_START (5)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id1_END (7)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd2_START (8)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd2_END (8)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id2_START (9)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id2_END (11)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd3_START (12)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd3_END (12)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id3_START (13)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id3_END (15)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd4_START (16)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd4_END (16)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id4_START (17)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id4_END (19)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd5_START (20)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd5_END (20)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id5_START (21)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id5_END (23)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd6_START (24)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd6_END (24)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id6_START (25)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id6_END (27)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd7_START (28)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_cmd7_END (28)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id7_START (29)
#define SOC_LOCK_RESOURCE7_UNLOCK_resource7unlock_id7_END (31)
#ifndef __SOC_H_FOR_ASM__
typedef union
{
    unsigned int value;
    struct
    {
        unsigned int resource7lock_st0 : 1;
        unsigned int resource7lock_st_id0 : 3;
        unsigned int resource7lock_st1 : 1;
        unsigned int resource7lock_st_id1 : 3;
        unsigned int resource7lock_st2 : 1;
        unsigned int resource7lock_st_id2 : 3;
        unsigned int resource7lock_st3 : 1;
        unsigned int resource7lock_st_id3 : 3;
        unsigned int resource7lock_st4 : 1;
        unsigned int resource7lock_st_id4 : 3;
        unsigned int resource7lock_st5 : 1;
        unsigned int resource7lock_st_id5 : 3;
        unsigned int resource7lock_st6 : 1;
        unsigned int resource7lock_st_id6 : 3;
        unsigned int resource7lock_st7 : 1;
        unsigned int resource7lock_st_id7 : 3;
    } reg;
} SOC_LOCK_RESOURCE7_LOCK_ST_UNION;
#endif
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st0_START (0)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st0_END (0)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id0_START (1)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id0_END (3)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st1_START (4)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st1_END (4)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id1_START (5)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id1_END (7)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st2_START (8)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st2_END (8)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id2_START (9)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id2_END (11)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st3_START (12)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st3_END (12)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id3_START (13)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id3_END (15)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st4_START (16)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st4_END (16)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id4_START (17)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id4_END (19)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st5_START (20)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st5_END (20)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id5_START (21)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id5_END (23)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st6_START (24)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st6_END (24)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id6_START (25)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id6_END (27)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st7_START (28)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st7_END (28)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id7_START (29)
#define SOC_LOCK_RESOURCE7_LOCK_ST_resource7lock_st_id7_END (31)
#ifdef __cplusplus
    #if __cplusplus
        }
    #endif
#endif
#endif
