#ifndef __TYPE__DEF__H__
#define __TYPE__DEF__H__

#if defined (__CC_ARM)
typedef char  S8;
typedef short	S16;
typedef int     S32;
typedef long long  S64;

typedef char   __s8;
typedef short  __s16;
typedef int   	  __s32;
typedef long long    __s64;


typedef unsigned char  U8;
typedef unsigned short	U16;
typedef unsigned int     U32;
typedef unsigned long  long   U64;


/*typedef unsigned char  u8;
typedef unsigned short	u16;
typedef unsigned int   u32;*/
typedef unsigned long long   u64; 

typedef unsigned char   __u8;
typedef unsigned short  __u16;
typedef unsigned int   	 __u32;
typedef unsigned long long    __u64; 

typedef unsigned char 		uint8_t;
typedef unsigned short 		uint16_t;
typedef unsigned int     	uint32_t; 

typedef unsigned char 		int8_t;
typedef unsigned short 		int16_t;
typedef unsigned int     	int32_t;

typedef unsigned short 		__le16;
typedef unsigned int 		__le32;
typedef unsigned long long	__le64;

typedef unsigned short 		__be16;
typedef unsigned int		__be32;

#endif

#define cpu_to_le16(v16) (v16)
#define cpu_to_le32(v32) (v32)
#define cpu_to_le64(v64) (v64)
#define le16_to_cpu(v16) (v16)
#define le32_to_cpu(v32) (v32)
#define le64_to_cpu(v64) (v64)

/*error base*/

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */


#define	EWOULDBLOCK	EAGAIN	/* Operation would block */

#if 0
#define	EINPROGRESS	36	/* Operation now in progress */
#define	EALREADY	37	/* Operation already in progress */
#define	ENOTSOCK	38	/* Socket operation on non-socket */
#define	EDESTADDRREQ	39	/* Destination address required */
#define	EMSGSIZE	40	/* Message too long */
#define	EPROTOTYPE	41	/* Protocol wrong type for socket */
#define	ENOPROTOOPT	42	/* Protocol not available */
#define	EPROTONOSUPPORT	43	/* Protocol not supported */
#define	ESOCKTNOSUPPORT	44	/* Socket type not supported */
#define	EOPNOTSUPP	45	/* Op not supported on transport endpoint */
#define	EPFNOSUPPORT	46	/* Protocol family not supported */
#define	EAFNOSUPPORT	47	/* Address family not supported by protocol */
#define	EADDRINUSE	48	/* Address already in use */
#define	EADDRNOTAVAIL	49	/* Cannot assign requested address */
#define	ENETDOWN	50	/* Network is down */
#define	ENETUNREACH	51	/* Network is unreachable */
#define	ENETRESET	52	/* Net dropped connection because of reset */
#define	ECONNABORTED	53	/* Software caused connection abort */
#define	ECONNRESET	54	/* Connection reset by peer */
#define	ENOBUFS		55	/* No buffer space available */
#define	EISCONN		56	/* Transport endpoint is already connected */
#define	ENOTCONN	57	/* Transport endpoint is not connected */
#define	ESHUTDOWN	58	/* No send after transport endpoint shutdown */
#define	ETOOMANYREFS	59	/* Too many references: cannot splice */
#define	ETIMEDOUT	60	/* Connection timed out */
#define	ECONNREFUSED	61	/* Connection refused */
#define	ELOOP		62	/* Too many symbolic links encountered */
#define	ENAMETOOLONG	63	/* File name too long */
#define	EHOSTDOWN	64	/* Host is down */
#define	EHOSTUNREACH	65	/* No route to host */
#define	ENOTEMPTY	66	/* Directory not empty */
#define EPROCLIM        67      /* SUNOS: Too many processes */

#define	EUSERS		68	/* Too many users */
#define	EDQUOT		69	/* Quota exceeded */
#define	ESTALE		70	/* Stale NFS file handle */
#define	EREMOTE		71	/* Object is remote */
#define	ENOSTR		72	/* Device not a stream */
#define	ETIME		73	/* Timer expired */
#define	ENOSR		74	/* Out of streams resources */
#define	ENOMSG		75	/* No message of desired type */
#define	EBADMSG		76	/* Not a data message */
#define	EIDRM		77	/* Identifier removed */
#define	EDEADLK		78	/* Resource deadlock would occur */
#define	ENOLCK		79	/* No record locks available */
#define	ENONET		80	/* Machine is not on the network */


