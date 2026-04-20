/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef __RTW_HASHTABLE_H__
#define __RTW_HASHTABLE_H__

//#if defined(CONFIG_RTW_MESH) ||  defined(CONFIG_RTW_A4_STA)/* for now, only promised for kernel versions we support mesh */

/* directly reference rhashtable in kernel */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
#ifdef PLATFORM_ECOS
#include "hashtable.h"
#else /* !PLATFORM_ECOS */
#include <linux/hashtable.h>
#endif /* PLATFORM_ECOS */
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) */

/* Use rhashtable from kernel 4.4 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
#define NULLS_MARKER(value) (1UL | (((long)value) << 1))
#endif
#include "hashtable.h"
#endif /* (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0)) */


//#endif /* CONFIG_RTW_MESH */

#endif /* __RTW_RHASHTABLE_H__ */

