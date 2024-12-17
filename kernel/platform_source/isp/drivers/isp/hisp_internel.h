/*
 * hisp_internel.h
 *
 * Copyright (c) 2013 ISP Technologies CO., Ltd.
 *
 */

#ifndef _HISP_INTERNEL_H
#define _HISP_INTERNEL_H

#include <linux/platform_device.h>
#include <linux/remoteproc.h>
#include <linux/pci.h>
#include <uapi/linux/virtio_types.h>
#include <platform_include/isp/linux/hisp_remoteproc.h>
#include <linux/rpmsg.h>

#if defined(HISP_VERSION_V500)
#include <platform_include/isp/linux/v500/hisp_msg_handle.h>
#else
#endif

#define ISP_DYNBOOT_ENABLE 1

#define ISP_NORMAL_MODE 0
#define ISP_PCIE_MODE   1

#define ISP_MAX_NUM_MAGIC            (0xFFFFFFFF)
#define DUMP_ISP_BOOT_SIZE           (64)
#define ISP_CPU_POWER_DOWN           (1 << 7)

enum hisp_rdr_enable_mask
{
	RDR_LOG_TRACE       = 0x1,
	RDR_LOG_LAST_WORD   = 0x2,
	RDR_LOG_ALGO        = 0x4,
	RDR_LOG_CVDR        = 0x8,
	RDR_LOG_IRQ         = 0x10,
	RDR_LOG_TASK        = 0x20,
	RDR_LOG_CPUP        = 0x40,
	RDR_LOG_MONITOR     = 0x80,
};

#define ISPCPU_RDR_USE_APCTRL        (1UL << 31)
#define ISPCPU_DEFAULT_RDR_LEVEL     (RDR_LOG_TRACE | RDR_LOG_LAST_WORD)

#define ISPCPU_LOG_USE_APCTRL        (1UL << 31)
#define ISPCPU_LOG_TIMESTAMP_FPGAMOD (1 << 30)

#define IRQ_NUM                      (64)
#define MONITOR_CHAN_NUM             4
#define ISP_MEM_MAX_SIZE             64

#define VIRTIO_ID_HISP_RPMSG            31

#define SEC_TYPE_INVALID 0
#define SEC_TYPE_VALID   1

struct level_switch_s {
	unsigned int level;
	char enable_cmd[8];
	char disable_cmd[8];
	char info[32];
};

enum rpmsg_client_choice {
	ISP_DEBUG_RPMSG_CLIENT  = 0x1111,
	INVALID_CLIENT          = 0xFFFF,
};

enum hisp_vring_e {
	HISP_VRING0          = 0x0,
	HISP_VRING1,
	HISP_VRING_MAX,
};

enum hisp_sec_boot_mem_type {
	HISP_SEC_TEXT       = 0,
	HISP_SEC_DATA       = 1,
	HISP_SEC_BOOT_MAX_TYPE
};

enum hisp_sec_mem_pool_type {
	HISP_DYNAMIC_POOL        = 0,
	HISP_SEC_POOL            = 1,
	HISP_ISPSEC_POOL         = 2,
	HISP_SEC_POOL_MAX_TYPE
};

enum hisp_sec_rsv_mem_type {
	HISP_SEC_VR0       = 0,
	HISP_SEC_VR1       = 1,
	HISP_SEC_VQ        = 2,
	HISP_SEC_SHARE     = 3,
	HISP_SEC_RDR       = 4,
	HISP_SEC_RSV_MAX_TYPE
};


enum hisp_mem_status_type {
	HISP_MEM_UNUSED   = 0,
	HISP_MEM_MAP      = 1,
	HISP_MEM_MAX_TYPE
};

struct isp_plat_cfg {
	unsigned int platform_id;
	unsigned int isp_local_timer;
	unsigned int perf_power;
};

struct bw_boot_status {
	unsigned int entry:1;
	unsigned int invalid_tlb:1;
	unsigned int enable_mmu:1;
	unsigned int reserved:29;
};

