#ifndef LOCK_DETECTOR_H
#define LOCK_DETECTOR_H

#ifdef CONFIG_DFX_HUNGTASK
void lock_detector_start(void);
#endif

unsigned int is_lock_detector_enabled(void);

#endif
