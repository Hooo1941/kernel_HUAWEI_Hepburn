/*
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 * 功能说明 : 2G/5G/6G频段管制域信息定义
 * 作    者 : wifi
 * 创建日期 : 2022年11月29日
 */

#include "mac_regdomain_bandwidth.h"
#include "mac_device.h"
#include "securec.h"

const mac_supp_mode_table_stru g_bw_mode_table_2g[WLAN_2G_CHANNEL_NUM] = {
    /* 1  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 2  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 3  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 4  */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS } },
    /* 5  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 6  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 7  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 8  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 9  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_40MINUS } },
    /* 10 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 11 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 12 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 13 */
    { 2, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS } },
    /* 14 */
    { 1, { WLAN_BAND_WIDTH_20M } },
};

const mac_supp_mode_table_stru g_bw_mode_table_5g[WLAN_5G_CHANNEL_NUM] = {

#ifdef _PRE_WLAN_FEATURE_160M
    /* 36  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 40  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 44  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 48  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 52  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 56  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 60  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 64  */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },

    /* 100 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 104 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 108 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 112 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 116 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 120 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 124 */
    {   4,
        { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 128 */
    {   4,
        { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
#else
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 36  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 40  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 44  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 48  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 52  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 56  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 60  */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 64  */

    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 100 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 104 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 108 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 112 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 116 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 120 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 124 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 128 */
#endif

    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 132 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 136 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 140 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 144 */

    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 149 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 153 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 157 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 161 */
    { 1, { WLAN_BAND_WIDTH_20M } },                                                        /* 165 */

    /* for JP 4.9G */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSPLUS } },    /* 184 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSPLUS } },  /* 188 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40PLUS, WLAN_BAND_WIDTH_80PLUSMINUS } },   /* 192 */
    { 3, { WLAN_BAND_WIDTH_20M, WLAN_BAND_WIDTH_40MINUS, WLAN_BAND_WIDTH_80MINUSMINUS } }, /* 196 */
};

#ifdef _PRE_WLAN_FEATURE_6G_EXTEND
/* 6G信道带宽能力 */
const mac_supp_mode_table_stru g_bw_mode_table_6g[WLAN_6G_CHANNEL_NUM] = {
    /* 1信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 5信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 9信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 13信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 17信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 21信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 25信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 29信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 33信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 37信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 41信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 45信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 49信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 53信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 57信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 61信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 65信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 69信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 73信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 77信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 81信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 85信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 89信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 93信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 97信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 101信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 105信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 109信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 113信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 117信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 121信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 125信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 129信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 133信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 137信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 141信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 145信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 149信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 153信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 157信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 161信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 165信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 169信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 173信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 177信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 181信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 185信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 189信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 193信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSPLUS
        }
    },
    /* 197信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSPLUS
        }
    },
    /* 201信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSPLUS
        }
    },
    /* 205信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSPLUS
        }
    },
    /* 209信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSPLUS,
          WLAN_BAND_WIDTH_160PLUSPLUSMINUS
        }
    },
    /* 213信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSPLUS,
          WLAN_BAND_WIDTH_160MINUSPLUSMINUS
        }
    },
    /* 217信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
          WLAN_BAND_WIDTH_80PLUSMINUS,
          WLAN_BAND_WIDTH_160PLUSMINUSMINUS
        }
    },
    /* 221信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
          WLAN_BAND_WIDTH_80MINUSMINUS,
          WLAN_BAND_WIDTH_160MINUSMINUSMINUS
        }
    },
    /* 225信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40MINUS,
        }
    },
    /* 229信道 */
    {   4,
        { WLAN_BAND_WIDTH_20M,
          WLAN_BAND_WIDTH_40PLUS,
        }
    },
    /* 233信道 */
    {4, {WLAN_BAND_WIDTH_20M}},
};
#endif
