#ifndef SOC_DPU_FORMAT_H
#define SOC_DPU_FORMAT_H 
enum DPU_SDMA_STATIC_FORMAT {
 SDMA_FMT_RGB_565 = 0x0,
 SDMA_FMT_XRGB_4444 = 0x1,
 SDMA_FMT_ARGB_4444 = 0x2,
 SDMA_FMT_XRGB_1555 = 0x3,
 SDMA_FMT_ARGB_1555 = 0x4,
 SDMA_FMT_XRGB_8888 = 0x5,
 SDMA_FMT_ARGB_8888 = 0x6,
 SDMA_FMT_BGR_565 = 0x7,
 SDMA_FMT_XBGR_4444 = 0x8,
 SDMA_FMT_ABGR_4444 = 0x9,
 SDMA_FMT_XBGR_1555 = 0xA,
 SDMA_FMT_ABGR_1555 = 0xB,
 SDMA_FMT_XBGR_8888 = 0xC,
 SDMA_FMT_ABGR_8888 = 0xD,
 SDMA_FMT_RGBG = 0b100000,
 SDMA_FMT_RGBG_HIDIC = 0b100001,
 SDMA_FMT_RGBG_IDLEPACK = 0b100010,
 SDMA_FMT_RGBG_DEBURNIN = 0b100011,
 SDMA_FMT_RGBG_DELTA = 0b100100,
 SDMA_FMT_RGBG_DELTA_HIDIC = 0b110110,
 SDMA_FMT_RGBG_DELTA_IDLEPACK = 0b100101,
 SDMA_FMT_RGB_DEBURNIN = 0b100110,
 SDMA_FMT_RGB_10BIT = 0b100111,
 SDMA_FMT_BGR_10BIT = 0b101000,
 SDMA_FMT_ARGB_10101010 = 0b111001,
 SDMA_FMT_XRGB_10101010 = 0b111000,
 SDMA_FMT_BGRA_1010102 = 0b10100,
 SDMA_FMT_RGBA_1010102 = 0b110100,
 SDMA_FMT_AYUV_10101010 = 0b110111,
 SDMA_FMT_YUV444 = 0b1110,
 SDMA_FMT_YVU444 = 0b1111,
 SDMA_FMT_YUYV_422_8BIT_PKG = 0b10000,
 SDMA_FMT_VYUY_422_8BIT_PKG = 0b10010,
 SDMA_FMT_UYVY_422_8BIT_PKG = 0b10011,
 SDMA_FMT_YUYV_422_8BIT_SP = 0b101001,
 SDMA_FMT_YUYV_422_8BIT_P = 0b101010,
 SDMA_FMT_YUYV_422_10BIT_PKG = 0b11000,
 SDMA_FMT_YUYV_422_10BIT_SP = 0b101011,
 SDMA_FMT_YUYV_422_10BIT_P = 0b101100,
 SDMA_FMT_YVYU_422_8BIT_PKG = 0b10001,
 SDMA_FMT_YUYV_420_8BIT_SP = 0b101111,
 SDMA_FMT_YUYV_420_8BIT_P = 0b110000,
 SDMA_FMT_YUYV_420_10BIT_SP = 0b110001,
 SDMA_FMT_YUYV_420_10BIT_P = 0b110010,
 SDMA_FMT_YUVA_1010102 = 0b110100,
 SDMA_FMT_UYVA_1010102 = 0b110100,
 SDMA_FMT_VUYA_1010102 = 0b110100,
 SDMA_FMT_D1_128 = 0b110100,
 SDMA_FMT_D3_128 = 0b110101,
 SDMA_FMT_D3_RGBG = 0b111010,
 SDMA_FMT_YUV_444_P = 0b101000,
};
enum DPU_WDMA_STATIC_FORMAT {
 WDMA_FMT_RGB_565 = 0x0,
 WDMA_FMT_ARGB_4444 = 0x1,
 WDMA_FMT_XRGB_4444 = 0x2,
 WDMA_FMT_ARGB_5551 = 0x3,
 WDMA_FMT_XRGB_5551 = 0x4,
 WDMA_FMT_ARGB_8888 = 0x5,
 WDMA_FMT_XRGB_8888 = 0x6,
 WDMA_FMT_RESERVED0 = 0x7,
 WDMA_FMT_YUYV_422_PKG = 0x8,
 WDMA_FMT_YUV_420_SP_HP = 0x9,
 WDMA_FMT_YUV_420_P_HP = 0xA,
 WDMA_FMT_YUV_422_SP_HP = 0xB,
 WDMA_FMT_YUV_422_P_HP = 0xC,
 WDMA_FMT_AYUV_4444 = 0xD,
 WDMA_FMT_YUV_444_P = 0xE,
 WDMA_FMT_RESERVED2 = 0xF,
 WDMA_FMT_RGBA_1010102 = 0x10,
 WDMA_FMT_Y410_10BIT = 0x11,
 WDMA_FMT_YUV422_10BIT = 0x12,
 WDMA_FMT_YUV420_SP_10BIT = 0x13,
 WDMA_FMT_YUV422_SP_10BIT = 0x14,
 WDMA_FMT_YUV420_P_10BIT = 0x15,
 WDMA_FMT_YUV422_P_10BIT = 0x16,
 WDMA_FMT_RGB_DELTA_8BIT = 0b10101,
 WDMA_FMT_RGB_DELTA_10BIT = 0b10110,
 WDMA_FMT_RGBG_8BIT = 0b10111,
 WDMA_FMT_RGBG_10BIT = 0b11000,
};
enum DPU_DFC_STATIC_FORMAT {
 DFC_STATIC_FMT_RGB_565 = 0x0,
 DFC_STATIC_FMT_XRGB_4444 = 0x1,
 DFC_STATIC_FMT_ARGB_4444 = 0x2,
 DFC_STATIC_FMT_XRGB_1555 = 0x3,
 DFC_STATIC_FMT_ARGB_1555 = 0x4,
 DFC_STATIC_FMT_XRGB_8888 = 0x5,
 DFC_STATIC_FMT_ARGB_8888 = 0x6,
 DFC_STATIC_FMT_BGR_565 = 0x7,
 DFC_STATIC_FMT_XBGR_4444 = 0x8,
 DFC_STATIC_FMT_ABGR_4444 = 0x9,
 DFC_STATIC_FMT_XBGR_1555 = 0xA,
 DFC_STATIC_FMT_ABGR_1555 = 0xB,
 DFC_STATIC_FMT_XBGR_8888 = 0xC,
 DFC_STATIC_FMT_ABGR_8888 = 0xD,
 DFC_STATIC_FMT_YUV444 = 0xE,
 DFC_STATIC_FMT_YVU444 = 0xF,
 DFC_STATIC_FMT_YUYV422 = 0x10,
 DFC_STATIC_FMT_YUYV420_SP_8BIT = 0x10,
 DFC_STATIC_FMT_YVYU422 = 0x11,
 DFC_STATIC_FMT_VYUY422 = 0x12,
 DFC_STATIC_FMT_UYVY422 = 0x13,
 DFC_STATIC_FMT_RGBA_1010102 = 0x14,
 DFC_STATIC_FMT_YUVA_1010102 = 0x15,
 DFC_STATIC_FMT_UYVA_1010102 = 0x16,
 DFC_STATIC_FMT_VUYA_1010102 = 0x17,
 DFC_STATIC_FMT_YUYV_10 = 0x18,
 DFC_STATIC_FMT_YUYV420_SP_10BIT = 0x18,
 DFC_STATIC_FMT_UYVY_10 = 0x19,
 DFC_STATIC_FMT_UYVY_Y8 = 0x1A,
 DFC_STATIC_FMT_BGRA_1010102 = 0x34,
 DFC_STATIC_FMT_AYUV_10101010 = SDMA_FMT_AYUV_10101010,
 DFC_STATIC_FMT_XRGB_10101010 = SDMA_FMT_XRGB_10101010,
 DFC_STATIC_FMT_ARGB_10101010 = SDMA_FMT_ARGB_10101010,
};
enum DPU_DFC_DYNAMIC_FORMAT {
 DFC_DYNAMIC_FMT_RGB_565 = 0x0,
 DFC_DYNAMIC_FMT_XRGB_4444 = 0x1,
 DFC_DYNAMIC_FMT_ARGB_4444 = 0x2,
 DFC_DYNAMIC_FMT_XRGB_1555 = 0x3,
 DFC_DYNAMIC_FMT_ARGB_1555 = 0x4,
 DFC_DYNAMIC_FMT_XRGB_8888 = 0x5,
 DFC_DYNAMIC_FMT_ARGB_8888 = 0x6,
 DFC_DYNAMIC_FMT_BGR_565 = 0x7,
 DFC_DYNAMIC_FMT_XBGR_4444 = 0x8,
 DFC_DYNAMIC_FMT_ABGR_4444 = 0x9,
 DFC_DYNAMIC_FMT_XBGR_1555 = 0xA,
 DFC_DYNAMIC_FMT_ABGR_1555 = 0xB,
 DFC_DYNAMIC_FMT_XBGR_8888 = 0xC,
 DFC_DYNAMIC_FMT_ABGR_8888 = 0xD,
 DFC_DYNAMIC_FMT_YUV422_PACKET_8BIT = 0x10,
 DFC_DYNAMIC_FMT_YUV_444_P = 0x28,
 DFC_DYNAMIC_FMT_YUV422_SP_8BIT = 0x29,
 DFC_DYNAMIC_FMT_YUV422_P_8BIT = 0x2A,
 DFC_DYNAMIC_FMT_YUV422_PACKET_10BIT = 0x18,
 DFC_DYNAMIC_FMT_YUV422_SP_10BIT = 0x2B,
 DFC_DYNAMIC_FMT_YUV422_P_10BIT = 0x2C,
 DFC_DYNAMIC_FMT_YUV420_SP_8BIT = 0x2F,
 DFC_DYNAMIC_FMT_YUV420_P_8BIT = 0x30,
 DFC_DYNAMIC_FMT_YUV420_SP_10BIT = 0x31,
 DFC_DYNAMIC_FMT_YUV420_P_10BIT = 0x32,
 DFC_DYNAMIC_FMT_YUV444 = 0xE,
 DFC_DYNAMIC_FMT_YVU444 = 0xF,
 DFC_DYNAMIC_FMT_YUYV422 = 0x10,
 DFC_DYNAMIC_FMT_YVYU422 = 0x11,
 DFC_DYNAMIC_FMT_VYUY422 = 0x12,
 DFC_DYNAMIC_FMT_UYVY422 = 0x13,
 DFC_DYNAMIC_FMT_RGBA_1010102 = 0x14,
 DFC_DYNAMIC_FMT_YUVA_1010102 = 0x15,
 DFC_DYNAMIC_FMT_UYVA_1010102 = 0x16,
 DFC_DYNAMIC_FMT_VUYA_1010102 = 0x17,
 DFC_DYNAMIC_FMT_YUYV_10 = 0x18,
 DFC_DYNAMIC_FMT_UYVY_10 = 0x19,
 DFC_DYNAMIC_FMT_UYVY_Y8 = 0x1A,
 DFC_DYNAMIC_FMT_BGRA_1010102 = 0x34,
 DFC_DYNAMIC_FMT_AYUV_10101010 = SDMA_FMT_AYUV_10101010,
 DFC_DYNAMIC_FMT_XRGB_10101010 = SDMA_FMT_XRGB_10101010,
 DFC_DYNAMIC_FMT_ARGB_10101010 = SDMA_FMT_ARGB_10101010,
};
enum DmaTransformation {
 DMA_TRANSFORM_NOP = 0x0,
 DMA_TRANSFORM_ROT = 0x1,
 DMA_TRANSFORM_FLIP_H = 0x2,
 DMA_TRANSFORM_FLIP_V = 0x04,
};
enum WdmaCompressType {
 WDMA_LINEAR = 0,
 WDMA_HEBC = 1,
 WDMA_HEMC = 3,
};
#endif