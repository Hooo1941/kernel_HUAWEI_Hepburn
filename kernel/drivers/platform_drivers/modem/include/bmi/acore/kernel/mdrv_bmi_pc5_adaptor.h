/**
 *  @brief   飞龙架构2.0 pc5对外头文件
 *  @file    mdrv_bmi_pc5_adaptor.h
 *  @version v1.0
 *  @date    2023.07.19
 *  @note    该文件增删改需通过接口变更评审
 *  <ul><li>v1.0|2023.07.19|创建文件</li></ul>
 *  @since
 */

#ifndef _MDRV_BMI_PC5_ADAPTOR_H_
#define _MDRV_BMI_PC5_ADAPTOR_H_

#include <linux/skbuff.h>

/**
 * @brief ltev接收回调函数指针结构体
 */
typedef int (*mdrv_pc5_rx_cb)(struct sk_buff *skb);

/**
 * @brief pc-5数据接收回调注册接口
 *
 * @par 描述:
 * pc-5数据接收回调注册接口
 *
 * @attention
 * <ul><li>NA。</li></ul>
 *
 * @param[in]  cb , 接收回调函数指针
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 * @par 依赖:
 * <ul><li>mdrv_bmi_pc5_adaptor.h：该接口声明所在的头文件。</li></ul>
 *
 * @see mdrv_pc5_rx_cb
 */
int mdrv_pc5_cb_register(mdrv_pc5_rx_cb cb);

/**
 * @brief pc-5数据发送接口
 *
 * @par 描述:
 * pc-5数据发送接口
 *
 * @attention
 * <ul><li>NA。</li></ul>
 *
 * @param[in]  skb , sk_buff指针
 *
 * @retval 0, 成功。
 * @retval !0, 失败。
 * @par 依赖:
 * <ul><li>mdrv_bmi_pc5_adaptor.h：该接口声明所在的头文件。</li></ul>
 *
 */
int mdrv_pc5_tx(struct sk_buff *skb);

#endif