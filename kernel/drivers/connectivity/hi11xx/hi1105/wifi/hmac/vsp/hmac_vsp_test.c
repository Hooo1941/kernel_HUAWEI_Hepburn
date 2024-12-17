/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明   : vsp模拟发包
 * 作者       : wifi
 * 创建日期   : 2021年08月27日
 */

#include "hmac_vsp_test.h"
#include "hmac_vsp_source.h"
#include "hmac_vsp_sink.h"
#include "pcie_linux.h"
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
#include "oal_kernel_file.h"
#endif

#undef THIS_FILE_ID
#define THIS_FILE_ID OAM_FILE_ID_HMAC_VSP_TEST_C

#ifdef _PRE_WLAN_FEATURE_VSP
#define VSP_FIX_PACKET_LEN (VSP_MSDU_CB_LEN + 1400)
#define VSP_MAX_WAIT_SLICES_IN_Q 200
static struct {
    uint8_t curr_frame;
    uint8_t curr_slice;
    uint32_t frm_cnt;
    uint8_t slice_num;
    uint8_t layer_num;
    uint32_t pkt_num[MAX_LAYER_NUM];
    uint32_t slice_interval;
    uint8_t ra[WLAN_MAC_ADDR_LEN], sa[WLAN_MAC_ADDR_LEN];
    struct task_struct *test_loop;
} g_vsp_test_hdl;

static rx_slice_mgmt *alloc_vdec_slice_buffer_test(uint32_t size);
static void wifi_rx_slice_done_test(rx_slice_mgmt *rx_slice);
static void wifi_tx_pkg_done_test(send_result *send_result);
static const hmac_vsp_vcodec_ops test_ops = {
    .alloc_slice_mgmt = alloc_vdec_slice_buffer_test,
    .rx_slice_done = wifi_rx_slice_done_test,
    .wifi_tx_pkg_done = wifi_tx_pkg_done_test,
};

void vsp_free_layer_list(tx_layer_ctrl *head)
{
#if _PRE_OS_VERSION_LINUX == _PRE_OS_VERSION
    tx_layer_ctrl *curr = NULL;
    tx_layer_ctrl *next = NULL;
    oal_pci_dev_stru *pcie_dev = oal_get_wifi_pcie_dev();
    if (pcie_dev == NULL) {
        oam_error_log0(0, 0, "{hmac_tx_dma_map_phyaddr::pcie_dev NULL}");
        return;
    }
    curr = head;
    while (curr != NULL) {
        next = (tx_layer_ctrl *)curr->next;
        dma_free_coherent(&pcie_dev->dev, VSP_FIX_PACKET_LEN * curr->paket_number,
            (void *)(uintptr_t)(curr->data_addr), curr->iova_addr);
        oal_free(curr);
        curr = next;
    }
#endif
}

static void vsp_test_encap_packet(uint8_t *pkt, uint16_t len, uint16_t pkt_num, uint8_t layer_num, bool is_last)
{
    vsp_msdu_hdr_stru *head = (vsp_msdu_hdr_stru *)(pkt + VSP_MSDU_CB_LEN);
    oal_set_mac_addr(head->ra, g_vsp_test_hdl.ra);
    oal_set_mac_addr(head->sa, g_vsp_test_hdl.sa);
    head->type = oal_host2net_short(ETHER_TYPE_IP);
    head->frame_id = g_vsp_test_hdl.curr_frame;
    head->slice_num = g_vsp_test_hdl.curr_slice;
    head->layer_num = layer_num;
    head->number = pkt_num;
    head->ts_type = VSP_TIMESTAMP_NOT_USED;
    head->len = len - sizeof(vsp_msdu_hdr_stru) - VSP_MSDU_CB_LEN;
    head->last = is_last;
}

