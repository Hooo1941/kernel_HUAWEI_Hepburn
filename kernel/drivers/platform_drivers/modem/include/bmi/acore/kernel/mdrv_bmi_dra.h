/**
 *  @brief   飞龙架构2.0 maa模块对外头文件
 *  @file    mdrv_bmi_dra.h
 *  @version v1.0
 *  @date    2023.07.19
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.19|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_DRA_H_
#define _MDRV_BMI_DRA_H_

#include <linux/skbuff.h>

/**
 * @brief 重置skb指针
 *
 * @par 描述:
 * 重置skb指针
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[in]  skb, sk_buff。
 * @param[out] NA, NA。
 *
 * @retval NA。
 * @par 依赖:
 * <ul><li>mdrv_bmi_dra.h：该接口声明所在的头文件。</li></ul>
 *
 * @see NA
 */
void mdrv_dra_reset_skb(struct sk_buff *skb);

/**
 * @brief 通过maa地址获取skb
 *
 * @par 描述:
 * 通过maa地址获取skb
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[in]  addr, maa地址。
 * @param[out] struct sk_buff *, 获取的skb。
 *
 * @retval NA。
 * @par 依赖:
 * <ul><li>mdrv_bmi_dra.h：该接口声明所在的头文件。</li></ul>
 *
 * @see NA
 */
struct sk_buff *mdrv_dra_get_skb(unsigned long long addr);

/**
 * @brief 申请dra的skb
 *
 * @par 描述:
 * 申请dra上的skb
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out]  NA
 * @param[in]  size，需要申请skb的大小
 *
 * @retval unsigned long long，已申请skb的大小
 *
 * @see NA
 */
unsigned long long mdrv_dra_alloc(unsigned int size);

/**
 * @brief 释放dra上的skb
 *
 * @par 描述:
 * 释放dra上的skb
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[out]  NA,NA
 * @param[in]  changed,需要释放skb大小
 *
 * @retval NA
 *
 * @see NA
 */
void mdrv_dra_free(unsigned long long changed);

/**
 * @brief 获取skb的dra地址
 *
 * @par 描述:
 * 获取skb的dra地址
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[in]  skb, sk_buff。
 * @param[out]  unsigned long long，获取到的dra地址
 *
 * @retval NA
 *
 * @see NA
 */
unsigned long long mdrv_dra_skb_map(struct sk_buff *skb, unsigned int reserve);

/**
 * @brief 由dra地址获取skb
 *
 * @par 描述:
 * 由dra地址获取skb
 *
 * @attention
 * <ul><li> NA </li></ul>
 *
 * @param[in]  addr, dra地址
 * @param[out]  struct sk_buff, 获取到的sk_buff
 *
 * @retval NA
 *
 * @see NA
 */
struct sk_buff *mdrv_dra_skb_unmap(unsigned long long addr);
#endif