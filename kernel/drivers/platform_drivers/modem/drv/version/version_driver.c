/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <product_config.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/atomic.h>
#include <linux/of.h>
#include <nva_stru.h>
#include <securec.h>
#include <mdrv_memory.h>
#include <bsp_nvim.h>
#include <bsp_version.h>
#include <bsp_sec_call.h>
#include <bsp_dt.h>
#ifdef BSP_CONFIG_PHONE_TYPE
#include <adrv_soc_pg.h> /* get_soc_modem_regulator_strategy */
#include <adrv_version.h> /* get_product_id, get_checked_product_id */
#endif

#undef THIS_MODU
#define THIS_MODU mod_version
#define VER_SHM_VALID 0x1234abcd
#define VER_SHM_INVALID 0x4321dcba


u32 g_shm_init_flag = VER_SHM_INVALID;
void *g_ver_shm_addr = 0;
u32 g_is_verison_ddr_init;

u32 g_ver_nv_init_state = 0;
version_shm_info g_version_shm_info = {};
product_info_nv_s g_hardware_product_info = { 0 };

u32 bsp_version_hw_sub_id(void)
{
    return g_version_shm_info.product_info_nv.hw_id_sub;
}

/*
 * 功能描述：获取共享内存基地址和大小
 */
void bsp_version_get_shm(void)
{
    u32 shm_size = 0;
    phy_addr_t phy_addr = 0;
    const char *name = "nsroshm_version";

    g_ver_shm_addr = mdrv_mem_share_get(name, &phy_addr, &shm_size, SHM_NSRO);
    if (g_ver_shm_addr == NULL) {
        ver_print_error("bsp_version_shm_init fail: get shared mem err!\n");
        return;
    }

    g_shm_init_flag = VER_SHM_VALID;
    return;
}

#ifdef BSP_CONFIG_PHONE_TYPE
//for phone
#define UDP_DTS_INDEX (2)
#define MAX_BOARDID_LEN 4
#define CALC_BOARDID(a) ((((a)[0])*1000) + (((a)[1])*100) + (((a)[2])*10) + ((a)[3]))

u32 bsp_version_get_udp_flag(void)
{
    struct device_node *np = NULL;
    unsigned int  udp_flag = 0;
    int ret;
    ver_print_error("get_udp_flag\n");
    np = of_find_node_by_path("/");
    if (np == NULL) {
        ver_print_error("of_find_node_by_name failed\n");
        return 0;
    }
    ret = of_property_read_u32_index(np, "hisi,boardid", UDP_DTS_INDEX, &udp_flag);

    if (ret < 0) {
        ver_print_error("bsp_version_get_udp_flag failed\n");
        return 0;
    }
    ver_print_error("udp_flag = %d \n", udp_flag);
    return udp_flag;
}

u32 bsp_version_get_base_board_id(void)
{
    struct device_node *np = NULL;
    unsigned int id[MAX_BOARDID_LEN] = {0};
    unsigned int base_board_id = 0;
    int ret;

    ver_print_error("get_boardid\n");
    np = of_find_node_by_path("/");
    if (np == NULL) {
        ver_print_error("of_find_node_by_name failed\n");
        return 0;
    }
    ret = of_property_read_u32_array(np, "hisi,boardid", id, MAX_BOARDID_LEN);

    if (ret < 0) {
        ver_print_error("bsp_version_get_base_board_id failed\n");
        return 0;
    }

    base_board_id = CALC_BOARDID(id);
    ver_print_error("base_board_id = %d \n", base_board_id);
    return base_board_id;
}

u32 version_get_productid(void)
{
    u32 productid;
    productid = get_product_id();
    ver_print_error("productid: %#x \n", productid);
    return productid;
}

unsigned int bsp_version_get_chip_level(void) {
#ifdef CONFIG_PHONE_VERSION_DIFF_SOC_LEVEL
    int strategy = get_soc_modem_regulator_strategy();
    ver_print_error("get soc modem regulator strategy: %d \n", strategy);
    // 只有返回值是2级芯片时, 将芯片看做2级，其余情况(1级、avs策略、不支持区分级别)均看做1级芯片
    if (strategy == CHIP_LEVEL_II) {
        return CHIP_LEVEL_II;
    } else {
        return CHIP_LEVEL_I;
    }
#else
    return CHIP_LEVEL_I;
#endif
}

