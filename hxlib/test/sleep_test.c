/*
 * test_posix_time.c
 *
 * Comprehensive test suite for the user-space POSIX time functions:
 *   - clock_gettime()
 *   - nanosleep()
 *   - usleep()
 *   - sleep()
 *
 * The functions are implemented on top of a custom kernel providing
 * GetCPUInfo(), GetSystemTime(), and a 5 ms granularity Sleep().
 *
 * Build:
 *   gcc -O2 -pthread -o test_posix_time test_posix_time.c \
 *       posix_time.c kernel_apis.c
 *
 * Run:
 *   ./test_posix_time
 *
 * Do NOT include <unistd.h>, since it declares usleep() and sleep()
 * with different semantics from the ones under test.
 */

#define _POSIX_C_SOURCE 199309L
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

/* ==================== Test framework ==================== */

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do {                              \
    if (cond) { g_pass++; printf("  [PASS] %s\n", msg); }  \
    else      { g_fail++; printf("  [FAIL] %s\n", msg); }  \
} while (0)

/* Convert a timespec to a signed total of nanoseconds. */
static long long ts_to_ns(const struct timespec *ts) {
	return (long long)ts->tv_sec * 1000000000LL + ts->tv_nsec;
}

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

/* ==================== Group A: clock_gettime ==================== */

/* T01: CLOCK_REALTIME returns a plausible value. */
static void test_A01_realtime_sanity(void) {
	printf("\n[A01] CLOCK_REALTIME sanity\n");

	struct timespec ts;
	int ret = clock_gettime(CLOCK_REALTIME, &ts);
	CHECK(ret == 0, "clock_gettime(CLOCK_REALTIME) returns 0");
	CHECK(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000L,
		"tv_nsec is within [0, 999999999]");

	time_t sys = sys_seconds();
	long long diff = (long long)ts.tv_sec - (long long)sys;
	if (diff < 0) diff = -diff;
	printf("    REALTIME=%lld, system_sec=%lld, diff=%lld\n",
		(long long)ts.tv_sec, (long long)sys, diff);
	CHECK(diff <= 1,
		"Deviation from second-resolution API <= 1 second");
}

/* T02: CLOCK_MONOTONIC returns a plausible value. */
static void test_A02_monotonic_sanity(void) {
	printf("\n[A02] CLOCK_MONOTONIC sanity\n");

	struct timespec ts;
	int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
	CHECK(ret == 0, "clock_gettime(CLOCK_MONOTONIC) returns 0");
	CHECK(ts.tv_sec >= 0, "tv_sec is non-negative");
	CHECK(ts.tv_nsec >= 0 && ts.tv_nsec < 1000000000L,
		"tv_nsec is within [0, 999999999]");
}

/* T03: NULL pointer yields EFAULT. */
static void test_A03_null_pointer(void) {
	printf("\n[A03] NULL pointer handling\n");

	errno = 0;
	int ret = clock_gettime(CLOCK_REALTIME, NULL);
	CHECK(ret == -1 && errno == EFAULT,
		"NULL tp returns -1 with errno=EFAULT");
}

/* T04: Invalid clock id yields EINVAL. */
static void test_A04_invalid_clockid(void) {
	printf("\n[A04] Invalid clockid\n");

	struct timespec ts;
	errno = 0;
	int ret = clock_gettime((clockid_t)0xdeadbeef, &ts);
	CHECK(ret == -1 && errno == EINVAL,
		"Unknown clockid returns -1 with errno=EINVAL");
}

/* T05: tv_nsec stays in range over many calls. */
static void test_A05_nsec_range(void) {
	printf("\n[A05] tv_nsec range under load\n");

	struct timespec ts;
	bool ok = true;
	for (int i = 0; i < 100000; i++) {
		if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) { ok = false; break; }
		if (ts.tv_nsec < 0 || ts.tv_nsec >= 1000000000L) { ok = false; break; }
	}
	CHECK(ok, "tv_nsec stays within range over 100,000 calls");
}

