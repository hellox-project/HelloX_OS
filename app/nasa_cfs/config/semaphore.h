/* semaphore.h compatibility shim: maps to hxlib's pthread.h, which
 * provides the POSIX unnamed semaphore API (sem_t, sem_init, sem_wait,
 * sem_trywait, sem_timedwait, sem_post, sem_getvalue). */
#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include "pthread.h"

#endif /* __SEMAPHORE_H__ */
