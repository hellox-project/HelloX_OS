/***********************************************************************/
/*    Module Name   : mq.c                                             */
/*    Module Funciton:                                                  */
/*       POSIX message queue (mqueue) implementation for HelloX.       */
/*                                                                     */
/*       Implemented entirely in user space on top of the existing     */
/*       hxlib primitives:                                             */
/*         - sem_t  (sem_init/sem_wait/sem_trywait/sem_timedwait/      */
/*                   sem_post/sem_destroy)  -> blocking + timeout       */
/*         - pthread_mutex_t               -> protecting the ring buf   */
/*                                                                     */
/*       Synchronization model (classic producer/consumer):            */
/*         sem_space : free slots in queue (init = mq_maxmsg)          */
/*         sem_msgs  : available messages  (init = 0)                  */
/*         send:    wait(sem_space) -> lock -> enqueue -> unlock       */
/*                  -> post(sem_msgs)                                  */
/*         receive: wait(sem_msgs)  -> lock -> dequeue -> unlock       */
/*                  -> post(sem_space)                                 */
/*                                                                     */
/*       Only includes/references existing hxlib headers; no kernel    */
/*       or existing-source modification.                              */
/***********************************************************************/

#include "mq.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>   /* provides clang-compatible va_list/va_start/va_arg/va_end */
#include <pthread.h>

/* Maximum number of simultaneously open queues. */
#define MQ_MAX_OPEN 64

/* Internal queue control block. */
typedef struct mq_qcb {
    char             name[MQ_NAME_MAX + 2];  /* queue name incl. leading '/' */
    long             mq_maxmsg;              /* max messages */
    long             mq_msgsize;             /* max message size */
    int              oflag;                  /* 0 or O_NONBLOCK */
    int              in_use;                 /* slot allocated */
    int              unlinked;               /* name removed from namespace */
    int              refcount;               /* open descriptor count */

    sem_t            sem_msgs;               /* available messages (init 0) */
    sem_t            sem_space;              /* free slots (init mq_maxmsg) */
    pthread_mutex_t  lock;                   /* guards head/tail/curmsgs */

    unsigned char   *buf;                    /* mq_maxmsg * mq_msgsize bytes */
    size_t          *msg_len;                /* per-slot actual length */
    long             head;                   /* read index */
    long             tail;                   /* write index */
    long             mq_curmsgs;             /* current message count */
} mq_qcb_t;

static mq_qcb_t g_qcb[MQ_MAX_OPEN];

/* Check whether mqdes refers to a currently-open queue. */
static int mq_valid(mqd_t mqdes)
{
    return (mqdes >= 0 && mqdes < MQ_MAX_OPEN && g_qcb[mqdes].in_use);
}