/* T06: CLOCK_MONOTONIC is monotonic. */
static void test_A06_monotonicity(void) {
	printf("\n[A06] Monotonicity of CLOCK_MONOTONIC\n");

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

/* T07: Two clocks advance at the same rate. */
static void test_A07_step_consistency(void) {
	printf("\n[A07] Monotonic vs realtime step consistency\n");

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
	CHECK(diff < 2000000LL,
		"Monotonic and realtime deltas agree within 2 ms over 100 ms");
}

/* T08: Nanosecond field is correctly carried across a second boundary. */
static void test_A08_second_carry(void) {
	printf("\n[A08] Second boundary carry\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	secondsleep(2);
	clock_gettime(CLOCK_MONOTONIC, &b);

	long long delta = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    delta after sleep(2) = %lld ns\n", delta);

	CHECK(b.tv_nsec >= 0 && b.tv_nsec < 1000000000L,
		"tv_nsec is still in range after crossing seconds");
	CHECK(delta >= 1800000000LL && delta < 2500000000LL,
		"Delta is in [1.8 s, 2.5 s)");
}

/* ==================== Group B: nanosleep ==================== */

/* T09: Basic short nanosleep. */
static void test_B09_nanosleep_basic(void) {
	printf("\n[B09] nanosleep basic\n");

	struct timespec req = { .tv_sec = 0,.tv_nsec = 10000000L }; /* 10 ms */
	struct timespec a, b;

	clock_gettime(CLOCK_MONOTONIC, &a);
	int ret = nanosleep(&req, NULL);
	clock_gettime(CLOCK_MONOTONIC, &b);

	CHECK(ret == 0, "nanosleep returns 0");

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	long long err = elapsed - 10000000LL;
	if (err < 0) err = -err;
	printf("    requested=10 ms, measured=%lld ns, error=%lld ns\n",
		elapsed, err);
	CHECK(elapsed >= 10000000LL, "Sleep is not shorter than requested");
	CHECK(err < 1000000LL, "Error is below 1 ms");
}

/* T10: Zero nanosleep returns immediately. */
static void test_B10_nanosleep_zero(void) {
	printf("\n[B10] nanosleep with zero interval\n");

	struct timespec req = { .tv_sec = 0,.tv_nsec = 0 };
	struct timespec rem = { .tv_sec = 123,.tv_nsec = 456 };
	int ret = nanosleep(&req, &rem);

	CHECK(ret == 0, "Zero sleep returns 0");
	CHECK(rem.tv_sec == 0 && rem.tv_nsec == 0,
		"rem is zeroed on successful return");
}

/* T11: NULL request yields EFAULT. */
static void test_B11_nanosleep_null(void) {
	printf("\n[B11] nanosleep with NULL request\n");

	errno = 0;
	int ret = nanosleep(NULL, NULL);
	CHECK(ret == -1 && errno == EFAULT,
		"NULL req returns -1 with errno=EFAULT");
}

/* T12: Invalid tv_nsec yields EINVAL. */
static void test_B12_nanosleep_invalid(void) {
	printf("\n[B12] nanosleep with invalid tv_nsec\n");

	struct timespec req = { .tv_sec = 0,.tv_nsec = 1000000000L };
	errno = 0;
	int ret = nanosleep(&req, NULL);
	CHECK(ret == -1 && errno == EINVAL,
		"tv_nsec == 1e9 returns -1 with errno=EINVAL");

	req.tv_nsec = -1;
	errno = 0;
	ret = nanosleep(&req, NULL);
	CHECK(ret == -1 && errno == EINVAL,
		"Negative tv_nsec returns -1 with errno=EINVAL");

	req.tv_sec = -1;
	req.tv_nsec = 0;
	errno = 0;
	ret = nanosleep(&req, NULL);
	CHECK(ret == -1 && errno == EINVAL,
		"Negative tv_sec returns -1 with errno=EINVAL");
}

/* T13: nanosleep across a second boundary. */
static void test_B13_nanosleep_cross_second(void) {
	printf("\n[B13] nanosleep crossing a second boundary\n");

	struct timespec req = { .tv_sec = 1,.tv_nsec = 200000000L }; /* 1.2 s */
	struct timespec a, b;

	clock_gettime(CLOCK_MONOTONIC, &a);
	int ret = nanosleep(&req, NULL);
	clock_gettime(CLOCK_MONOTONIC, &b);

	CHECK(ret == 0, "nanosleep(1.2 s) returns 0");

	long long delta = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    measured delta = %lld ns\n", delta);
	CHECK(delta >= 1200000000LL && delta < 1500000000LL,
		"Delta is in [1.2 s, 1.5 s)");
}

/* T14: rem is zeroed when no signal interrupts. */
static void test_B14_nanosleep_rem_zeroed(void) {
	printf("\n[B14] nanosleep zeroes rem\n");

	struct timespec req = { .tv_sec = 0,.tv_nsec = 5000000L }; /* 5 ms */
	struct timespec rem = { .tv_sec = 99,.tv_nsec = 99 };
	int ret = nanosleep(&req, &rem);

	CHECK(ret == 0, "nanosleep returns 0");
	CHECK(rem.tv_sec == 0 && rem.tv_nsec == 0,
		"rem is cleared on normal return");
}

/* T15: nanosleep precision over several durations. */
static void test_B15_nanosleep_precision(void) {
	printf("\n[B15] nanosleep precision over several durations\n");

	const long long durations_ns[] = {
		1000000LL,    /* 1 ms:  pure busy-wait path  */
		5000000LL,    /* 5 ms:  boundary around kernel granularity */
		20000000LL,   /* 20 ms: coarse + fine phase  */
		100000000LL   /* 100 ms: coarse + fine phase */
	};
	const char *labels[] = { "1 ms", "5 ms", "20 ms", "100 ms" };
	const int n = sizeof(durations_ns) / sizeof(durations_ns[0]);

	for (int i = 0; i < n; i++) {
		struct timespec req = {
			.tv_sec = durations_ns[i] / 1000000000LL,
			.tv_nsec = durations_ns[i] % 1000000000LL
		};
		struct timespec a, b;

		clock_gettime(CLOCK_MONOTONIC, &a);
		nanosleep(&req, NULL);
		clock_gettime(CLOCK_MONOTONIC, &b);

		long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
		long long err = elapsed - durations_ns[i];
		if (err < 0) err = -err;

		printf("    %-6s requested=%lld ns, measured=%lld ns, error=%lld ns\n",
			labels[i], durations_ns[i], elapsed, err);

		char msg[128];
		snprintf(msg, sizeof(msg), "%s sleep error below 1 ms", labels[i]);
		CHECK(err < 1000000LL, msg);
	}
}

/* ==================== Group C: usleep ==================== */

/* T16: Basic usleep. */
static void test_C16_usleep_basic(void) {
	printf("\n[C16] usleep basic\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	int ret = usleep(20000);   /* 20 ms */
	clock_gettime(CLOCK_MONOTONIC, &b);

	CHECK(ret == 0, "usleep(20000) returns 0");

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	long long err = elapsed - 20000000LL;
	if (err < 0) err = -err;
	printf("    requested=20 ms, measured=%lld ns, error=%lld ns\n",
		elapsed, err);
	CHECK(elapsed >= 20000000LL, "Sleep is not shorter than requested");
	CHECK(err < 1000000LL, "Error is below 1 ms");
}

/* T17: usleep(0) returns immediately. */
static void test_C17_usleep_zero(void) {
	printf("\n[C17] usleep(0)\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	int ret = usleep(0);
	clock_gettime(CLOCK_MONOTONIC, &b);

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    elapsed = %lld ns\n", elapsed);

	CHECK(ret == 0, "usleep(0) returns 0");
	CHECK(elapsed < 1000000LL, "Elapsed is below 1 ms");
}

/* T18: usleep precision at various values. */
static void test_C18_usleep_precision(void) {
	printf("\n[C18] usleep precision\n");

	const unsigned int values_us[] = { 500, 2000, 10000, 50000 };
	const int n = sizeof(values_us) / sizeof(values_us[0]);

	for (int i = 0; i < n; i++) {
		struct timespec a, b;
		clock_gettime(CLOCK_MONOTONIC, &a);
		usleep(values_us[i]);
		clock_gettime(CLOCK_MONOTONIC, &b);

		long long req_ns = (long long)values_us[i] * 1000LL;
		long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
		long long err = elapsed - req_ns;
		if (err < 0) err = -err;

		printf("    requested=%u us, measured=%lld ns, error=%lld ns\n",
			values_us[i], elapsed, err);

		char msg[128];
		snprintf(msg, sizeof(msg), "%u us sleep error below 1 ms",
			values_us[i]);
		CHECK(err < 1000000LL, msg);
	}
}

/* ==================== Group D: sleep ==================== */

/* T19: sleep(1) returns 0 and takes about 1 second. */
static void test_D19_sleep_one(void) {
	printf("\n[D19] sleep(1)\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	unsigned int ret = secondsleep(1);
	clock_gettime(CLOCK_MONOTONIC, &b);

	CHECK(ret == 0, "sleep(1) returns 0");

	long long delta = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    measured delta = %lld ns\n", delta);
	CHECK(delta >= 1000000000LL && delta < 1500000000LL,
		"Delta is in [1.0 s, 1.5 s)");
}

/* T20: sleep(2) and cross-check with system seconds. */
static void test_D20_sleep_two_seconds(void) {
	printf("\n[D20] sleep(2) consistency with system seconds\n");

	struct timespec a, b;
	time_t sys_a, sys_b;

	clock_gettime(CLOCK_REALTIME, &a);
	sys_a = sys_seconds();

	unsigned int ret = secondsleep(2);

	clock_gettime(CLOCK_REALTIME, &b);
	sys_b = sys_seconds();

	long long user_elapsed = b.tv_sec - a.tv_sec;
	long long sys_elapsed = (long long)sys_b - (long long)sys_a;

	printf("    user_clock_elapsed=%lld s, system_elapsed=%lld s\n",
		user_elapsed, sys_elapsed);

	CHECK(ret == 0, "sleep(2) returns 0");
	CHECK(user_elapsed == sys_elapsed,
		"User clock and system clock agree on elapsed seconds");
}

/* T21: sleep(0) returns immediately. */
static void test_D21_sleep_zero(void) {
	printf("\n[D21] sleep(0)\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	unsigned int ret = secondsleep(0);
	clock_gettime(CLOCK_MONOTONIC, &b);

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    elapsed = %lld ns\n", elapsed);

	CHECK(ret == 0, "sleep(0) returns 0");
	CHECK(elapsed < 1000000LL, "Elapsed is below 1 ms");
}

/* ==================== Group E: integration ==================== */

/* T22: Multiple threads using sleep functions concurrently. */
struct thread_args {
	int id;
	int result;
};

static void *thread_sleeper(void *arg) {
	struct thread_args *ta = (struct thread_args *)arg;
	struct timespec a, b;

	clock_gettime(CLOCK_MONOTONIC, &a);
	int ret = usleep(20000);   /* 20 ms */
	clock_gettime(CLOCK_MONOTONIC, &b);

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	long long err = elapsed - 20000000LL;
	if (err < 0) err = -err;

	if (ret != 0 || err >= 5000000LL) ta->result = -1;
	else                              ta->result = 0;
	return NULL;
}

static void test_E22_concurrent_sleep(void) {
	printf("\n[E22] Concurrent sleeping threads\n");

	enum { N = 8 };
	pthread_t th[N];
	struct thread_args args[N];

	for (int i = 0; i < N; i++) {
		args[i].id = i;
		args[i].result = 0;
		pthread_create(&th[i], NULL, thread_sleeper, &args[i]);
	}

	bool ok = true;
	for (int i = 0; i < N; i++) {
		pthread_join(th[i], NULL);
		if (args[i].result != 0) ok = false;
	}
	CHECK(ok, "8 threads sleeping 20 ms concurrently all finish within 5 ms error");
}

#if 0
/* T23: Throughput of clock_gettime. */
static void test_E23_clock_gettime_throughput(void) {
	printf("\n[E23] clock_gettime throughput\n");

	const int N = 1000000;
	struct timespec ts, t0, t1;

	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (int i = 0; i < N; i++) {
		clock_gettime(CLOCK_MONOTONIC, &ts);
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);

	long long total = ts_to_ns(&t1) - ts_to_ns(&t0);
	double per = (double)total / N;

	printf("    %d calls took %lld ns, average %.1f ns/call\n",
		N, total, per);

	CHECK(total > 0, "Total duration is positive");
	CHECK(per < 200000.0, "Average per-call overhead < 200 us");
}
#endif

/* T24: Interleaved calls to all four functions stay consistent. */
static void test_E24_mixed_consistency(void) {
	printf("\n[E24] Mixed calls consistency\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);

	nanosleep(&(struct timespec) { .tv_sec = 0, .tv_nsec = 5000000L }, NULL);
	usleep(5000);
	secondsleep(0);
	usleep(5000);
	nanosleep(&(struct timespec) { .tv_sec = 0, .tv_nsec = 5000000L }, NULL);

	clock_gettime(CLOCK_MONOTONIC, &b);

	long long delta = ts_to_ns(&b) - ts_to_ns(&a);
	printf("    cumulative delta = %lld ns (expected about 20 ms)\n", delta);

	CHECK(delta >= 20000000LL && delta < 30000000LL,
		"Cumulative delay in [20 ms, 30 ms)");
}

/* T25: Busy-wait comparison. */
static void test_E25_busy_wait_reference(void) {
	printf("\n[E25] Comparison against busy-wait reference\n");

	struct timespec a, b;
	clock_gettime(CLOCK_MONOTONIC, &a);
	usleep(50000);   /* 50 ms */
	clock_gettime(CLOCK_MONOTONIC, &b);

	long long elapsed = ts_to_ns(&b) - ts_to_ns(&a);
	long long err = elapsed - 50000000LL;
	if (err < 0) err = -err;

	printf("    usleep(50 ms) measured = %lld ns, error = %lld ns\n",
		elapsed, err);

	CHECK(elapsed >= 50000000LL, "Sleep is not shorter than requested");
	CHECK(err < 3000000LL, "Error is below 3 ms");
}

/* ==================== Main ==================== */

int sleep_main(void) {
	printf("========== POSIX time functions test suite ==========\n");

	/* Warm up: trigger ensure_init() before timed tests. */
	struct timespec warmup;
	if (clock_gettime(CLOCK_MONOTONIC, &warmup) != 0) {
		printf("Initialization failed; aborting.\n");
		return 1;
	}
	printf("Initialization OK.\n");

	/* Group A: clock_gettime */
	test_A01_realtime_sanity();
	test_A02_monotonic_sanity();
	test_A03_null_pointer();
	test_A04_invalid_clockid();
	test_A05_nsec_range();
	test_A06_monotonicity();
	test_A07_step_consistency();
	test_A08_second_carry();

	/* Group B: nanosleep */
	test_B09_nanosleep_basic();
	test_B10_nanosleep_zero();
	test_B11_nanosleep_null();
	test_B12_nanosleep_invalid();
	test_B13_nanosleep_cross_second();
	test_B14_nanosleep_rem_zeroed();
	test_B15_nanosleep_precision();

	/* Group C: usleep */
	test_C16_usleep_basic();
	test_C17_usleep_zero();
	test_C18_usleep_precision();

	/* Group D: sleep */
	test_D19_sleep_one();
	test_D20_sleep_two_seconds();
	test_D21_sleep_zero();

	/* Group E: integration */
	test_E22_concurrent_sleep();
	//test_E23_clock_gettime_throughput();
	test_E24_mixed_consistency();
	test_E25_busy_wait_reference();

	printf("\n========== Test results ==========\n");
	printf("PASS: %d\n", g_pass);
	printf("FAIL: %d\n", g_fail);
	printf("Total: %d\n", g_pass + g_fail);

	return g_fail == 0 ? 0 : 1;
}
