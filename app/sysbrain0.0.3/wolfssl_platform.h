/* wolfssl_platform.h
 *
 * HelloX platform adaptation for wolfSSL.
 * Provides macros that map wolfSSL's abstracted functions to HelloX APIs.
 *
 * IMPORTANT: This header must be included BEFORE wolfssl headers in any
 * translation unit that uses wolfSSL. We achieve this by including it via
 * user_settings.h or requiring users to include it first.
 */

#ifndef WOLFSSL_PLATFORM_HX_H
#define WOLFSSL_PLATFORM_HX_H

#include <hellox.h>       /* HelloX core API */

#ifdef __cplusplus
extern "C" {
#endif

/* ===================================================================
 * Memory management — via HelloX ptmalloc (stdlib.h wraps them)
 * =================================================================== */
#include <stdlib.h>   /* provides malloc/free macros -> _hx_malloc/_hx_free */

/* ===================================================================
 * String / memory — HelloX provides these in string.h
 * =================================================================== */
#include <string.h>

/* ===================================================================
 * File I/O — via HelloX File System API
 * Define these BEFORE wc_port.h sees them (thanks to WOLFSSL_USER_FILESYSTEM)
 * =================================================================== */

#ifndef XFILE
#define XFILE               HANDLE
#endif
#ifndef XBADFILE
#define XBADFILE            INVALID_HANDLE_VALUE
#endif

#ifndef XFOPEN
#define XFOPEN(p, m)        HxFileOpen((p), (m))
#endif
#ifndef XFCLOSE
#define XFCLOSE(f)          HxFileClose(f)
#endif
#ifndef XFREAD
#define XFREAD(b, s, c, f)  HxFileRead((b), (s), (c), (f))
#endif
#ifndef XFWRITE
#define XFWRITE(b, s, c, f) HxFileWrite((b), (s), (c), (f))
#endif
#ifndef XFSEEK
#define XFSEEK(f, o, w)     HxFileSeek((f), (o), (w))
#endif
#ifndef XFEOF
#define XFEOF(f)            HxFileEof(f)
#endif
#ifndef XREWIND
#define XREWIND(f)          HxFileRewind(f)
#endif
#ifndef XFTELL
#define XFTELL(f)           HxFileTell(f)
#endif

#ifndef XSEEK_SET
#define XSEEK_SET           SEEK_SET
#endif
#ifndef XSEEK_CUR
#define XSEEK_CUR           SEEK_CUR
#endif
#ifndef XSEEK_END
#define XSEEK_END           SEEK_END
#endif

/* ===================================================================
 * Time — HelloX provides time_t in <time.h>
 * =================================================================== */
#include <time.h>
#ifndef XTIME
#define XTIME(t)            time((t))
#endif

/* ===================================================================
 * Mutex — via HelloX mutex API
 * =================================================================== */
#define WOLFSSL_MUTEX_TYPE          HANDLE
#define WOLFSSL_MUTEX_GLOBAL(x)     x
#if !defined(SINGLE_THREADED)
    #define WOLFSSL_MUTEX_INITIALIZER    NULL
#endif

/* ===================================================================
 * Random seed — custom, provided in wolfssl_platform.c
 * =================================================================== */

/* ===================================================================
 * Function prototypes for platform implementation
 * =================================================================== */

/* File I/O functions */
HANDLE  HxFileOpen   (const char* path, const char* mode);
void    HxFileClose  (HANDLE f);
size_t  HxFileRead   (void* buf, size_t size, size_t count, HANDLE f);
size_t  HxFileWrite  (const void* buf, size_t size, size_t count, HANDLE f);
int     HxFileSeek   (HANDLE f, long offset, int whence);
int     HxFileEof    (HANDLE f);
void    HxFileRewind (HANDLE f);
long    HxFileTell   (HANDLE f);

/* I/O callbacks for wolfSSL's transport layer */
/* NOTE: I/O callback function prototypes use WOLFSSL type from wolfssl/ssl.h,
 * so they cannot be declared here (before ssl.h is included).
 * They are instead defined in wolfssl_platform.c with local prototypes. */

/* Custom GenerateSeed (used when CUSTOM_RAND_GENERATE_SEED is defined) */

#ifdef __cplusplus
}
#endif

#endif /* WOLFSSL_PLATFORM_HX_H */
