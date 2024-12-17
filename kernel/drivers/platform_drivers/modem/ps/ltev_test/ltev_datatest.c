/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
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
 *
 */

#include <linux/jiffies.h>
#include <linux/kthread.h>
#include <linux/math64.h>
#include <linux/netdevice.h>
#include <linux/semaphore.h>
#include <linux/skbuff.h>
#include <linux/timer.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <linux/version.h>

#include "securec.h"
#include "mdrv_bmi_pc5_adaptor.h"
#include "mdrv_print.h"
#include "mdrv_timer.h"


#define mod_ltevtest "letvtest"
#define THIS_MODU mod_ltevtest


#if defined(DMT) || defined(__UT_CENTER__)
#define STATIC
#else
#define STATIC static
#endif

#define OK 0
#define ERR (-1)

#define PRINT_ERR(fmt, ...) (mdrv_err(fmt, ##__VA_ARGS__))

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

#define LTEV_DATA_MAGIC_NUM 0x5A5A

#define LTEV_DBG_STAT 1
#define LTEV_DBG_SEQ  2

#define LTEV_TEST_INVALID_SEQ 0xFFFFFFFF
#define LTEV_TEST_SRC_IDX 2
#define LTEV_TEST_SEQ_MOD 65536
#define LTEV_TEST_WINDOW_SIZE 32

/* 设置标志比特位的值为1，bitpos:[0..31] */
#define LTEV_TEST_SET_BIT(value, bitpos) ((value) |= (1UL << (bitpos)))
/* 清除标志比特位的值为0，bitpos:[0..31] */
#define LTEV_TEST_CLEAR_BIT(value, bitpos) ((value) &= ~(1UL << (bitpos)))
/* 获取标志比特位的值，结果返回1和0，bitpos:[0..31] */
#define LTEV_TEST_GET_BIT(value, bitpos) ((unsigned int)(((value) & (1UL << (bitpos))) >> (bitpos)))
#define LTEV_TEST_MOD_ADD(x, y, z) (((x) + (y)) % (z))
#define LTEV_TEST_MOD_SUB(x, y, z) ((((x) + (z)) - (y)) % (z))
#define LTEV_TEST_GET_BIT_POS(x) ((x) % LTEV_TEST_WINDOW_SIZE)

#define LTEV_TEST_ADDR_LEN 3
#define VPS_TEST_VERSION 2

#define LTEV_TEST_ACCESS_HEAD_LEN sizeof(LTEV_TEST_DataReqAccesHead)
#define LTEV_TEST_MAX_APP_DATA_LEN 9000
#define MAX_PKT_LEN (LTEV_TEST_MAX_APP_DATA_LEN + LTEV_TEST_ACCESS_HEAD_LEN)
#define DEFAULT_PKT_LEN 1200

typedef struct {
    unsigned short seq;
    unsigned short magicNum;
    unsigned char  data[0];
} LTEV_TEST_DataReqAppHead;

typedef struct {
    unsigned short version;
    unsigned char  srcAddr[LTEV_TEST_ADDR_LEN];
    unsigned char  dstAddr[LTEV_TEST_ADDR_LEN];
    unsigned char  priority;
    unsigned char  pduType;
    unsigned short pdb;
    unsigned short trafficPeriod;
    unsigned char  txProfile; /* 上层应用期望与RRC配置能力的交集 */
    unsigned char rsv[3];
    unsigned short dataLen;
    unsigned char  data[0];
} LTEV_TEST_DataReqAccesHead;

typedef struct {
    unsigned short version;
    unsigned char  srcAddr[LTEV_TEST_ADDR_LEN];
    unsigned char  dstAddr[LTEV_TEST_ADDR_LEN];
    unsigned int   maxDataRate;
    unsigned char  priority;
    unsigned char  cbr;

    unsigned short frmNo;
    unsigned char  subFrmNo;
    unsigned char  resv;
    unsigned short dataLen;
    unsigned short seq;
    unsigned short magicNum;
    unsigned char  data[0];
} LTEV_TEST_DataInd;

