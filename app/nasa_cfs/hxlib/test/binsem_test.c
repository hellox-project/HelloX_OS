/*
 * bin_sem_test.c
 *
 * Comprehensive test suite for the binary semaphore implementation.
 *
 * Build (Linux / POSIX):
 *   gcc -Wall -Wextra -O2 -pthread bin_sem.c bin_sem_test.c -o bin_sem_test
 *
 * Run:
 *   ./bin_sem_test
 *
 * Exit code:
 *   0 -> all tests passed
 *   1 -> at least one test failed
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>

#include "binsem.h"

 /* =========================================================================
  * Test framework
  * ========================================================================= */

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_BEGIN(name)                                                    \
    do {                                                                    \
        printf("\n=== Test: %s ===\n", name);                               \
        g_tests_run++;                                                      \
        int _test_failed = 0;

#define CHECK(cond, msg)                                                    \
        do {                                                                \
            if (cond) {                                                     \
                printf("  [PASS] %s\n", msg);                               \
            } else {                                                        \
                printf("  [FAIL] %s  (line %d)\n", msg, __LINE__);          \
                _test_failed = 1;                                           \
            }                                                               \
        } while (0)

#define TEST_END()                                                          \
        if (_test_failed) {                                                 \
            g_tests_failed++;                                               \
            printf("  >>> Test FAILED.\n");                                 \
        } else {                                                            \
            g_tests_passed++;                                               \
            printf("  >>> Test passed.\n");                                 \
        }                                                                   \
    } while (0)

/* =========================================================================
 * Helpers
 * ========================================================================= */

static void msleep(int ms)
{
	Sleep(ms);
#if 0
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (long)(ms % 1000) * 1000000L;
	nanosleep(&ts, NULL);
#endif 
}

typedef struct {
	bin_sem_t   *sem;
	volatile int done;
	int          result;
} take_arg_t;

static void *take_thread(void *arg)
{
	take_arg_t *a = (take_arg_t *)arg;
	a->result = bin_sem_take(a->sem);
	a->done = 1;
	return NULL;
}

typedef struct {
	bin_sem_t   *sem;
	int          rounds;
	volatile int shared_counter;
	volatile int errors;
	int thread_id;
} mutex_style_arg_t;

/*
 * Emulates a classic mutex usage pattern on top of the binary semaphore:
 * each thread loops { take; critical_section; give; }.
 *
 * Precondition: sem->val == 1 before starting, and the semaphore is
 * re-given at the end of each critical section. This guarantees that
 * every take eventually succeeds (as long as the semaphore has no
 * correctness bug that would cause val to be lost).
 */
static void *mutex_style_thread(void *arg)
{
	mutex_style_arg_t *a = (mutex_style_arg_t *)arg;
	int i;

	printf("Thread[%d] start...\r\n", a->thread_id);

	for (i = 0; i < a->rounds; i++) {
		if (bin_sem_take(a->sem) != 0) {
			a->errors++;
			continue;
		}
		//if (0 == i % 100)
		//{
		//	printf("Thread[%d] show process.\r\n", a->thread_id);
		//}
		a->shared_counter++;
		if (bin_sem_give(a->sem) != 0) {
			a->errors++;
		}
	}
	printf("Thread[%d] exit, share cnt:%d, rounds:%d, sem_val:%d, err:%d\r\n", a->thread_id,
		a->shared_counter,
		a->rounds,
		(a->sem)->val,
		a->errors);
	return NULL;
}

/* =========================================================================
 * Test cases
 * ========================================================================= */

static void test_init_destroy(void)
{
	bin_sem_t sem;

	TEST_BEGIN("init / destroy basic");

	CHECK(bin_sem_init(&sem) == 0, "init returns 0");
	CHECK(sem.val == 0, "initial val is 0");
	CHECK(sem.generation == 0, "initial generation is 0");
	CHECK(bin_sem_destroy(&sem) == 0, "destroy returns 0");

	TEST_END();
}

static void test_init_destroy_twice(void)
{
	bin_sem_t sem;

	TEST_BEGIN("init -> destroy -> init -> destroy");

	CHECK(bin_sem_init(&sem) == 0, "first init");
	CHECK(bin_sem_destroy(&sem) == 0, "first destroy");
	CHECK(bin_sem_init(&sem) == 0, "second init");
	CHECK(sem.val == 0, "val reset to 0 on re-init");
	CHECK(sem.generation == 0, "generation reset to 0 on re-init");
	CHECK(bin_sem_destroy(&sem) == 0, "second destroy");

	TEST_END();
}

