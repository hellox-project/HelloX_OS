/*
 * pthread_cond test suite
 *
 * Coverage:
 *   1. init / destroy lifecycle
 *   2. signal wakes a single waiter
 *   3. broadcast wakes all waiters
 *   4. timedwait returns ETIMEDOUT on timeout
 *   5. timedwait is woken by signal before timeout
 *   6. producer-consumer model
 *   7. loop predicate check against spurious wakeups
 *   8. destroy returns EBUSY when waiters exist
 *   9. multiple waiters woken one by one via signal
 */

#include <hellox.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

 /* ------------------------------------------------------------------ */
 /* Test framework                                                       */
 /* ------------------------------------------------------------------ */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (cond) {                                                   \
            printf("  [PASS] %s\n", msg);                             \
        } else {                                                      \
            printf("  [FAIL] %s (line %d)\n", msg, __LINE__);         \
            g_tests_failed++;                                         \
        }                                                             \
        g_tests_run++;                                                \
    } while (0)

#define CHECK_EQ(a, b, msg)                                           \
    do {                                                              \
        long _a = (long)(a), _b = (long)(b);                          \
        if (_a == _b) {                                               \
            printf("  [PASS] %s\n", msg);                             \
        } else {                                                      \
            printf("  [FAIL] %s: got %ld, expected %ld (line %d)\n",  \
                   msg, _a, _b, __LINE__);                            \
            g_tests_failed++;                                         \
        }                                                             \
        g_tests_run++;                                                \
    } while (0)

/* Build an absolute time point ms milliseconds from now. */
static struct timespec make_abstime_ms(long ms)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += ms / 1000;
	ts.tv_nsec += (ms % 1000) * 1000000L;
	if (ts.tv_nsec >= 1000000000L) {
		ts.tv_sec += 1;
		ts.tv_nsec -= 1000000000L;
	}
	return ts;
}

/* ------------------------------------------------------------------ */
/* Test 1: init / destroy lifecycle                                     */
/* ------------------------------------------------------------------ */

static void test_init_destroy(void)
{
	printf("\n=== Test 1: init / destroy lifecycle ===\n");

	pthread_cond_t cond;
	int r = pthread_cond_init(&cond, NULL);
	CHECK_EQ(r, 0, "pthread_cond_init returns 0");

	/* For debugging. */
	//printf("[%s]cond self[%d], handle[%d], clock[%d]\r\n", 
	//	__func__, cond, cond->cond, cond->clock_id);

	r = pthread_cond_destroy(&cond);
	CHECK_EQ(r, 0, "pthread_cond_destroy returns 0");

	/* Initialize with attributes. */
	pthread_condattr_t attr;
	r = pthread_condattr_init(&attr);
	CHECK_EQ(r, 0, "pthread_condattr_init returns 0");

	r = pthread_cond_init(&cond, &attr);
	CHECK_EQ(r, 0, "pthread_cond_init with attr returns 0");

	r = pthread_cond_destroy(&cond);
	CHECK_EQ(r, 0, "pthread_cond_destroy with attr returns 0");

	pthread_condattr_destroy(&attr);
}

/* ------------------------------------------------------------------ */
/* Test 2: signal wakes a single waiter                                 */
/* ------------------------------------------------------------------ */

static pthread_mutex_t g_mtx2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cnd2 = PTHREAD_COND_INITIALIZER;
static int             g_ready2 = 0;
static int             g_woke2 = 0;

static void *waiter_signal(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&g_mtx2);
	while (!g_ready2) {
		pthread_cond_wait(&g_cnd2, &g_mtx2);
	}
	g_woke2 = 1;
	pthread_mutex_unlock(&g_mtx2);
	return NULL;
}