/*
 * 功能描述: 手机代码，在kernal阶段读acore dtb数据
 */
int bsp_version_get_product_info(product_ver_s *product_ver)
{
    const char *temp_str = NULL;
    int ret;

    ret = memset_s(product_ver, sizeof(product_ver_s), 0, sizeof(product_ver_s));
    if (ret) {
        ver_print_error("product_ver memset_s error\n");
        return VER_ERROR;
    }

    device_node_s *node = NULL;
    node = bsp_dt_find_compatible_node(NULL, NULL, "modem_product_name_ver");
    if (node == NULL) {
        ver_print_error("of_find_compatible_node modem_product_name_ver error\n");
        return VER_ERROR;
    }
    ret = bsp_dt_property_read_string(node, "product_name", &temp_str);
    if (ret || temp_str == NULL) {
        ver_print_error("bsp_dt_property_read_string product_name error\n");
        return VER_ERROR;
    }
    ret = strcpy_s(product_ver->product_name, PRODUCT_NAME_LEN, temp_str);
    if (ret) {
        ver_print_error("strcpy_s product_ver->product_name error\n");
        return VER_ERROR;
    }
    ret = bsp_dt_property_read_string(node, "product_release_ver", &temp_str);
    if (ret || temp_str == NULL) {
        ver_print_error("bsp_dt_property_read_string product_release_ver error\n");
        return VER_ERROR;
    }
    ret = strcpy_s(product_ver->product_release_ver, PRODUCT_NAME_LEN, temp_str);
    if (ret) {
        ver_print_error("strcpy_s product_ver->product_release_ver error\n");
        return VER_ERROR;
    }
    ret = bsp_dt_property_read_string(node, "product_download_ver", &temp_str);
    if (ret || temp_str == NULL) {
        ver_print_error("bsp_dt_property_read_string product_download_ver error\n");
        return VER_ERROR;
    }
    ret = strcpy_s(product_ver->product_download_ver, PRODUCT_NAME_LEN, temp_str);
    if (ret) {
        ver_print_error("strcpy_s product_ver->product_download_ver error\n");
        return VER_ERROR;
    }

    return VER_OK;
}

int version_write_to_mem(u32 productid)
{
    int ret;
    version_sec_info_s info;
    info.udp_flag = bsp_version_get_udp_flag();
    info.product_id = productid;
    info.base_board_id = bsp_version_get_base_board_id();
    info.chip_level = bsp_version_get_chip_level();
    ret = bsp_version_get_product_info(&info.product_ver);
    if (ret) {
        ver_print_error("bsp_version_get_product_info fail\n");
        return ret;
    }

    ret = bsp_sec_call_ext(FUNC_MDRV_VERSION_INIT, 0, (void *)&info, sizeof(info));
    if (ret) {
        ver_print_error("sec call write to mem fail\n");
        return ret;
    }
    return 0;
}

int bsp_version_get_checked_productid(u32 *prdt_list, u32 cnt)
{
    u32 productid;
    productid = get_checked_product_id(prdt_list, cnt);
    ver_print_error("checked productid: %#x \n", productid);
    if (productid == 0xFFFFFFFF) {
        ver_print_error("get checked productid err\n");
        return -1;
    }
    return version_write_to_mem(productid);
}

/*
 * 功能描述: version的共享内存数据初始化
 */
int bsp_version_base_init(void)
{
    int ret;
    u32 productid;
    productid = version_get_productid();
    if (g_is_verison_ddr_init != 1) {
        ret = version_write_to_mem(productid);
        if (ret) {
            return ret;
        }
        g_is_verison_ddr_init = 1;
    }
    return VER_OK;
}

/*
 * 功能描述: acore版本号初始化
 */
int bsp_version_acore_init(void)
{
    return VER_OK;
}
#else
//for mbb
int bsp_version_base_init(void)
{
    g_is_verison_ddr_init = 1;
    return VER_OK;
}

