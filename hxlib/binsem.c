/*
 * bin_sem.c
 *
 * Binary semaphore implemented on top of mutex + condition variable.
 *
 * Design summary:
 *
 *   Give:
 *     lock(mutex);
 *     if (val < 1) {
 *         val = 1;
 *         cond_signal(cond);
 *     }
 *     unlock(mutex);
 *
 *   Take:
 *     lock(mutex);
 *     local_gen = generation;
 *     while ((val < 1) && (local_gen == generation)) {
 *         cond_wait(cond, mutex);
 *     }
 *     if (local_gen != generation) {
 *         // Woken up by flush, do not consume val.
 *         unlock(mutex);
 *         return SUCCESS;
 *     }
 *     // Woken up by give, consume val.
 *     val = 0;
 *     unlock(mutex);
 *     return SUCCESS;
 *
 *   Flush:
 *     lock(mutex);
 *     generation++;
 *     cond_broadcast(cond);
 *     // val is intentionally left unchanged.
 *     unlock(mutex);
 */

#include <hellox.h>
#include <stdlib.h>
#include <string.h>
#include "binsem.h"

int bin_sem_init(bin_sem_t *sem)
{
	int ret;

	if (sem == NULL) {
		return -1;
	}

	/*
	 * Zero the structure first to avoid leaving stale data behind.
	 * Note: clear the memory before calling pthread_mutex_init /
	 * pthread_cond_init.
	 */
	memset(sem, 0, sizeof(*sem));
	sem->val = 0;
	sem->generation = 0;

	ret = pthread_mutex_init(&sem->mutex, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = pthread_cond_init(&sem->cond, NULL);
	if (ret != 0) {
		/* Roll back the mutex initialization. */
		(void)pthread_mutex_destroy(&sem->mutex);
		return ret;
	}

	return 0;
}

int bin_sem_destroy(bin_sem_t *sem)
{
	int ret;

	if (sem == NULL) {
		return -1;
	}

	/*
	 * Note: the caller must guarantee that no thread is currently
	 * waiting on this semaphore. Otherwise pthread_cond_destroy
	 * has undefined behavior.
	 */
	ret = pthread_cond_destroy(&sem->cond);
	if (ret != 0) {
		return ret;
	}

	ret = pthread_mutex_destroy(&sem->mutex);
	if (ret != 0) {
		return ret;
	}

	sem->val = 0;
	sem->generation = 0;

	return 0;
}

int bin_sem_give(bin_sem_t *sem)
{
	int ret;

	if (sem == NULL) {
		return -1;
	}

	ret = pthread_mutex_lock(&sem->mutex);
	if (ret != 0) {
		return ret;
	}

	if (sem->val < 1) {
		sem->val = 1;
		(void)pthread_cond_signal(&sem->cond);
	}
	/* val is already 1: binary semaphore does not accumulate, just ignore. */

	(void)pthread_mutex_unlock(&sem->mutex);
	return 0;
}

int bin_sem_take(bin_sem_t *sem)
{
	int      ret;
	uint32_t local_gen;

	if (sem == NULL) {
		return -1;
	}

	ret = pthread_mutex_lock(&sem->mutex);
	if (ret != 0) {
		return ret;
	}

	local_gen = sem->generation;

	/*
	 * Wait condition: val is still 0 and generation has not changed.
	 *   - As soon as val becomes 1 (by give), the loop exits.
	 *   - As soon as generation changes (by flush), the loop exits.
	 *
	 * A while loop is used instead of an if because:
	 *   1) Condition variables may have spurious wakeups.
	 *   2) When multiple waiters compete, a woken-up thread may find
	 *      that val has already been consumed by another thread and
	 *      must go back to waiting.
	 */
	while ((sem->val < 1) && (local_gen == sem->generation)) {
		(void)pthread_cond_wait(&sem->cond, &sem->mutex);
	}

	if (local_gen != sem->generation) {
		/*
		 * Woken up by flush: do not consume val, just return success.
		 * Note that val is left unchanged, so if val happens to be 1,
		 * other waiters can still take it successfully.
		 */
		(void)pthread_mutex_unlock(&sem->mutex);
		return 0;
	}

	/* Woken up by give: consume val by setting it back to 0. */
	sem->val = 0;

	(void)pthread_mutex_unlock(&sem->mutex);
	return 0;
}

int bin_sem_flush(bin_sem_t *sem)
{
	int ret;

	if (sem == NULL) {
		return -1;
	}

	ret = pthread_mutex_lock(&sem->mutex);
	if (ret != 0) {
		return ret;
	}

	/*
	 * Increment generation so that all waiters that captured the old
	 * generation can tell, after they wake up, whether they were
	 * woken up by flush or by give. val is left unchanged.
	 */
	sem->generation++;

	(void)pthread_cond_broadcast(&sem->cond);

	(void)pthread_mutex_unlock(&sem->mutex);
	return 0;
}