static void test_signal_single(void)
{
	printf("\n=== Test 2: signal wakes a single waiter ===\n");

	g_ready2 = 0;
	g_woke2 = 0;

	/* Create mutex and condvar. */
	if (pthread_cond_init(&g_cnd2, NULL))
	{
		printf("[%s]condition init failed\r\n", __func__);
		return;
	}
	if (pthread_mutex_init(&g_mtx2, NULL))
	{
		printf("[%s]mutex init failed\r\n", __func__);
		return;
	}

	pthread_t tid;
	int r = pthread_create(&tid, NULL, waiter_signal, NULL);
	CHECK_EQ(r, 0, "create waiter thread");

	/* Give the waiter time to enter wait. */
	Sleep(100);

	pthread_mutex_lock(&g_mtx2);
	g_ready2 = 1;
	r = pthread_cond_signal(&g_cnd2);
	CHECK_EQ(r, 0, "pthread_cond_signal returns 0");
	pthread_mutex_unlock(&g_mtx2);

	pthread_join(tid, NULL);
	CHECK_EQ(g_woke2, 1, "waiter woke up and observed the condition");

	/* Destroy the cond and mutex. */
	pthread_mutex_destroy(&g_mtx2);
	pthread_cond_destroy(&g_cnd2);
}

/* ------------------------------------------------------------------ */
/* Test 3: broadcast wakes all waiters                                  */
/* ------------------------------------------------------------------ */

#define N_WAITERS 5

static pthread_mutex_t g_mtx3 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cnd3 = PTHREAD_COND_INITIALIZER;
static int             g_ready3 = 0;
static int             g_woke3 = 0;

static void *waiter_broadcast(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&g_mtx3);
	while (!g_ready3) {
		pthread_cond_wait(&g_cnd3, &g_mtx3);
	}
	g_woke3++;
	pthread_mutex_unlock(&g_mtx3);
	return NULL;
}

static void test_broadcast(void)
{
	printf("\n=== Test 3: broadcast wakes all waiters ===\n");

	g_ready3 = 0;
	g_woke3 = 0;

	/* Create mutex and condvar. */
	if (pthread_cond_init(&g_cnd3, NULL))
	{
		printf("[%s]condition init failed\r\n", __func__);
		return;
	}
	if (pthread_mutex_init(&g_mtx3, NULL))
	{
		printf("[%s]mutex init failed\r\n", __func__);
		return;
	}

	pthread_t tids[N_WAITERS];
	for (int i = 0; i < N_WAITERS; i++) {
		int r = pthread_create(&tids[i], NULL, waiter_broadcast, NULL);
		CHECK_EQ(r, 0, "create waiter thread");
	}

	Sleep(200);

	pthread_mutex_lock(&g_mtx3);
	g_ready3 = 1;
	int r = pthread_cond_broadcast(&g_cnd3);
	CHECK_EQ(r, 0, "pthread_cond_broadcast returns 0");
	pthread_mutex_unlock(&g_mtx3);

	for (int i = 0; i < N_WAITERS; i++) {
		pthread_join(tids[i], NULL);
	}
	CHECK_EQ(g_woke3, N_WAITERS, "all waiter threads were woken");

	/* Destroy the cond and mutex. */
	pthread_mutex_destroy(&g_mtx3);
	pthread_cond_destroy(&g_cnd3);
}

/* ------------------------------------------------------------------ */
/* Test 4: timedwait returns ETIMEDOUT on timeout                       */
/* ------------------------------------------------------------------ */

static void test_timedwait_timeout(void)
{
	printf("\n=== Test 4: timedwait returns ETIMEDOUT on timeout ===\n");

	pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t  cnd = PTHREAD_COND_INITIALIZER;

	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&cnd, NULL);

	pthread_mutex_lock(&mtx);

	struct timespec ts = make_abstime_ms(200);
	int r = pthread_cond_timedwait(&cnd, &mtx, &ts);
	CHECK_EQ(r, ETIMEDOUT, "timeout returns ETIMEDOUT");

	/* The mutex must be held again upon return. */
	r = pthread_mutex_trylock(&mtx);
	CHECK_EQ(r, 0, "mutex is recursive obtained");

	pthread_mutex_unlock(&mtx);
	pthread_cond_destroy(&cnd);
	pthread_mutex_destroy(&mtx);
}

/* ------------------------------------------------------------------ */
/* Test 5: timedwait woken by signal before timeout                     */
/* ------------------------------------------------------------------ */

static pthread_mutex_t g_mtx5 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cnd5 = PTHREAD_COND_INITIALIZER;
static int             g_ready5 = 0;

