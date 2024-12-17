/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 * Description: hifi kernel hal ipc msg interface header file
 * Create: 2021-12-3
 */

#ifndef AUDIO_IPC_AUDIO_COMMON_MSG_H
#define AUDIO_IPC_AUDIO_COMMON_MSG_H

#include "audio_common_msg_id.h"

#define HEARING_PROTECTION_VOLUME_NUM 10

// maximum of mmap stream num
#define MAX_MMAP_OUT_NUM      6
#define MAX_MMAP_IN_NUM       1
#define MAX_MMAP_STREAM_NUM   (MAX_MMAP_OUT_NUM + MAX_MMAP_IN_NUM)
#define MAX_MMAP_SINGLE_NUM   MAX_MMAP_OUT_NUM

#define AAUDIO_OUT_MMAP_SIZE 0x2000
#define AAUDIO_IN_MMAP_SIZE 0x2000

enum stream_type {
    STREAM_AUDIO_PRIMARY,
    STREAM_AUDIO_OFFLOAD,
    STREAM_AUDIO_DIRECT,
    STREAM_AUDIO_LOWLATENCY,
    STREAM_AUDIO_MMAP,
    STREAM_AUDIO_MMAP_MAX = STREAM_AUDIO_MMAP + MAX_MMAP_SINGLE_NUM - 1,
    STREAM_AUDIO_SPATIAL_AUDIO,
    STREAM_AUDIO_ALL,
    STREAM_VOICE_ALL,
    STREAM_ALL,
    STREAM_CNT,
};

enum stream_direction {
    STREAM_OUTPUT,
    STREAM_INPUT
};

typedef enum {
    AUDIO_MONO_CHANNEL_MODE_OFF = 0,
    AUDIO_MONO_CHANNEL_MODE_LRMIX,
    AUDIO_MONO_CHANNEL_MODE_LL,
    AUDIO_MONO_CHANNEL_MODE_RR,
    AUDIO_MONO_CHANNEL_MODE_CNT,
} audio_mono_channel_mode;

enum mem_type_enum {
    MEM_TYPE_HIFI_DATA = 0,
    MEM_TYPE_CMA,
    MEM_TYPE_BUTT
};

#define MEM_CMA_RESERVE_MUSIC_DATA_SIZE (10 * 1024 * 1024)

enum AUDIO_FILE_FORMAT_ENUM {
    AUDIO_FILE_FORMAT_MP3 = 0,
    AUDIO_FILE_FORMAT_AAC,
    AUDIO_FILE_FORMAT_WAV,
    AUDIO_FILE_PCM_FORMAT_S16_LE,
    AUDIO_FILE_PCM_FORMAT_S24_LE_LA,       // left alignment
    AUDIO_FILE_PCM_FORMAT_S24_LE_RA,       // right alignment
    AUDIO_FILE_PCM_FORMAT_S32_LE,
    AUDIO_FILE_PCM_FORMAT_S24_3LE,
    AUDIO_FILE_FORMAT_BUT
};

// bluetooth definde begin
enum CodecType {
    CODEC_UNKNOWN = 0x00,
    SBC = 0x01,
    AAC = 0x02,
    APTX = 0x04,
    APTX_HD = 0x08,
    LDAC = 0x10,
    LC3 = 0x20,
    LC3_LL = 0x40,
    L2HC = 0x80,
};

enum BitsPerSample {
    BITS_UNKNOWN = 0x00,
    BITS_16 = 0x01,
    BITS_24 = 0x02,
    BITS_32 = 0x04,
};

enum SampleRate {
    RATE_UNKNOWN = 0x00,
    RATE_44100 = 0x01,
    RATE_48000 = 0x02,
    RATE_88200 = 0x04,
    RATE_96000 = 0x08,
    RATE_176400 = 0x10,
    RATE_192000 = 0x20,
    RATE_16000 = 0x40,
    RATE_24000 = 0x80,
};

enum ChannelMode {
    ChannelMode_UNKNOWN = 0x00,
    MONO = 0x01,
    STEREO = 0x02,
};