static void test_null_args(void)
{
	TEST_BEGIN("NULL argument handling");

	CHECK(bin_sem_init(NULL) != 0, "init(NULL) fails");
	CHECK(bin_sem_destroy(NULL) != 0, "destroy(NULL) fails");
	CHECK(bin_sem_give(NULL) != 0, "give(NULL) fails");
	CHECK(bin_sem_take(NULL) != 0, "take(NULL) fails");
	CHECK(bin_sem_flush(NULL) != 0, "flush(NULL) fails");

	TEST_END();
}

static void test_give_take_immediate(void)
{
	bin_sem_t sem;

	TEST_BEGIN("give then take without blocking");

	CHECK(bin_sem_init(&sem) == 0, "init");

	CHECK(bin_sem_give(&sem) == 0, "give returns 0");
	CHECK(sem.val == 1, "val becomes 1 after give");

	CHECK(bin_sem_take(&sem) == 0, "take returns 0");
	CHECK(sem.val == 0, "val becomes 0 after take");

	bin_sem_destroy(&sem);
	TEST_END();
}

static void test_take_blocks_until_give(void)
{
	bin_sem_t   sem;
	pthread_t   tid;
	take_arg_t  arg;

	TEST_BEGIN("take blocks when val=0, give wakes it up");

	CHECK(bin_sem_init(&sem) == 0, "init");

	memset(&arg, 0, sizeof(arg));
	arg.sem = &sem;
	arg.result = -1;

	CHECK(pthread_create(&tid, NULL, take_thread, &arg) == 0,
		"create take thread");

	msleep(150);
	CHECK(arg.done == 0, "take thread is still blocked");
	CHECK(sem.val == 0, "val is still 0 while blocked");

	CHECK(bin_sem_give(&sem) == 0, "give");

	msleep(150);
	CHECK(arg.done == 1, "take thread finished after give");
	CHECK(arg.result == 0, "take returned success");
	CHECK(sem.val == 0, "val was consumed by take");

	pthread_join(tid, NULL);
	bin_sem_destroy(&sem);
	TEST_END();
}

static void test_double_give_is_idempotent(void)
{
	bin_sem_t   sem;
	pthread_t   tid;
	take_arg_t  arg;

	TEST_BEGIN("two gives -> one take -> second take blocks");

	CHECK(bin_sem_init(&sem) == 0, "init");

	CHECK(bin_sem_give(&sem) == 0, "first give");
	CHECK(bin_sem_give(&sem) == 0, "second give");
	CHECK(sem.val == 1, "val remains 1 (binary semantics)");

	CHECK(bin_sem_take(&sem) == 0, "first take succeeds");
	CHECK(sem.val == 0, "val is 0 after first take");

	memset(&arg, 0, sizeof(arg));
	arg.sem = &sem;
	arg.result = -1;

	CHECK(pthread_create(&tid, NULL, take_thread, &arg) == 0,
		"start second take thread");
	msleep(150);
	CHECK(arg.done == 0, "second take blocks (no accumulated value)");

	/* Cleanup: give once to release the blocked thread. */
	bin_sem_give(&sem);
	msleep(150);
	CHECK(arg.done == 1, "second take released by give");
	CHECK(arg.result == 0, "second take returned success");

	pthread_join(tid, NULL);
	bin_sem_destroy(&sem);
	TEST_END();
}

static void test_flush_preserves_val(void)
{
	bin_sem_t sem;

	TEST_BEGIN("flush does not change val");

	CHECK(bin_sem_init(&sem) == 0, "init");

	CHECK(bin_sem_give(&sem) == 0, "give -> val = 1");
	CHECK(sem.val == 1, "val is 1");

	CHECK(bin_sem_flush(&sem) == 0, "flush");
	CHECK(sem.val == 1, "val is still 1 after flush");

	CHECK(bin_sem_take(&sem) == 0, "take succeeds immediately");
	CHECK(sem.val == 0, "val becomes 0 after take");

	bin_sem_destroy(&sem);
	TEST_END();
}

