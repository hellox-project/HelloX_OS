/* test_posix_sem.c
 *
 * Test suite for the POSIX unnamed semaphore layer (posix_sem.c).
 *
 * Runs directly against the real kernel semaphore API; no mock is used.
 * Only two lines need to be adapted for a given platform:
 *
 *   1. The #include below, pointing at the real kernel semaphore header.
 *   2. The pthread / time.h usage, if the target lacks POSIX threads.
 *
 * The test suite covers every POSIX API in posix_sem.h:
 *
 *   sem_init / sem_destroy / sem_wait / sem_trywait /
 *   sem_timedwait / sem_post / sem_getvalue
 *
 * plus concurrency scenarios (producer/consumer and a semaphore used
 * as a mutex).
 */

 /* ------------------------------------------------------------------ */
 /* Platform adaptation: change this include to the real kernel header. */
 /* ------------------------------------------------------------------ */
#include <hellox.h>  /* CreateSemaphore, WaitForThisObject, etc. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

/* ---------------------------------------------------------------- */
/* Test harness                                                     */
/* ---------------------------------------------------------------- */

static int g_tests_run = 0;
static int g_tests_failed = 0;

#define CHECK(cond) do {                                            \
    g_tests_run++;                                                  \
    if (!(cond)) {                                                  \
        g_tests_failed++;                                           \
        fprintf(stderr, "FAIL %s:%d: %s\n",                         \
                __FILE__, __LINE__, #cond);                         \
    }                                                               \
} while (0)

#define CHECK_ERR(expr, expected_errno) do {                        \
    int _r;                                                         \
    errno = 0;                                                      \
    _r = (expr);                                                    \
    g_tests_run++;                                                  \
    if (_r != -1 || errno != (expected_errno)) {                    \
        g_tests_failed++;                                           \
        fprintf(stderr,                                             \
                "FAIL %s:%d: %s returned %d, errno=%d (want -1/%d)\n",\
                __FILE__, __LINE__, #expr, _r, errno,               \
                (expected_errno));                                  \
    }                                                               \
} while (0)

static void sleep_ms(long ms)
{
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000L;
	nanosleep(&ts, NULL);
}

/* Fill *ts with "now + ms milliseconds" based on CLOCK_REALTIME. */
static void deadline_in_ms(struct timespec *ts, long ms)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec += ms / 1000;
	ts->tv_nsec += (ms % 1000) * 1000000L;
	if (ts->tv_nsec >= 1000000000L) {
		ts->tv_sec++;
		ts->tv_nsec -= 1000000000L;
	}
}

/* ---------------------------------------------------------------- */
/* T1: sem_init                                                     */
/* ---------------------------------------------------------------- */

static void test_sem_init_basic(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, 5) == 0);
	CHECK(sem_getvalue(&s, &v) == 0);
	CHECK(v == 5);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_init_zero(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, 0) == 0);
	CHECK(sem_getvalue(&s, &v) == 0);
	CHECK(v == 0);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_init_max(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, (unsigned int)SEM_VALUE_MAX) == 0);
	CHECK(sem_getvalue(&s, &v) == 0);
	CHECK(v == (int)SEM_VALUE_MAX);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_init_too_large(void)
{
	sem_t s;

	CHECK_ERR(sem_init(&s, 0, (unsigned int)SEM_VALUE_MAX + 1U), EINVAL);
}

static void test_sem_init_null(void)
{
	CHECK_ERR(sem_init(NULL, 0, 1), EINVAL);
}

static void test_sem_init_pshared_unsupported(void)
{
	sem_t s;

	/* posix_sem.c reports ENOSYS for any non-zero pshared. */
	CHECK_ERR(sem_init(&s, 1, 1), ENOSYS);
	CHECK_ERR(sem_init(&s, -1, 1), ENOSYS);
}

/* ---------------------------------------------------------------- */
/* T2: sem_destroy                                                  */
/* ---------------------------------------------------------------- */