#define ERREMOTE        81      /* SunOS: Too many lvls of remote in path */

#define	ENOLINK		82	/* Link has been severed */
#define	EADV		83	/* Advertise error */
#define	ESRMNT		84	/* Srmount error */
#define	ECOMM		85      /* Communication error on send */
#define	EPROTO		86	/* Protocol error */
#define	EMULTIHOP	87	/* Multihop attempted */
#define	EDOTDOT		88	/* RFS specific error */
#define	EREMCHG		89	/* Remote address changed */
#define	ENOSYS		90	/* Function not implemented */

/* The rest have no SunOS equivalent. */
#define	ESTRPIPE	91	/* Streams pipe error */
#define	EOVERFLOW	92	/* Value too large for defined data type */
#define	EBADFD		93	/* File descriptor in bad state */
#define	ECHRNG		94	/* Channel number out of range */
#define	EL2NSYNC	95	/* Level 2 not synchronized */
#define	EL3HLT		96	/* Level 3 halted */
#define	EL3RST		97	/* Level 3 reset */
#define	ELNRNG		98	/* Link number out of range */
#define	EUNATCH		99	/* Protocol driver not attached */
#define	ENOCSI		100	/* No CSI structure available */
#define	EL2HLT		101	/* Level 2 halted */
#define	EBADE		102	/* Invalid exchange */
#define	EBADR		103	/* Invalid request descriptor */
#define	EXFULL		104	/* Exchange full */
#define	ENOANO		105	/* No anode */
#define	EBADRQC		106	/* Invalid request code */
#define	EBADSLT		107	/* Invalid slot */
#define	EDEADLOCK	108	/* File locking deadlock error */
#define	EBFONT		109	/* Bad font file format */
#define	ELIBEXEC	110	/* Cannot exec a shared library directly */
#define	ENODATA		111	/* No data available */
#define	ELIBBAD		112	/* Accessing a corrupted shared library */
#define	ENOPKG		113	/* Package not installed */
#define	ELIBACC		114	/* Can not access a needed shared library */
#define	ENOTUNIQ	115	/* Name not unique on network */
#define	ERESTART	116	/* Interrupted syscall should be restarted */
#endif

#define	EUCLEAN		117	/* Structure needs cleaning */
#define	ENOTNAM		118	/* Not a XENIX named type file */
#define	ENAVAIL		119	/* No XENIX semaphores available */
#define	EISNAM		120	/* Is a named type file */
#define	EREMOTEIO	121	/* Remote I/O error */

#if !defined(__CFG_NET_IPv4) //The following constants already defined in lwip headers.
#define	EILSEQ		36	/* Illegal byte sequence */
#define	ELIBMAX		123	/* Atmpt to link in too many shared libs */
#define	ELIBSCN		124	/* .lib section in a.out corrupted */
#define	ENOBUFS		55	/* No buffer space available */
#define	ETIME		73	/* Timer expired */

#define	ENOMEDIUM	125	/* No medium found */
#define	EMEDIUMTYPE	126	/* Wrong medium type */
#endif //__CFG_NET_IPv4

#define	ECANCELED	127	/* Operation Cancelled */
#define	ENOKEY		128	/* Required key not available */
#define	EKEYEXPIRED	129	/* Key has expired */
#define	EKEYREVOKED	130	/* Key has been revoked */
#define	EKEYREJECTED	131	/* Key was rejected by service */
//#define	ETIMEDOUT	60	/* Connection timed out */
//#define	ENETUNREACH	51	/* Network is unreachable */

/* for robust mutexes */
#define	EOWNERDEAD	132	/* Owner died */
#define	ENOTRECOVERABLE	133	/* State not recoverable */

#define	ERFKILL		134	/* Operation not possible due to RF-kill */

#define false FALSE
#define true  TRUE

#endif
