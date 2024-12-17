/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:Defines the address related to the SoC.
 *             Because the host shares the code, it cannot be
 *             contained in other header files.
 *             It can be contained only in the source code file
 * Author: @CompanyNameTag
 */

#ifndef PCIE_SOC_MP16_H
#define PCIE_SOC_MP16_H

#define HISI_PCIE_SLAVE_START_ADDRESS        (0x80000000)
#define HISI_PCIE_SLAVE_END_ADDRESS          (0xFFFFFFFFFFFFFFFF)
#define HISI_PCIE_IP_REGION_SIZE             (HISI_PCIE_SLAVE_END_ADDRESS - HISI_PCIE_SLAVE_START_ADDRESS + 1)
#define HISI_PCIE_MASTER_START_ADDRESS       (0x0)
#define HISI_PCIE_MASTER_END_ADDRESS         (HISI_PCIE_MASTER_START_ADDRESS + HISI_PCIE_IP_REGION_SIZE - 1)
#define HISI_PCIE_IP_REGION_OFFSET           (HISI_PCIE_SLAVE_START_ADDRESS - HISI_PCIE_MASTER_START_ADDRESS)

#endif