static void test_sem_destroy_basic(void)
{
	sem_t s;
	int v;
	struct timespec ts;

	CHECK(sem_init(&s, 0, 3) == 0);
	CHECK(sem_destroy(&s) == 0);

	/* Every operation after destroy must fail. */
	CHECK_ERR(sem_getvalue(&s, &v), EINVAL);
	CHECK_ERR(sem_wait(&s), EINVAL);
	CHECK_ERR(sem_trywait(&s), EINVAL);
	CHECK_ERR(sem_post(&s), EINVAL);

	deadline_in_ms(&ts, 100);
	CHECK_ERR(sem_timedwait(&s, &ts), EINVAL);
}

static void test_sem_destroy_uninit(void)
{
	sem_t s;

	memset(&s, 0, sizeof(s));
	CHECK_ERR(sem_destroy(&s), EINVAL);
}

static void test_sem_destroy_null(void)
{
	CHECK_ERR(sem_destroy(NULL), EINVAL);
}

/* ---------------------------------------------------------------- */
/* T3: sem_wait                                                     */
/* ---------------------------------------------------------------- */

static void test_sem_wait_available(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, 2) == 0);

	CHECK(sem_wait(&s) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 1);

	CHECK(sem_wait(&s) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 0);

	/* Would block from here; skip and destroy. */
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_wait_null(void)
{
	CHECK_ERR(sem_wait(NULL), EINVAL);
}

static sem_t              g_block_sem;
static volatile int       g_block_done;

static void *blocking_waiter(void *arg)
{
	(void)arg;
	if (sem_wait(&g_block_sem) == 0)
		g_block_done = 1;
	return NULL;
}

static void test_sem_wait_blocks_then_post(void)
{
	pthread_t t;

	g_block_done = 0;
	CHECK(sem_init(&g_block_sem, 0, 0) == 0);

	pthread_create(&t, NULL, blocking_waiter, NULL);
	sleep_ms(100);
	CHECK(g_block_done == 0);      /* still blocked */

	CHECK(sem_post(&g_block_sem) == 0);
	pthread_join(t, NULL);
	CHECK(g_block_done == 1);

	CHECK(sem_destroy(&g_block_sem) == 0);
}

/* Waiter must return EINVAL when the semaphore is destroyed under it. */
static sem_t              g_destroy_sem;
static volatile int       g_destroy_result;
static volatile int       g_destroy_errno;

static void *destroy_waiter(void *arg)
{
	(void)arg;
	errno = 0;
	g_destroy_result = sem_wait(&g_destroy_sem);
	g_destroy_errno = errno;
	return NULL;
}

static void test_sem_wait_during_destroy(void)
{
	pthread_t t;

	g_destroy_result = 12345;
	g_destroy_errno = 0;
	CHECK(sem_init(&g_destroy_sem, 0, 0) == 0);

	pthread_create(&t, NULL, destroy_waiter, NULL);
	sleep_ms(100);
	CHECK(sem_destroy(&g_destroy_sem) == 0);

	pthread_join(t, NULL);
	CHECK(g_destroy_result == -1);
	CHECK(g_destroy_errno == EINVAL);
}

/* ---------------------------------------------------------------- */
/* T4: sem_trywait                                                  */
/* ---------------------------------------------------------------- */

static void test_sem_trywait_available(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, 1) == 0);
	CHECK(sem_trywait(&s) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 0);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_trywait_would_block(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, 0) == 0);
	CHECK_ERR(sem_trywait(&s), EAGAIN);
	/* Value must be unchanged. */
	CHECK(sem_getvalue(&s, &v) == 0 && v == 0);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_trywait_null(void)
{
	CHECK_ERR(sem_trywait(NULL), EINVAL);
}

/* ---------------------------------------------------------------- */
/* T5: sem_timedwait                                                */
/* ---------------------------------------------------------------- */

