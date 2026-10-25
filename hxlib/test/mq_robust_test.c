/***********************************************************************/
/*    Module Name   : mq_robust_test.c                                 */
/*    Module Funciton:                                                  */
/*       Extended / robustness test for the HelloX POSIX message       */
/*       queue (mq.h/mq.c).                                            */
/*                                                                     */
/*       Covers:                                                       */
/*         - name & attribute validation (EINVAL/ENAMETOOLONG/...)     */
/*         - error paths (EBADF/ENOENT/EMSGSIZE/EAGAIN)                */
/*         - zero-length / max-length messages                         */
/*         - binary payload integrity (magic + checksum)               */
/*         - O_NONBLOCK via mq_setattr                                 */
/*         - mq_curmsgs attribute tracking                             */
/*         - reference counting (reopen / close / unlink lifecycle)    */
/*         - multi-queue isolation                                    */
/*         - deep queue / large message                                */
/*         - timed send/receive success paths                          */
/*         - concurrent stress (multi-producer / multi-consumer)       */
/***********************************************************************/

#include <hellox.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <mq.h>

static int g_pass = 0;
static int g_fail = 0;

static void report(const char *name, int ok)
{
    if (ok)
    {
        g_pass++;
        _hx_printf("  [PASS] %s\r\n", name);
    }
    else
    {
        g_fail++;
        _hx_printf("  [FAIL] %s\r\n", name);
    }
}

/* Convenience: create a queue with the given dimensions. */
static mqd_t create_q(const char *name, long maxmsg, long msgsize)
{
    struct mq_attr a;
    mqd_t          m;
    a.mq_maxmsg  = maxmsg;
    a.mq_msgsize = msgsize;
    m = mq_open(name, O_CREAT | O_RDWR, 0666, &a);
    if (m == (mqd_t)-1)
    {
        _hx_printf("  [create_q FAIL] %s errno=%d\r\n", name, errno);
    }
    return m;
}

/* =====================================================================
 * Concurrent stress-test support
 * ===================================================================== */

#define NPROD         4
#define NCONS         4
#define PER_THREAD    50
#define TOTAL         (NPROD * PER_THREAD)
#define STRESS_ROUNDS 20

/* Producer: sends PER_THREAD messages, each a unique id = tid*PER_THREAD+seq. */
typedef struct {
    mqd_t mq;
    int   tid;
} prod_arg_t;

static void *stress_producer(void *arg)
{
    prod_arg_t *a = (prod_arg_t *)arg;
    int seq;

    for (seq = 0; seq < PER_THREAD; seq++)
    {
        int id = a->tid * PER_THREAD + seq;
        if (mq_send(a->mq, (const char *)&id, sizeof(id), 1) != 0)
        {
            _hx_printf("    [producer %d] mq_send failed errno=%d\r\n", a->tid, errno);
        }
    }
    return NULL;
}

/* Consumer: receives cnt messages, marks each received id (detects loss/dup). */
typedef struct {
    mqd_t mq;
    int   cnt;
} cons_arg_t;

static unsigned char  g_received[TOTAL];
static int            g_dup;
static pthread_mutex_t g_lock;

static void *stress_consumer(void *arg)
{
    cons_arg_t *a = (cons_arg_t *)arg;
    int i;

    for (i = 0; i < a->cnt; i++)
    {
        int id = -1;
        if (mq_receive(a->mq, (char *)&id, sizeof(id), NULL) < 0)
        {
            _hx_printf("    [consumer] mq_receive failed errno=%d\r\n", errno);
            continue;
        }
        pthread_mutex_lock(&g_lock);
        if (id < 0 || id >= TOTAL || g_received[id])
        {
            g_dup = 1;
        }
        else
        {
            g_received[id] = 1;
        }
        pthread_mutex_unlock(&g_lock);
    }
    return NULL;
}

/* =====================================================================
 * One round of the concurrent stress test
 * ===================================================================== */

/* Round A: multi-producer -> single consumer, verify per-thread FIFO order.
 * Returns 0 on success, 1 on any failure. */