typedef struct {
    unsigned long statTime;
    unsigned long lostCntPer;
    unsigned long sendCntPer;
    unsigned long rcvCntPer;
    unsigned long sendBytesPer;
    unsigned long rcvBytesPer;
    unsigned long seqErrCntPer;
    unsigned long totalLostCnt;
    unsigned long totalSendCnt;
    unsigned long totalRcvCnt;
    unsigned long totalSendBytes;
    unsigned long totalRcvBytes;
    unsigned long seqErrCnt;
} StatInfo;
#define MAX_USER_CNT 256
typedef struct {
    struct timer_list statTimer;
    unsigned short sendSeq;
    unsigned short resv;
    unsigned short timeVal;
    unsigned short dataLen;
    unsigned int sendCnt;
    unsigned int cycleCnt;
    unsigned int loopFlag;

    struct semaphore taskSem;
} TestCtrl;

typedef struct {
    unsigned int nextSeq; /* 期望收到下一个报文的序号 */
    unsigned int maxSeq; /* 重排序窗口的最大序号 */
    unsigned int recvBitmap; /* recvBitmap中的第K位表示seq mod 32 对应位置的报文是否收到 */
} LTEV_TestRecvEntity;

STATIC StatInfo g_stat;

STATIC TestCtrl g_ctrl;

STATIC unsigned int g_dbgFlag = 1;

STATIC bool g_isRunning = false;

STATIC LTEV_TEST_DataReqAccesHead g_pktHead = {
    0x2, { 0x11, 0x22, 0x33 }, { 0x44, 0x55, 0x66 }, 1, 1, 0, 0, 0, { 0, 0, 0 }, 0
};

STATIC unsigned char g_testData[255];
STATIC bool g_isTestInited = false;

STATIC LTEV_TestRecvEntity g_recvEntity[MAX_USER_CNT];

void LTEV_TEST_SetTxProfile(unsigned char txProfile)
{
    g_pktHead.txProfile = txProfile;
}

STATIC void LTEV_InitRecvEntity(void)
{
    unsigned int idx;
    for (idx = 0; idx < MAX_USER_CNT; idx++) {
        g_recvEntity[idx].nextSeq = LTEV_TEST_INVALID_SEQ;
        g_recvEntity[idx].maxSeq = LTEV_TEST_INVALID_SEQ;
    }
}

void LTEV_DebugSw(unsigned int sw)
{
    g_dbgFlag = sw;
}

void LTEV_SetHead(unsigned short version, unsigned int srcAddr, unsigned int dstAddr, unsigned char priority,
    unsigned char pduType)
{
    g_pktHead.srcAddr[0] = (unsigned char)(srcAddr >> 16);
    g_pktHead.srcAddr[1] = (unsigned char)(srcAddr >> 8);
    g_pktHead.srcAddr[2] = (unsigned char)srcAddr;
    g_pktHead.dstAddr[0] = (unsigned char)(dstAddr >> 16);
    g_pktHead.dstAddr[1] = (unsigned char)(dstAddr >> 8);
    g_pktHead.dstAddr[2] = (unsigned char)dstAddr;
    g_pktHead.priority = priority;
    g_pktHead.pduType = pduType;
}

void LTEV_SetTestData(char *data)
{
    unsigned int loop;

    for (loop = 0; loop < MIN(sizeof(g_testData), strnlen(data, sizeof(g_testData))); loop++) {
        g_testData[loop] = data[loop];
    }
    g_isTestInited = true;
}

STATIC void LTEV_InitTestData(void)
{
    unsigned int loop;

    for (loop = 0; loop < sizeof(g_testData); loop++) {
        g_testData[loop] = loop + 1;
    }
    g_isTestInited = true;
}