static tx_layer_ctrl *vsp_test_prepare_layer(uint8_t layer_id)
{
#if _PRE_OS_VERSION_LINUX == _PRE_OS_VERSION
    tx_layer_ctrl *layer = NULL;
    uint16_t i;
    uint32_t pkt_num;
    oal_pci_dev_stru *pcie_dev = NULL;
    layer = (tx_layer_ctrl *)oal_memalloc(sizeof(tx_layer_ctrl));
    if (layer == NULL) {
        oam_error_log0(0, 0, "{vsp_test_prepare_layer::alloc tx_layer_ctrl failed}");
        return NULL;
    }
    pcie_dev = oal_get_wifi_pcie_dev();
    if (pcie_dev == NULL) {
        oam_error_log0(0, 0, "{hmac_tx_dma_map_phyaddr::pcie_dev NULL}");
        oal_free(layer);
        return NULL;
    }
    pkt_num = g_vsp_test_hdl.pkt_num[layer_id];
    layer->data_addr = (uintptr_t)dma_alloc_coherent(&pcie_dev->dev, VSP_FIX_PACKET_LEN * pkt_num,
        &layer->iova_addr, GFP_KERNEL);
    if (!layer->data_addr) {
        oam_error_log0(0, 0, "{vsp_test_prepare_layer::data alloc failed}");
        oal_free(layer);
        return NULL;
    }

    oal_set_mac_addr(layer->mac_ra_address, g_vsp_test_hdl.ra);
    oal_set_mac_addr(layer->mac_sa_address, g_vsp_test_hdl.sa);
    layer->qos_type = (g_vsp_test_hdl.curr_frame << BIT_OFFSET_8) | (g_vsp_test_hdl.curr_slice << BIT_OFFSET_4);
    layer->layer_number = layer_id;
    layer->normal_pack_length = VSP_FIX_PACKET_LEN;
    layer->last_paket_len = VSP_FIX_PACKET_LEN;
    layer->paket_number = pkt_num;
    for (i = 0; i < pkt_num; i++) {
        vsp_test_encap_packet((uint8_t *)(layer->data_addr + i * VSP_FIX_PACKET_LEN), VSP_FIX_PACKET_LEN, i, layer_id,
            i == pkt_num - 1);
    }
    return layer;
#else
    return NULL;
#endif
}

static tx_layer_ctrl *vsp_test_prepare_slice(void)
{
    uint8_t layer_id;
    tx_layer_ctrl *tmp = NULL;
    tx_layer_ctrl *curr, head;
    curr = &head;
    curr->next = 0;
    for (layer_id = 0; layer_id < g_vsp_test_hdl.layer_num; layer_id++) {
        tmp = vsp_test_prepare_layer(layer_id);
        if (tmp == NULL) {
            vsp_free_layer_list((tx_layer_ctrl *)head.next);
            return NULL;
        }
        tmp->next = 0;
        curr->next = (uintptr_t)tmp;
        curr = tmp;
    }

    if (++g_vsp_test_hdl.curr_slice >= g_vsp_test_hdl.slice_num) {
        g_vsp_test_hdl.curr_frame = (g_vsp_test_hdl.curr_frame + 1) & VSP_FRAME_ID_MASK;
        g_vsp_test_hdl.curr_slice = 0;
    }
    return (tx_layer_ctrl *)head.next;
}

static void wifi_tx_pkg_done_test(send_result *send_result)
{
    tx_layer_ctrl *head = NULL;
    if (send_result->slice_layer == NULL) {
        oam_error_log0(0, 0, "{wifi_tx_pkg_done_test::slice is null!}");
        return;
    }
    head = send_result->slice_layer;
    vsp_free_layer_list(head);
}

static uint8_t vsp_test_generate_slice_allowed(void)
{
    hmac_vsp_source_stru *source = (hmac_vsp_source_stru *)hmac_vsp_get_current_handle();

    return hmac_vsp_slice_queue_len(&source->free_queue) < 2; // 2: min free queue length
}

static int vsp_test_send_loop(void *data)
{
    int64_t slice_cnt = g_vsp_test_hdl.frm_cnt * g_vsp_test_hdl.slice_num;
    tx_layer_ctrl *slice = NULL;

    oam_warning_log1(0, 0, "{vsp_test_send_loop::enter, slice cnt %d}", slice_cnt);
    while (!oal_kthread_should_stop()) {
        if (!vsp_test_generate_slice_allowed()) {
            oal_udelay(500); // delay 500us
            continue;
        }

        if (slice_cnt-- <= 0) {
            oam_warning_log0(0, 0, "{vsp_test_send_loop::send slices finished!}");
            break;
        }

        slice = vsp_test_prepare_slice();
        if (slice == NULL) {
            oam_error_log0(0, 0, "{vsp_test_send_loop::prepare slice failed!}");
            break;
        }
        if (wifi_tx_venc_pkg(slice) == OAL_FALSE) {
            oam_error_log0(0, 0, "{vsp_test_send_loop::tx slice fialed!}");
            vsp_free_layer_list(slice);
        }
        // sleep: min[interval-500us] max[interval]
        oal_usleep_range(g_vsp_test_hdl.slice_interval - 500, g_vsp_test_hdl.slice_interval);
    }
    g_vsp_test_hdl.test_loop = NULL;
    oam_warning_log1(0, 0, "{vsp_test_send_loop::exit, slice cnt %d}", slice_cnt);
    return 0;
}

