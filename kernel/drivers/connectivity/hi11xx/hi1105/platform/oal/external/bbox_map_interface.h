/**
 * Copyright (c) @CompanyNameMagicTag 2021-2023. All rights reserved.
 *
 * Description:bbox_map_interface.h header file
 * Author: @CompanyNameTag
 */
#ifndef BBOX_MAP_INTERFACE_H
#define BBOX_MAP_INTERFACE_H

#ifdef CONFIG_ARCH_PLATFORM
extern void *dfx_bbox_map(phys_addr_t paddr, size_t size);
extern void dfx_bbox_unmap(const void *vaddr);
#define external_bbox_map dfx_bbox_map
#define external_bbox_unmap dfx_bbox_unmap
#else
extern void *hisi_bbox_map(phys_addr_t paddr, size_t size);
extern void hisi_bbox_unmap(const void *vaddr);
#define external_bbox_map hisi_bbox_map
#define external_bbox_unmap hisi_bbox_unmap
#endif /* end for CONFIG_ARCH_PLATFORM */

#endif /* end for BBOX_MAP_INTERFACE_H */