/*--------------------------------------------------------------------*/
/* mq_open                                                            */
/*--------------------------------------------------------------------*/
mqd_t mq_open(const char *name, int oflag, ...)
{
    int             mode = 0;
    struct mq_attr *attr = NULL;
    mq_qcb_t       *q;
    int             i;
    size_t          name_len;

    if (oflag & O_CREAT)
    {
        va_list ap;
        va_start(ap, oflag);
        mode = va_arg(ap, int);
        attr = va_arg(ap, struct mq_attr *);
        va_end(ap);
    }
    (void)mode;

    if (name == NULL || name[0] != '/')
    {
        errno = EINVAL;
        return (mqd_t)-1;
    }

    name_len = strlen(name);
    if (name_len > MQ_NAME_MAX + 1)
    {
        errno = ENAMETOOLONG;
        return (mqd_t)-1;
    }

    /* Look for an existing (still linked) queue with this name. */
    for (i = 0; i < MQ_MAX_OPEN; i++)
    {
        if (g_qcb[i].in_use && !g_qcb[i].unlinked &&
            strcmp(g_qcb[i].name, name) == 0)
        {
            if (oflag & O_EXCL)
            {
                errno = EEXIST;
                return (mqd_t)-1;
            }
            g_qcb[i].refcount++;
            if (oflag & O_NONBLOCK)
            {
                g_qcb[i].oflag |= O_NONBLOCK;
            }
            return i;
        }
    }

    if (!(oflag & O_CREAT))
    {
        errno = ENOENT;
        return (mqd_t)-1;
    }

    if (attr == NULL || attr->mq_maxmsg <= 0 || attr->mq_msgsize <= 0)
    {
        errno = EINVAL;
        return (mqd_t)-1;
    }

    /* Allocate a free slot. */
    for (i = 0; i < MQ_MAX_OPEN; i++)
    {
        if (!g_qcb[i].in_use)
        {
            break;
        }
    }
    if (i >= MQ_MAX_OPEN)
    {
        errno = ENFILE;
        return (mqd_t)-1;
    }

    q = &g_qcb[i];
    memset(q, 0, sizeof(*q));

    q->buf = (unsigned char *)malloc((size_t)attr->mq_maxmsg * (size_t)attr->mq_msgsize);
    if (q->buf == NULL)
    {
        errno = ENOMEM;
        return (mqd_t)-1;
    }

    q->msg_len = (size_t *)malloc((size_t)attr->mq_maxmsg * sizeof(size_t));
    if (q->msg_len == NULL)
    {
        free(q->buf);
        q->buf = NULL;
        errno = ENOMEM;
        return (mqd_t)-1;
    }

    strncpy(q->name, name, sizeof(q->name) - 1);
    q->name[sizeof(q->name) - 1] = '\0';
    q->mq_maxmsg  = attr->mq_maxmsg;
    q->mq_msgsize = attr->mq_msgsize;
    q->oflag      = (oflag & O_NONBLOCK) ? O_NONBLOCK : 0;
    q->in_use     = 1;
    q->unlinked   = 0;
    q->refcount   = 1;
    q->head       = 0;
    q->tail       = 0;
    q->mq_curmsgs = 0;

    if (sem_init(&q->sem_msgs, 0, 0) != 0)
    {
        free(q->buf);
        free(q->msg_len);
        q->in_use = 0;
        errno = ENOSPC;
        return (mqd_t)-1;
    }

    if (sem_init(&q->sem_space, 0, (unsigned int)attr->mq_maxmsg) != 0)
    {
        sem_destroy(&q->sem_msgs);
        free(q->buf);
        free(q->msg_len);
        q->in_use = 0;
        errno = ENOSPC;
        return (mqd_t)-1;
    }

    if (pthread_mutex_init(&q->lock, NULL) != 0)
    {
        sem_destroy(&q->sem_msgs);
        sem_destroy(&q->sem_space);
        free(q->buf);
        free(q->msg_len);
        q->in_use = 0;
        errno = ENOMEM;
        return (mqd_t)-1;
    }

    return i;
}

/*--------------------------------------------------------------------*/
/* mq_close                                                           */
/*--------------------------------------------------------------------*/
int mq_close(mqd_t mqdes)
{
    mq_qcb_t *q;

    if (!mq_valid(mqdes))
    {
        errno = EBADF;
        return -1;
    }

    q = &g_qcb[mqdes];
    q->refcount--;
    if (q->refcount <= 0)
    {
        sem_destroy(&q->sem_msgs);
        sem_destroy(&q->sem_space);
        pthread_mutex_destroy(&q->lock);
        free(q->buf);
        free(q->msg_len);
        q->buf     = NULL;
        q->msg_len = NULL;
        q->in_use  = 0;
    }

    return 0;
}

/*--------------------------------------------------------------------*/
/* mq_unlink                                                          */
/*--------------------------------------------------------------------*/
int mq_unlink(const char *name)
{
    int i;

    if (name == NULL || name[0] != '/')
    {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < MQ_MAX_OPEN; i++)
    {
        if (g_qcb[i].in_use && !g_qcb[i].unlinked &&
            strcmp(g_qcb[i].name, name) == 0)
        {
            /* Remove the name; the queue stays alive until the last
             * mq_close() drops the reference. */
            g_qcb[i].unlinked = 1;
            return 0;
        }
    }

    errno = ENOENT;
    return -1;
}