struct fw_boot_status {
	unsigned int entry:1;
	unsigned int hard_boot_init:1;
	unsigned int hard_drv_init:1;
	unsigned int app_init:1;
	unsigned int reserved:28;
};

union hisp_mem_usage_vote {
	struct {
		unsigned int isp_cpu_use : 1;
		unsigned int kernel_use  : 1;
	} bits;
	unsigned int usage_vote;
};

#ifdef ISP_LINX_CPU_SUPPORT
struct hisp_mem_para {
	unsigned int index;
	unsigned int offset;
	unsigned int mem_info;
	union hisp_mem_usage_vote use_vote;
	unsigned long long cpu_addr;        /* record kernel r8 map addr */
	unsigned long long cpu_size;        /* record kernel r8 map size */
	unsigned int core_addr;             /* record kernel core map addr */
	unsigned int core_size;             /* record kernel core map size */
	unsigned int oisp_addr;             /* record kernel oisp map addr */
	unsigned int oisp_size;             /* record kernel oisp map size */
	unsigned int prot;
	unsigned int state;
};
#else
struct hisp_mem_para {
	unsigned int type;
	unsigned int da;            /* record share map addr */
	unsigned int size;          /* record share map addr */
#ifdef ISP_BUF_PRIVATIZE
	unsigned int r8_addr;       /* record kernel r8 map addr */
	unsigned int r8_size;       /* record kernel r8 map size */
	unsigned int r8_core_addr;  /* record kernel r8_core map addr */
	unsigned int r8_core_size;  /* record kernel r8_core map size */
	unsigned int ispfw_addr;
	unsigned int ispfw_size;
#endif
#ifdef CONFIG_CAMERA_ISPV230
	unsigned int ispcpu_addr;   /* record kernel r8 map addr */
	unsigned int ispcpu_size;   /* record kernel r8 map size */
#endif
	unsigned int prot;
	unsigned int state;
};
#endif

struct hisp_shared_para {
	struct isp_plat_cfg plat_cfg;
	int rdr_enable;
	unsigned int rdr_enable_type;
	unsigned char irq[IRQ_NUM];
	int log_flush_flag;
	unsigned int log_head_size;
	unsigned int log_cache_write;
	struct bw_boot_status bw_stat;
	struct fw_boot_status fw_stat;
	unsigned int logx_switch;
	u64 bootware_paddr;
	u64 dynamic_pgtable_base;
	unsigned long long  coresight_addr_da;
	unsigned long long  coresight_addr_vir;
	unsigned int perf_switch;
	u64 perf_addr;
	u32 coredump_addr;
	u32 exc_flag; /* bit 0:exc cur ;bit 1:ion flag ; bit 2:rtos dump over  bit3:handle over */
	u64 mdc_pa_addr;
	unsigned int monitor_addr[MONITOR_CHAN_NUM];
	unsigned int monitor_ctrl;
	unsigned int monitor_pa_va;
	unsigned int monitor_hit_flag;
	unsigned int clk_value[ISP_CLK_MAX];
	unsigned char isp_efuse;
	unsigned int chip_flag;
	struct hisp_mem_para hisp_mem_record[ISP_MEM_MAX_SIZE];
#ifdef HISP_FUZZ
	int kcov_enable;
	int fuzz_excep_flag;
#endif
};

struct isp_a7mapping_s {
	unsigned int a7va;
	unsigned int size;
	unsigned int prot;
	unsigned int offset;
	unsigned int reserve;
	unsigned long a7pa;
	void *apva;
};

struct hisp_ca_meminfo {
	unsigned int type;
	unsigned int da;
	unsigned int size;
	unsigned int prot;
	unsigned int sec_flag; /* SEC or NESC */
	int sharefd;
	u64 pa;
};

struct rpmsg_hdr {
	__virtio32 src;
	__virtio32 dst;
	__virtio32 reserved;
	__virtio16 len;
	__virtio16 flags;
	u8 data[];
} __packed;

