/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:PCIE linux header file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_LINUX_H
#define PCIE_LINUX_H

#if (_PRE_PRODUCT_ID == _PRE_PRODUCT_ID_GF61_HOST)
#include "pcie_linux_gfxx.h"
#else
#include "pcie_linux_mpxx.h"
#endif

#endif