enum wakeup_status_enum {
    STATUS_NOT_START = 0,
    STATUS_MAD_NOT_INTERRUPT,
    STATUS_MAD_INTERRUPT_COME,
};

enum time_stamp_pos {
    STAMP_PRE_SMART_PA_PROCESS,
    STAMP_AFT_SMART_PA_PROCESS,
    STAMP_PLAY_POS_BUTT,
    STAMP_CAP_AFT_COVERSION = STAMP_PLAY_POS_BUTT,
    STAMP_POS_BUTT
};

enum audio_pcm_device_enum {
    AUDIO_PCM_DEVICE_PRIMARY = 0,
    AUDIO_PCM_DEVICE_BT = 1,
    AUDIO_PCM_DEVICE_FM_AP = 2,
    AUDIO_PCM_DEVICE_DIRECT = 4, /* according to pcm driver */
    AUDIO_PCM_DEVICE_LOW_LATENCY = 5,
    AUDIO_PCM_DEVICE_MMAP = 6,
    AUDIO_PCM_DEVICE_SPATIAL_AUDIO = 7,
    AUDIO_PCM_DEVICE_KARAOKE_RECORD = 8,
    AUDIO_PCM_DEVICE_XCALL = 9,
    AUDIO_PCM_DEVICE_WAKEUP = 10,
    AUDIO_PCM_DEVICE_WAKEUP_LP = 11,
    AUDIO_PCM_DEVICE_ULTRASONIC = 12,
    AUDIO_PCM_DEVICE_MMAP_EXTEND = 13,
    AUDIO_PCM_DEVICE_MMAP_MAX = AUDIO_PCM_DEVICE_MMAP_EXTEND + MAX_MMAP_SINGLE_NUM - 2,
    AUDIO_PCM_DEVICE_CNT,
};

enum audio_pcm_mode_enum {
    AUDIO_PCM_MODE_PLAY,
    AUDIO_PCM_MODE_CAPTURE,
    AUDIO_PCM_MODE_CNT,
};

enum pcm_type_enum {
    PCM_TYPE_DEF = 0,
    PCM_TYPE_FAST_MMAP, /* OH mmap */
    PCM_TYPE_BUTT
};

enum audio_cmd_vol_type_enum {
    HIFI_VOL_TYPE_VOICE = 1,
    HIFI_VOL_TYPE_AUDIO_OFFLOAD,
    HIFI_VOL_TYPE_AUDIO_DIRECT,
    HIFI_VOL_TYPE_AUDIO_MMAP,
    HIFI_VOL_TYPE_AUDIO_MMAP_MAX = HIFI_VOL_TYPE_AUDIO_MMAP + MAX_MMAP_SINGLE_NUM - 1,
    HIFI_VOL_TYPE_SPATIAL_AUDIO,
    HIFI_VOL_TYPE_CNT,
};

#define A2DP_FRAME_DUR_10_MS 0x00
#define A2DP_FRAME_DUR_7_MS 0x01
#define A2DP_FRAME_DUR_5_MS 0x02
#define A2DP_FRAME_DUR_INDEX 6

#define A2DP_CODEC_INFO_SIZE 32
#define A2DP_INFO_INIT_FLAG 0x5a5a5a5a

#define BT_EVENT_MAX_RECORD_CNT 10
#define A2DP_OFFLOAD_MAX_INFO_LEN 200
#define A2DP_OFFLOAD_REPORT_POP_CNT 3
// bluetooth defined end

#pragma pack(push, 4)
/* 请求Hifi启动超低功耗播放 */
typedef struct {
    unsigned short              msg_id;          /* 0xDD51 */
    unsigned short              reserved;
    unsigned int                mem_type;        /* 共享内存类型, 查看 enum mem_type_enum */
    unsigned int                mem_offset;      /* 共享内存地址偏移 */
    unsigned int                format;          /* 音频文件格式 */
    unsigned int                sample_rate;     /* 音频数据采样率 */
    unsigned int                total_file_size; /* 整首MP3/AAC音乐的大小，含文件头信息，单位byte */
    unsigned int                wav_data_size;   /* 整首wav文件数据大小,不包括文件头 */
    unsigned long long          start_pos;       /* 开始解码位置，含文件头 */
} audio_player_start_req;

