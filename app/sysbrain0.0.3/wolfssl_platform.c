/* wolfssl_platform.c
 *
 * HelloX platform adaptation implementation for wolfSSL.
 * Implements file I/O, random seed, mutex, and transport I/O callbacks.
 */

/* Include our platform header first — it defines XFILE/etc. before wolfSSL
 * headers get pulled in via wc_port.h. */
#include <wolfssl_platform.h>

/* Now include wolfSSL headers — they'll see our XFILE/FILE overrides */
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/types.h>
#include <wolfssl/wolfcrypt/error-crypt.h>
#include <wolfssl/wolfcrypt/logging.h>
#include <wolfssl/wolfcrypt/random.h>
#include <wolfssl/ssl.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Include HelloX sockets headers.
 * Note: sockets.h defines recv/send as macros to lwip_recv/lwip_send
 * via #define recv(a,b,c,d) lwip_recv(a,b,c,d)
 * So we can call send()/recv() directly. */
#include <sockets/sockets.h>

/* ===================================================================
 * File I/O — via HelloX FS API
 * =================================================================== */

/* Map fopen-style mode string to HelloX access flags */
static DWORD HxModeToAccess(const char* mode)
{
    DWORD access = 0;
    if (!mode) return FILE_ACCESS_READ;

    if (strchr(mode, 'r')) {
        access |= FILE_ACCESS_READ;
    }
    if (strchr(mode, 'w') || strchr(mode, 'a') || strchr(mode, '+')) {
        access |= FILE_ACCESS_WRITE | FILE_ACCESS_CREATE;
    }
    /* Append mode also needs to create (handled by FILE_ACCESS_CREATE) */
    if (!access) access = FILE_ACCESS_READ;
    return access;
}

HANDLE HxFileOpen(const char* path, const char* mode)
{
    DWORD access = HxModeToAccess(mode);
    HANDLE hFile = CreateFile((LPSTR)path, access, 0, NULL);

    if (INVALID_HANDLE_VALUE == hFile) {
        return XBADFILE;
    }

    /* For append mode, seek to end */
    if (mode && strchr(mode, 'a')) {
        DWORD fileSize = GetFileSize(hFile, NULL);
        DWORD distLow = fileSize, distHigh = 0;
        SetFilePointer(hFile, &distLow, &distHigh, FILE_FROM_BEGIN);
    }

    return hFile;
}

void HxFileClose(HANDLE f)
{
    if (f && f != INVALID_HANDLE_VALUE) {
        CloseFile(f);
    }
}

size_t HxFileRead(void* buf, size_t size, size_t count, HANDLE f)
{
    DWORD readBytes;
    DWORD totalSize = (DWORD)(size * count);

    if (!f || f == INVALID_HANDLE_VALUE || !buf || totalSize == 0)
        return 0;

    if (!ReadFile(f, totalSize, buf, &readBytes))
        return 0;

    return readBytes / size;
}

size_t HxFileWrite(const void* buf, size_t size, size_t count, HANDLE f)
{
    DWORD written;
    DWORD totalSize = (DWORD)(size * count);

    if (!f || f == INVALID_HANDLE_VALUE || !buf || totalSize == 0)
        return 0;

    if (!WriteFile(f, totalSize, (LPVOID)buf, &written))
        return 0;

    return written / size;
}

int HxFileSeek(HANDLE f, long offset, int whence)
{
    DWORD distLow, distHigh;
    DWORD moveFlags;

    if (!f || f == INVALID_HANDLE_VALUE)
        return -1;

    switch (whence) {
    case SEEK_SET:
        moveFlags = FILE_FROM_BEGIN;
        break;
    case SEEK_CUR:
        moveFlags = FILE_FROM_CURRENT;
        break;
    case SEEK_END:
        moveFlags = FILE_FROM_BEGIN;  /* No FILE_FROM_END in HelloX; use size + BEGIN */
        /* Get file size, then set pointer there */
        {
            DWORD sizeLow = GetFileSize(f, &distHigh);
            distLow = sizeLow + (DWORD)(offset & 0xFFFFFFFF);
            distHigh += (DWORD)(((long long)offset >> 32) & 0xFFFFFFFF);
        }
        return SetFilePointer(f, &distLow, &distHigh, moveFlags) ? 0 : -1;
        break;
    default:
        return -1;
    }

    distLow  = (DWORD)(offset & 0xFFFFFFFF);
    distHigh = (DWORD)(((long long)offset >> 32) & 0xFFFFFFFF);

    if (!SetFilePointer(f, &distLow, &distHigh, moveFlags))
        return -1;

    return 0;
}

