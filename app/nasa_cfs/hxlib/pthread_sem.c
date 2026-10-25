/* AI GENERATED CODE FOR SEMAPHORE */

/* pthread_sem.c
 *
 * POSIX unnamed semaphore layer on top of the kernel primitives:
 *
 *   HANDLE CreateSemaphore(int max_count, int curr_count);
 *   VOID   DestroySemaphore(HANDLE hSemaphore);
 *   DWORD  ReleaseSemaphore(HANDLE hSemaphore);
 *   unsigned long GetSemaphoreValue(HANDLE hSemaphore);
 *   DWORD  WaitForThisObject(HANDLE hObject);
 *   DWORD  WaitForThisObjectEx(HANDLE hObject, DWORD dwMillionSecond);
 *
 * Known semantic gaps (documented, not fixable with the given API):
 *
 *   1. pshared != 0 returns ENOSYS, because the kernel API does not
 *      demonstrate that HANDLEs are usable across processes.
 *   2. EINTR cannot be reported: WaitForThisObject does not distinguish
 *      "interrupted by a signal" from "internal error".
 *   3. EOVERFLOW on sem_post cannot be detected: ReleaseSemaphore
 *      returns no status.
 *   4. No named semaphores: sem_open/sem_close/sem_unlink are absent.
 */

#include <hellox.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

 /* Internal helper: returns non-zero if the semaphore object is usable. */
static int sem_is_valid(const sem_t *sem)
{
	return sem != NULL && sem->valid != 0;
}

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	HANDLE h;

	if (sem == NULL || value > (unsigned int)SEM_VALUE_MAX) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * The kernel API gives no evidence that a HANDLE created in one
	 * process is usable in another. Report cross-process sharing as
	 * unsupported rather than pretending it works.
	 */
	if (pshared != 0) {
		errno = ENOSYS;
		return -1;
	}

	h = CreateSemaphore((int)SEM_VALUE_MAX, (int)value);
	if (h == INVALID_HANDLE_VALUE) {
		/* CreateSemaphore has no documented error code; assume resources. */
		errno = ENOSPC;
		return -1;
	}

	sem->handle = h;
	sem->pshared = pshared;
	sem->valid = 1;
	return 0;
}

int sem_destroy(sem_t *sem)
{
	if (!sem_is_valid(sem)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * Mark invalid first so that concurrent wrapper calls that observe
	 * "valid == 0" fail fast instead of racing with DestroySemaphore.
	 * Note: destroying a semaphore that has waiters is undefined by
	 * POSIX; our kernel wakes them with OBJECT_WAIT_DELETED, which
	 * sem_wait/sem_timedwait translate to EINVAL.
	 */
	sem->valid = 0;
	DestroySemaphore(sem->handle);
	sem->handle = INVALID_HANDLE_VALUE;
	return 0;
}

int sem_wait(sem_t *sem)
{
	DWORD r;

	if (!sem_is_valid(sem)) {
		errno = EINVAL;
		return -1;
	}

	r = WaitForThisObject(sem->handle);
	if (r == OBJECT_WAIT_RESOURCE)
		return 0;

	if (r == OBJECT_WAIT_DELETED) {
		errno = EINVAL;
		return -1;
	}

	/* OBJECT_WAIT_FAILED or an unknown status. */
	errno = EINVAL;
	return -1;
}

int sem_trywait(sem_t *sem)
{
	DWORD r;

	if (!sem_is_valid(sem)) {
		errno = EINVAL;
		return -1;
	}

	r = WaitForThisObjectEx(sem->handle, 0);
	if (r == OBJECT_WAIT_RESOURCE)
		return 0;

	if (r == OBJECT_WAIT_TIMEOUT) {
		errno = EAGAIN;
		return -1;
	}

	if (r == OBJECT_WAIT_DELETED) {
		errno = EINVAL;
		return -1;
	}

	errno = EINVAL;
	return -1;
}

int sem_timedwait(sem_t *SEM_RESTRICT sem,
	const struct timespec *SEM_RESTRICT abstime)
{
	struct timespec now;
	int64_t  diff_ns;
	DWORD    ms;
	DWORD    r;

	if (!sem_is_valid(sem) || abstime == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (abstime->tv_nsec < 0 || abstime->tv_nsec >= 1000000000L) {
		errno = EINVAL;
		return -1;
	}

	if (clock_gettime(CLOCK_REALTIME, &now) != 0)
		return -1; /* clock_gettime sets errno. */

	/*
	 * POSIX sem_timedwait takes an absolute CLOCK_REALTIME deadline,
	 * but WaitForThisObjectEx takes a relative timeout in milliseconds.
	 */
	diff_ns = ((int64_t)abstime->tv_sec - (int64_t)now.tv_sec) * 1000000000LL
		+ ((int64_t)abstime->tv_nsec - (int64_t)now.tv_nsec);

	if (diff_ns <= 0) {
		/*
		 * Deadline already elapsed. POSIX still requires us to check
		 * whether the semaphore can be acquired immediately, so use a
		 * zero timeout rather than failing outright.
		 */
		ms = 0;
	}
	else {
		uint64_t ms64 = ((uint64_t)diff_ns + 999999ULL) / 1000000ULL;
		ms = (ms64 > 0xFFFFFFFFULL) ? 0xFFFFFFFFUL : (DWORD)ms64;
	}

	r = WaitForThisObjectEx(sem->handle, ms);
	if (r == OBJECT_WAIT_RESOURCE)
		return 0;

	if (r == OBJECT_WAIT_TIMEOUT) {
		errno = ETIMEDOUT;
		return -1;
	}

	if (r == OBJECT_WAIT_DELETED) {
		errno = EINVAL;
		return -1;
	}

	errno = EINVAL;
	return -1;
}

int sem_post(sem_t *sem)
{
	if (!sem_is_valid(sem)) {
		errno = EINVAL;
		return -1;
	}

	/*
	 * ReleaseSemaphore returns the previous value but has no error
	 * status, so overflow cannot be reported as EOVERFLOW. Capping the
	 * kernel maximum at SEM_VALUE_MAX in sem_init() is the closest
	 * approximation available.
	 */
	(void)ReleaseSemaphore(sem->handle);
	return 0;
}

int sem_getvalue(sem_t *SEM_RESTRICT sem, int *SEM_RESTRICT sval)
{
	unsigned long v;

	if (!sem_is_valid(sem) || sval == NULL) {
		errno = EINVAL;
		return -1;
	}

	v = GetSemaphoreValue(sem->handle);
	*sval = (v > (unsigned long)SEM_VALUE_MAX) ? SEM_VALUE_MAX : (int)v;
	return 0;
}
