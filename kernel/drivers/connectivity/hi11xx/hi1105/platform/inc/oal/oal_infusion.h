/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description: Infusion static check macro definition
 * Author: @CompanyNameTag
 */

#ifndef OAL_INFUSION_H
#define OAL_INFUSION_H

/* ∫Í∂®“Â */
#define OAL_STATIC static
#ifdef INLINE_TO_FORCEINLINE
#define OAL_INLINE __forceinline
#else
#define OAL_INLINE inline
#endif

#endif /* end of oal_ext_if.h */