/* 返回Hifi启动超低功耗播放的执行结果 */
typedef struct {
    unsigned short              msg_id;
    unsigned short              result;          /* 是否执行成功 */
} audio_player_start_cnf;

/* 请求Hifi暂停或停止超低功耗解码 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD53 */
    unsigned short              play_pending_samples;   /* 暂停还是停止播放 */
    unsigned short              reserve;
    unsigned short              is_change_dev;
} audio_player_pause_req;

/* 返回Hifi暂停时播放的数据量和结果 */
typedef struct {
    unsigned short              msg_id;
    unsigned short              result;          /* 是否执行成功 */
    unsigned long long          pause_pos;      /* 当前已播放的数据量 */
} audio_player_pause_cnf;

/* 通知Hifi更新播放BUFFER */
typedef struct {
    unsigned short              msg_id;         /* 0xDD57 */
    unsigned short              format;
    /* AP侧Kernel音频驱动发送数据给hifi时的buff描述，bufAddr为总线地址，bufSize和dataSize单位byte */
    unsigned int                buff_offset;
    unsigned int                buff_size;
    unsigned int                data_size;
} audio_player_update_buf_cmd;

/* 查询Hifi播放进度状态信息 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD59 */
    unsigned short              reserve;
    unsigned int                write_offset;
} audio_player_query_time_req;

/* 返回Hifi上报播放进度状态信息 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD60 */
    unsigned short              reserve;
    unsigned long long          played_bytes;   /* 当前已播放的音乐文件大小，单位byte，含文件头 */
} audio_player_query_time_cnf;

/* 通知Hifi更新播放BUFFER_WATERLINE */
typedef struct {
    unsigned short              msg_id;         /* 0xDD61 */
    unsigned int                buff_waterline;
} audio_player_set_buf_waterline_req;

/* 返回Hifi更新播放BUFFER_WATERLINE状态信息 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD62 */
    unsigned short              result;
} audio_player_set_buf_waterline_cnf;
#pragma pack(pop)

/*
 * 实体名称  : audio_cmd_set_vol_req_stru
 * 功能描述  : 该消息请求设置音量
 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD70 */
    unsigned short              vol_type;
    unsigned int                left_volume;    /* 左声道音量值，Q15  */
    unsigned int                right_volume;   /* 右声道音量值，Q15  */
} audio_cmd_set_vol_req_stru;

struct notify_trigger_msg {
    unsigned short              msg_id;         /* 0xDD7F */
    unsigned short              value;
};

/*
 * 实体名称  : soc_adsp_set_device_cmd
 * 功能描述  : 切换设备设置
 */
struct soc_adsp_set_device_cmd {
    unsigned short              msg_id;         /* 0xDD96 */
    unsigned short              reserve;
    unsigned int                device;         /* audio_devices_t */
};

struct set_angle_cmd {
    unsigned short              msg_id;         /* 0xDD99 */
    unsigned short              reverse;
    unsigned int                value;
};

struct set_audio_effect_param_cmd {
    unsigned short              msg_id;         /* 0xDD9A */
    unsigned short              reserved;
    unsigned int                value;
};

struct audio_effect_rslt {
    unsigned short              msg_id;         /* 0xDD9B */
    unsigned short              result;
};

struct set_mono_channel {
    unsigned short              msg_id;         /* 0xDD9C */
    unsigned short              reserved;
    unsigned int                stream_type;
    unsigned int                mode;
};

struct socdsp_signal_timestamp {
    unsigned int dsp_time_stamp;
    enum time_stamp_pos stamp_pos;
};

struct audio_latency_measure {
    unsigned short              msg_id;        /* 0xDD9E */
    unsigned short              value;
};

struct audio_set_effect_para {
    unsigned short              msg_id;        /* 0xDDA9 */
    unsigned short              reserved;
    unsigned int                value;
};

/*
 * 实体名称  : audio_record_start
 * 功能描述  : AP侧HAL层通过DSP驱动发送消息ID_AP_AUDIO_RECORD_START_REQ通知HIFI开始录音
 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD40 */
    unsigned short              reserve;
    unsigned short              record_type;    /*  录音类型 */
    unsigned short              tar_smp_rate;
} audio_record_start;

