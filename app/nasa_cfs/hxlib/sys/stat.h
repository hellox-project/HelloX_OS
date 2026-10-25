/***
*sys/stat.h - defines structure used by stat() and fstat()
*
*       Copyright (c) 1985-1997, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file defines the structure used by the _stat() and _fstat()
*       routines.
*       [System V]
*
*       [Public]
*
****/

#include "time.h"
#if _MSC_VER > 1000
#pragma once
#endif  /* _MSC_VER > 1000 */

#ifndef _INC_STAT
#define _INC_STAT

#ifdef _MSC_VER
#pragma pack(push,8)
#endif  /* _MSC_VER */

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "types.h"

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

/* define structure for returning status information */

#ifndef _STAT_DEFINED

struct _stat {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        _off_t st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

/* Non-ANSI names for compatibility */

struct stat {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        _off_t st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };

#if _INTEGRAL_MAX_BITS >= 64   
struct _stati64 {
        _dev_t st_dev;
        _ino_t st_ino;
        unsigned short st_mode;
        short st_nlink;
        short st_uid;
        short st_gid;
        _dev_t st_rdev;
        long long st_size;
        time_t st_atime;
        time_t st_mtime;
        time_t st_ctime;
        };
#endif  /* _INTEGRAL_MAX_BITS >= 64    */

#define _STAT_DEFINED
#endif  /* _STAT_DEFINED */

//Check if the file is a dir.
//#define S_ISDIR(mode) (mode)

#define _S_IFMT         0170000         /* file type mask */
#define _S_IFDIR        0040000         /* directory */
#define _S_IFCHR        0020000         /* character special */
#define _S_IFIFO        0010000         /* pipe */
#define _S_IFREG        0100000         /* regular */
#define _S_IREAD        0000400         /* read permission, owner */
#define _S_IWRITE       0000200         /* write permission, owner */
#define _S_IEXEC        0000100         /* execute/search permission, owner */

/* Flags definitions. */
#define	S_ISUID	0004000			/* set user id on execution */
#define	S_ISGID	0002000			/* set group id on execution */
#ifndef _POSIX_SOURCE
#define	S_ISTXT	0001000			/* sticky bit */
#endif

#define	S_IRWXU	0000700			/* RWX mask for owner */
#define	S_IRUSR	0000400			/* R for owner */
#define	S_IWUSR	0000200			/* W for owner */
#define	S_IXUSR	0000100			/* X for owner */

#ifndef _POSIX_SOURCE
#define	S_IREAD		S_IRUSR
#define	S_IWRITE	S_IWUSR
#define	S_IEXEC		S_IXUSR
#endif

#define	S_IRWXG	0000070			/* RWX mask for group */
#define	S_IRGRP	0000040			/* R for group */
#define	S_IWGRP	0000020			/* W for group */
#define	S_IXGRP	0000010			/* X for group */

#define	S_IRWXO	0000007			/* RWX mask for other */
#define	S_IROTH	0000004			/* R for other */
#define	S_IWOTH	0000002			/* W for other */
#define	S_IXOTH	0000001			/* X for other */

/* All permision bits. */
#define ALLPERMS (S_IRWXU | S_IRWXG | S_IRWXO)

#ifndef _POSIX_SOURCE
#define	S_IFMT	 0170000		/* type of file mask */
#define	S_IFIFO	 0010000		/* named pipe (fifo) */
#define	S_IFCHR	 0020000		/* character special */
#define	S_IFDIR	 0040000		/* directory */
#define	S_IFBLK	 0060000		/* block special */
#define	S_IFREG	 0100000		/* regular */
#define	S_IFLNK	 0120000		/* symbolic link */
#define	S_IFSOCK 0140000		/* socket */
#define	S_IFWHT  0160000		/* whiteout */
#define	S_ISVTX	 0001000		/* save swapped text even after use */
#endif

#define	S_ISDIR(m)	((m & 0170000) == 0040000)	/* directory */
#define	S_ISCHR(m)	((m & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	((m & 0170000) == 0060000)	/* block special */
#define	S_ISREG(m)	((m & 0170000) == 0100000)	/* regular file */
#define	S_ISFIFO(m)	((m & 0170000) == 0010000 || \
			 (m & 0170000) == 0140000)	/* fifo or socket */
#ifndef _POSIX_SOURCE
#define	S_ISLNK(m)	((m & 0170000) == 0120000)	/* symbolic link */
#define	S_ISSOCK(m)	((m & 0170000) == 0010000 || \
			 (m & 0170000) == 0140000)	/* fifo or socket */
#define	S_ISWHT(m)	((m & 0170000) == 0160000)	/* whiteout */
#endif

/* Non-ANSI names for compatibility */

//#define S_IFMT   _S_IFMT
//#define S_IFDIR  _S_IFDIR
//#define S_IFCHR  _S_IFCHR
//#define S_IFREG  _S_IFREG
//#define S_IREAD  _S_IREAD
//#define S_IWRITE _S_IWRITE
//#define S_IEXEC  _S_IEXEC

int __CDECL fstat(int, struct stat *);
int __CDECL stat(const char *, struct stat *);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#ifdef _MSC_VER
#pragma pack(pop)
#endif  /* _MSC_VER */

#endif  /* _INC_STAT */