/*--------------------------------------------------------------------*/
/* Internal send                                                      */
/*--------------------------------------------------------------------*/
static int mq_send_internal(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                            unsigned int msg_prio, const struct timespec *abs_timeout)
{
    mq_qcb_t *q;
    int       r;
    size_t    off;

    (void)msg_prio;

    if (!mq_valid(mqdes))
    {
        errno = EBADF;
        return -1;
    }
    q = &g_qcb[mqdes];

    if (msg_len > (size_t)q->mq_msgsize)
    {
        errno = EMSGSIZE;
        return -1;
    }

    if (abs_timeout != NULL)
    {
        r = sem_timedwait(&q->sem_space, abs_timeout);
    }
    else if (q->oflag & O_NONBLOCK)
    {
        r = sem_trywait(&q->sem_space);
    }
    else
    {
        r = sem_wait(&q->sem_space);
    }
    if (r != 0)
    {
        return -1;
    }

    pthread_mutex_lock(&q->lock);
    off = (size_t)q->tail * (size_t)q->mq_msgsize;
    memcpy(q->buf + off, msg_ptr, msg_len);
    q->msg_len[q->tail] = msg_len;
    q->tail = (q->tail + 1) % q->mq_maxmsg;
    q->mq_curmsgs++;
    pthread_mutex_unlock(&q->lock);

    sem_post(&q->sem_msgs);

    return 0;
}

/*--------------------------------------------------------------------*/
/* Internal receive                                                   */
/*--------------------------------------------------------------------*/
static ssize_t mq_receive_internal(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                                   unsigned int *msg_prio, const struct timespec *abs_timeout)
{
    mq_qcb_t *q;
    int       r;
    size_t    actual_len;
    size_t    off;

    if (!mq_valid(mqdes))
    {
        errno = EBADF;
        return -1;
    }
    q = &g_qcb[mqdes];

    if (msg_len < (size_t)q->mq_msgsize)
    {
        errno = EMSGSIZE;
        return -1;
    }

    if (abs_timeout != NULL)
    {
        r = sem_timedwait(&q->sem_msgs, abs_timeout);
    }
    else if (q->oflag & O_NONBLOCK)
    {
        r = sem_trywait(&q->sem_msgs);
    }
    else
    {
        r = sem_wait(&q->sem_msgs);
    }
    if (r != 0)
    {
        return -1;
    }

    pthread_mutex_lock(&q->lock);
    actual_len = q->msg_len[q->head];
    off = (size_t)q->head * (size_t)q->mq_msgsize;
    memcpy(msg_ptr, q->buf + off, actual_len);
    q->head = (q->head + 1) % q->mq_maxmsg;
    q->mq_curmsgs--;
    pthread_mutex_unlock(&q->lock);

    sem_post(&q->sem_space);

    if (msg_prio != NULL)
    {
        *msg_prio = 0;
    }

    return (ssize_t)actual_len;
}

/*--------------------------------------------------------------------*/
/* Public wrappers                                                    */
/*--------------------------------------------------------------------*/
int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio)
{
    return mq_send_internal(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}

int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned int msg_prio, const struct timespec *abs_timeout)
{
    return mq_send_internal(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio)
{
    return mq_receive_internal(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned int *msg_prio, const struct timespec *abs_timeout)
{
    return mq_receive_internal(mqdes, msg_ptr, msg_len, msg_prio, abs_timeout);
}

/*--------------------------------------------------------------------*/
/* mq_getattr / mq_setattr                                            */
/*--------------------------------------------------------------------*/
int mq_getattr(mqd_t mqdes, struct mq_attr *attr)
{
    mq_qcb_t *q;

    if (!mq_valid(mqdes))
    {
        errno = EBADF;
        return -1;
    }
    if (attr == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    q = &g_qcb[mqdes];
    pthread_mutex_lock(&q->lock);
    attr->mq_flags   = q->oflag;
    attr->mq_maxmsg  = q->mq_maxmsg;
    attr->mq_msgsize = q->mq_msgsize;
    attr->mq_curmsgs = q->mq_curmsgs;
    pthread_mutex_unlock(&q->lock);

    return 0;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr, struct mq_attr *oldattr)
{
    mq_qcb_t *q;

    if (!mq_valid(mqdes))
    {
        errno = EBADF;
        return -1;
    }
    if (newattr == NULL)
    {
        errno = EINVAL;
        return -1;
    }

    q = &g_qcb[mqdes];

    if (oldattr != NULL)
    {
        oldattr->mq_flags   = q->oflag;
        oldattr->mq_maxmsg  = q->mq_maxmsg;
        oldattr->mq_msgsize = q->mq_msgsize;
        oldattr->mq_curmsgs = q->mq_curmsgs;
    }

    if (newattr->mq_flags & O_NONBLOCK)
    {
        q->oflag |= O_NONBLOCK;
    }
    else
    {
        q->oflag &= ~O_NONBLOCK;
    }

    return 0;
}