/*
 * 实体名称  : audio_record_stop
 * 功能描述  : AP侧HAL层通过DSP驱动发送消息ID_AP_AUDIO_RECORD_STOP_REQ通知HIFI停止录音
 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD41 */
    unsigned short              reserve;
    unsigned short              record_type;    /* 录音类型 */
    unsigned short              reserve1;
} audio_record_stop;

/*
 * 实体名称  : ap_hifi_mocker_hifi_reset_cmd
 * 功能描述  : 测试用: hal层通知HIFI, 通过特定危险操作触发对应的HIFI单独复位
 */
typedef struct {
    unsigned short              msg_id;              /* _H2ASN_Replace VOS_UINT16 msg_id; */
    unsigned short              en_reset_case;       /* _H2ASN_Replace VOS_UINT16 en_reset_case; */
    unsigned short              reset_case;
    unsigned short              reserve2;            /* _H2ASN_Replace VOS_UINT16 reserve2; */
} ap_hifi_mocker_hifi_reset_cmd;

/*
 * 实体名称  : ap_hifi_ccpu_reset_req
 * 功能描述  : AP通知HIFI C核复位的消息结构体
 */
typedef struct {
    unsigned short              msg_id;         /* 0xDDE1 */ /* _H2ASN_Skip */
    unsigned short              reserve;        /* _H2ASN_Replace VOS_UINT16 reserve; */
} ap_hifi_ccpu_reset_req;

/*
 * 实体名称  : hifi_ap_ccpu_reset_cnf
 * 功能描述  : 响应ID_AP_HIFI_CCPU_RESET_REQ，如果hifi有语音业务时，停止语音业务，回复AP
 */
typedef struct {
    unsigned short              msg_id;         /* 0xDDE2 */
    unsigned short              result;         /* 0表示succ， 1表示fail */
} hifi_ap_ccpu_reset_cnf;

/*
 * 实体名称  : audio_effect_dts_set_enable_req
 * 功能描述  : AP发送该消息ID_AP_AUDIO_SET_DTS_ENABLE_REQ给DSP_PID_AUDIO，通知Hifi设置DTS音效使能与否
 */
typedef struct {
    unsigned short              msg_id;         /* 0xDD36 */ /* 设置DTS音效使能与否ID */
    unsigned short              enable_flag;    /* DTS音效是否使能 */
} audio_effect_dts_set_enable_req;

/*
 * 实体名称  : ap_audio_bsd_control_req
 * 功能描述  : AP侧HAL层通过DSP驱动发送消息，通知HIFI BSD语音检测开始or停止
 */
typedef struct {
    unsigned short              msg_id;
    unsigned short              reserve;
    unsigned int                value;
} ap_audio_bsd_control_req;

struct req_pcm_config_msg {
    unsigned short msg_id;
    unsigned short reserved;
};

struct req_pcm_config_msg_cnf {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int channels;
    unsigned int rate;
    unsigned int period_size;
    unsigned int period_count;
    unsigned int format;
};

/*
 * name: audio_cmd_set_param_req
 * func: used for send set param msg from AP
 */
typedef struct {
    unsigned short msg_id;              /* 0xDF08 */
    unsigned short module_id;            /* id for algo */
    char           payload[0];          /* payload point */
} audio_cmd_set_param_req;

/*
 * name: audio_cmd_get_param_req_stru
 * func: used for send get param msg from DSP
 */
typedef struct {
    unsigned short msg_id;
    unsigned short module_id;
    unsigned int   uw_process_id;
    unsigned int   uw_module_id;
    unsigned int   size;
    unsigned char  payload[0];
} audio_cmd_get_param_req;

/*
 * name: audio_cmd_get_param_ret_head
 * func: used for send get param result from DSP
 */
typedef struct {
    unsigned short msg_id;
    unsigned short module_id;
    unsigned char  payload[0];
} audio_cmd_get_param_ret_head;


struct audio_cmd_mute_msg {
    unsigned short msg_id;
    unsigned short is_mute;
    unsigned int stream_direction;
    unsigned int stream_type;
};

