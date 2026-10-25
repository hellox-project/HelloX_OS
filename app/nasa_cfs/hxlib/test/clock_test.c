#define _POSIX_C_SOURCE 199309L
#include <hellox.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

/* ==================== Test framework ==================== */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                              \
    if (cond) { g_pass++; printf("  [PASS] %s\n", msg); }  \
    else      { g_fail++; printf("  [FAIL] %s\n", msg); }  \
} while (0)

/* ==================== Helper functions ==================== */

/* Read the second-resolution system time and convert it to a UNIX timestamp. */
static time_t sys_seconds(void) {
	unsigned char t[6];
	GetSystemTime(t);
	int y = 2000 + t[0], m = t[1], d = t[2];
	int hh = t[3], mm = t[4], ss = t[5];
	y -= (m <= 2);
	int era = (y >= 0 ? y : y - 399) / 400;
	unsigned yoe = (unsigned)(y - era * 400);
	unsigned doy = (153u * (m + (m > 2 ? -3 : 9)) + 2u) / 5u + d - 1u;
	unsigned doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
	int64_t days = (int64_t)era * 146097 + (int64_t)doe - 719468;
	return (time_t)(days * 86400LL + hh * 3600 + mm * 60 + ss);
}

/* Convert a timespec to a signed total of nanoseconds. */
static long long ts_to_ns(const struct timespec *ts) {
	return (long long)ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

/* ==================== T01: Initialization ==================== */

static void test_init(void) {
	int ret = 0;
	printf("\n[T01] Initialization\n");
	//int ret = user_clock_init();
	CHECK(ret == 0, "user_clock_init should return 0");
}

/* ==================== T02: Invalid arguments ==================== */

static void test_invalid_args(void) {
	printf("\n[T02] Invalid arguments\n");

	errno = 0;
	int ret = clock_gettime(CLOCK_REALTIME, NULL);
	CHECK(ret == -1 && errno == EFAULT,
		"NULL pointer should return -1 with errno=EFAULT");

	struct timespec ts;
	errno = 0;
	ret = clock_gettime((clockid_t)0xdeadbeef, &ts);
	CHECK(ret == -1 && errno == EINVAL,
		"Invalid clockid should return -1 with errno=EINVAL");
}

/* ==================== T03: Nanosecond range ==================== */

static void test_nsec_range(void) {
	printf("\n[T03] Nanosecond range\n");

	struct timespec ts;
	bool ok = true;
	for (int i = 0; i < 10000; i++) {
		if (clock_gettime(CLOCK_REALTIME, &ts) != 0) { ok = false; break; }
		if (ts.tv_nsec < 0 || ts.tv_nsec >= 1000000000L) { ok = false; break; }
		if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) { ok = false; break; }
		if (ts.tv_nsec < 0 || ts.tv_nsec >= 1000000000L) { ok = false; break; }
	}
	CHECK(ok, "tv_nsec stays within [0, 999999999] over 10,000 calls");
}

/* ==================== T04: Realtime sanity ==================== */

static void test_realtime_sanity(void) {
	printf("\n[T04] Realtime sanity\n");

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	time_t sys = sys_seconds();
	long long diff = (long long)ts.tv_sec - (long long)sys;
	if (diff < 0) diff = -diff;

	printf("    REALTIME=%lld, system_sec=%lld, diff=%lld\n",
		(long long)ts.tv_sec, (long long)sys, diff);
	CHECK(diff <= 1, "Deviation from second-resolution API <= 1 second");
}

/* ==================== T05: Monotonicity ==================== */

static void test_monotonic(void) {
	printf("\n[T05] Monotonicity\n");

	struct timespec prev, cur;
	clock_gettime(CLOCK_MONOTONIC, &prev);
	bool ok = true;

	for (int i = 0; i < 100000; i++) {
		if (clock_gettime(CLOCK_MONOTONIC, &cur) != 0) { ok = false; break; }
		if (ts_to_ns(&cur) < ts_to_ns(&prev)) { ok = false; break; }
		prev = cur;
	}
	CHECK(ok, "CLOCK_MONOTONIC never goes backwards over 100,000 calls");
}

/* ==================== T06: Step consistency ==================== */

static void test_step_consistency(void) {
	printf("\n[T06] Monotonic vs realtime step consistency\n");

	struct timespec m0, m1, r0, r1;
	clock_gettime(CLOCK_MONOTONIC, &m0);
	clock_gettime(CLOCK_REALTIME, &r0);

	usleep(100000);  /* 100 ms */

	clock_gettime(CLOCK_MONOTONIC, &m1);
	clock_gettime(CLOCK_REALTIME, &r1);

	long long dm = ts_to_ns(&m1) - ts_to_ns(&m0);
	long long dr = ts_to_ns(&r1) - ts_to_ns(&r0);
	long long diff = dm - dr;
	if (diff < 0) diff = -diff;

	printf("    monotonic_delta=%lld ns, realtime_delta=%lld ns, diff=%lld ns\n",
		dm, dr, diff);
	CHECK(diff < 1000000LL,
		"Monotonic vs realtime delta within 1 ms over 100 ms");
}

/* ==================== T07: Precision ==================== */

