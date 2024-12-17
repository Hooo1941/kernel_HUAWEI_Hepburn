/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:PCIE host header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_HOST_H
#define PCIE_HOST_H

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_GF61_HOST)
#include "pcie_host_gfxx.h"
#else
#include "pcie_host_mpxx.h"
#endif

#endif