/*
 * 功能描述: acore版本号初始化
 */
int bsp_version_acore_init(void)
{
    // MB产品，kernel启动时所有数据已经准备完毕，可以直接从共享内存读出
    return bsp_version_acore_get_info();
}
#endif

/*
 * 功能描述: 为其他模块提供各种版本号信息
 * 返回值: version的共享内存数据结构体的地址
 */
const bsp_version_info_s *bsp_get_version_info(void)
{
    bsp_version_info_s *p_version_info = NULL;
    int ret;
    if (g_shm_init_flag == VER_SHM_VALID && g_ver_shm_addr != NULL) {
        p_version_info = (bsp_version_info_s *)g_ver_shm_addr;
    } else {
        bsp_version_get_shm();
        if (g_ver_shm_addr == NULL) {
            ver_print_error("version shm addr is NULL\n");
            return NULL;
        }
        p_version_info = (bsp_version_info_s *)g_ver_shm_addr;
    }
    if (g_is_verison_ddr_init != 1) {
        ret = bsp_version_base_init();
        if (ret) {
            return NULL;
        }
    }
    if (p_version_info->version_magic == VERSION_MODULE_MAGIC) {
        return p_version_info;
    } else {
        ver_print_error("version magic error:0x%x(!=0x2017)\n", p_version_info->version_magic);
        return NULL;
    }
}

/*
 * 以下接口提供给mdrv接口调用
 */
int bsp_version_acore_get_info(void)
{
    int ret = VER_OK;
    const char *ver_shm_addr = NULL;

    ver_shm_addr = (const char *)bsp_get_version_info();
    if (ver_shm_addr == NULL) {
        ver_print_error("get version shm addr err\n");
        return VER_ERROR;
    }
    ret = memcpy_s(&g_version_shm_info, sizeof(version_shm_info), ver_shm_addr + sizeof(bsp_version_info_s), sizeof(version_shm_info));
    if (ret) {
        ver_print_error("memcpy_s err\n");
        return VER_ERROR;
    }
    ret = memcpy_s(&g_hardware_product_info, sizeof(product_info_nv_s), &g_version_shm_info.product_info_nv, sizeof(product_info_nv_s));
    if (ret) {
        ver_print_error("memcpy_s err\n");
        return VER_ERROR;
    }
    if ((g_version_shm_info.flag & VERSION_PRODUCT_VER_MASK) != VERSION_PRODUCT_VER_MASK) {
        ver_print_error("ver nv init err\n");
        return VER_ERROR;
    }

    mdrv_ver_init();
    return ret;
}

/*
 * 函 数: bsp_version_get_hardware
 * 功 能: 获取硬件版本号(硬件版本名称+ Ver.+硬件子版本号+A)
 */
char *bsp_version_get_hardware(void)
{
    u32 len;
    int ret;
    static bool b_geted = false;
    static char hardware_version[VERSION_MAX_LEN];
    char hardware_sub_ver;

    if (g_version_shm_info.product_info_nv.index == HW_VER_INVALID) {
        ver_print_error("g_version_shm_info.product_info_nv.index = HW_VER_INVALID\n");
        return NULL;
    }

    if (!b_geted) {
        len = (unsigned int)(strlen(g_version_shm_info.product_info_nv.hw_ver) + strlen(" Ver.X"));
        hardware_sub_ver = (char)g_version_shm_info.product_info_nv.hw_id_sub + 'A';
        if (memset_s((void *)hardware_version, VERSION_MAX_LEN, 0, len)) {
            ver_print_error("hardware_version memset err\n");
        }
        ret = strncat_s(hardware_version, VERSION_MAX_LEN, g_version_shm_info.product_info_nv.hw_ver,
            strlen(g_version_shm_info.product_info_nv.hw_ver));
        if (ret) {
            ver_print_error("strncat hardware version err\n");
            return NULL;
        }
        ret = strncat_s(hardware_version, VERSION_MAX_LEN, " Ver.", strlen(" Ver."));
        if (ret) {
            ver_print_error("strncat hardware version err\n");
            return NULL;
        }
        if (len < VERSION_MAX_LEN) {
            *((hardware_version + len) - 1) = hardware_sub_ver;
            *(hardware_version + len) = 0;
        } else {
            return NULL;
        }
        b_geted = true;
    }

    return (char *)hardware_version;
}