STATIC int LTEV_TxOnePkt(unsigned short appDataLen)
{
    int ret;
    unsigned int copyLen = 0;
    unsigned int dataLen = appDataLen + LTEV_TEST_ACCESS_HEAD_LEN;
    unsigned int contentLen = appDataLen - sizeof(LTEV_TEST_DataReqAppHead);
    LTEV_TEST_DataReqAccesHead *pkt = NULL;
    LTEV_TEST_DataReqAppHead *appData = NULL;

    struct sk_buff *mem = dev_alloc_skb(dataLen);
    if (mem == NULL) {
        PRINT_ERR("LTEV_TxOnePkt: alloc skb mem fail\n");
        return ERR;
    }

    skb_put(mem, dataLen);
    pkt = (LTEV_TEST_DataReqAccesHead *)(mem->data);
    *pkt = g_pktHead;

    appData = (LTEV_TEST_DataReqAppHead *)(pkt->data);
    appData->seq = g_ctrl.sendSeq++;
    appData->magicNum = LTEV_DATA_MAGIC_NUM;
    while (copyLen < contentLen) {
        ret = memcpy_s(&appData->data[copyLen], contentLen - copyLen, g_testData,
            MIN(sizeof(g_testData), contentLen - copyLen));
        if (ret != EOK) {
            PRINT_ERR("TTF_PRINT_ERR: memcpy_s fail,%d\n", ret);
        }
        copyLen += MIN(sizeof(g_testData), contentLen - copyLen);
    }

    if ((g_dbgFlag & LTEV_DBG_SEQ) == LTEV_DBG_SEQ) {
        PRINT_ERR("LTEV_TxOnePkt, seq: %u., len, %d\n", appData->seq, dataLen);
    }

    g_stat.sendCntPer++;
    g_stat.sendBytesPer += dataLen;
    g_stat.totalSendCnt++;
    g_stat.totalSendBytes += dataLen;
    ret = mdrv_pc5_tx(mem);
    if (ret != 0) {
        PRINT_ERR("LTEV_TxOnePkt:mdrv_pc5_tx fail: %d\n", ret);
        return ERR;
    }

    return OK;
}

STATIC int LTEV_LoopPkt(LTEV_TEST_DataInd *dlPkt)
{
    LTEV_TEST_DataReqAccesHead *pkt = NULL;
    LTEV_TEST_DataReqAppHead *appData = NULL;
    unsigned int skbLen = dlPkt->dataLen + sizeof(LTEV_TEST_DataReqAccesHead);
    int ret;

    struct sk_buff *mem = dev_alloc_skb(skbLen);
    if (mem == NULL) {
        PRINT_ERR("LTEV_LoopPkt: alloc mem fail.\n");
        return ERR;
    }

    skb_put(mem, skbLen);
    pkt = (LTEV_TEST_DataReqAccesHead *)(mem->data);

    pkt->version = g_pktHead.version;
    (void)memcpy_s(pkt->dstAddr, sizeof(pkt->dstAddr), dlPkt->dstAddr, sizeof(dlPkt->dstAddr));
    (void)memcpy_s(pkt->srcAddr, sizeof(pkt->srcAddr), dlPkt->srcAddr, sizeof(dlPkt->srcAddr));
    pkt->priority = dlPkt->priority;
    pkt->pduType = g_pktHead.pduType;
    (void)memcpy_s(pkt->data, dlPkt->dataLen, dlPkt->data, dlPkt->dataLen);

    appData = (LTEV_TEST_DataReqAppHead *)(pkt->data);
    appData->seq = dlPkt->seq;
    appData->magicNum = LTEV_DATA_MAGIC_NUM;

    if ((g_dbgFlag & LTEV_DBG_SEQ) == LTEV_DBG_SEQ) {
        PRINT_ERR("LTEV_LoopPkt, seq: %u, len: %d.\n", appData->seq, dlPkt->dataLen);
    }

    g_stat.sendCntPer++;
    g_stat.sendBytesPer += skbLen;
    g_stat.totalSendCnt++;
    g_stat.totalSendBytes += skbLen;
    ret = mdrv_pc5_tx(mem);
    if (ret != 0) {
        PRINT_ERR("LTEV_LoopPkt:mdrv_pc5_tx fail: %d\n", ret);
        return ERR;
    }

    return OK;
}