struct audio_cmd_audio2voicetx_msg {
    unsigned short msg_id;
    unsigned short reserved[2]; // reserve 2 units
    unsigned short enable;
};

struct audio_cmd_audiomix2voicetx_msg {
    unsigned short msg_id;
    unsigned short enable;
};

struct audio_cmd_mos_test_msg {
    unsigned short msg_id;
    unsigned short enable;
};

struct audio_cmd_force_mute_mic_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int is_mute; /* 0: unmute, 1: mute */
};

#define AUDIO_PCM_MSG_HEADER unsigned short msg_id; \
                            unsigned short pcm_device; \
                            unsigned short pcm_mode; \
                            unsigned short pcm_type

enum audio_pcm_format {
    AUDIO_PCM_FORMAT_INVALID = -1,
    AUDIO_PCM_FORMAT_S16_LE = 0,
    AUDIO_PCM_FORMAT_S24_LE_LA, /* left alignment */
    AUDIO_PCM_FORMAT_S24_LE_RA, /* right alignment */
    AUDIO_PCM_FORMAT_S32_LE,
    AUDIO_PCM_FORMAT_S24_3LE,
    AUDIO_PCM_FORMAT_MAX,
};

struct audio_pcm_config {
    unsigned int channels;
    unsigned int rate;
    unsigned int period_size;
    unsigned int period_count;
    enum audio_pcm_format format;
};

struct audio_pcm_open_msg {
    AUDIO_PCM_MSG_HEADER;
    unsigned short is_support_low_power;
    unsigned short reserve2;
    struct audio_pcm_config config;
};

struct audio_pcm_trigger_msg {
    AUDIO_PCM_MSG_HEADER;
    unsigned short cmd;
    unsigned short reserved;
    unsigned int buff_offset;
    unsigned int buff_size;
};

struct audio_pcm_trigger_cnf_msg {
    AUDIO_PCM_MSG_HEADER;
};

struct audio_pcm_set_buff_msg {
    AUDIO_PCM_MSG_HEADER;
    unsigned int buff_offset;
    unsigned int buff_size;
};

struct audio_pcm_period_elapsed_cmd {
    AUDIO_PCM_MSG_HEADER;
    unsigned int time_stamp;
    unsigned short frame_num;
    unsigned short discard;
};

struct audio_pcm_close_msg {
    AUDIO_PCM_MSG_HEADER;
};

/*
 * 实体名称  : audio_cmd_adsp_set_cmd_struct
 * 功能描述  : 该消息为AP下发的辅听set param消息
 */
typedef struct {
    unsigned short msg_id;
    unsigned short reserve;
    unsigned int   value;     /* can be audio_mode_t, audio_source_t, etc. */
} audio_cmd_set_aux_hear_cmd_struct;

/*
 * 实体名称  : audio_cmd_sound_enhance_struct
 * 功能描述  : 该消息为AP下发的听感优化set param消息
 */
typedef struct {
    unsigned short msg_id;
    unsigned short reserve;
    unsigned int   value;     /* can be audio_mode_t, audio_source_t, etc. */
} audio_cmd_sound_enhance_struct;

/*
 * 实体名称  : AUDIO_PLAYER_DONE_IND_STRU
 * 功能描述  : 该消息通知AP当前音频文件播放结果
 * dsp misc interface
 */
typedef struct misc_recmsg_param {
    unsigned short msg_id; /* 0xDD56 */
    unsigned short play_status; /* 播放结果枚举 */
    unsigned int dsp_timestamp;
} AUDIO_PLAYER_DONE_IND_STRU;

/*
 * 实体名称  : VOICE_NO_PARAM_MSG_STRU
 * 功能描述  : 其他的消息只有消息ID，没有附加的消息参数。
 */
typedef struct {
    unsigned int uwSenderCpuId;
    unsigned int uwSenderPid;
    unsigned int uwReceiverCpuId;
    unsigned int uwReceiverPid;
    unsigned int uwLength;
    unsigned short   usMessageId;
    unsigned short   usReserve;
} VOICE_AP_NO_PARAM_MSG_STRU;