static int stress_round_fifo(void)
{
    mqd_t      mq;
    pthread_t  pt[NPROD];
    prod_arg_t pa[NPROD];
    int        started[NPROD];
    int        i;
    int        fifo_ok = 1;
    int        last_seq[NPROD];

    mq = create_q("/q_stress1", 16, 4);
    if (mq == (mqd_t)-1)
    {
        return 1;
    }

    for (i = 0; i < NPROD; i++)
    {
        pa[i].mq     = mq;
        pa[i].tid    = i;
        started[i]   = (pthread_create(&pt[i], NULL, stress_producer, &pa[i]) == 0);
        if (!started[i])
        {
            _hx_printf("    [pthread_create FAIL] producer %d\r\n", i);
            fifo_ok = 0;
        }
    }

    for (i = 0; i < NPROD; i++)
    {
        last_seq[i] = -1;
    }

    for (i = 0; i < TOTAL; i++)
    {
        int id = -1;
        if (mq_receive(mq, (char *)&id, sizeof(id), NULL) < 0)
        {
            fifo_ok = 0;
            break;
        }
        if (id < 0 || id >= TOTAL)
        {
            fifo_ok = 0;
        }
        else
        {
            int tid = id / PER_THREAD;
            int seq = id % PER_THREAD;
            if (seq != last_seq[tid] + 1)
            {
                fifo_ok = 0;
            }
            last_seq[tid] = seq;
        }
    }

    for (i = 0; i < NPROD; i++)
    {
        if (started[i])
        {
            pthread_join(pt[i], NULL);
        }
    }
    mq_close(mq);

    return fifo_ok ? 0 : 1;
}

/* Round B: multi-producer -> multi-consumer, verify no loss / no dup.
 * Returns 0 on success, 1 on any failure. */
static int stress_round_multi(void)
{
    mqd_t      mq;
    pthread_t  pt[NPROD], ct[NCONS];
    prod_arg_t pa[NPROD];
    cons_arg_t ca[NCONS];
    int        started[NPROD + NCONS];
    int        i;
    int        multi_ok = 1;

    mq = create_q("/q_stress2", 16, 4);
    if (mq == (mqd_t)-1)
    {
        return 1;
    }

    memset(g_received, 0, sizeof(g_received));
    g_dup = 0;

    for (i = 0; i < NPROD; i++)
    {
        pa[i].mq    = mq;
        pa[i].tid   = i;
        started[i]  = (pthread_create(&pt[i], NULL, stress_producer, &pa[i]) == 0);
        if (!started[i])
        {
            _hx_printf("    [pthread_create FAIL] producer %d\r\n", i);
            multi_ok = 0;
        }
    }
    for (i = 0; i < NCONS; i++)
    {
        ca[i].mq             = mq;
        ca[i].cnt            = TOTAL / NCONS;
        started[NPROD + i]   = (pthread_create(&ct[i], NULL, stress_consumer, &ca[i]) == 0);
        if (!started[NPROD + i])
        {
            _hx_printf("    [pthread_create FAIL] consumer %d\r\n", i);
            multi_ok = 0;
        }
    }

    for (i = 0; i < NPROD; i++)
    {
        if (started[i])
        {
            pthread_join(pt[i], NULL);
        }
    }
    for (i = 0; i < NCONS; i++)
    {
        if (started[NPROD + i])
        {
            pthread_join(ct[i], NULL);
        }
    }

    for (i = 0; i < TOTAL; i++)
    {
        if (!g_received[i])
        {
            multi_ok = 0;
        }
    }
    if (g_dup)
    {
        multi_ok = 0;
    }

    mq_close(mq);

    return multi_ok ? 0 : 1;
}

/* =====================================================================
 * Binary integrity payload
 * ===================================================================== */

typedef struct {
    unsigned int  magic;
    int           seq;
    unsigned char data[24];
    unsigned int  checksum;
} bin_msg_t;

#define BIN_MAGIC 0x5A5A5A5Au

static void fill_bin_msg(bin_msg_t *m, int seq)
{
    unsigned int sum = 0;
    int i;

    m->magic = BIN_MAGIC;
    m->seq   = seq;
    for (i = 0; i < (int)sizeof(m->data); i++)
    {
        m->data[i] = (unsigned char)(seq + i * 7);
    }
    for (i = 0; i < (int)sizeof(m->data); i++)
    {
        sum += m->data[i];
    }
    m->checksum = sum;
}

static int verify_bin_msg(const bin_msg_t *m, int expect_seq)
{
    unsigned int sum = 0;
    int i;

    if (m->magic != BIN_MAGIC || m->seq != expect_seq)
    {
        return 0;
    }
    for (i = 0; i < (int)sizeof(m->data); i++)
    {
        if (m->data[i] != (unsigned char)(expect_seq + i * 7))
        {
            return 0;
        }
        sum += m->data[i];
    }
    return m->checksum == sum;
}

