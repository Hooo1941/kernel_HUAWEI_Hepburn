/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * This program is free software and is provided to you under the terms of the
 * GNU General Public License version 2 as published by the Free Software
 * Foundation, and any use by you of this program is subject to the terms
 * of such GNU licence.
 */

#ifndef HVGR_GPU_ID_H
#define HVGR_GPU_ID_H

/* GPU_ID register */
#define GPU_ID_VERSION_STATUS_SHIFT      0
#define GPU_ID_VERSION_MINOR_SHIFT       4
#define GPU_ID_VERSION_MAJOR_SHIFT       12
#define GPU_ID_VERSION_PRODUCT_ID_SHIFT  16
#define GPU_ID_VERSION_STATUS        ((u32)0xF  << GPU_ID_VERSION_STATUS_SHIFT)
#define GPU_ID_VERSION_MINOR         ((u32)0xFF << GPU_ID_VERSION_MINOR_SHIFT)
#define GPU_ID_VERSION_MAJOR         ((u32)0xF  << GPU_ID_VERSION_MAJOR_SHIFT)
#define GPU_ID_VERSION_PRODUCT_ID \
	((u32)0xFFFF << GPU_ID_VERSION_PRODUCT_ID_SHIFT)

#define GPU_ID1_VERSION_STATUS_SHIFT      0
#define GPU_ID1_VERSION_MINOR_SHIFT       4
#define GPU_ID1_VERSION_MAJOR_SHIFT       12
#define GPU_ID1_PRODUCT_MAJOR_SHIFT       16
#define GPU_ID1_ARCH_REV_SHIFT            20
#define GPU_ID1_ARCH_MINOR_SHIFT          24
#define GPU_ID1_ARCH_MAJOR_SHIFT          28
#define GPU_ID1_VERSION_STATUS       ((u32)0xF << GPU_ID1_VERSION_STATUS_SHIFT)
#define GPU_ID1_VERSION_MINOR        ((u32)0xFF << GPU_ID1_VERSION_MINOR_SHIFT)
#define GPU_ID1_VERSION_MAJOR        ((u32)0xF << GPU_ID1_VERSION_MAJOR_SHIFT)
#define GPU_ID1_PRODUCT_MAJOR        ((u32)0xF << GPU_ID1_PRODUCT_MAJOR_SHIFT)
#define GPU_ID1_ARCH_REV             ((u32)0xF << GPU_ID1_ARCH_REV_SHIFT)
#define GPU_ID1_ARCH_MINOR           ((u32)0xF << GPU_ID1_ARCH_MINOR_SHIFT)
#define GPU_ID1_ARCH_MAJOR           ((u32)0xF << GPU_ID1_ARCH_MAJOR_SHIFT)
#define GPU_ID1_PRODUCT_MODEL  (GPU_ID1_ARCH_MAJOR | GPU_ID1_PRODUCT_MAJOR)
#define GPU_ID1_VERSION        (GPU_ID1_VERSION_MAJOR | GPU_ID1_VERSION_MINOR | \
					GPU_ID1_VERSION_STATUS)

/*
 * Helper macro to create a partial GPU_ID (new format) that defines
 * a product ignoring its version.
 */
#define gpu_id2_product_make(arch_major, arch_minor, arch_rev, product_major) \
		((((u32)(arch_major)) << GPU_ID1_ARCH_MAJOR_SHIFT)  | \
		 (((u32)(arch_minor)) << GPU_ID1_ARCH_MINOR_SHIFT)  | \
		 (((u32)(arch_rev)) << GPU_ID1_ARCH_REV_SHIFT)      | \
		 (((u32)(product_major)) << GPU_ID1_PRODUCT_MAJOR_SHIFT))

/*
 * Helper macro to create a partial GPU_ID (new format) that specifies the
 * revision (major, minor, status) of a product.
 */
#define gpu_id2_version_make(version_major, version_minor, version_status) \
		((((u32)(version_major)) << GPU_ID1_VERSION_MAJOR_SHIFT)  | \
		 (((u32)(version_minor)) << GPU_ID1_VERSION_MINOR_SHIFT)  | \
		 (((u32)(version_status)) << GPU_ID1_VERSION_STATUS_SHIFT))

/* Helper macro to create a complete GPU_ID (new format) */
#define gpu_id2_make(arch_major, arch_minor, arch_rev, product_major, \
	version_major, version_minor, version_status) \
		(gpu_id2_product_make(arch_major, arch_minor, arch_rev, \
			product_major) | \
		 gpu_id2_version_make(version_major, version_minor,     \
			version_status))

/*
 * Helper macro to create a partial GPU_ID (new format) that identifies
 * a particular GPU model by its arch_major and product_major.
 */
#define gpu_id2_model_make(arch_major, product_major) \
		((((u32)(arch_major)) << GPU_ID1_ARCH_MAJOR_SHIFT)  | \
		(((u32)(product_major)) << GPU_ID1_PRODUCT_MAJOR_SHIFT))

/*
 * Strip off the non-relevant bits from a product_id value and make it suitable
 * for comparison against the GPU_ID2_PRODUCT_xxx values which identify a GPU
 * model.
 */
#define gpu_id2_model_watch_value(product_id) \
		((((u32)(product_id)) << GPU_ID1_PRODUCT_MAJOR_SHIFT) & \
			GPU_ID1_PRODUCT_MODEL)

#define GPU_ID2_PRODUCT_TV200            gpu_id2_model_make(1u, 0)
#define GPU_ID2_PRODUCT_TV210            gpu_id2_model_make(1u, 1)
#define GPU_ID2_PRODUCT_TV211            gpu_id2_model_make(1u, 2)
#define GPU_ID2_PRODUCT_TV300            gpu_id2_model_make(2u, 0)
#define GPU_ID2_PRODUCT_TV310            gpu_id2_model_make(2u, 1)
#define GPU_ID2_PRODUCT_TV350            gpu_id2_model_make(2u, 2)

#endif /* HVGR_GPU_ID_H */