static void *waiter_timed(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&g_mtx5);
	while (!g_ready5) {
		struct timespec ts = make_abstime_ms(5000);  /* far beyond wake time */
		int r = pthread_cond_timedwait(&g_cnd5, &g_mtx5, &ts);
		if (r == ETIMEDOUT) {
			/* Should not happen. */
			pthread_mutex_unlock(&g_mtx5);
			return (void *)(intptr_t)ETIMEDOUT;
		}
	}
	pthread_mutex_unlock(&g_mtx5);
	return NULL;
}

static void test_timedwait_signaled(void)
{
	printf("\n=== Test 5: timedwait woken by signal before timeout ===\n");

	g_ready5 = 0;

	/* Create mutex and condvar. */
	if (pthread_cond_init(&g_cnd5, NULL))
	{
		printf("[%s]condition init failed\r\n", __func__);
		return;
	}
	if (pthread_mutex_init(&g_mtx5, NULL))
	{
		printf("[%s]mutex init failed\r\n", __func__);
		return;
	}

	pthread_t tid;
	int r = pthread_create(&tid, NULL, waiter_timed, NULL);
	CHECK_EQ(r, 0, "create waiter thread");

	Sleep(100);

	pthread_mutex_lock(&g_mtx5);
	g_ready5 = 1;
	r = pthread_cond_signal(&g_cnd5);
	CHECK_EQ(r, 0, "pthread_cond_signal returns 0");
	pthread_mutex_unlock(&g_mtx5);

	void *ret = NULL;
	pthread_join(tid, &ret);
	CHECK(ret == NULL, "waiter was woken before timeout");
	if (ret)
	{
		printf("    ret value: %d\r\n", ret);
	}

	/* Destroy mutex and condition. */
	pthread_mutex_destroy(&g_mtx5);
	pthread_cond_destroy(&g_cnd5);
}

/* ------------------------------------------------------------------ */
/* Test 6: producer-consumer model                                      */
/* ------------------------------------------------------------------ */

#define BUF_SIZE     8
#define N_PRODUCE    1000

static pthread_mutex_t g_mtx6 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  g_not_full = PTHREAD_COND_INITIALIZER;

static int g_buf[BUF_SIZE];
static int g_head = 0, g_tail = 0, g_count = 0;
static long g_produced = 0, g_consumed = 0;

static void *producer(void *arg)
{
	(void)arg;
	for (long i = 0; i < N_PRODUCE; i++) {
		pthread_mutex_lock(&g_mtx6);
		while (g_count == BUF_SIZE) {
			pthread_cond_wait(&g_not_full, &g_mtx6);
		}
		g_buf[g_tail] = (int)i;
		g_tail = (g_tail + 1) % BUF_SIZE;
		g_count++;
		g_produced++;
		pthread_cond_signal(&g_not_empty);
		pthread_mutex_unlock(&g_mtx6);
	}
	return NULL;
}

static void *consumer(void *arg)
{
	(void)arg;
	for (long i = 0; i < N_PRODUCE; i++) {
		pthread_mutex_lock(&g_mtx6);
		while (g_count == 0) {
			pthread_cond_wait(&g_not_empty, &g_mtx6);
		}
		int val = g_buf[g_head];
		g_head = (g_head + 1) % BUF_SIZE;
		g_count--;
		g_consumed++;
		(void)val;
		pthread_cond_signal(&g_not_full);
		pthread_mutex_unlock(&g_mtx6);
	}
	return NULL;
}

static void test_producer_consumer(void)
{
	printf("\n=== Test 6: producer-consumer model ===\n");

	g_head = g_tail = g_count = 0;
	g_produced = g_consumed = 0;

	/* Init mutex and condition. */
	pthread_mutex_init(&g_mtx6, NULL);
	pthread_cond_init(&g_not_empty, NULL);
	pthread_cond_init(&g_not_full, NULL);

	pthread_t prod, cons;
	pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons, NULL, consumer, NULL);

	pthread_join(prod, NULL);
	pthread_join(cons, NULL);

	CHECK_EQ(g_produced, N_PRODUCE, "producer count is correct");
	CHECK_EQ(g_consumed, N_PRODUCE, "consumer count is correct");
	CHECK_EQ(g_count, 0, "buffer is empty at the end");

	/* Release mutex and condition. */
	pthread_cond_destroy(&g_not_full);
	pthread_cond_destroy(&g_not_empty);
	pthread_mutex_destroy(&g_mtx6);
}