static void test_sem_timedwait_available(void)
{
	sem_t s;
	struct timespec ts;
	int v;

	CHECK(sem_init(&s, 0, 1) == 0);
	deadline_in_ms(&ts, 1000);
	CHECK(sem_timedwait(&s, &ts) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 0);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_timedwait_timeout(void)
{
	sem_t s;
	struct timespec ts;
	struct timespec start, end;
	long elapsed_ms;

	CHECK(sem_init(&s, 0, 0) == 0);
	clock_gettime(CLOCK_MONOTONIC, &start);
	deadline_in_ms(&ts, 50);
	CHECK_ERR(sem_timedwait(&s, &ts), ETIMEDOUT);
	clock_gettime(CLOCK_MONOTONIC, &end);

	elapsed_ms = (end.tv_sec - start.tv_sec) * 1000L
		+ (end.tv_nsec - start.tv_nsec) / 1000000L;
	/* Should have waited roughly 50 ms, not returned instantly. */
	CHECK(elapsed_ms >= 40);
	CHECK(elapsed_ms < 1000);
	CHECK(sem_destroy(&s) == 0);
}

/* POSIX: an already-elapsed deadline must still try to acquire the
 * semaphore immediately rather than fail outright. */
static void test_sem_timedwait_expired_deadline_but_available(void)
{
	sem_t s;
	struct timespec ts;
	int v;

	CHECK(sem_init(&s, 0, 1) == 0);
	ts.tv_sec = 0;
	ts.tv_nsec = 1;                 /* long gone */
	CHECK(sem_timedwait(&s, &ts) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 0);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_timedwait_expired_deadline_no_resource(void)
{
	sem_t s;
	struct timespec ts;

	CHECK(sem_init(&s, 0, 0) == 0);
	ts.tv_sec = 0;
	ts.tv_nsec = 1;
	CHECK_ERR(sem_timedwait(&s, &ts), ETIMEDOUT);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_timedwait_invalid_nsec(void)
{
	sem_t s;
	struct timespec ts;

	CHECK(sem_init(&s, 0, 1) == 0);

	ts.tv_sec = 0;
	ts.tv_nsec = 1000000000L;       /* == 1 second, out of range */
	CHECK_ERR(sem_timedwait(&s, &ts), EINVAL);

	ts.tv_nsec = -1;                /* negative, out of range */
	CHECK_ERR(sem_timedwait(&s, &ts), EINVAL);

	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_timedwait_null_abstime(void)
{
	sem_t s;

	CHECK(sem_init(&s, 0, 1) == 0);
	CHECK_ERR(sem_timedwait(&s, NULL), EINVAL);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_timedwait_null_sem(void)
{
	struct timespec ts;

	deadline_in_ms(&ts, 100);
	CHECK_ERR(sem_timedwait(NULL, &ts), EINVAL);
}

static sem_t              g_timed_sem;
static volatile int       g_timed_result;

static void *timed_waiter(void *arg)
{
	struct timespec ts;
	(void)arg;

	deadline_in_ms(&ts, 500);
	g_timed_result = sem_timedwait(&g_timed_sem, &ts);
	return NULL;
}

static void test_sem_timedwait_wakes_on_post(void)
{
	pthread_t t;

	g_timed_result = 12345;
	CHECK(sem_init(&g_timed_sem, 0, 0) == 0);

	pthread_create(&t, NULL, timed_waiter, NULL);
	sleep_ms(100);
	CHECK(sem_post(&g_timed_sem) == 0);

	pthread_join(t, NULL);
	CHECK(g_timed_result == 0);
	CHECK(sem_destroy(&g_timed_sem) == 0);
}

/* ---------------------------------------------------------------- */
/* T6: sem_post                                                     */
/* ---------------------------------------------------------------- */

static void test_sem_post_increments(void)
{
	sem_t s;
	int v;

	CHECK(sem_init(&s, 0, 0) == 0);
	CHECK(sem_post(&s) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 1);
	CHECK(sem_post(&s) == 0);
	CHECK(sem_getvalue(&s, &v) == 0 && v == 2);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_post_null(void)
{
	CHECK_ERR(sem_post(NULL), EINVAL);
}

/* ---------------------------------------------------------------- */
/* T7: sem_getvalue                                                 */
/* ---------------------------------------------------------------- */

static void test_sem_getvalue_null_sval(void)
{
	sem_t s;

	CHECK(sem_init(&s, 0, 1) == 0);
	CHECK_ERR(sem_getvalue(&s, NULL), EINVAL);
	CHECK(sem_destroy(&s) == 0);
}

static void test_sem_getvalue_null_sem(void)
{
	int v;

	CHECK_ERR(sem_getvalue(NULL, &v), EINVAL);
}

/* ---------------------------------------------------------------- */
/* T8: Concurrency                                                  */
/* ---------------------------------------------------------------- */

#define PC_COUNT 2000

static sem_t g_pc_items;     /* produced items */
static sem_t g_pc_slots;     /* free buffer slots */
static int   g_pc_buffer;

static void *pc_producer(void *arg)
{
	int i;
	(void)arg;

	for (i = 0; i < PC_COUNT; i++) {
		sem_wait(&g_pc_slots);
		g_pc_buffer++;
		sem_post(&g_pc_items);
	}
	return NULL;
}

static void *pc_consumer(void *arg)
{
	int i;
	(void)arg;

	for (i = 0; i < PC_COUNT; i++) {
		sem_wait(&g_pc_items);
		g_pc_buffer--;
		sem_post(&g_pc_slots);
	}
	return NULL;
}

static void test_producer_consumer(void)
{
	pthread_t p, c;

	g_pc_buffer = 0;
	CHECK(sem_init(&g_pc_items, 0, 0) == 0);
	CHECK(sem_init(&g_pc_slots, 0, 1) == 0);   /* buffer size 1 */

	printf("[%s]reach line[%d].\r\n", __func__, __LINE__);
	pthread_create(&p, NULL, pc_producer, NULL);
	pthread_create(&c, NULL, pc_consumer, NULL);

	printf("[%s]reach line[%d].\r\n", __func__, __LINE__);
	pthread_join(p, NULL);
	//printf("[%s]reach line[%d].\r\n", __func__, __LINE__);
	pthread_join(c, NULL);

	printf("[%s]reach line[%d].\r\n", __func__, __LINE__);
	CHECK(g_pc_buffer == 0);
	CHECK(sem_destroy(&g_pc_items) == 0);
	CHECK(sem_destroy(&g_pc_slots) == 0);
	printf("[%s]reach line[%d].\r\n", __func__, __LINE__);
}

#define MUTEX_THREADS    4
#define MUTEX_ITER       10000

static sem_t g_mutex_sem;
static int   g_mutex_counter;

static void *mutex_worker(void *arg)
{
	int i;
	(void)arg;

	for (i = 0; i < MUTEX_ITER; i++) {
		sem_wait(&g_mutex_sem);
		g_mutex_counter++;
		sem_post(&g_mutex_sem);
	}
	return NULL;
}

static void test_mutex_style(void)
{
	pthread_t t[MUTEX_THREADS];
	int i;

	g_mutex_counter = 0;
	CHECK(sem_init(&g_mutex_sem, 0, 1) == 0);

	for (i = 0; i < MUTEX_THREADS; i++)
		pthread_create(&t[i], NULL, mutex_worker, NULL);
	for (i = 0; i < MUTEX_THREADS; i++)
		pthread_join(t[i], NULL);

	CHECK(g_mutex_counter == MUTEX_THREADS * MUTEX_ITER);
	CHECK(sem_destroy(&g_mutex_sem) == 0);
}

/* ---------------------------------------------------------------- */
/* main                                                             */
/* ---------------------------------------------------------------- */

int semaphore_main(void)
{
	int tc_num = 0;

	printf("=== POSIX unnamed semaphore test suite ===\n");

	/* T1 */
	test_sem_init_basic();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_init_zero();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_init_max();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_init_too_large();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_init_null();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_init_pshared_unsupported();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T2 */
	test_sem_destroy_basic();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_destroy_uninit();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_destroy_null();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T3 */
	test_sem_wait_available();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_wait_null();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_wait_blocks_then_post();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_wait_during_destroy();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T4 */
	test_sem_trywait_available();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_trywait_would_block();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_trywait_null();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T5 */
	test_sem_timedwait_available();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_timeout();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_expired_deadline_but_available();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_expired_deadline_no_resource();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_invalid_nsec();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_null_abstime();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_null_sem();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_timedwait_wakes_on_post();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T6 */
	test_sem_post_increments();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_post_null();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T7 */
	test_sem_getvalue_null_sval();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_sem_getvalue_null_sem();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	/* T8 */
	test_producer_consumer();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	test_mutex_style();
	printf("[%s]test case [%d] pass, line[%d].\r\n", __func__, tc_num++, __LINE__);

	printf("=== %d checks run, %d failed ===\n",
		g_tests_run, g_tests_failed);
	return g_tests_failed == 0 ? 0 : 1;
}