/* =====================================================================
 * main
 * ===================================================================== */
int main(void)
{
    mqd_t          mq, mq2;
    struct mq_attr attr, got;
    struct timespec ts, now;
    char           sbuf[2048];
    char           rbuf[2048];
    char           longname[MQ_NAME_MAX + 4];
    ssize_t        n;
    int            i;

    _hx_printf("=== HelloX mqueue robustness test ===\r\n");

    /*------------------------------------------------------------------
     * Section 1: name & attribute validation
     *------------------------------------------------------------------*/
    _hx_printf("--- name/attribute validation ---\r\n");

    errno = 0;
    mq = mq_open(NULL, O_CREAT | O_RDWR, 0666, &attr);
    report("mq_open NULL name -> EINVAL", mq == (mqd_t)-1 && errno == EINVAL);

    errno = 0;
    mq = mq_open("no_leading_slash", O_CREAT | O_RDWR, 0666, &attr);
    report("mq_open no leading '/' -> EINVAL", mq == (mqd_t)-1 && errno == EINVAL);

    memset(longname, 'a', sizeof(longname));
    longname[0] = '/';
    longname[MQ_NAME_MAX + 2] = '\0';
    errno = 0;
    mq = mq_open(longname, O_CREAT | O_RDWR, 0666, &attr);
    report("mq_open over-long name -> ENAMETOOLONG",
           mq == (mqd_t)-1 && errno == ENAMETOOLONG);

    errno = 0;
    mq = mq_open("/q_nullattr", O_CREAT | O_RDWR, 0666, NULL);
    report("mq_open O_CREAT NULL attr -> EINVAL", mq == (mqd_t)-1 && errno == EINVAL);

    attr.mq_maxmsg = 0;
    attr.mq_msgsize = 16;
    errno = 0;
    mq = mq_open("/q_zero_max", O_CREAT | O_RDWR, 0666, &attr);
    report("mq_open maxmsg=0 -> EINVAL", mq == (mqd_t)-1 && errno == EINVAL);

    attr.mq_maxmsg = 4;
    attr.mq_msgsize = 0;
    errno = 0;
    mq = mq_open("/q_zero_size", O_CREAT | O_RDWR, 0666, &attr);
    report("mq_open msgsize=0 -> EINVAL", mq == (mqd_t)-1 && errno == EINVAL);

    /*------------------------------------------------------------------
     * Section 2: error paths on invalid descriptors / names
     *------------------------------------------------------------------*/
    _hx_printf("--- error paths ---\r\n");

    errno = 0;
    report("mq_close invalid -> EBADF", mq_close(999) == -1 && errno == EBADF);

    errno = 0;
    report("mq_close negative -> EBADF", mq_close(-5) == -1 && errno == EBADF);

    errno = 0;
    report("mq_unlink missing -> ENOENT", mq_unlink("/does_not_exist") == -1 && errno == ENOENT);

    errno = 0;
    report("mq_send invalid -> EBADF", mq_send(999, "x", 2, 0) == -1 && errno == EBADF);

    errno = 0;
    report("mq_receive invalid -> EBADF", mq_receive(999, rbuf, sizeof(rbuf), NULL) == -1 && errno == EBADF);

    /*------------------------------------------------------------------
     * Section 3: zero-length and max-length messages
     *------------------------------------------------------------------*/
    _hx_printf("--- message size boundaries ---\r\n");

    mq = create_q("/q_boundary", 4, 16);

    /* zero-length message: send length 0, receive returns 0 */
    report("mq_send zero-length", mq_send(mq, "", 0, 0) == 0);
    errno = 0;
    n = mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    report("mq_receive zero-length (returns 0)", n == 0);

    /* max-length message (exactly mq_msgsize) */
    memset(sbuf, 'B', 16);
    report("mq_send max-size", mq_send(mq, sbuf, 16, 0) == 0);
    memset(rbuf, 0, sizeof(rbuf));
    n = mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    report("mq_receive max-size", n == 16 && memcmp(rbuf, sbuf, 16) == 0);
    mq_close(mq);

    /*------------------------------------------------------------------
     * Section 4: binary payload integrity
     *------------------------------------------------------------------*/
    _hx_printf("--- binary integrity ---\r\n");

    mq = create_q("/q_bin", 8, (long)sizeof(bin_msg_t));
    {
        bin_msg_t out, in;
        int ok = 1;
        for (i = 0; i < 8; i++)
        {
            fill_bin_msg(&out, i);
            if (mq_send(mq, (const char *)&out, sizeof(out), 0) != 0)
            {
                ok = 0;
            }
        }
        for (i = 0; i < 8; i++)
        {
            memset(&in, 0, sizeof(in));
            if (mq_receive(mq, (char *)&in, sizeof(in), NULL) != (ssize_t)sizeof(in) ||
                !verify_bin_msg(&in, i))
            {
                ok = 0;
            }
        }
        report("binary payload integrity (magic+checksum)", ok);
    }
    mq_close(mq);

    /*------------------------------------------------------------------
     * Section 5: O_NONBLOCK via mq_setattr
     *------------------------------------------------------------------*/
    _hx_printf("--- non-blocking mode ---\r\n");

    mq = create_q("/q_nb", 2, 16);

    /* set O_NONBLOCK */
    got.mq_flags = O_NONBLOCK;
    report("mq_setattr set O_NONBLOCK", mq_setattr(mq, &got, NULL) == 0);

    /* receive on empty -> EAGAIN */
    errno = 0;
    report("mq_receive empty (O_NONBLOCK) -> EAGAIN",
           mq_receive(mq, rbuf, sizeof(rbuf), NULL) == -1 && errno == EAGAIN);

    /* fill to capacity, then send -> EAGAIN */
    mq_send(mq, "a", 2, 0);
    mq_send(mq, "b", 2, 0);
    errno = 0;
    report("mq_send full (O_NONBLOCK) -> EAGAIN",
           mq_send(mq, "c", 2, 0) == -1 && errno == EAGAIN);

    /* clear O_NONBLOCK (restore blocking) */
    got.mq_flags = 0;
    report("mq_setattr clear O_NONBLOCK", mq_setattr(mq, &got, NULL) == 0);

    /* drain */
    mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    mq_close(mq);

    /*------------------------------------------------------------------
     * Section 6: mq_curmsgs attribute tracking
     *------------------------------------------------------------------*/
    _hx_printf("--- attribute tracking ---\r\n");

    mq = create_q("/q_attr", 4, 16);
    {
        int c0 = -1, c1 = -1, c2 = -1;
        mq_getattr(mq, &got);
        c0 = (int)got.mq_curmsgs;
        mq_send(mq, "x", 2, 0);
        mq_send(mq, "y", 2, 0);
        mq_getattr(mq, &got);
        c1 = (int)got.mq_curmsgs;
        mq_receive(mq, rbuf, sizeof(rbuf), NULL);
        mq_getattr(mq, &got);
        c2 = (int)got.mq_curmsgs;
        report("mq_curmsgs tracks 0->2->1", c0 == 0 && c1 == 2 && c2 == 1);
        mq_receive(mq, rbuf, sizeof(rbuf), NULL);   /* drain */
    }
    mq_close(mq);

    /*------------------------------------------------------------------
     * Section 7: lifecycle (reference counting, reopen after unlink)
     *------------------------------------------------------------------*/
    _hx_printf("--- lifecycle ---\r\n");

    mq  = create_q("/q_life", 4, 16);
    mq2 = mq_open("/q_life", O_RDWR);       /* reopen: same name, no O_CREAT */
    report("reopen same name returns same queue", mq2 == mq);

    report("mq_close once (queue still alive)", mq_close(mq) == 0);
    report("queue usable after one close", mq_send(mq2, "z", 2, 0) == 0);
    mq_receive(mq2, rbuf, sizeof(rbuf), NULL);

    report("mq_close second time (queue destroyed)", mq_close(mq2) == 0);
    errno = 0;
    report("mq_close third time -> EBADF", mq_close(mq2) == -1 && errno == EBADF);

    /* unlink then reopen same name creates a fresh, empty queue */
    mq = create_q("/q_life2", 4, 16);
    mq_send(mq, "old", 4, 0);               /* old queue holds one message */
    mq_unlink("/q_life2");
    mq2 = create_q("/q_life2", 4, 16);      /* new queue with same name */
    memset(&ts, 0, sizeof(ts));
    errno = 0;
    report("reopened queue after unlink is empty",
           mq_timedreceive(mq2, rbuf, sizeof(rbuf), NULL, &ts) == -1 && errno == ETIMEDOUT);
    report("old queue still holds its message",
           mq_receive(mq, rbuf, sizeof(rbuf), NULL) == 4 && strcmp(rbuf, "old") == 0);
    mq_close(mq);
    mq_close(mq2);

    /*------------------------------------------------------------------
     * Section 8: multi-queue isolation
     *------------------------------------------------------------------*/
    _hx_printf("--- multi-queue isolation ---\r\n");

    mq  = create_q("/q_iso_a", 4, 16);
    mq2 = create_q("/q_iso_b", 4, 16);
    mq_send(mq, "A", 2, 0);
    mq_send(mq2, "B", 2, 0);
    {
        int iso_ok = 1;
        memset(rbuf, 0, sizeof(rbuf));
        mq_receive(mq, rbuf, sizeof(rbuf), NULL);
        if (strcmp(rbuf, "A") != 0) iso_ok = 0;
        memset(rbuf, 0, sizeof(rbuf));
        mq_receive(mq2, rbuf, sizeof(rbuf), NULL);
        if (strcmp(rbuf, "B") != 0) iso_ok = 0;
        report("queues are isolated", iso_ok);
    }
    mq_close(mq);
    mq_close(mq2);

    /*------------------------------------------------------------------
     * Section 9: deep queue and large message
     *------------------------------------------------------------------*/
    _hx_printf("--- deep queue / large message ---\r\n");

    mq = create_q("/q_deep", 50, 16);
    {
        int deep_ok = 1;
        for (i = 0; i < 50; i++)
        {
            sbuf[0] = (char)(i & 0x7F);
            sbuf[1] = '\0';
            if (mq_send(mq, sbuf, 2, 0) != 0) deep_ok = 0;
        }
        for (i = 0; i < 50; i++)
        {
            memset(rbuf, 0, sizeof(rbuf));
            if (mq_receive(mq, rbuf, sizeof(rbuf), NULL) != 2 ||
                rbuf[0] != (char)(i & 0x7F))
            {
                deep_ok = 0;
            }
        }
        report("deep queue (50 msgs) FIFO round-trip", deep_ok);
    }
    mq_close(mq);

    mq = create_q("/q_big", 2, 1024);
    memset(sbuf, 0x5C, 1024);
    report("large message (1024B) send", mq_send(mq, sbuf, 1024, 0) == 0);
    memset(rbuf, 0, sizeof(rbuf));
    n = mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    report("large message (1024B) receive",
           n == 1024 && memcmp(rbuf, sbuf, 1024) == 0);
    mq_close(mq);

    /*------------------------------------------------------------------
     * Section 10: timed send/receive success paths
     *------------------------------------------------------------------*/
    _hx_printf("--- timed send/receive success ---\r\n");

    mq = create_q("/q_timed", 4, 16);

    clock_gettime(CLOCK_REALTIME, &now);
    ts.tv_sec  = now.tv_sec + 1;
    ts.tv_nsec = now.tv_nsec;
    report("mq_timedsend future deadline succeeds",
           mq_timedsend(mq, "t", 2, 0, &ts) == 0);

    clock_gettime(CLOCK_REALTIME, &now);
    ts.tv_sec  = now.tv_sec + 1;
    ts.tv_nsec = now.tv_nsec;
    memset(rbuf, 0, sizeof(rbuf));
    n = mq_timedreceive(mq, rbuf, sizeof(rbuf), NULL, &ts);
    report("mq_timedreceive future deadline succeeds",
           n == 2 && strcmp(rbuf, "t") == 0);
    mq_close(mq);

    /*------------------------------------------------------------------
     * Section 11: concurrent stress tests (looped)
     *------------------------------------------------------------------*/
    _hx_printf("--- concurrent stress (%d rounds) ---\r\n", STRESS_ROUNDS);
    {
        int round;
        int fifo_fail  = 0;
        int multi_fail = 0;

        pthread_mutex_init(&g_lock, NULL);

        for (round = 1; round <= STRESS_ROUNDS; round++)
        {
            int r1 = stress_round_fifo();
            int r2 = stress_round_multi();

            _hx_printf("  [round %2d] fifo=%s multi=%s\r\n",
                       round, r1 ? "FAIL" : "OK", r2 ? "FAIL" : "OK");

            if (r1) fifo_fail++;
            if (r2) multi_fail++;
        }

        pthread_mutex_destroy(&g_lock);

        report("stress: fifo all rounds", fifo_fail == 0);
        report("stress: multi-consumer all rounds", multi_fail == 0);
    }

    _hx_printf("=== SUMMARY: %d pass, %d fail ===\r\n", g_pass, g_fail);

    return (g_fail == 0) ? 0 : 1;
}
