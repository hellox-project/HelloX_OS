/***********************************************************************/
/*    Module Name   : mq_test.c                                        */
/*    Module Funciton:                                                  */
/*       Test program for the HelloX POSIX message queue (mq.h/mq.c). */
/*       Prints [PASS]/[FAIL] for each case plus a summary.            */
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

/* Context passed to a producer/consumer helper thread. */
typedef struct {
    mqd_t       mq;
    const char *msg;
    size_t      len;
    int         delay_ms;
} thread_ctx_t;

/* Thread that sends a message after a delay (to test blocking receive). */
static void *producer_thread(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    Sleep(ctx->delay_ms);
    if (mq_send(ctx->mq, ctx->msg, ctx->len, 1) != 0)
    {
        _hx_printf("    [producer] mq_send failed errno=%d\r\n", errno);
    }
    return NULL;
}

/* Thread that receives a message after a delay (to test blocking send). */
static void *consumer_thread(void *arg)
{
    thread_ctx_t *ctx = (thread_ctx_t *)arg;
    char buf[128];
    Sleep(ctx->delay_ms);
    if (mq_receive(ctx->mq, buf, sizeof(buf), NULL) < 0)
    {
        _hx_printf("    [consumer] mq_receive failed errno=%d\r\n", errno);
    }
    return NULL;
}