typedef struct {
    unsigned int                 mode_3A;
    unsigned int                 sample_rate;
    unsigned int                 net_mode;
    unsigned int                 modem_no;
    unsigned int                 current_nvid;
    unsigned int                 multi_mic;
    unsigned int                 source;
    unsigned int                 mode;
    unsigned char                device_name[16];
} voice_algo_state_info;

typedef struct {
    unsigned short                  msg_id;
    unsigned short                  reverse;
    voice_algo_state_info           para;
} voice_algo_state_para;

struct audio_player_pcm_buff_update_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int size;
};

typedef struct {
    unsigned short                      msg_id;
    unsigned short                      head_protect_word;
    unsigned short                      cmd_id;
    unsigned short                      req_size;
    unsigned int                        rult_size;
    unsigned char                       data[0];
} ap_hifi_get_smartpa_para_msg;

typedef struct {
    unsigned short                      msg_id;
    unsigned short                      head_protect_word;
    unsigned int                        para;
    unsigned int                        size;
    unsigned char                       data[0];
} ap_hifi_return_smartpa_para_msg;

typedef struct {
    unsigned short                  msg_id;   /* 0xDF22 */
    unsigned short                  reserved;
    unsigned int                    value;
} set_combine_record_func_cmd;

struct scene_denoise_cmd_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int scene_denoise_cmd;
};

struct hearing_protection_switch_msg {
    unsigned short msg_id;
    unsigned short enable;
};

struct hearing_protection_get_volume_msg_cnf {
    unsigned short msg_id;
    unsigned short size;
    int volume[HEARING_PROTECTION_VOLUME_NUM];
};

struct dprx_cmd_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int cmd_id;
    unsigned int channels;
    unsigned int channel_mask;
    unsigned int rate;
    unsigned int format;
};

/*
 * 实体名称  : AUDIO_HIFIMISC_MSG_TAIL_STRU
 * 功能描述  : AP HifiMisc设备的消息标识结构体，包括消息类型、消息编号，要求紧跟着
 * 原消息内容写在消息末尾
 */
typedef struct {
    unsigned int                           uwCmdId;
    unsigned int                           uwSn;
} AUDIO_HIFIMISC_MSG_TAIL_STRU;

typedef struct {
    unsigned int uwSenderCpuId;
    unsigned int uwSenderPid;
    unsigned int uwReceiverCpuId;
    unsigned int uwReceiverPid;
    unsigned int uwLength;
    unsigned short                      msg_id;
    unsigned short                      head_protect_word;
    unsigned short                      cmd_id;
    unsigned short                      req_size;
    unsigned int                        rult_size;
    AUDIO_HIFIMISC_MSG_TAIL_STRU        msg_tail;
    unsigned char                       data[0];
} smartpa_para_msg;


typedef struct {
    unsigned int uwSenderCpuId;
    unsigned int uwSenderPid;
    unsigned int uwReceiverCpuId;
    unsigned int uwReceiverPid;
    unsigned int uwLength;
    unsigned short                      msg_id;
    unsigned short                      value;
    AUDIO_HIFIMISC_MSG_TAIL_STRU        msg_tail;
} ap_hifi_get_voice_para_msg;

struct wakeup_period_elapsed {
    unsigned int seq;
    unsigned int start;
    unsigned int len;
};

struct wakeup_rcv_msg {
    unsigned short  msg_id;
    unsigned short  vendor_alg_mask;
    union {
        int socdsp_handle;
        unsigned int fast_len;
        struct wakeup_period_elapsed elapsed;
    };
};

struct parameter_set {
    int key;
    union {
        int value;
        struct {
            unsigned int index;
            unsigned int length;
            char piece[0];
        } model;
    };
};

struct parameter_set_meg {
    unsigned short  msg_id;
    unsigned short  vendor_alg_mask;
    unsigned int module_id;
    struct parameter_set parameter;
};

struct syscache_quota_msg {
    unsigned int msg_type;
    unsigned int session;
};

struct syscache_quota_ack_msg {
    unsigned short msg_id;
    unsigned short reserve;
    unsigned int msg_type;
    unsigned int session;
};