/*
 * 函 数: bsp_get_product_inner_name
 * 功 能: 获取内部产品名称(内部产品名+ 内部产品名plus)
 */
char *bsp_version_get_product_inner_name(void)
{
    unsigned int len;
    int ret;
    static bool b_geted = false;
    static char product_inner_name[VERSION_MAX_LEN];

    if (g_version_shm_info.product_info_nv.index == HW_VER_INVALID) {
        ver_print_error("g_version_shm_info.product_info_nv.index = HW_VER_INVALID\n");
        return NULL;
    }

    if (!b_geted) {
        len = (unsigned int)(strlen(g_version_shm_info.product_info_nv.name) + strlen(g_version_shm_info.product_info_nv.name_plus));
        ret = memset_s((void *)product_inner_name, VERSION_MAX_LEN, 0, len);
        if (ret) {
            ver_print_error("product_inner_name memset err\n");
        }

        ret = strncat_s(product_inner_name, VERSION_MAX_LEN, g_version_shm_info.product_info_nv.name,
            strlen(g_version_shm_info.product_info_nv.name));
        if (ret) {
            ver_print_error("strncat product_inner_name err\n");
            return NULL;
        }
        ret = strncat_s(product_inner_name, VERSION_MAX_LEN, g_version_shm_info.product_info_nv.name_plus,
            strlen(g_version_shm_info.product_info_nv.name_plus));
        if (ret) {
            ver_print_error("strncat product_inner_name err\n");
            return NULL;
        }
        b_geted = true;
    }

    return (char *)product_inner_name;
}

/*
 * 函 数: bsp_get_product_out_name
 * 功 能: 获取外部产品名称
 */
char *bsp_version_get_product_out_name(void)
{
    if (g_version_shm_info.product_info_nv.index == HW_VER_INVALID) {
        ver_print_error("g_version_shm_info.product_info_nv.index = HW_VER_INVALID\n");
        return NULL;
    }

    return (char *)g_version_shm_info.product_info_nv.product_id;
}

/*
 * 以下接口提供给dump模块调用
 */
/*
 * 函 数: bsp_get_build_date_time
 * 功 能: 获取编译日期和时间
 */
char *bsp_version_get_build_date_time(void)
{
#ifdef BSP_CONFIG_PHONE_TYPE
    return NULL;
#else
     char *build_date = VERSION_BUILD_DATE ", " VERSION_BUILD_TIME;
     return build_date;
#endif
}


/*
 * 函 数: bsp_get_firmware_version
 * 功 能: 获取软件版本号
 */
char *bsp_version_get_firmware(void)
{
    return (char *)g_version_shm_info.product_ver.product_download_ver;
}

/*
 * 函 数: bsp_version_get_release_ver
 * 功 能: 获取发布版本号
 */
char *bsp_version_get_release_ver(void)
{
    return (char *)g_version_shm_info.product_ver.product_release_ver;
}

/*
 * 函 数: bsp_version_get_product_name
 * 功 能: 获取产品平台名
 */
char *bsp_version_get_product_name(void)
{
    return (char *)g_version_shm_info.product_ver.product_name;
}

/*
 * 函 数: bsp_version_debug
 * 功 能: 用于调试查看版本号相关信息
 */