uint32_t hmac_vsp_test_stop(void)
{
    if (!g_vsp_test_hdl.test_loop) {
        oam_warning_log0(0, 0, "{hmac_vsp_test_stop::test loop not start yet!}");
        return OAL_FAIL;
    }

    oal_thread_stop(g_vsp_test_hdl.test_loop, NULL);
    g_vsp_test_hdl.test_loop = NULL;
    oam_warning_log0(0, 0, "{hmac_vsp_test_stop::test loop stop succ!}");

    return OAL_SUCC;
}

static void vsp_set_packet_num(uint32_t base_pkt_num)
{
    int i;
    g_vsp_test_hdl.pkt_num[0] = base_pkt_num;
    for (i = 1; i < MAX_LAYER_NUM; i++) {
        g_vsp_test_hdl.pkt_num[i] = g_vsp_test_hdl.pkt_num[i - 1] + 10; // 10: 每个layer +10pkt
    }
}

static uint32_t vsp_test_init_param(void)
{
    hmac_vap_stru *vap = NULL;
    hmac_user_stru *user = NULL;
    hmac_vsp_info_stru *vsp_info = hmac_vsp_get_current_handle();
    if (!vsp_info->enable) {
        oam_error_log0(0, 0, "{hmac_vsp_test_start::enable vsp first!}");
        return OAL_FAIL;
    }
    g_vsp_test_hdl.curr_frame = 0;
    g_vsp_test_hdl.curr_slice = 0;
    g_vsp_test_hdl.slice_num = vsp_info->param.slice_cnt;
    g_vsp_test_hdl.layer_num = vsp_info->param.layer_cnt;
    g_vsp_test_hdl.slice_interval = vsp_info->param.tslice;
    if (g_vsp_test_hdl.pkt_num[0] == 0) {
        vsp_set_packet_num(30); // 30: layer0 pkt num
    }
    oam_warning_log3(0, 0, "{vsp_test_init_param::slice %d per frm, layer %d, slice interval %d}",
        g_vsp_test_hdl.slice_num, g_vsp_test_hdl.layer_num, g_vsp_test_hdl.slice_interval);
    user = vsp_info->hmac_user;
    vap = vsp_info->hmac_vap;
    if (user == NULL || vap == NULL) {
        oam_error_log0(0, 0, "{hmac_vsp_test_start::user or vap is null!}");
        return OAL_FAIL;
    }
    oal_set_mac_addr(g_vsp_test_hdl.ra, user->st_user_base_info.auc_user_mac_addr);
    oal_set_mac_addr(g_vsp_test_hdl.sa, mac_mib_get_StationID(&vap->st_vap_base_info));
    return OAL_SUCC;
}

static void hmac_vsp_test_thread_set_fast_cpus(void)
{
#if defined(CONFIG_ARCH_HISI) && defined(CONFIG_NR_CPUS)
#if CONFIG_NR_CPUS > OAL_BUS_HPCPU_NUM
    struct cpumask fast_cpus;
    external_get_fast_cpus(&fast_cpus);
    oal_cpumask_clear_cpu(cpumask_first(&fast_cpus), &fast_cpus);
    set_cpus_allowed_ptr(g_vsp_test_hdl.test_loop, &fast_cpus);
#endif
#endif
}

uint32_t hmac_vsp_test_start_source(uint32_t frame_cnt)
{
    g_vsp_test_hdl.frm_cnt = frame_cnt;
    if (g_vsp_test_hdl.test_loop != NULL) {
        oam_error_log0(0, 0, "{hmac_vsp_test_start::test is sill running, stop and try again}");
        return OAL_FAIL;
    }

    if (vsp_test_init_param() != OAL_SUCC) {
        return OAL_FAIL;
    }

    hmac_vsp_set_vcodec_ops(&test_ops);

    g_vsp_test_hdl.test_loop = oal_thread_create(vsp_test_send_loop, NULL, NULL, "vsp_test_loop", SCHED_FIFO,
                                                 99,  // 99: priority
                                                 -1); // -1: cpuid(not bind)
    if (g_vsp_test_hdl.test_loop == NULL) {
        oam_error_log0(0, 0, "{hmac_vsp_test_start::start test loop failed!}");
        return OAL_FAIL;
    }

    hmac_vsp_test_thread_set_fast_cpus();
    oam_warning_log1(0, 0, "{hmac_vsp_test_start::start test loop succ, frame cnt %d!}", frame_cnt);

    return OAL_SUCC;
}