/* seq是否满足[windLow, winHigh) */
STATIC bool LTEV_IsSeqInWindow(unsigned short seq, unsigned int winLow, unsigned int winHigh)
{
    /* seq减去下沿在上沿和下沿之间 */
    return (LTEV_TEST_MOD_SUB(seq, winLow, LTEV_TEST_SEQ_MOD) < LTEV_TEST_MOD_SUB(winHigh, winLow, LTEV_TEST_SEQ_MOD));
}

STATIC bool LTEV_IsDuplicatePacket(unsigned short seq, LTEV_TestRecvEntity *curEntity)
{
    unsigned int winLow = LTEV_TEST_MOD_SUB(curEntity->maxSeq, LTEV_TEST_WINDOW_SIZE, LTEV_TEST_SEQ_MOD);
    /* seq满足[windLow, nextSeq) 或者 seq满足(nextSeq, maxSeqseq)且x已经收到过 */
    if (LTEV_IsSeqInWindow(seq, winLow, curEntity->nextSeq) ||
        ((LTEV_IsSeqInWindow(seq, curEntity->nextSeq, curEntity->maxSeq) &&
        (seq != curEntity->nextSeq)) &&
        (LTEV_TEST_GET_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(seq)) == 1))) {
        return true;
    }
    return false;
}

STATIC unsigned int LTEV_FindNextExpectSeq(LTEV_TestRecvEntity *curEntity)
{
    unsigned int offset;
    for (offset = 0; offset < LTEV_TEST_WINDOW_SIZE; offset++) {
        if (LTEV_TEST_GET_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(curEntity->nextSeq + offset)) == 0) {
            break;
        }
    }

    return curEntity->nextSeq + offset;
}

STATIC bool LTEV_IsFirstPacket(LTEV_TestRecvEntity *curEntity)
{
    return (curEntity->nextSeq == LTEV_TEST_INVALID_SEQ || curEntity->maxSeq == LTEV_TEST_INVALID_SEQ);
}

STATIC int LTEV_CountRecvLostPacket(LTEV_TestRecvEntity *curEntity, unsigned int lastNextSeq)
{
    unsigned int idx = 0;
    unsigned int lostCnt = 0;
    unsigned int gap = LTEV_TEST_MOD_SUB(curEntity->nextSeq, lastNextSeq, LTEV_TEST_SEQ_MOD);

    if (gap > LTEV_TEST_WINDOW_SIZE) {
        for (idx = 0; idx < LTEV_TEST_WINDOW_SIZE; idx++) {
            if (LTEV_TEST_GET_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(lastNextSeq + idx)) == 0) {
                lostCnt++;
            }
        }
        lostCnt += (gap - LTEV_TEST_WINDOW_SIZE);
        curEntity->recvBitmap = 0;
    } else {
        for (idx = 0; idx < gap; idx++) {
            if (LTEV_TEST_GET_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(lastNextSeq + idx)) == 0) {
                lostCnt++;
            }
            LTEV_TEST_CLEAR_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(lastNextSeq + idx));
        }
    }

    return lostCnt;
}