static void test_flush_on_empty_sem(void)
{
	bin_sem_t sem;

	TEST_BEGIN("flush with no waiters is a no-op on val");

	CHECK(bin_sem_init(&sem) == 0, "init");
	CHECK(bin_sem_flush(&sem) == 0, "flush on empty sem");
	CHECK(sem.val == 0, "val still 0");
	CHECK(sem.generation == 1, "generation incremented");

	/* Take should still block; verify via a thread. */
	pthread_t  tid;
	take_arg_t arg;
	memset(&arg, 0, sizeof(arg));
	arg.sem = &sem;
	arg.result = -1;

	pthread_create(&tid, NULL, take_thread, &arg);
	msleep(150);
	CHECK(arg.done == 0, "take still blocks after a no-op flush");

	bin_sem_give(&sem);
	msleep(150);
	CHECK(arg.done == 1, "give releases the waiter");
	CHECK(sem.val == 0, "val consumed");

	pthread_join(tid, NULL);
	bin_sem_destroy(&sem);
	TEST_END();
}

#define FLUSH_WAITERS 8

static void test_flush_wakes_all_waiters(void)
{
	bin_sem_t   sem;
	pthread_t   tids[FLUSH_WAITERS];
	take_arg_t  args[FLUSH_WAITERS];
	int         i;

	TEST_BEGIN("flush wakes up all waiters without consuming val");

	CHECK(bin_sem_init(&sem) == 0, "init");

	for (i = 0; i < FLUSH_WAITERS; i++) {
		memset(&args[i], 0, sizeof(args[i]));
		args[i].sem = &sem;
		args[i].result = -1;
		pthread_create(&tids[i], NULL, take_thread, &args[i]);
	}
	msleep(200);

	for (i = 0; i < FLUSH_WAITERS; i++) {
		CHECK(args[i].done == 0, "all waiters still blocked");
	}

	CHECK(bin_sem_flush(&sem) == 0, "flush");
	msleep(200);

	for (i = 0; i < FLUSH_WAITERS; i++) {
		CHECK(args[i].done == 1, "waiter woken up");
		CHECK(args[i].result == 0, "waiter returned success");
	}
	CHECK(sem.val == 0, "flush did not consume val");

	for (i = 0; i < FLUSH_WAITERS; i++) {
		pthread_join(tids[i], NULL);
	}
	bin_sem_destroy(&sem);
	TEST_END();
}

#undef FLUSH_WAITERS

static void test_flush_then_give(void)
{
	bin_sem_t   sem;
	pthread_t   tids[2];
	take_arg_t  args[2];
	int         i;

	TEST_BEGIN("flush wakes waiters, then give still works normally");

	CHECK(bin_sem_init(&sem) == 0, "init");

	for (i = 0; i < 2; i++) {
		memset(&args[i], 0, sizeof(args[i]));
		args[i].sem = &sem;
		args[i].result = -1;
		pthread_create(&tids[i], NULL, take_thread, &args[i]);
	}
	msleep(150);

	CHECK(bin_sem_flush(&sem) == 0, "flush wakes both waiters");
	msleep(150);

	for (i = 0; i < 2; i++) {
		CHECK(args[i].done == 1, "waiter returned");
		CHECK(args[i].result == 0, "waiter succeeded");
	}
	CHECK(sem.val == 0, "val unchanged by flush");

	/* A subsequent give must still transition val from 0 to 1. */
	CHECK(bin_sem_give(&sem) == 0, "give after flush");
	CHECK(sem.val == 1, "val becomes 1 after give");

	CHECK(bin_sem_take(&sem) == 0, "take consumes the new value");
	CHECK(sem.val == 0, "val is 0");

	for (i = 0; i < 2; i++) {
		pthread_join(tids[i], NULL);
	}
	bin_sem_destroy(&sem);
	TEST_END();
}

static void test_give_then_flush_all_return(void)
{
	/*
	 * Racy-but-safe scenario: a give sets val = 1 and wakes one waiter,
	 * then a flush wakes the rest. Every waiter must eventually return
	 * successfully. We do not assert on the final val here because it
	 * depends on who ran first, but every waiter must complete.
	 */
	bin_sem_t   sem;
	pthread_t   tids[4];
	take_arg_t  args[4];
	int         i;

	TEST_BEGIN("give + flush: all waiters eventually return");

	CHECK(bin_sem_init(&sem) == 0, "init");

	for (i = 0; i < 4; i++) {
		memset(&args[i], 0, sizeof(args[i]));
		args[i].sem = &sem;
		args[i].result = -1;
		pthread_create(&tids[i], NULL, take_thread, &args[i]);
	}
	msleep(150);

	bin_sem_give(&sem);   /* Wakes one waiter (val: 0 -> 1 -> 0). */
	msleep(50);
	bin_sem_flush(&sem);  /* Wakes the remaining waiters. */
	msleep(200);

	for (i = 0; i < 4; i++) {
		CHECK(args[i].done == 1, "waiter returned");
		CHECK(args[i].result == 0, "waiter succeeded");
	}

	for (i = 0; i < 4; i++) {
		pthread_join(tids[i], NULL);
	}
	bin_sem_destroy(&sem);
	TEST_END();
}