/* 参数设置结构体 */
typedef struct {
    unsigned short              msg_name;       /* 原语类型 */  /* 0xDD92 */ /* _H2ASN_Skip */
    unsigned short              reserve;        /* 保留 */      /* _H2ASN_Replace VOS_UINT16 reserve; */
    unsigned int                path_id;        /* 路径ID */    /* _H2ASN_Replace VOS_UINT32 path_id; */
    unsigned int                module_id;      /* 模块ID */    /* _H2ASN_Replace VOS_UINT32 module_id; */
    unsigned int                size;           /* 数据大小 */  /* _H2ASN_Replace VOS_UINT32 size; */
    unsigned char               auc_data[0];    /* 数据起始 */  /* _H2ASN_Replace VOS_UINT8 auc_data; */
} ap_audio_mlib_set_para_ind;

struct a2dp_offload_codec_packet_info {
    unsigned short media_packet_header;
    unsigned char m_pt;
    unsigned int ssrc;
    unsigned char boundary_flag;
    unsigned char broadcast_flag;
    unsigned int codec_type; /* codec types ex: SBC/AAC/LDAC/APTx */
    unsigned short max_latency; /* maximum latency */
    unsigned short scms_t_enable; /* content protection enable */
    unsigned int sample_rate; /* Sample rates ex: 44.1/48/88.2/96 Khz */
    unsigned int encoded_audio_bitrate; /* encoder audio bitrates */
    unsigned char bits_per_sample; /* bits per sample ex: 16/24/32 */
    unsigned char ch_mode; /* None:0 Left:1 Right:2 */
    unsigned short acl_hdl; /* connection handle */
    unsigned short l2c_rcid; /* l2cap channel id */
    unsigned short mtu; /* MTU size */
    unsigned char codec_info[A2DP_CODEC_INFO_SIZE]; /* Codec specific information */
    unsigned int init_flag;
};

struct set_codec_packet_info_msg {
    unsigned short msg_id;
    unsigned short reserved;
    struct a2dp_offload_codec_packet_info info;
};

typedef struct {
    unsigned int sender_cpuid;
    unsigned int sender_pid;
    unsigned int receiver_cpuId;
    unsigned int receiver_pid;
    unsigned int length;
    unsigned short msg_name;
    unsigned short state;
} hifi_ap_call_state_report_cnf;

typedef struct {
    unsigned int sender_cpuid;
    unsigned int sender_pid;
    unsigned int receiver_cpuId;
    unsigned int receiver_pid;
    unsigned int length;
    unsigned short msg_name;
    unsigned short reserve;
} ap_hifi_call_state_report_req;

struct soundtrigger_mic_state_get_req {
    unsigned short msg_id;
    unsigned short reserve;
};

struct soundtrigger_mic_state_get_cnf {
    unsigned short msg_id;
    unsigned short reserve;
    int state;
};

struct record_start_dma_stamp_get_req {
    unsigned short msg_id;
    unsigned short reserve;
};

typedef enum spatial_audio_stream_type {
    STREAM_3DA,
    STREAM_MULTI_CHANNEL,
    STREAM_TYPE_BUTT
} spatial_audio_stream_type;

struct spatial_audio_stream_info {
    unsigned short msg_id;
    unsigned short stream_type;
    unsigned int channel_mask;
};


struct spatial_audio_init_req {
    unsigned short msg_id;
    unsigned short reserved;
    int offset;
};

struct spatial_audio_init_cnf {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int result;
};

struct spatial_audio_deinit_req {
    unsigned short msg_id;
    unsigned short reserved;
};

struct spatial_audio_deinit_cnf {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int result;
};

struct bbic_cmd_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int bbic_cmd;
    unsigned int loop_test;
};

typedef struct {
    unsigned int cmd_id;
    unsigned int sn;
} hifi_rda_msg_tail;

struct ap_hifi_chime_start_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int sample_rate;
    unsigned int channel;
    unsigned int format;
    unsigned int total_size;
};

struct ap_hifi_chime_update_buf_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int buffer_size;
    unsigned int left_size;
};