STATIC unsigned int LTEV_StatRecvLostCnt(unsigned short seq, LTEV_TestRecvEntity *curEntity)
{
    unsigned int lostCnt = 0;
    unsigned int lastNextSeq = 0;
    unsigned int nextUnRcvedSeq = 0;
    unsigned int winLow = LTEV_TEST_MOD_SUB(curEntity->maxSeq, LTEV_TEST_WINDOW_SIZE, LTEV_TEST_SEQ_MOD);

    /* seq是否在重排序窗口中 */
    if (!LTEV_IsSeqInWindow(seq, winLow, curEntity->maxSeq)) {
        curEntity->maxSeq = LTEV_TEST_MOD_ADD(seq, 1, LTEV_TEST_SEQ_MOD);
    }

    winLow = LTEV_TEST_MOD_SUB(curEntity->maxSeq, LTEV_TEST_WINDOW_SIZE, LTEV_TEST_SEQ_MOD);
    /* nextSeq是否在重排序窗口中 */
    if (!LTEV_IsSeqInWindow(curEntity->nextSeq, winLow, curEntity->maxSeq)) {
        lastNextSeq = curEntity->nextSeq;
        curEntity->nextSeq = winLow;
        lostCnt += LTEV_CountRecvLostPacket(curEntity, lastNextSeq);
    }

    /* seq在窗内更新bit */
    LTEV_TEST_SET_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(seq));

    /* nextSeq是否已经收到 */
    if (LTEV_TEST_GET_BIT(curEntity->recvBitmap, LTEV_TEST_GET_BIT_POS(curEntity->nextSeq)) == 1) {
        lastNextSeq = curEntity->nextSeq;
        /* 需要查找到nextSeq之后到maxSeq之间第一个未收到的报文序号 */
        nextUnRcvedSeq = LTEV_FindNextExpectSeq(curEntity);
        curEntity->nextSeq = nextUnRcvedSeq;
        lostCnt += LTEV_CountRecvLostPacket(curEntity, lastNextSeq);
    }

    return lostCnt;
}

STATIC int LTEV_RxPkt(struct sk_buff *skb)
{
    unsigned int dataLen;
    LTEV_TEST_DataInd *pkt = NULL;
    unsigned int lostCnt = 0;
    LTEV_TestRecvEntity *curEntity = NULL;

    if (skb->len <= sizeof(LTEV_TEST_DataInd)) {
        PRINT_ERR("LTEV_RxPkt, pktLen err, %d\n", skb->len);
        return ERR;
    }

    pkt = (LTEV_TEST_DataInd *)(skb->data);
    dataLen = skb->len;
    g_stat.rcvCntPer++;
    g_stat.rcvBytesPer += dataLen;
    g_stat.totalRcvCnt++;
    g_stat.totalRcvBytes += dataLen;

    curEntity = &g_recvEntity[pkt->srcAddr[LTEV_TEST_SRC_IDX]];
    /* 报文是该用户收到的第一包 */
    if (LTEV_IsFirstPacket(curEntity)) {
        curEntity->nextSeq = pkt->seq;
        curEntity->maxSeq = pkt->seq;
    }

    /* 当前收到的是否是重复包 */
    if (LTEV_IsDuplicatePacket(pkt->seq, curEntity)) {
        g_stat.seqErrCnt++;
        g_stat.seqErrCntPer++;
    } else {
        lostCnt = LTEV_StatRecvLostCnt(pkt->seq, curEntity);
        g_stat.lostCntPer += lostCnt;
        g_stat.totalLostCnt += lostCnt;
    }

    if ((g_dbgFlag & LTEV_DBG_SEQ) == LTEV_DBG_SEQ) {
        PRINT_ERR("LTEV_RxPkt, seq: %u, len: %d.\n", pkt->seq, dataLen);
    }

    if (g_ctrl.loopFlag != 0) {
        LTEV_LoopPkt(pkt);
    }

    return OK;
}

STATIC bool g_ltevTestAlive = false;