struct hisp_msg_info {
	unsigned int message_size;
	unsigned int api_name;
	unsigned int message_id;
	unsigned int message_hash;
};
#ifdef DEBUG_HISP
struct isp_pcie_cfg {
	struct pci_dev *isp_pci_dev;
	void __iomem *isp_pci_reg;
	unsigned int isp_pci_addr;
	unsigned int isp_pci_size;
	unsigned int isp_pci_irq;
	unsigned int isp_pci_inuse_flag;
	unsigned int isp_pci_ready;
};
#endif

/* hisp rproc api */
int hisp_rproc_set_domain(struct rproc *rproc,
				struct iommu_domain *domain);
struct iommu_domain *hisp_rproc_get_domain(struct rproc *rproc);
void hisp_rproc_clr_domain(struct rproc *rproc);
void *hisp_rproc_get_imgva(struct rproc *rproc);
void hisp_rproc_get_vqmem(struct rproc *rproc, u64 *pa,
		size_t *size, u32 *flags);
u64 hisp_rproc_get_vringmem_pa(struct rproc *rproc, int id);
void hisp_rproc_set_rpmsg_ready(struct rproc *rproc);
void *hisp_vqmem_info_get(struct virtio_device *vdev,
dma_addr_t *dma_handle, size_t size);
#ifdef ISP_LINX_CPU_SUPPORT
int hisp_reg_space_memmap(struct rproc *rproc);
int hisp_reg_space_unmap(struct rproc *rproc);
#endif
/* hisp smc api in hisp_smc.c */
int hisp_smc_setparams(u64 shrd_paddr);
int hisp_smc_cpuinfo_dump(u64 param_pa_addr);
void hisp_smc_isp_init(u64 pgd_base);
void hisp_smc_isp_exit(void);
int hisp_smc_ispcpu_init(void);
void hisp_smc_ispcpu_exit(void);
int hisp_smc_ispcpu_map(void);
void hisp_smc_ispcpu_unmap(void);
void hisp_smc_set_nonsec(void);
int hisp_smc_disreset_ispcpu(void);
void hisp_smc_ispsmmu_ns_init(u64 pgt_addr);
int hisp_smc_get_ispcpu_idle(void);
int hisp_smc_send_fiq2ispcpu(void);
int hisp_smc_isptop_power_up(void);
int hisp_smc_isptop_power_down(void);
int hisp_smc_phy_csi_connect(u64 info_addr);
int hisp_smc_phy_csi_disconnect(void);
int hisp_smc_qos_cfg(void);
int hisp_smc_csi_mode_sel(u64 mode);
int hisp_smc_i2c_pad_type(void);
int hisp_smc_i2c_deadlock(u64 type);

/* Hisp sp func start */
#ifdef CONFIG_HISP_FFA_SUPPORT
int hisp_ffa_disreset_ispcpu(unsigned int canary, unsigned int dynboot);
int hisp_ffa_powerup_isptop(void);
int hisp_ffa_powerdn_isptop(void);
int hisp_ffa_connect_phy_csi(u32 phy_id, u32 phy_work_mode, u32 csi_idx);
int hisp_ffa_disconnect_phy_csi(void);
int hisp_ffa_send_fiq(void);
int hisp_ffa_cfg_qos(void);
void hisp_ffa_isp_ecc(void);
unsigned int hisp_ffa_clr_memecc_intr(unsigned int type);
int hisp_ffa_ispmmu_init(u64 pgd_base);
#else
static inline void hisp_ffa_isp_ecc(void) { return; }
static inline unsigned int hisp_ffa_clr_memecc_intr(unsigned int type __attribute__((__unused__))) { return 0; }
#endif
/* Hisp sp func end */

