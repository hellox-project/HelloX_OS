/*
 * bin_sem.h
 *
 * Binary semaphore implemented on top of mutex + condition variable.
 * Intended for porting NASA cFS to the HelloX OS.
 *
 * Semantics:
 *   - val is either 0 or 1.
 *   - give  : if val is 0, set it to 1 and signal; if val is already 1, do nothing.
 *   - take  : if val is 1, set it to 0 and return; if val is 0, block and wait.
 *   - flush : increment generation and broadcast to wake up all waiters,
 *             without changing val. A take that is woken up by flush
 *             returns success immediately without consuming val.
 *
 * Return value convention:
 *   Return 0 (equivalent to cFS OS_SUCCESS) on success, non-zero error
 *   code on failure. This matches the POSIX convention so that the caller
 *   can use strerror() to print the error.
 */

#ifndef __BIN_SEM_H__
#define __BIN_SEM_H__

#include <pthread.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct bin_sem {
		pthread_mutex_t   mutex;       /* Protects val and generation. */
		pthread_cond_t    cond;        /* Blocks / wakes up waiters. */
		volatile int      val;         /* Binary value: 0 or 1. */
		volatile uint32_t generation;  /* Generation counter, distinguishes give from flush wakeup. */
	} bin_sem_t;

	/*
	 * Initialize the binary semaphore.
	 * Initial val = 0, generation = 0.
	 * Returns 0 on success, non-zero error code on failure.
	 */
	int bin_sem_init(bin_sem_t *sem);

	/*
	 * Destroy the binary semaphore.
	 * The caller must ensure no thread is waiting on it before calling.
	 * Returns 0 on success, non-zero error code on failure.
	 */
	int bin_sem_destroy(bin_sem_t *sem);

	/*
	 * Give operation: release the semaphore.
	 * If val is already 1, do nothing (binary semaphore does not accumulate).
	 * Returns 0 on success, non-zero error code on failure.
	 */
	int bin_sem_give(bin_sem_t *sem);

	/*
	 * Take operation: acquire the semaphore.
	 * If val is 1, set it to 0 and return immediately.
	 * If val is 0, block and wait until woken up by give or by flush.
	 *   - Woken up by give : consume val and return success.
	 *   - Woken up by flush: do not consume val, return success directly.
	 * Returns 0 on success, non-zero error code on failure.
	 */
	int bin_sem_take(bin_sem_t *sem);

	/*
	 * Flush operation: wake up all waiters without changing val.
	 * Used to make all threads blocked in take return immediately,
	 * for example in a task-cancellation scenario.
	 * Returns 0 on success, non-zero error code on failure.
	 */
	int bin_sem_flush(bin_sem_t *sem);

#ifdef __cplusplus
}
#endif

#endif /* __BIN_SEM_H__ */