STATIC void LTEV_Stat(struct timer_list * unused)
{
    char *unitList[] = {"bps", "kbps", "mbps"};
    int unitBase[] = {1, 1000, 1000000};
    unsigned long sendRat, RcvRat, sendCntPer, rcvCntPer, lostCntPer, rcvBytesPer, sendBytesPer, seqErrCntPer;
    unsigned int sendIdx, rcvIdx;

    if (g_ltevTestAlive == true) {
        mod_timer(&g_ctrl.statTimer, jiffies + HZ);
    }

    sendCntPer = g_stat.sendCntPer;
    g_stat.sendCntPer = 0;
    rcvCntPer = g_stat.rcvCntPer;
    g_stat.rcvCntPer = 0;
    lostCntPer = g_stat.lostCntPer;
    g_stat.lostCntPer = 0;
    rcvBytesPer = g_stat.rcvBytesPer;
    g_stat.rcvBytesPer = 0;
    sendBytesPer = g_stat.sendBytesPer;
    g_stat.sendBytesPer = 0;
    seqErrCntPer = g_stat.seqErrCntPer;
    g_stat.seqErrCntPer = 0;
    g_stat.statTime++;

    if ((g_dbgFlag & LTEV_DBG_STAT) == LTEV_DBG_STAT) {
        if ((sendCntPer > 0) || (rcvCntPer > 0) || (lostCntPer > 0)) {
            rcvIdx = (rcvBytesPer > 1250) + (rcvBytesPer > 1250000);
            RcvRat = rcvBytesPer * 8 / unitBase[rcvIdx];
            sendIdx = (sendBytesPer > 1250) + (sendBytesPer > 1250000);
            sendRat = sendBytesPer * 8 / unitBase[sendIdx];
            PRINT_ERR("Send: %u(%u%s), Rcv: %u(%u%s), Lost: %u, Err: %u\n", sendCntPer, sendRat, unitList[sendIdx],
                rcvCntPer, RcvRat, unitList[rcvIdx], lostCntPer, seqErrCntPer);
        }
    }
}

STATIC int LTEV_TimerProc(int agrv)
{
    up(&g_ctrl.taskSem);
    return OK;
}

STATIC int LTEV_TestTask(void *data)
{
    unsigned int cycleCnt;
    unsigned int sendCnt;
    timer_attr_s timerAttr = {0};
    timer_handle timer = -1;

    PRINT_ERR("LTEV Test Start, dataCnt:%d, dataLen:%d, timeVal:%d\n", g_ctrl.sendCnt, g_ctrl.dataLen, g_ctrl.timeVal);

    sema_init(&g_ctrl.taskSem, 0);

    if ((g_ctrl.dataLen == 0) && (g_ctrl.sendCnt == 0)) {
        g_ctrl.loopFlag = g_ctrl.timeVal;
        down(&g_ctrl.taskSem);
    } else {
        if ((g_ctrl.dataLen < sizeof(LTEV_TEST_DataReqAppHead)) || (g_ctrl.dataLen > LTEV_TEST_MAX_APP_DATA_LEN)) {
            g_ctrl.dataLen = DEFAULT_PKT_LEN;
        }
    }

    if (g_ctrl.timeVal != 0) {
        timerAttr.time = g_ctrl.timeVal * 1000;
        timerAttr.flag = TIMER_PERIOD;
        timerAttr.callback = LTEV_TimerProc;
        timerAttr.para = 0;
        timer = mdrv_timer_add(&timerAttr);
    }

    cycleCnt = g_ctrl.cycleCnt;
    while ((g_isRunning == true) && ((g_ctrl.cycleCnt == 0) || (cycleCnt-- > 0))) {
        for (sendCnt = 0; sendCnt < g_ctrl.sendCnt; sendCnt++) {
            LTEV_TxOnePkt(g_ctrl.dataLen);
        }
        if (g_ctrl.timeVal != 0) {
            down(&g_ctrl.taskSem);
        }
    }
    g_isRunning = false;
    if (timer >= 0) {
        mdrv_timer_delete(timer);
    }
    return 0;
}