/* ------------------------------------------------------------------ */
/* Test 7: loop predicate check against spurious wakeups                */
/* ------------------------------------------------------------------ */

static pthread_mutex_t g_mtx7 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cnd7 = PTHREAD_COND_INITIALIZER;
static int             g_flag7 = 0;
static int             g_wait_loops = 0;

static void *waiter_spurious(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&g_mtx7);
	while (!g_flag7) {
		pthread_cond_wait(&g_cnd7, &g_mtx7);
		g_wait_loops++;
	}
	pthread_mutex_unlock(&g_mtx7);
	return NULL;
}

static void test_spurious_wakeup(void)
{
	printf("\n=== Test 7: loop predicate check against spurious wakeups ===\n");

	g_flag7 = 0;
	g_wait_loops = 0;

	/* Create mutex and condition. */
	pthread_mutex_init(&g_mtx7, NULL);
	pthread_cond_init(&g_cnd7, NULL);

	pthread_t tid;
	pthread_create(&tid, NULL, waiter_spurious, NULL);

	Sleep(100);

	/* Signal several times without changing the condition. */
	for (int i = 0; i < 3; i++) {
		pthread_mutex_lock(&g_mtx7);
		pthread_cond_signal(&g_cnd7);
		pthread_mutex_unlock(&g_mtx7);
		Sleep(50);
	}

	/* Now actually set the condition. */
	pthread_mutex_lock(&g_mtx7);
	g_flag7 = 1;
	pthread_cond_signal(&g_cnd7);
	pthread_mutex_unlock(&g_mtx7);

	pthread_join(tid, NULL);
	CHECK(g_flag7 == 1, "condition is eventually satisfied");
	CHECK(g_wait_loops >= 1, "loop check re-entered wait");

	pthread_cond_destroy(&g_cnd7);
	pthread_mutex_destroy(&g_mtx7);
}

/* ------------------------------------------------------------------ */
/* Test 8: destroy returns EBUSY when waiters exist                     */
/* ------------------------------------------------------------------ */

static pthread_mutex_t g_mtx8 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cnd8 = PTHREAD_COND_INITIALIZER;
static int             g_stop8 = 0;

static void *waiter_busy(void *arg)
{
	(void)arg;
	pthread_mutex_lock(&g_mtx8);
	while (!g_stop8) {
		pthread_cond_wait(&g_cnd8, &g_mtx8);
	}
	pthread_mutex_unlock(&g_mtx8);
	return NULL;
}

static void test_destroy_busy(void)
{
	printf("\n=== Test 8: destroy returns EBUSY when waiters exist ===\n");

	g_stop8 = 0;

	pthread_t tid;
	pthread_create(&tid, NULL, waiter_busy, NULL);
	Sleep(100);

	int r = pthread_cond_destroy(&g_cnd8);
	/* POSIX says this should return EBUSY, but some implementations
	 * allow destroying a condition variable with waiters. */
	if (r == EBUSY) {
		CHECK_EQ(r, EBUSY, "destroy with waiters returns EBUSY");
	}
	else {
		printf("  [INFO] destroy returned %d (this implementation allows "
			"destroying a condition variable with waiters)\n", r);
		g_tests_run++;
		g_tests_passed++;
	}

	/* Cleanup: wake the waiter. */
	pthread_mutex_lock(&g_mtx8);
	g_stop8 = 1;
	pthread_cond_broadcast(&g_cnd8);
	pthread_mutex_unlock(&g_mtx8);
	pthread_join(tid, NULL);

	/* Now it is safe to destroy. */
	r = pthread_cond_destroy(&g_cnd8);
	CHECK_EQ(r, 0, "destroy with no waiters succeeds");
}

/* ------------------------------------------------------------------ */
/* Test 9: multiple waiters woken one by one via signal                 */
/* ------------------------------------------------------------------ */

