/* sys/statvfs.h compatibility shim: provides a minimal struct statvfs.
 * HelloX has no real statvfs(); the OSAL filesystem impl returns
 * NOT_IMPLEMENTED, so only the type is needed to compile. */
#ifndef __SYS_STATVFS_H__
#define __SYS_STATVFS_H__

#include <sys/types.h>

struct statvfs
{
    unsigned long f_bsize;
    unsigned long f_frsize;
    unsigned long f_blocks;
    unsigned long f_bfree;
    unsigned long f_bavail;
    unsigned long f_files;
    unsigned long f_ffree;
    unsigned long f_favail;
    unsigned long f_fsid;
    unsigned long f_flag;
    unsigned long f_namemax;
};

int statvfs(const char *path, struct statvfs *buf);

#endif /* __SYS_STATVFS_H__ */