void LTEV_StopTest(void)
{
    if (g_isRunning != true) {
        return;
    }
    g_isRunning = false;
    up(&g_ctrl.taskSem);
}


unsigned short g_ltevTestPort = 65432;
struct socket *g_ltevSock = NULL;
struct sockaddr_in g_ltevAddr = {
    .sin_family = AF_INET,
    .sin_port = 0x98ff,
    .sin_addr.s_addr = 0x6401a8c0
};
void LTEV_SetTestPort(unsigned short port)
{
    g_ltevTestPort = port;
}
void LTEV_SetAddr(char *addr, short port)
{
    g_ltevAddr.sin_family = AF_INET;
    g_ltevAddr.sin_port = htons(port);
    g_ltevAddr.sin_addr.s_addr = in_aton(addr);
}

void LTEV_SetAddrNew(unsigned addr, short port)
{
    g_ltevAddr.sin_family = AF_INET;
    g_ltevAddr.sin_port = port;
    g_ltevAddr.sin_addr.s_addr = addr;
}

int LTEV_TestServer(void *data)
{
    struct kvec iov;
    struct msghdr msg = { 0 };
    static char buf[MAX_PKT_LEN];
    int dataLen;
    struct sk_buff *mem = NULL;

    while (g_ltevTestAlive) {
        iov.iov_len = MAX_PKT_LEN;
        iov.iov_base = buf;
        if (g_ltevSock == NULL) {
            return 0;
        }
        dataLen = kernel_recvmsg(g_ltevSock, &msg, &iov, 1, MAX_PKT_LEN, 0);
        if (dataLen == -1) {
            PRINT_ERR("ltev recv msg fail\n");
            g_ltevSock = NULL;
            return 0;
        }
        mem = dev_alloc_skb(dataLen);
        if (mem == NULL) {
            PRINT_ERR("ltev alloc mem fail\n");
            continue;
        }
        skb_put(mem, dataLen);
        (void)memcpy_s(mem->data, dataLen, buf, dataLen);
        if (mdrv_pc5_tx(mem) != 0) {
            PRINT_ERR("tx pc5 pkt fail\n");
        }
    }
    PRINT_ERR("ltev test server exit\n");
    return 0;
}

STATIC struct sk_buff_head g_ltevDlQueue;
STATIC struct semaphore g_ltevDlDataSem;

int LTEV_TestRcvPc5PktProc(struct sk_buff *skb)
{
    if (skb == NULL) {
        return 0;
    }
    if (g_ltevTestAlive == false) {
        dev_kfree_skb_any(skb);
        return 0;
    }
    skb_queue_tail(&g_ltevDlQueue, skb);
    up(&g_ltevDlDataSem);
    return 0;
}

int LTEV_TestDlTask(void *data)
{
    struct msghdr msg = { 0 };
    struct kvec iov;
    int i, cnt;
    struct sk_buff *skb = NULL;

    while (g_ltevTestAlive) {
        down(&g_ltevDlDataSem);
        cnt = skb_queue_len(&g_ltevDlQueue);
        for (i = 0; i < cnt; i++) {
            skb = skb_dequeue(&g_ltevDlQueue);
            if (skb == NULL) {
                continue;
            }
            (void)LTEV_RxPkt(skb);
            iov.iov_base = skb->data;
            iov.iov_len = skb->len;
            msg.msg_name = &g_ltevAddr;
            msg.msg_namelen = sizeof(g_ltevAddr);
            msg.msg_flags |= MSG_DONTWAIT;
            if (g_ltevSock == NULL) {
                return 0;
            }
            if (kernel_sendmsg(g_ltevSock, &msg, &iov, 1, skb->len) == -1) {
                PRINT_ERR("send pc5 pkt fail\n");
            }
            dev_kfree_skb_any(skb);
        }
    }
    return 0;
}
void LTEV_TestInit(void)
{
    struct socket *sock = 0;
    struct sockaddr_in addr = { 0 };

    if (g_ltevTestAlive == true) {
        PRINT_ERR("ltev test already init\n");
        return;
    }

    skb_queue_head_init(&g_ltevDlQueue);
    sema_init(&g_ltevDlDataSem, 0);
    g_ltevTestAlive = true;
    (void)memset_s(&g_stat, sizeof(g_stat), 0, sizeof(g_stat));
    timer_setup(&g_ctrl.statTimer, LTEV_Stat, 0);
    g_ctrl.statTimer.expires = jiffies + HZ;
    add_timer(&g_ctrl.statTimer);
    LTEV_InitRecvEntity();

    if (sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock) < 0) {
        PRINT_ERR("create sock fail\n");
        return;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(g_ltevTestPort);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (kernel_bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        PRINT_ERR("bind sock fail\n");
        sock_release(sock);
        return;
    }
    g_ltevSock = sock;

    if (mdrv_pc5_cb_register(LTEV_TestRcvPc5PktProc) != 0) {
        PRINT_ERR("reg rcv func fail\n");
    }

    (void)kthread_run(LTEV_TestServer, NULL, "LtevTestServer");
    (void)kthread_run(LTEV_TestDlTask, NULL, "LtevTestDlTask");
}

