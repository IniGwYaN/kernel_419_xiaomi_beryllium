/* SPDX-License-Identifier: GPL-2.0 */
/*
 * fsociety tuner (drivers/misc/fsociety_tuner.c): power profiles for the POCO F1.
 */
#ifndef _LINUX_FSOCIETY_H
#define _LINUX_FSOCIETY_H

#include <linux/types.h>

enum {
	FSOC_ECO,
	FSOC_BALANCED,
	FSOC_GAME,
};

#ifdef CONFIG_FSOCIETY_TUNER
int fsociety_profile(void);
bool fsociety_screen_on(void);
bool fsociety_mglru_locked(void);
/* kgsl side: re-apply the GPU ceiling; force also drops a manual max_pwrlevel override */
void fsociety_kgsl_update(bool force);
#else
static inline int fsociety_profile(void) { return FSOC_BALANCED; }
static inline bool fsociety_screen_on(void) { return true; }
static inline bool fsociety_mglru_locked(void) { return false; }
static inline void fsociety_kgsl_update(bool force) { }
#endif

#endif /* _LINUX_FSOCIETY_H */