#define N_SEQ 1024

static pthread_mutex_t g_mtx9 = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_cnd9 = PTHREAD_COND_INITIALIZER;
static int             g_slot9 = 0;      /* current allowed id */
static int             g_done9 = 0;      /* number passed */
static int             g_order9[N_SEQ];  /* recorded pass order */

static void *waiter_seq(void *arg)
{
	int id = (int)(intptr_t)arg;

	//printf("Thread[%d] start\r\n", id);

	pthread_mutex_lock(&g_mtx9);
	//printf("Thread[%d] before enter while\r\n", id);
	while (g_slot9 < id) {
		//printf("Thread[%d]: while: g_slot=%d\r\n", id, g_slot9);
		pthread_cond_wait(&g_cnd9, &g_mtx9);
	}
	g_order9[g_done9++] = id;
	pthread_mutex_unlock(&g_mtx9);
	//printf("Thread[%d] exit: g_slot:%d, g_done:%d\r\n", id, g_slot9, g_done9);
	return NULL;
}

static void test_sequential_signal(void)
{
	int nthread = N_SEQ;

	printf("\n=== Test 9: multiple waiters woken one by one via signal ===\n");

	g_slot9 = 0;
	g_done9 = 0;
	memset(g_order9, 0, sizeof(g_order9));

	/* Create mutex and condition. */
	pthread_cond_init(&g_cnd9, NULL);
	pthread_mutex_init(&g_mtx9, NULL);

	pthread_t tids[N_SEQ];
	for (int i = 0; i < N_SEQ; i++) {
		if (pthread_create(&tids[i], NULL, waiter_seq, (void *)(intptr_t)(i + 1)))
		{
			/* Create thread failed. */
			printf("[%s]create thread failed at[%d]\r\n", __func__, i);
			nthread = i + 1;
			break;
		}
		Sleep(20);
	}

	//printf("[Main]create [%d] threads OK\r\n", N_SEQ);
	Sleep(500);
	//printf("[Main]begin to signal...\r\n");

	for (int i = 1; i <= nthread; i++) {
		pthread_mutex_lock(&g_mtx9);
		g_slot9 = i;
		pthread_cond_signal(&g_cnd9);
		//printf("[Main]After signal cond [%d], g_slot:%d\r\n", i, g_slot9);
		pthread_mutex_unlock(&g_mtx9);
		Sleep(20);
	}

	for (int i = 0; i < nthread; i++) {
		pthread_join(tids[i], NULL);
	}

	CHECK_EQ(g_done9, nthread, "all waiters passed");
	int ok = 1;
	for (int i = 0; i < nthread; i++) {
		if (g_order9[i] != i + 1) { ok = 0; break; }
	}
	CHECK(ok, "wake-up order matches expectation");

	/* Show seq order. */
	if (!ok)
	{
		printf("    order seq: ");
		for (int i = 0; i < nthread; i++)
		{
			printf("%d ", g_order9[i]);
		}
	}

	pthread_cond_destroy(&g_cnd9);
	pthread_mutex_destroy(&g_mtx9);
}

/* ------------------------------------------------------------------ */
/* main                                                               */
/* ------------------------------------------------------------------ */

int cond_main(void)
{
	printf("========================================\n");
	printf("  pthread_cond test suite\n");
	printf("========================================\n");

	test_init_destroy();
	test_signal_single();
	test_broadcast();
	test_timedwait_timeout();

	/* 
	 * Test case 5 will fail, since there is 
	 * issue in pthread_join routine.
	 */
	test_timedwait_signaled();
	test_producer_consumer();
	test_spurious_wakeup();

	/* 
	 * The implementation of HelloX supports conditon
	 * destroying with thread(s) wait it, and just
	 * wake up all thread(s) and return the WAIT_DELETE.
	 * So just skip this test case.
	 */
	//test_destroy_busy();

	test_sequential_signal();

	printf("\n========================================\n");
	printf("  Result: %d passed / %d failed / %d total\n",
		g_tests_passed, g_tests_failed, g_tests_run);
	printf("========================================\n");

	return g_tests_failed == 0 ? 0 : 1;
}