void LTEV_TestDeInit(void)
{
    if (g_ltevTestAlive == true) {
        g_ltevTestAlive = false;
        del_timer(&g_ctrl.statTimer);
        up(&g_ltevDlDataSem);
        sock_release(g_ltevSock);
        g_ltevSock = NULL;
    }
}

void LTEV_TestClearStat(void)
{
    (void)memset_s(&g_stat, sizeof(g_stat), 0, sizeof(g_stat));
    LTEV_InitRecvEntity();
}

void LTEV_TestShowStat(void)
{
    char *unitList[] = {"bps", "kbps", "mbps"};
    int unitBase[] = {1, 1000, 1000000};
    int sendIdx, rcvIdx;
    unsigned long sendRat, RcvRat, statTime;

    statTime = (g_stat.statTime == 0) ? 1 : g_stat.statTime;
    rcvIdx = (g_stat.totalRcvBytes / statTime > 1250) + (g_stat.totalRcvBytes / statTime > 1250000);
    RcvRat = g_stat.totalRcvBytes * 8 / unitBase[rcvIdx] / statTime;
    sendIdx = (g_stat.totalSendBytes / statTime > 1250) + (g_stat.totalSendBytes / statTime > 1250000);
    sendRat = g_stat.totalSendBytes * 8 / unitBase[sendIdx] / statTime;
    PRINT_ERR("LTEV Test Stat, Send:%u(%u%s), Rcv:%u(%u%s), Lost:%u, Err:%u\n", g_stat.totalSendCnt, sendRat,
        unitList[sendIdx], g_stat.totalRcvCnt, RcvRat, unitList[rcvIdx], g_stat.totalLostCnt, g_stat.seqErrCnt);
}

int LTEV_StartTest(unsigned int dataCnt, unsigned short dataLen, unsigned short timeVal, unsigned int cycleCnt)
{
    if (g_isRunning == true) {
        PRINT_ERR("LTEV_Test: is running");
        return ERR;
    }

    if (g_isTestInited == false) {
        LTEV_InitTestData();
    }

    if (g_ltevTestAlive == false) {
        LTEV_TestInit();
    }

    g_isRunning = true;

    g_ctrl.timeVal = timeVal;
    g_ctrl.sendCnt = dataCnt;
    g_ctrl.dataLen = dataLen;
    g_ctrl.cycleCnt = cycleCnt;

    (void)kthread_run(LTEV_TestTask, NULL, "ltevTest");

    return OK;
}

unsigned short LTEV_GetVersion(void)
{
    PRINT_ERR("LTEV_GetVersion, version =  %d\n", VPS_TEST_VERSION);
    return VPS_TEST_VERSION;
}