int bsp_version_debug(void)
{
    if (g_version_shm_info.product_info_nv.index == HW_VER_INVALID) {
        ver_print_error("g_version_shm_info.product_info_nv.index = HW_VER_INVALID\n");
        return VER_ERROR;
    }

    ver_print_error("\n\n1 . the element value of g_version_shm_info.product_info_nv:\n");
    ver_print_error("Hardware index :0x%x\n", g_version_shm_info.product_info_nv.index);
    ver_print_error("hw_Sub_ver     :0x%x\n", g_version_shm_info.product_info_nv.hw_id_sub);
    ver_print_error("Inner name     :%s\n", g_version_shm_info.product_info_nv.name);
    ver_print_error("name plus      :%s\n", g_version_shm_info.product_info_nv.name_plus);
    ver_print_error("HardWare ver   :%s\n", g_version_shm_info.product_info_nv.hw_ver);
    ver_print_error("DLOAD_ID       :%s\n", g_version_shm_info.product_info_nv.download_id);
    ver_print_error("Out name       :%s\n", g_version_shm_info.product_info_nv.product_id);

    ver_print_error("\n\n3 . get from func(bsp_version_get_xxx):\n");
    ver_print_error("HardWare ver   :%s\n", bsp_version_get_hardware());
    ver_print_error("Inner name     :%s\n", bsp_version_get_product_inner_name());
    ver_print_error("Out name       :%s\n", bsp_version_get_product_out_name());
    ver_print_error("Build_time     :%s\n", bsp_version_get_build_date_time());
    ver_print_error("Firmware       :%s\n", bsp_version_get_firmware());
    ver_print_error("Release_ver    :%s\n", bsp_version_get_release_ver());
    ver_print_error("product_name   :%s\n", bsp_version_get_product_name());

    if (bsp_get_version_info() == NULL) {
        ver_print_error("bsp_get_version_info == NULL\n");
        return VER_ERROR;
    }

    ver_print_error("\n\n4 . get from bsp_get_version_info:\n");
    ver_print_error("product_id               :0x%x\n", bsp_get_version_info()->product_id);
    ver_print_error("product_id_udp_masked    :0x%x\n", bsp_get_version_info()->product_id_udp_masked);
    ver_print_error("product_id_with_board_id :0x%x\n", bsp_get_version_info()->product_id_with_board_id);
    ver_print_error("chip_version        :0x%x\n", bsp_get_version_info()->chip_version);
    ver_print_error("chip_type           :0x%x\n", bsp_get_version_info()->chip_type);
    ver_print_error("chip_level          :0x%x\n", bsp_get_version_info()->chip_level);
    ver_print_error("plat_type           :0x%x  (0:asic 1:plat_esl 2:hybrid a:porting e:emu)\n",
        bsp_get_version_info()->plat_type);
    ver_print_error("plat_info           :0x%x(0:asic 1:esl_vdk 2:esl_cand 3:esl_self 4:emu_zebu  \
        5:emu_z1 6:hybrid_vdk&zebu 7:hybrid_cand&z1)\n",
        bsp_get_version_info()->plat_info);
    ver_print_error("base_board_id       :%d \n", bsp_get_version_info()->base_board_id);
    ver_print_error("board_type(for drv) :0x%x  (0:bbit 1:sft 2:asic 3:soc 4:porting)\n",
        bsp_get_version_info()->board_type);
    ver_print_error("board_type(for mdrv):0x%x  (0:bbit 1:sft 2:asic)\n",
        (board_actual_type_e)bsp_get_version_info()->board_type);
    ver_print_error("product_type        :0x%x  (0:mbb 1:phone)\n", bsp_get_version_info()->product_type);
    ver_print_error("cses_type           :0x%x(1:es 2:cs)\n", bsp_get_version_info()->cses_type);
    ver_print_error("version_magic       :0x%x\n", bsp_get_version_info()->version_magic);

    return VER_OK;
}

#if !IS_ENABLED(CONFIG_MODEM_DRIVER)
/* 注意:需在nv模块初始化之后 */
module_init(bsp_version_acore_init);
#endif
EXPORT_SYMBOL_GPL(g_hardware_product_info);
EXPORT_SYMBOL_GPL(bsp_version_acore_init);
EXPORT_SYMBOL_GPL(bsp_version_get_hardware);
EXPORT_SYMBOL_GPL(bsp_version_get_product_inner_name);
EXPORT_SYMBOL_GPL(bsp_version_get_product_out_name);
EXPORT_SYMBOL_GPL(bsp_get_version_info);
EXPORT_SYMBOL_GPL(bsp_version_debug);
#ifdef BSP_CONFIG_PHONE_TYPE
EXPORT_SYMBOL_GPL(bsp_version_get_udp_flag);
#endif