static void test_flush_generation_isolated(void)
{
	/*
	 * Verify that a flush that happens BEFORE a thread calls take()
	 * does not spuriously affect that thread. Only waiters that
	 * captured the old generation should be released by a flush.
	 */
	bin_sem_t   sem;
	pthread_t   tid;
	take_arg_t  arg;

	TEST_BEGIN("flush before take does not affect future take");

	CHECK(bin_sem_init(&sem) == 0, "init");

	/* Flush before any waiter exists. */
	CHECK(bin_sem_flush(&sem) == 0, "flush with no waiters");
	CHECK(sem.generation == 1, "generation advanced");

	/* Now a new take should still block, since val is 0. */
	memset(&arg, 0, sizeof(arg));
	arg.sem = &sem;
	arg.result = -1;

	pthread_create(&tid, NULL, take_thread, &arg);
	msleep(150);
	CHECK(arg.done == 0, "new take still blocks");

	bin_sem_give(&sem);
	msleep(150);
	CHECK(arg.done == 1, "give releases the new waiter");
	CHECK(arg.result == 0, "waiter succeeded");

	pthread_join(tid, NULL);
	bin_sem_destroy(&sem);
	TEST_END();
}

#define CONC_THREADS 16
#define CONC_ROUNDS  128

static void test_concurrent_mutex_style(void)
{
	bin_sem_t           sem;
	pthread_t           tids[CONC_THREADS];
	mutex_style_arg_t   args[CONC_THREADS];
	int                 i;
	int                 total_expected;

	TEST_BEGIN("concurrent take/give in mutex-like pattern");

	CHECK(bin_sem_init(&sem) == 0, "init");

	/* Seed the semaphore so the first take succeeds immediately. */
	CHECK(bin_sem_give(&sem) == 0, "seed give");
	CHECK(sem.val == 1, "val seeded to 1");

	for (i = 0; i < CONC_THREADS; i++) {
		memset(&args[i], 0, sizeof(args[i]));
		args[i].sem = &sem;
		args[i].rounds = CONC_ROUNDS;
		args[i].errors = 0;
		args[i].thread_id = i;
		pthread_create(&tids[i], NULL, mutex_style_thread, &args[i]);
	}

	for (i = 0; i < CONC_THREADS; i++) {
		pthread_join(tids[i], NULL);
	}

	total_expected = CONC_THREADS * CONC_ROUNDS;

	for (i = 0; i < CONC_THREADS; i++) {
		CHECK(args[i].errors == 0, "no errors in worker thread");
	}

	{
		int total_observed = 0;
		for (i = 0; i < CONC_THREADS; i++) {
			total_observed += args[i].shared_counter;
		}
		CHECK(total_observed == total_expected,
			"shared counter reached expected total");
	}

	CHECK(sem.val == 1, "val is 1 after all workers finish");
	CHECK(bin_sem_destroy(&sem) == 0, "destroy");

	TEST_END();
}

#undef CONC_THREADS
#undef CONC_ROUNDS

/* =========================================================================
 * main
 * ========================================================================= */

int binsem_main(void)
{
	printf("Binary Semaphore Test Suite\n");
	printf("===========================\n");

	test_init_destroy();
	test_init_destroy_twice();
	test_null_args();
	test_give_take_immediate();
	test_take_blocks_until_give();
	test_double_give_is_idempotent();
	test_flush_preserves_val();
	test_flush_on_empty_sem();
	test_flush_wakes_all_waiters();
	test_flush_then_give();
	test_give_then_flush_all_return();
	test_flush_generation_isolated();

	test_concurrent_mutex_style();

	printf("\n===========================\n");
	printf("Total tests run    : %d\n", g_tests_run);
	printf("Total tests passed : %d\n", g_tests_passed);
	printf("Total tests failed : %d\n", g_tests_failed);

	return (g_tests_failed == 0) ? 0 : 1;
}
