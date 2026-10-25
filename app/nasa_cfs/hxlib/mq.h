/***********************************************************************/
/*    Module Name   : mq.h                                             */
/*    Module Funciton:                                                  */
/*       POSIX message queue (mqueue) header for HelloX.               */
/*       Implemented on top of the existing hxlib primitives           */
/*       (sem_t + pthread_mutex_t), no kernel change required.         */
/*    Notes:                                                           */
/*       - mqd_t is an integer index into an internal queue table      */
/*         (so (mqd_t)-1 is the invalid-descriptor sentinel, matching  */
/*         the POSIX/Linux convention used by cFS OSAL).               */
/*       - Messages are FIFO; priority is accepted but not used for    */
/*         ordering (sufficient for cFS, which always sends prio=1     */
/*         and receives with a NULL priority pointer).                 */
/***********************************************************************/

#ifndef __MQ_H__
#define __MQ_H__

#include <stddef.h>      /* size_t */
#include <time.h>        /* struct timespec */
#include <sys/types.h>   /* ssize_t */
#include <fcntl.h>       /* O_CREAT / O_RDWR / O_NONBLOCK / O_EXCL */

#ifdef __cplusplus
extern "C" {
#endif

/* Message queue descriptor type (integer index into internal table). */
typedef int mqd_t;

/* Message queue attributes. */
struct mq_attr {
    long mq_flags;      /* 0 or O_NONBLOCK (read/write) */
    long mq_maxmsg;     /* maximum number of messages (read only after open) */
    long mq_msgsize;    /* maximum message size in bytes (read only after open) */
    long mq_curmsgs;    /* number of messages currently queued (read only) */
};

#define MQ_PRIO_MAX  32768   /* maximum message priority (not used for ordering) */
#define MQ_NAME_MAX  255     /* maximum name length, excluding leading '/' */

/* Open (and optionally create) a message queue. */
mqd_t   mq_open(const char *name, int oflag, ...);

/* Close a message queue descriptor. */
int     mq_close(mqd_t mqdes);

/* Remove a message queue name from the namespace. */
int     mq_unlink(const char *name);

/* Send a message (blocking). */
int     mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                unsigned int msg_prio);

/* Send a message with an absolute timeout. */
int     mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                     unsigned int msg_prio, const struct timespec *abs_timeout);

/* Receive a message (blocking). Returns message length, or -1 on error. */
ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned int *msg_prio);

/* Receive a message with an absolute timeout. */
ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned int *msg_prio, const struct timespec *abs_timeout);

/* Query queue attributes. */
int     mq_getattr(mqd_t mqdes, struct mq_attr *attr);

/* Set queue attributes (only the O_NONBLOCK flag is settable). */
int     mq_setattr(mqd_t mqdes, const struct mq_attr *newattr,
                   struct mq_attr *oldattr);

#ifdef __cplusplus
}
#endif

#endif  /* __MQ_H__ */