struct ap_hifi_chime_effect_cfg_msg {
    unsigned short msg_id;
    unsigned short reserved;
    int update_param; /* 0: all params 1：in volume 2：out volume 3: in & out volume 4: scene 5: gain */
    int mode;
    int position_mask;
    int scene; /* 1：support fade-in & fade-out 2: immediately fade-out */
    int in_volume;
    int out_volume;
    int total_time_ms;
    int gain_group_cnt;
    signed char gain[10][16];
    int reserve[2];
};

struct ap_hifi_chime_stop_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int value; // reserved
};

struct hifi_ap_chime_comm_msg {
    unsigned short msg_id;
    unsigned short reserved;
};

enum chime_abnormal_msg_type {
    TYPE_RECV_START_NO_UPDATE_MSG,
    TYPE_PLAYEND_SWAPBUF_NO_UPDATE_MSG,
    TYPE_RECV_REPEATED_START_MSG,
    TYPE_RECV_REPEATED_UPDATE_MSG,
    TYPE_ABNORMAL_MAX
};

struct hifi_ap_rda_abnormal_msg {
    unsigned short msg_id;
    unsigned short reserved;
    unsigned int type;
};

struct a2dp_offload_event_info {
    unsigned short codec_type;
    unsigned short reserved;
    unsigned int paly_start_timestamp;
    unsigned int paly_stop_timestamp;
    unsigned int bitrate[BT_EVENT_MAX_RECORD_CNT];
    unsigned int bitrate_modify_cnt;
    unsigned int mtu[BT_EVENT_MAX_RECORD_CNT];
    unsigned int mtu_modify_cnt;
    unsigned int clear_queue_timestamp[BT_EVENT_MAX_RECORD_CNT];
    unsigned int clear_queue_cnt;
};

enum A2DP_OFFLOAD_INFO {
    CODEC_TYPE,
    START_TIMESTAMP,
    STOP_TIMESTAMP,
    BITRATE_CNT,
    MTU_CNT,
    CLEAR_QUEUE_CNT,
    INT_INFO_BUT,
    BITRATE_INFO = INT_INFO_BUT,
    MTU_INFO,
    CLEAR_QUEUE_INFO,
    STR_INFO_BUT,
};

static const char * const A2DP_OFFLOAD_REPORT_STR[STR_INFO_BUT] = {
    "CurrCodec",
    "PlayStartTime",
    "PlayStopTime",
    "BitrateCnt",
    "MTUCnt",
    "ClearQueueCnt",
    "BitrateInfo",
    "MTUInfo",
    "ClearQueueInfo",
};

struct a2dp_offload_report_info {
    int int_info[INT_INFO_BUT];
    char str_info[STR_INFO_BUT - INT_INFO_BUT][A2DP_OFFLOAD_MAX_INFO_LEN];
};

struct set_sco_scene_cmd {
    unsigned short msg_id;
    unsigned short reserve;
    unsigned int value;
};

struct mmap_position_msg {
    unsigned short msg_id;
    unsigned short reserve;
    unsigned short pcm_device;
    unsigned short pcm_mode;
};

struct record_start_dma_stamp_get_cnf {
    unsigned short msg_id;
    unsigned short reserve;
    unsigned int dsp_stamp;
    unsigned int kernel_stamp;
    long long kernel_time_s;
    long long kernel_time_us;
    unsigned int position_frames;
};

typedef struct record_start_dma_stamp_get_cnf mmap_position_ack_msg;

#define AUDIO_PARAMETER_SIZE 64

enum audio_dsp_info_params_type {
    AUDIO_DSP_INFO_PARAMS_TYPE_CPULOAD,
    AUDIO_DSP_INFO_PARAMS_TYPE_MAX,
};

struct cpuload_info {
    unsigned short avg_cpu_load;
    unsigned short max_cpu_load;
};

struct audio_dsp_info_msg {
    unsigned short msg_id;
    unsigned short params_type;
};

struct audio_dsp_info_ack_msg {
    unsigned short msg_id;
    unsigned short params_type;
    char data[AUDIO_PARAMETER_SIZE];
};
#endif // AUDIO_IPC_AUDIO_COMMON_MSG_H