#ifdef DEBUG_HISP
int hisp_smc_ispnoc_r8_power_up(u64 pcie_flag, u64 pcie_addr);
int hisp_smc_ispnoc_r8_power_down(u64 pcie_flag, u64 pcie_addr);
int hisp_smc_media2_vcodec_power_up(u64 pcie_flag, u64 pcie_addr);
int hisp_smc_media2_vcodec_power_down(u64 pcie_flag, u64 pcie_addr);
int hisp_ffa_media2_power_up(void);
int hisp_ffa_media2_power_down(void);
#endif

/* hisp dpm api in hisp_dpm.c */
#ifdef CONFIG_HISP_DPM
void hisp_dpm_probe(void);
void hisp_dpm_release(void);
#endif

/* hisp fstcma api in hisp_cma.c */
void *hisp_fstcma_alloc(dma_addr_t *dma_handle, size_t size, gfp_t flag);
void hisp_fstcma_free(void *va, dma_addr_t dma_handle, size_t size);

/* hisp dump api in hisp_dump.c */
void hisp_boot_stat_dump(void);
int hisp_mntn_dumpregs(void);

/* hisp ca api in hisp_ca.c */
int hisp_ca_ta_open(void);
int hisp_ca_ta_close(void);
int hisp_ca_ispcpu_disreset(void);
int hisp_ca_ispcpu_reset(void);
int hisp_ca_sfdmem_map(struct hisp_ca_meminfo *buffer);
int hisp_ca_sfdmem_unmap(struct hisp_ca_meminfo *buffer);
int hisp_ca_dynmem_map(
	struct hisp_ca_meminfo *buffer, struct device *dev);
int hisp_ca_dynmem_unmap(
	struct hisp_ca_meminfo *buffer, struct device *dev);
int hisp_ca_bootmem_config(struct hisp_ca_meminfo *img, int imginfo_size,
		struct hisp_ca_meminfo *rsv, int rsvinfo_size);
int hisp_ca_bootmem_deconfig(void);
void hisp_ca_probe(void);
void hisp_ca_remove(void);

/* hisp pwr api in hisp_pwr.c */
int hisp_bsp_read_bin(const char *partion_name, unsigned int offset,
	unsigned int length, char *buffer);
int hisp_dvfs_enable(void);
unsigned int hisp_smmuv3_mode(void);
int hisp_pwr_subsys_powerup(void);
int hisp_pwr_subsys_powerdn(void);
int hisp_pwr_core_nsec_init(u64 pgd_base);
int hisp_pwr_core_nsec_exit(void);
int hisp_pwr_core_sec_init(u64 phy_pgd_base);
int hisp_pwr_core_sec_exit(void);
int hisp_pwr_cpu_sec_init(void);
int hisp_pwr_cpu_sec_exit(void);
int hisp_pwr_cpu_nsec_dst(u64 remap_addr, unsigned int canary);
int hisp_pwr_cpu_nsec_rst(void);
int hisp_pwr_cpu_sec_dst(int ispmem_reserved, unsigned long image_addr);
int hisp_pwr_cpu_sec_rst(void);
int hisp_pwr_cpu_ca_dst(void);
int hisp_pwr_cpu_ca_rst(void);
unsigned long hisp_pwr_getclkrate(unsigned int type);
int hisp_pwr_nsec_setclkrate(unsigned int type, unsigned int rate);
int hisp_pwr_sec_setclkrate(unsigned int type, unsigned int rate);
int hisp_ispcpu_qos_cfg(void);
int hisp_pwr_probe(struct platform_device *pdev);
int hisp_pwr_remove(struct platform_device *pdev);

#ifdef CONFIG_HISP_LOAD_SECISPIMAGE
int hisp_load_and_verify_image(void);
#else
static inline int hisp_load_and_verify_image(void) { return 0; }
#endif

/* hisp mempool api in hisp_mempool.c */
int hisp_mempool_init(unsigned int addr, unsigned int size);
void hisp_mempool_exit(void);