uint32_t hmac_vsp_test_start_sink(void)
{
    hmac_vsp_set_vcodec_ops(&test_ops);

    return OAL_SUCC;
}

uint32_t hmac_vsp_test_start(hmac_vsp_debug_cmd *cmd)
{
    hmac_vsp_info_stru *vsp_info = hmac_vsp_get_current_handle();

    if (!vsp_info || !vsp_info->enable) {
        oam_error_log0(0, 0, "{hmac_vsp_test_start::vsp not inited!}");
        return OAL_FAIL;
    }

    if (vsp_info->mode == VSP_MODE_SINK) {
        return hmac_vsp_test_start_sink();
    } else if (vsp_info->mode == VSP_MODE_SOURCE) {
        return hmac_vsp_test_start_source(cmd->test_frm_cnt);
    }

    oam_error_log1(0, 0, "{hmac_vsp_test_start::mode[%d] error}", vsp_info->mode);

    return OAL_FAIL;
}

static rx_slice_mgmt *alloc_vdec_slice_buffer_test(uint32_t size)
{
    return oal_memalloc(sizeof(rx_slice_mgmt));
}

static void wifi_rx_slice_done_test(rx_slice_mgmt *rx_slice)
{
    vdec_rx_slice_done(rx_slice);
    oal_free(rx_slice);
}

/* sysfs */
#ifdef _PRE_CONFIG_CONN_HISI_SYSFS_SUPPORT
OAL_STATIC ssize_t hmac_set_vsp_pkt_num(struct kobject *dev, struct kobj_attribute *attr,
    const char *buf, size_t count)
{
    uint32_t pkt_num;

    if (buf == NULL) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (attr == NULL) {
        oal_io_print("attr is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (dev == NULL) {
        oal_io_print("dev is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    if ((sscanf_s(buf, "%u", &pkt_num) != 1)) {
        oal_io_print("set value one char!\n");
        return -OAL_EINVAL;
    }

    vsp_set_packet_num(pkt_num);
    return count;
}

OAL_STATIC ssize_t hmac_vsp_show_pkt_num(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    int ret = 0;
    if (oal_any_null_ptr3(buf, attr, dev)) {
        oal_io_print("buf is null r failed!%s\n", __FUNCTION__);
        return 0;
    }

    ret = snprintf_s(buf, PAGE_SIZE, PAGE_SIZE - 1,
        "base layer pkt num %u\n", g_vsp_test_hdl.pkt_num[0]);
    if (ret < 0) {
        return 0;
    }
    return ret;
}

static struct kobj_attribute g_vsp_attr_pkt_num =
    __ATTR(pkt_num, S_IRUGO | S_IWUSR, hmac_vsp_show_pkt_num, hmac_set_vsp_pkt_num);

OAL_STATIC int32_t hmac_vsp_sink_stat_format(hmac_vsp_sink_stat_stru *stat, char *buf, int32_t buf_len)
{
    int ret;
    int32_t count = 0;
    uint32_t i, slice_total, partial;
    uint64_t airtime;
    slice_total = hmac_vsp_stat_get_slice_total(&stat->rate);
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
        "==========================[vsp sink stat]==========================\n"
        "rx slice : total %u, %u slices/s\n", slice_total, hmac_vsp_stat_get_slice_rate(&stat->rate));
    if (ret < 0) {
        return count;
    }
    count += ret;

    if (slice_total) {
        for (i = 0; i < MAX_LAYER_NUM; i++) {
            partial = hmac_vsp_sink_stat_get_rx_succ_layer(stat, i);
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "rx slice : [%u]layers %u (%u%%)\n",
                i, partial, partial * 100 / slice_total); // 100: 百分比
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
        for (i = 0; i < MAX_SLICE_NUM; i++) {
            airtime = hmac_vsp_stat_get_airtime(&stat->rate, i);
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "rx slice[%u]: avg airtime[%u]\n",
                i, do_div(airtime, slice_total));
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
        "rx bytes : %llu bytes, rate %u kbps\nrx recycle : %u\n", hmac_vsp_stat_get_bytes_total(&stat->rate),
        hmac_vsp_stat_get_data_rate(&stat->rate), hmac_vsp_sink_stat_get_netbuf_recycle_cnt(stat));
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

OAL_STATIC int32_t hmac_vsp_source_stat_format(hmac_vsp_source_stat_stru *stat, char *buf, int32_t buf_len)
{
    int ret;
    int32_t count = 0;
    uint32_t i, slice_total, partial;
    slice_total = hmac_vsp_stat_get_slice_total(&stat->rate);
    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
        "==========================[vsp source stat]==========================\n"
        "tx slice : total %u, %u slices/s\ntx slice : feedback %u\n",
        slice_total, hmac_vsp_stat_get_slice_rate(&stat->rate), hmac_vsp_source_stat_get_feedback_cnt(stat));
    if (ret < 0) {
        return count;
    }
    count += ret;

    if (slice_total) {
        for (i = 0; i < MAX_LAYER_NUM; i++) {
            partial = hmac_vsp_source_stat_get_rx_succ_feedback(stat, i);
            ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1, "tx slice : [%u]layers %u (%u%%)\n",
                i, partial, partial * 100 / slice_total); // 100: 百分比
            if (ret < 0) {
                return count;
            }
            count += ret;
        }
    }

    ret = snprintf_s(buf + count, buf_len - count, buf_len - count - 1,
        "tx bytes : %llu bytes, rate %u kbps\n", stat->rate.bytes_total, stat->rate.data_rate);
    if (ret < 0) {
        return count;
    }
    count += ret;

    return count;
}

