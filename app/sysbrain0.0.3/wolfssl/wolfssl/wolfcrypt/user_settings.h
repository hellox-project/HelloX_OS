/* user_settings.h
 *
 * Custom configuration for wolfSSL on HelloX OS (i386, 32-bit).
 * This is the FULL configuration — no size or memory constraints.
 */

#ifndef WOLFSSL_USER_SETTINGS_H
#define WOLFSSL_USER_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Architecture ---- */
#define SIZEOF_LONG            4
#define SIZEOF_LONG_LONG       8
#define WOLFSSL_GENERAL_ALIGNMENT 4

/* ---- Main ---- */
#define NO_MAIN_DRIVER

/* ---- CPU / Built-in Math ---- */
/* Use wolfSSL's single-precision integer math (sp_int) for performance.
 * Disable the old tfm/integer math to avoid conflicts. */
/* #define WOLFSSL_SP_INT */
/* #define WOLFSSL_SP_MATH */    /* DISABLED — conflicts with rsa.c's SP checks */
/* Use normal integer math (tfm.c) which is simpler to compile */
#define USE_FAST_MATH
#define TFM_TIMING_RESISTANT

/* ---- TLS Protocol ---- */
#define NO_OLD_TLS                    /* No SSLv3, TLS 1.0, TLS 1.1 */
#define HAVE_TLS_EXTENSIONS
#define HAVE_SNI                      /* Server Name Indication */
#define HAVE_SUPPORTED_CURVES
#define HAVE_FFDHE_2048              /* Finite-Field DHE 2048 */
#define HAVE_TLS_EXTRA
#define HAVE_EXTENDED_MASTER
#define HAVE_SESSION_TICKET          /* Session resumption */
/* TLS 1.3 requires HAVE_HKDF and additional code paths. Not needed for DeepSeek
 * which works perfectly with TLS 1.2. Keep it simple. */
/* #define WOLFSSL_TLS13 */          /* DISABLED — use TLS 1.2 only */
#define WC_RSA_PSS                   /* Required by tls.c even for TLS 1.2 */

/* ---- Ciphers (full set) ---- */
#define HAVE_AES_CBC
#define HAVE_AESGCM                   /* AES-GCM (what DeepSeek uses) */
#define HAVE_AES_DECRYPT
#define HAVE_AES_KEY_WRAP
#define HAVE_CHACHA
#define HAVE_POLY1305

/* ---- Hash Functions ---- */
#define NO_MD4
#define NO_MD2
#define NO_RC4
#define NO_DES
#define NO_RABBIT
#define NO_HC128

/* ---- Public Key Algorithms ---- */
#define HAVE_RSA
#define HAVE_ECC
#define HAVE_DH
#define ECC_SHAMIR
#define HAVE_ECC_DHE

/* ---- Certificate / ASN1 ---- */
#define HAVE_OCSP                      /* Online Certificate Status Protocol */
#define KEEP_OUR_CERT
#define KEEP_PEER_CERT

/* ---- Random ---- */
#define HAVE_HASHDRBG
#define NO_DEV_RANDOM                  /* No /dev/random on HelloX */
/* Provide our own wc_GenerateSeed implementation */
#define CUSTOM_RAND_GENERATE_SEED HxGenerateSeed

/* ---- Threading ---- */
#define WOLFSSL_THREADS
/* Single-threaded REPL application — disable condition variable support */
#define SINGLE_THREADED

/* ---- Debug ---- */
/* #define DEBUG_WOLFSSL */
/* #define WOLFSSL_DEBUG_TLS */
/* #define NO_ERROR_STRINGS */

/* ---- File System ---- */
/* HelloX has a complete file system for file I/O.
 * We override wolfSSL's file I/O via WOLFSSL_USER_FILESYSTEM,
 * providing XFILE/XFOPEN/... through our custom callbacks.
 */
#define WOLFSSL_USER_FILESYSTEM
/* Our XFILE/XFOPEN/... overrides are defined in wolfssl_platform.h */

/* Define NO_FILESYSTEM to skip directory listing and stat structures
 * (wc_ReadDirFirst/Next). File I/O still works via XFILE/XFOPEN. */
#define NO_FILESYSTEM

/* ---- Memory ---- */
/* Use HelloX heap directly (ptmalloc via _hx_malloc/_hx_free) */

/* ---- Misc ---- */
#define WOLFSSL_IGNORE_FILE_WARN
#define NO_WRITEV
#define WOLFSSL_NO_FENCE          /* HelloX freestanding clang lacks stdatomic.h */

/* I/O — We provide custom I/O callbacks (HxIORecv/HxIOSend) via wolfSSL_CTX_SetIORecv/IOSend.
 * This avoids needing <sys/socket.h> or BSD socket headers. */
#define WOLFSSL_USER_IO

/* snprintf — HelloX provides sprintf but not snprintf in user mode.
 * Use sprintf as fallback (safe enough for wolfSSL debug strings). */
#ifndef XSNPRINTF
#define XSNPRINTF(s, sz, fmt, ...)  sprintf((s), (fmt), ##__VA_ARGS__)
#endif
#ifndef XVSNPRINTF
#define XVSNPRINTF(s, sz, fmt, ap)  vsprintf((s), (fmt), (ap))
#endif

/* ---- Compatibility hacks for HelloX's minimal libc ---- */
/* wolfSSL usually includes <stdio.h>, <stdlib.h>, <string.h>, <time.h>.
 * These are all provided by HelloX's C library.
 */
#define XSNPRINTF(s, sz, fmt, ...)  \
    sprintf((s), (fmt), ##__VA_ARGS__)

/* HelloX has a complete file system.
 * We use custom file callbacks via XFILE/XFOPEN/... macros. */

/* Include other necessary headers that HelloX provides */
#ifndef STRING_USER
#include <string.h>
#endif

#ifndef STDIO_USER
#include <stdio.h>
#endif

#ifndef STDLIB_USER
#include <stdlib.h>
#endif

#include <time.h>
#include <stddef.h>

/* Include HelloX platform adaptation layer.
 * This must be included here (before wc_port.h) so that XFILE/XFOPEN/...
 * macros are available when WOLFSSL_USER_FILESYSTEM triggers their use. */
#ifndef WOLFSSL_PLATFORM_HX_H
#include <wolfssl_platform.h>
#endif

#ifdef __cplusplus
}
#endif

#endif /* WOLFSSL_USER_SETTINGS_H */