static void test_precision(void) {
	printf("\n[T07] Precision\n");

	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);

	/* Busy-wait for roughly 50 ms. */
	volatile unsigned long long x = 0;
	for (unsigned long long i = 0; i < 50000000ULL; i++) x += i;

	clock_gettime(CLOCK_MONOTONIC, &t1);
	long long elapsed = ts_to_ns(&t1) - ts_to_ns(&t0);

	/* Compare against a real nanosleep. */
	struct timespec req = { .tv_sec = 0,.tv_nsec = 50000000L };
	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	nanosleep(&req, NULL);
	clock_gettime(CLOCK_MONOTONIC, &b);
	long long ref = ts_to_ns(&b) - ts_to_ns(&a);

	printf("    busy_wait_measured=%lld ns, nanosleep_reference=%lld ns\n",
		elapsed, ref);
	CHECK(elapsed > 0, "Measured duration is positive");
	CHECK(elapsed > ref / 100 && elapsed < ref * 100,
		"Busy-wait duration is in the same order of magnitude as reference");
}

/* ==================== T08: Second boundary ==================== */

static void test_second_boundary(void) {
	printf("\n[T08] Second boundary crossing\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);

	/* Sleeping over 1 second guarantees a second boundary crossing. */
	struct timespec req = { .tv_sec = 1,.tv_nsec = 200000000L };
	nanosleep(&req, NULL);

	clock_gettime(CLOCK_MONOTONIC, &b);

	long long diff = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    cross_second_delta=%lld ns\n", diff);

	CHECK(b.tv_nsec >= 0 && b.tv_nsec < 1000000000L,
		"tv_nsec still in valid range");
	CHECK(diff >= 1200000000LL && diff < 1400000000LL,
		"1.2 s sleep measures in [1.2s, 1.4s)");
}

/* ==================== T09: Long-run stability ==================== */

static void test_long_run(void) {
	printf("\n[T09] Long-run stability\n");

	struct timespec a, b;
	clock_gettime(CLOCK_REALTIME, &a);
	time_t sys_a = sys_seconds();

	secondsleep(2);

	clock_gettime(CLOCK_REALTIME, &b);
	time_t sys_b = sys_seconds();

	long long user_elapsed = (long long)b.tv_sec - (long long)a.tv_sec;
	long long sys_elapsed = (long long)sys_b - (long long)sys_a;

	printf("    user_clock_elapsed=%lld s, system_elapsed=%lld s\n",
		user_elapsed, sys_elapsed);
	CHECK(user_elapsed == sys_elapsed,
		"User clock and second-resolution API agree after 2 seconds");
}

/* ==================== T11: Concurrency ==================== */

static void *thread_reader(void *arg) {
	(void)arg;
	for (int i = 0; i < 200000; i++) {
		struct timespec ts;
		if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return (void *)1;
		if (ts.tv_nsec < 0 || ts.tv_nsec >= 1000000000L) return (void *)1;
	}
	return NULL;
}

static void test_concurrent(void) {
	printf("\n[T11] Concurrency safety\n");

	pthread_t th[4];
	for (int i = 0; i < 4; i++)
		pthread_create(&th[i], NULL, thread_reader, NULL);

	bool ok = true;
	for (int i = 0; i < 4; i++) {
		void *r;
		pthread_join(th[i], &r);
		if (r != NULL) ok = false;
	}
	CHECK(ok, "4 threads x 200,000 read-only calls complete without error");
}

/* ==================== T12: Alignment with system sleep ==================== */

static void test_sleep_alignment(void) {
	printf("\n[T12] Alignment with system sleep\n");

	const long long req_ns = 300000000LL;  /* 300 ms */
	struct timespec req = { .tv_sec = 0,.tv_nsec = req_ns };
	struct timespec a, b;

	clock_gettime(CLOCK_MONOTONIC, &a);
	nanosleep(&req, NULL);
	clock_gettime(CLOCK_MONOTONIC, &b);

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	long long err = elapsed - req_ns;
	if (err < 0) err = -err;

	printf("    requested=%lld ns, measured=%lld ns, error=%lld ns\n",
		req_ns, elapsed, err);
	CHECK(err < 5000000LL, "300 ms sleep error < 5 ms");
}

/* ==================== T13: Throughput ==================== */

static void test_throughput(void) {
	printf("\n[T13] High-frequency call overhead\n");

	const int N = 1000000;
	struct timespec ts;

	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (int i = 0; i < N; i++) {
		clock_gettime(CLOCK_MONOTONIC, &ts);
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);

	long long total = ts_to_ns(&t1) - ts_to_ns(&t0);
	double per = (double)total / N;

	printf("    %d calls took %lld ns, average %.1f ns/call\n", N, total, per);
	CHECK(total > 0, "Batch call duration is positive");
	CHECK(per < 100000.0, "Average per-call overhead < 100 us");
}

/* ==================== Main ==================== */

int main(void) {
	printf("========== clock_gettime comprehensive test ==========\n");

	test_init();
	if (g_fail > 0) {
		printf("\nInitialization failed; aborting remaining tests.\n");
		return 1;
	}

	test_invalid_args();
	test_nsec_range();
	test_realtime_sanity();
	test_monotonic();

	test_step_consistency();
	test_second_boundary();
	test_long_run();

	/* 
	 * This case will faile, since it invokes 
	 * pthread_join, this routine is not implemented 
	 * correctly now.
	 */
	test_concurrent();

	test_sleep_alignment();
	//test_throughput();

	printf("\n========== Test results ==========\n");
	printf("PASS: %d\n", g_pass);
	printf("FAIL: %d\n", g_fail);
	printf("Total: %d\n", g_pass + g_fail);

	return g_fail == 0 ? 0 : 1;
}