int HxFileEof(HANDLE f)
{
    DWORD curLow, curHigh, sizeLow, sizeHigh;

    if (!f || f == INVALID_HANDLE_VALUE)
        return 1;

    /* Get current position */
    curLow = 0; curHigh = 0;
    SetFilePointer(f, &curLow, &curHigh, FILE_FROM_CURRENT);

    /* Get file size */
    sizeHigh = 0;
    sizeLow = GetFileSize(f, &sizeHigh);

    /* Compare low 32 bits (good enough) */
    if (curLow >= sizeLow && curHigh >= sizeHigh)
        return 1;
    return 0;
}

void HxFileRewind(HANDLE f)
{
    if (f && f != INVALID_HANDLE_VALUE) {
        DWORD zero = 0;
        SetFilePointer(f, &zero, &zero, FILE_FROM_BEGIN);
    }
}

long HxFileTell(HANDLE f)
{
    DWORD curLow = 0, curHigh = 0;
    if (!f || f == INVALID_HANDLE_VALUE)
        return -1;
    SetFilePointer(f, &curLow, &curHigh, FILE_FROM_CURRENT);
    return (long)curLow;
}

/* ===================================================================
 * Random seed — custom GenerateSeed function
 * Called by wolfSSL's random.c via CUSTOM_RAND_GENERATE_SEED macro.
 * Signature: int HxGenerateSeed(byte* output, word32 sz)
 * =================================================================== */

int HxGenerateSeed(byte* output, word32 sz)
{
    unsigned long hi, lo;
    word32 i;

    if (output == NULL || sz == 0) {
        return BAD_FUNC_ARG;
    }

    lo = GetClockCounter(&hi);

    for (i = 0; i < sz; i++) {
        output[i] = (byte)(lo & 0xFF);
        /* Simple LFSR-style mixing using two 32-bit state variables */
        lo = (lo >> 1) ^ (hi << 31) ^ ((lo & 1) ? 0xEDB88320u : 0);
        hi = (hi >> 1) | ((lo & 1) << 31);
        /* Stir in fresh clock entropy every 8 bytes */
        if ((i & 0x07) == 0) {
            lo ^= GetClockCounter(NULL);
        }
    }
    return 0;
}

/* ===================================================================
 * I/O callbacks for wolfSSL transport layer
 * =================================================================== */

int HxIORecv(WOLFSSL* ssl, char* buf, int sz, void* ctx)
{
    int fd;
    int ret;

    (void)ctx;
    fd = (int)(size_t)wolfSSL_get_fd(ssl);
    if (fd < 0) return -1;

    /* recv() is macro-expanded to lwip_recv() via sockets.h */
    ret = recv(fd, buf, sz, 0);
    if (ret < 0) {
        return WOLFSSL_CBIO_ERR_GENERAL;    /* -1: real error */
    }
    if (ret == 0) {
        return WOLFSSL_CBIO_ERR_CONN_CLOSE; /* -5: connection gracefully closed */
    }
    return ret;
}

int HxIOSend(WOLFSSL* ssl, char* buf, int sz, void* ctx)
{
    int fd;
    int ret;

    (void)ctx;
    fd = (int)(size_t)wolfSSL_get_fd(ssl);
    if (fd < 0) return -1;

    /* send() is macro-expanded to lwip_send() via sockets.h */
    ret = send(fd, buf, sz, 0);
    //printf("[HxIOSend] fd=%d buf=0x%08x sz=%d ret=%d\r\n", fd, (unsigned int)buf, sz, ret);
    if (ret < 0) {
        printf("[%s]ret with error[%d]\r\n", __func__,
            ret);
        return -1;
    }
    return ret;
}

/* ===================================================================
 * Mutex helpers
 * =================================================================== */

#if defined(WOLFSSL_THREADS) && !defined(SINGLE_THREADED)

int wolfSSL_MutexInit(wolfSSL_Mutex* m)
{
    if (!m) return BAD_FUNC_ARG;
    *m = CreateMutex();
    if (!*m) return MEMORY_E;
    return 0;
}

int wolfSSL_MutexLock(wolfSSL_Mutex* m)
{
    if (!m || !*m) return BAD_FUNC_ARG;
    WaitForThisObject(*m);
    return 0;
}

int wolfSSL_MutexUnlock(wolfSSL_Mutex* m)
{
    if (!m || !*m) return BAD_FUNC_ARG;
    ReleaseMutex(*m);
    return 0;
}

int wolfSSL_MutexFree(wolfSSL_Mutex* m)
{
    if (!m || !*m) return BAD_FUNC_ARG;
    DestroyMutex(*m);
    *m = 0;
    return 0;
}

#endif /* WOLFSSL_THREADS && !SINGLE_THREADED */

/* ===================================================================
 * wolfSSL logging (stub)
 * =================================================================== */

#if defined(DEBUG_WOLFSSL)
static void HxLoggingCallback(const int logLevel, const char* const message)
{
    (void)logLevel;
    PrintLine((LPSTR)message);
}
#endif