int main(void)
{
    mqd_t          mq, mq2;
    struct mq_attr attr, got;
    struct timespec ts, now;
    char           sbuf[128];
    char           rbuf[128];
    ssize_t        n;
    int            i;
    pthread_t      th;
    thread_ctx_t   tctx;

    _hx_printf("=== HelloX mqueue test ===\r\n");

    /*--------------------------------------------------------------
     * Case 1: create a queue and verify attributes
     *--------------------------------------------------------------*/
    attr.mq_maxmsg  = 3;
    attr.mq_msgsize = 32;
    mq = mq_open("/testq", O_CREAT | O_RDWR, 0666, &attr);
    report("mq_open create", mq != (mqd_t)-1);
    if (mq == (mqd_t)-1)
    {
        _hx_printf("    errno=%d\r\n", errno);
        _hx_printf("=== SUMMARY: %d pass, %d fail ===\r\n", g_pass, g_fail);
        return 1;
    }

    memset(&got, 0, sizeof(got));
    if (mq_getattr(mq, &got) == 0)
    {
        report("mq_getattr maxmsg/msgsize/curmsgs",
               got.mq_maxmsg == 3 && got.mq_msgsize == 32 && got.mq_curmsgs == 0);
    }
    else
    {
        report("mq_getattr maxmsg/msgsize/curmsgs", 0);
    }

    /*--------------------------------------------------------------
     * Case 2: basic send/receive round-trip
     *--------------------------------------------------------------*/
    strcpy(sbuf, "hello mqueue");
    report("mq_send basic",
           mq_send(mq, sbuf, strlen(sbuf) + 1, 1) == 0);
    memset(rbuf, 0, sizeof(rbuf));
    n = mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    report("mq_receive basic",
           n == (ssize_t)(strlen(sbuf) + 1) && strcmp(rbuf, sbuf) == 0);

    /*--------------------------------------------------------------
     * Case 3: FIFO ordering of multiple messages
     *--------------------------------------------------------------*/
    for (i = 0; i < 3; i++)
    {
        sbuf[0] = '0' + i;
        sbuf[1] = '\0';
        mq_send(mq, sbuf, 2, 1);
    }
    {
        int order_ok = 1;
        for (i = 0; i < 3; i++)
        {
            memset(rbuf, 0, sizeof(rbuf));
            mq_receive(mq, rbuf, sizeof(rbuf), NULL);
            if (rbuf[0] != ('0' + i))
            {
                order_ok = 0;
            }
        }
        report("mq FIFO order", order_ok);
    }

    /*--------------------------------------------------------------
     * Case 4: EMSGSIZE on over-long send
     *--------------------------------------------------------------*/
    {
        char big[64];
        memset(big, 'A', sizeof(big));
        errno = 0;
        report("mq_send EMSGSIZE",
               mq_send(mq, big, 64, 1) == -1 && errno == EMSGSIZE);
    }

    /*--------------------------------------------------------------
     * Case 5: EMSGSIZE on under-sized receive buffer
     *--------------------------------------------------------------*/
    strcpy(sbuf, "abc");
    mq_send(mq, sbuf, 4, 1);
    {
        char small[4];
        errno = 0;
        report("mq_receive EMSGSIZE",
               mq_receive(mq, small, sizeof(small), NULL) == -1 && errno == EMSGSIZE);
        mq_receive(mq, rbuf, sizeof(rbuf), NULL); /* drain */
    }

    /*--------------------------------------------------------------
     * Case 6: queue full detection via mq_timedsend (zero timeout)
     *--------------------------------------------------------------*/
    for (i = 0; i < 3; i++)
    {
        mq_send(mq, "x", 2, 1);   /* fill to capacity */
    }
    memset(&ts, 0, sizeof(ts));
    errno = 0;
    report("mq_timedsend full->ETIMEDOUT",
           mq_timedsend(mq, "y", 2, 1, &ts) == -1 && errno == ETIMEDOUT);

    /*--------------------------------------------------------------
     * Case 7: queue empty detection via mq_timedreceive (zero timeout)
     *--------------------------------------------------------------*/
    for (i = 0; i < 3; i++)
    {
        mq_receive(mq, rbuf, sizeof(rbuf), NULL);   /* drain */
    }
    memset(&ts, 0, sizeof(ts));
    errno = 0;
    report("mq_timedreceive empty->ETIMEDOUT",
           mq_timedreceive(mq, rbuf, sizeof(rbuf), NULL, &ts) == -1 && errno == ETIMEDOUT);

    /*--------------------------------------------------------------
     * Case 8: mq_timedreceive with a real future deadline (blocking)
     *--------------------------------------------------------------*/
    clock_gettime(CLOCK_REALTIME, &now);
    ts.tv_sec  = now.tv_sec + 1;
    ts.tv_nsec = now.tv_nsec;
    tctx.mq = mq;
    tctx.msg = "timed";
    tctx.len = 6;
    tctx.delay_ms = 200;
    pthread_create(&th, NULL, producer_thread, &tctx);
    n = mq_timedreceive(mq, rbuf, sizeof(rbuf), NULL, &ts);
    report("mq_timedreceive deadline ok",
           n == 6 && strcmp(rbuf, "timed") == 0);
    pthread_join(th, NULL);

    /*--------------------------------------------------------------
     * Case 9: blocking receive (producer thread sends after delay)
     *--------------------------------------------------------------*/
    tctx.mq = mq;
    tctx.msg = "blocked";
    tctx.len = 8;
    tctx.delay_ms = 200;
    pthread_create(&th, NULL, producer_thread, &tctx);
    memset(rbuf, 0, sizeof(rbuf));
    n = mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    report("mq_receive blocking", n == 8 && strcmp(rbuf, "blocked") == 0);
    pthread_join(th, NULL);

    /*--------------------------------------------------------------
     * Case 10: blocking send (consumer thread drains after delay)
     *--------------------------------------------------------------*/
    for (i = 0; i < 3; i++)
    {
        mq_send(mq, "f", 2, 1);   /* fill */
    }
    tctx.mq = mq;
    tctx.len = 0;
    tctx.delay_ms = 200;
    pthread_create(&th, NULL, consumer_thread, &tctx);
    report("mq_send blocking", mq_send(mq, "g", 2, 1) == 0);
    pthread_join(th, NULL);
    /* drain the remainder: 3 filled - 1 consumed + 1 sent = 3 messages */
    for (i = 0; i < 3; i++)
    {
        mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    }

    /*--------------------------------------------------------------
     * Case 11: O_EXCL on existing name -> EEXIST
     *--------------------------------------------------------------*/
    errno = 0;
    mq2 = mq_open("/testq", O_CREAT | O_EXCL | O_RDWR, 0666, &attr);
    report("mq_open O_EXCL->EEXIST", mq2 == (mqd_t)-1 && errno == EEXIST);

    /*--------------------------------------------------------------
     * Case 12: open non-existent without O_CREAT -> ENOENT
     *--------------------------------------------------------------*/
    errno = 0;
    mq2 = mq_open("/no_such_queue", O_RDWR);
    report("mq_open no O_CREAT->ENOENT", mq2 == (mqd_t)-1 && errno == ENOENT);

    /*--------------------------------------------------------------
     * Case 13: unlink removes the name but the queue stays usable
     *--------------------------------------------------------------*/
    report("mq_unlink", mq_unlink("/testq") == 0);
    strcpy(sbuf, "after unlink");
    report("mq still usable after unlink",
           mq_send(mq, sbuf, strlen(sbuf) + 1, 1) == 0);
    memset(rbuf, 0, sizeof(rbuf));
    n = mq_receive(mq, rbuf, sizeof(rbuf), NULL);
    report("mq receive after unlink",
           n == (ssize_t)(strlen(sbuf) + 1) && strcmp(rbuf, sbuf) == 0);

    /*--------------------------------------------------------------
     * Case 14: close
     *--------------------------------------------------------------*/
    report("mq_close", mq_close(mq) == 0);

    _hx_printf("=== SUMMARY: %d pass, %d fail ===\r\n", g_pass, g_fail);

    return (g_fail == 0) ? 0 : 1;
}
