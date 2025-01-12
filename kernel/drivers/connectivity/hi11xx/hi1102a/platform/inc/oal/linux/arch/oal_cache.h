

#ifndef __OAL_LINUX_CACHE_H__
#define __OAL_LINUX_CACHE_H__

/* 其他头文件包含 */
#include <linux/version.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44) || defined(_PRE_PLAT_FEATURE_HI110X_PCIE))
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#ifndef _PRE_WLAN_EXPORT
#include <arch/arm/mach-sd5115h-v100f/include/mach/cache-hil2.h>
#endif
#else
#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
#include <mach/cache-hil2.h>
#else
#include <../arch/arm/mach-hsan/include/mach/hi_cache.h>
#endif
#endif

#include <asm/cacheflush.h>
#if (defined(_PRE_BOARD_SD5610) || defined(_PRE_BOARD_SD5115))
#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
#include <mach/platform.h>
#else
#include <../arch/arm/mach-hsan/include/mach/hi_map.h>
#endif
#endif
#include <linux/device.h>
#include <linux/compiler.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/cache.h>

/* 宏定义 */
#define OAL_CACHELINE_ALIGNED ____cacheline_aligned

/* 全局变量声明 */
extern void __iomem *l2cache_base;

/* OTHERS定义 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 44) || defined(_PRE_PLAT_FEATURE_HI110X_PCIE))
#else
#ifndef _PRE_WLAN_EXPORT
extern struct cpu_cache_fns g_cpu_cache;

/*
 * 函 数 名  : oal_l1cache_flush
 * 功能描述  : flush L1 Cache
 */
OAL_STATIC OAL_INLINE oal_void oal_l1cache_flush(oal_void)
{
    g_cpu_cache.flush_kern_all();
}

/*
 * 函 数 名  : oal_l2cache_dcache_lock
 * 功能描述  : L2 cache锁住或释放对应的Dcache 的cache way
 * 输入参数  : en_flag: OAL_TRUE表示锁住，OAL_FALSE表示释放
 *             ul_way: 要锁住或者释放的对应的cache way
 */
OAL_STATIC OAL_INLINE oal_void oal_l2cache_dcache_lock(oal_bool_enum_uint8 en_flag, oal_uint32 ul_way)
{
    oal_uint32 ul_reg;

#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
    ul_reg = readl(l2cache_base + REG_L2_DLOCKWAY);
#else
    ul_reg = readl(l2cache_base + HI_REG_L2_DLOCKWAY);
#endif

    ul_reg = (ul_reg & (~(1 << ul_way))) | (en_flag << ul_way);

#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
    writel(ul_reg, l2cache_base + REG_L2_DLOCKWAY);
#else
    writel(ul_reg, l2cache_base + HI_REG_L2_DLOCKWAY);
#endif

    return;
}

/*
 * 函 数 名  : oal_l2cache_icache_lock
 * 功能描述  : L2 cache锁住或释放对应的Icache 的cache way
 * 输入参数  : en_flag: OAL_TRUE表示锁住，OAL_FALSE表示释放
 *             ul_way: 要锁住或者释放的对应的cache way
 */
OAL_STATIC OAL_INLINE oal_void oal_l2cache_icache_lock(oal_bool_enum_uint8 en_flag, oal_uint32 ul_way)
{
    oal_uint32 ul_reg;
#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
    ul_reg = readl(l2cache_base + REG_L2_ILOCKWAY);
#else
    ul_reg = readl(l2cache_base + HI_REG_L2_ILOCKWAY);
#endif
    ul_reg = (ul_reg & (~(1 << ul_way))) | (en_flag << ul_way);

#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
    writel(ul_reg, l2cache_base + REG_L2_ILOCKWAY);
#else
    writel(ul_reg, l2cache_base + HI_REG_L2_ILOCKWAY);
#endif
    return;
}

/*
 * 函 数 名  : oal_cache_flush_all
 * 功能描述  : clean 并 invalid L2 cache
 */
OAL_STATIC OAL_INLINE oal_void oal_cache_flush_all(oal_void)
{
    flush_cache_all();
}

/*
 * 函 数 名  : oal_cache_rxtx_lock
 * 功能描述  : 将rx tx流程关键代码关键数据锁进L2 cache
 */
OAL_STATIC OAL_INLINE oal_void oal_cache_rxtx_lock(oal_void)
{
#if ((_PRE_TARGET_PRODUCT_TYPE_5610EVB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_VSPM310DMB == _PRE_CONFIG_TARGET_PRODUCT) || \
     (_PRE_TARGET_PRODUCT_TYPE_WS835DMB == _PRE_CONFIG_TARGET_PRODUCT))
    l2cache_base = (void __iomem *)ioremap(REG_BASE_L2CACHE, OAL_SIZEOF(void *));
#else
    l2cache_base = (void __iomem *)ioremap(HI_REG_BASE_L2CACHE, OAL_SIZEOF(void *));
#endif
}
#endif
#endif
#endif /* end of oal_cache.h */