/* hisp nsec api in hisp_nsec.c */
u64 hisp_nsec_get_remap_pa(void);
void* hisp_nsec_get_remap_va(void);
u64 hisp_get_ispcpu_shrmem_nsecpa(void);
void *hisp_get_ispcpu_shrmem_nsecva(void);
void hisp_nsec_boot_dump(struct rproc *rproc, unsigned int size);
int hisp_nsec_jpeg_powerup(void);
int hisp_nsec_jpeg_powerdn(void);
int hisp_nsec_device_enable(struct rproc *rproc);
int hisp_nsec_device_disable(void);
u64 hisp_nsec_get_pgd(void);
int hisp_nsec_probe(struct platform_device *pdev);
int hisp_nsec_remove(struct platform_device *pdev);
#ifdef DEBUG_HISP
struct isp_pcie_cfg *hisp_get_pcie_cfg(void);
int hisp_check_pcie_stat(void);
#endif

/* hisp sec api in hisp_sec.c */
unsigned int hisp_sectype_is_valid(unsigned int type);
int hisp_ca_boot_mode(void);
u64 hisp_get_ispcpu_shrmem_secpa(void);
void *hisp_get_ispcpu_shrmem_secva(void);
void *hisp_get_dyna_array(void);
struct isp_a7mapping_s *hisp_get_ap_dyna_mapping(void);
int hisp_sec_jpeg_powerup(void);
int hisp_sec_jpeg_powerdn(void);
u64 hisp_get_param_info_pa(void);
void *hisp_get_param_info_va(void);
void hisp_sec_set_ispcpu_palist(void *listmem,
		struct scatterlist *sg, unsigned int size);
int hisp_request_sec_rsctable(struct rproc *rproc);
u64 hisp_sec_get_remap_pa(void);
unsigned int hisp_pool_mem_addr(unsigned int pool_num);
int hisp_sec_pwron_prepare(void);
int hisp_sec_device_enable(void);
int hisp_sec_device_disable(void);
int hisp_sec_probe(struct platform_device *pdev);
int hisp_sec_remove(struct platform_device *pdev);

/* hisp share api */
struct hisp_shared_para *hisp_share_get_para(void);
int hisp_share_set_para(void);
void hisp_share_update_clk_value(int type,unsigned int value);
void hisp_lock_sharedbuf(void);
void hisp_unlock_sharedbuf(void);
void hisp_rproc_state_lock(void);
void hisp_rproc_state_unlock(void);
int hisp_log_beta_mode(void);
u32 hisp_share_get_exc_flag(void);
unsigned int hisp_wait_excflag_timeout(unsigned int flag, unsigned int time);
int hisp_nsec_set_pgd(void);

/* hisp device api in hisp_device.c */
int hisp_ispcpu_is_powerup(void);
struct device *hisp_get_device(void);
#ifdef ISP_BUF_PRIVATIZE
struct device *hisp_get_ispfw_device(void);
#endif
int hisp_nsec_boot_mode(void);
int hisp_sec_boot_mode(void);
int hisp_get_last_boot_state(void);
void hisp_set_last_boot_state(int state);
enum hisp_rproc_case_attr hisp_get_boot_mode(void);
unsigned long hisp_get_clk_rate(unsigned int type);
u64 hisp_get_ispcpu_shrmem_pa(void);
void *hisp_get_ispcpu_shrmem_va(void);
u64 hisp_get_ispcpu_remap_pa(void);
int hisp_get_ispcpu_cfginfo(void);
int hisp_rproc_enable_status(void);
int hisp_wait_rpmsg_completion(struct rproc *rproc);
int hisp_bypass_power_updn(void);
void __iomem *hisp_dev_get_regaddr(unsigned int type);
unsigned int hisp_get_dyn_bootaddr(void);

/* hisp rdr api in hisp_rdr.c */
#ifdef CONFIG_HISP_RDR
void hisp_send_fiq2ispcpu(void);
int hisp_rdr_init(void);
void hisp_rdr_exit(void);
#else
static inline void hisp_send_fiq2ispcpu(void) { return; }
static inline int hisp_rdr_init(void) { return 0; }
static inline void hisp_rdr_exit(void) { return; }
#endif

