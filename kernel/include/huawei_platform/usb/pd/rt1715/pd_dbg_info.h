/*
 * Copyright (C) 2023 Richtek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef PD_DBG_INFO_H_INCLUDED
#define PD_DBG_INFO_H_INCLUDED

#ifdef CONFIG_PD_DBG_INFO
extern int pd_dbg_info(const char *fmt, ...);
extern void __init pd_dbg_info_init(void);
extern void __exit pd_dbg_info_exit(void);
#else
static inline int pd_dbg_info(const char *fmt, ...)
{
	return 0;
}

static inline void __init pd_dbg_info_init(void) {}
static inline void __exit pd_dbg_info_exit(void) {}
#endif	/* CONFIG_PD_DBG_INFO */

#endif /* PD_DBG_INFO_H_INCLUDED */