OAL_STATIC ssize_t hmac_vsp_show_stat(struct kobject *dev, struct kobj_attribute *attr, char *buf)
{
    hmac_vsp_info_stru *vsp_info = hmac_vsp_get_current_handle();

    if (!buf || !vsp_info || !vsp_info->enable) {
        oal_io_print("hmac_vsp_show_stat failed!%s\n", __FUNCTION__);
        return 0;
    }

    if (vsp_info->mode == VSP_MODE_SINK) {
        return hmac_vsp_sink_stat_format(&((hmac_vsp_sink_stru *)vsp_info)->sink_stat, buf, PAGE_SIZE);
    } else if (vsp_info->mode == VSP_MODE_SOURCE) {
        return hmac_vsp_source_stat_format(&((hmac_vsp_source_stru *)vsp_info)->source_stat, buf, PAGE_SIZE);
    } else {
        oal_io_print("hmac_vsp_show_stat mode[%d] failed!%s\n", vsp_info->mode, __FUNCTION__);
        return OAL_FAIL;
    }
}

static struct kobj_attribute g_vsp_attr_rx_stat =
    __ATTR(stat, S_IRUGO, hmac_vsp_show_stat, NULL);

static struct attribute *g_vsp_sysfs_entries[] = {
    &g_vsp_attr_rx_stat.attr,
    &g_vsp_attr_pkt_num.attr,
    NULL
};

static struct attribute_group g_vsp_attribute_group = {
    .attrs = g_vsp_sysfs_entries,
};

static oal_kobject *g_conn_syfs_vsp_object = NULL;

void hmac_vsp_init_sysfs(void)
{
    int32_t ret;
    oal_kobject *hisys_root = oal_get_sysfs_root_object();
    if (hisys_root == NULL) {
        oal_io_print("[E]get root sysfs object failed!\n");
        return;
    }

    g_conn_syfs_vsp_object = kobject_create_and_add("vsp", hisys_root);
    if (g_conn_syfs_vsp_object == NULL) {
        return;
    }

    ret = oal_debug_sysfs_create_group(g_conn_syfs_vsp_object, &g_vsp_attribute_group);
    if (ret) {
        kobject_put(g_conn_syfs_vsp_object);
        oal_io_print("sysfs create vsp group fail.ret=%d\n", ret);
        return;
    }
}

void hmac_vsp_deinit_sysfs(void)
{
    oal_debug_sysfs_remove_group(g_conn_syfs_vsp_object, &g_vsp_attribute_group);
    kobject_put(g_conn_syfs_vsp_object);
}
#endif
#endif