/* hisp log api in hisp_log.c */
#ifdef CONFIG_HISP_RDR
void hisploglevel_update(void);
int hisplogcat_sync(void);
int hisplogcat_start(void);
void hisplogcat_stop(void);
void hisplog_clear_info(void);
#else
static inline void hisploglevel_update(void) { return; }
static inline int hisplogcat_sync(void) { return -1; }
static inline int hisplogcat_start(void) { return -1; }
static inline void hisplogcat_stop(void) { return; }
static inline void hisplog_clear_info(void) { return; }
#endif

/* hisp rpmsg api in hisp_rpm.c */
int hisp_rpmsg_rdr_init(void);
int hisp_rpmsg_rdr_deinit(void);

/* hisp virtio rpmsg api in hisp_virtio_rpmsg.c */
#ifdef CONFIG_HISP_RPMSG
extern int hisp_virtio_rpmsg_init(void);
extern void hisp_virtio_rpmsg_exit(void);
#else
static inline int hisp_virtio_rpmsg_init(void) { return 0; }
static inline void hisp_virtio_rpmsg_exit(void) { return; }

#endif
#ifdef CONFIG_ISP_INTERNAL_MSG
extern int hisp_msg_service_recv(void *data, int len, unsigned int src);
extern int hisp_msg_service_send(void *data, int len);
extern void hisp_msg_service_init(struct rpmsg_endpoint *ept);
extern void hisp_msg_service_exit(void);
#else
static inline int hisp_msg_service_recv(void *data __attribute__((__unused__)),
	int len  __attribute__((__unused__)),
	unsigned int src  __attribute__((__unused__)))
{
	return 0;
}
static inline int hisp_msg_service_send(void *data __attribute__((__unused__)),
	int len __attribute__((__unused__)))
{
	return 0;
}
static inline void hisp_msg_service_init(struct rpmsg_endpoint *ept __attribute__((__unused__)))
{
}
static inline void hisp_msg_service_exit(void) { return; }
#endif
/* extern api not in hisp drivers */
#ifdef CONFIG_KERNEL_CAMERA_USE_HISP120
extern int hisp120_get_lowpower(void);
#endif
extern unsigned int a7_mmu_map(struct scatterlist *sgl, unsigned int size,
		unsigned int prot, unsigned int flag);
extern void a7_mmu_unmap(unsigned int va, unsigned int size);
extern int hw_is_fpga_board(void); /* form camera hal */
extern u64 isp_getcurtime(void);
extern int rpmsg_sensor_ioctl(unsigned int cmd, int index, char* name);
extern void *rproc_da_to_va(struct rproc *rproc, u64 da,
		size_t len, bool *is_iomem);
extern irqreturn_t rproc_vq_interrupt(struct rproc *rproc, int notifyid);

/* hisp debug api */
#ifdef DEBUG_HISP
#define NONSEC_MAX_SIZE 64
ssize_t hisp_rdr_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t hisp_rdr_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t hisp_clk_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t hisp_clk_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t hisp_power_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t hisp_power_store(struct device *pdev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t hisp_sec_test_regs_show(struct device *pdev,
		struct device_attribute *attr, char *buf);
ssize_t hisp_boot_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf);
ssize_t hisp_boot_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
void hisp_perf_dump(struct rproc *rproc);
void hisp_perf_mem_state_init(struct rproc *rproc);
void *hisp_get_perf_va(struct rproc *rproc);
unsigned int hisp_get_perf_size(struct rproc *rproc);
unsigned int hisp_get_perf_flag(struct rproc *rproc);
#if (defined(HISP_FUZZ))
int hisp_kcov_get_shmem(void **shmem_ptr, unsigned long *size_ptr);
int hisp_fuzz_get_shmem(void **shmem_ptr, unsigned long *size_ptr);
void hisp_kcov_shadow_mem_deinit(void);
int hisp_fuzz_kcov_shadow_mem_init(void);
#endif
#endif

#endif /* _HISP_INTERNEL_H */
