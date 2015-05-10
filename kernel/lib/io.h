
#ifndef	__IO_H__
#define	__IO_H__

/*
 * Attributes of files as returned by _findfirst et al.
 */
#define	_A_NORMAL	0x00000000
#define	_A_RDONLY	0x00000001
#define	_A_HIDDEN	0x00000002
#define	_A_SYSTEM	0x00000004
#define	_A_VOLID	0x00000008
#define	_A_SUBDIR	0x00000010
#define	_A_ARCH		0x00000020


#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */

#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */


#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */
#define _O_WTEXT        0x10000 /* file mode is UTF16 (translated) */
#define _O_U16TEXT      0x20000 /* file mode is UTF16 no BOM (translated) */
#define _O_U8TEXT       0x40000 /* file mode is UTF8  no BOM (translated) */

/* macro to translate the C 2.0 name used to force binary mode for files */
#define _O_RAW  _O_BINARY

#if     !__STDC__ || defined(_POSIX_)
/* Non-ANSI names for compatibility */
#define O_RDONLY        _O_RDONLY
#define O_WRONLY        _O_WRONLY
#define O_RDWR          _O_RDWR
#define O_APPEND        _O_APPEND
#define O_CREAT         _O_CREAT
#define O_TRUNC         _O_TRUNC
#define O_EXCL          _O_EXCL
#define O_TEXT          _O_TEXT
#define O_BINARY        _O_BINARY
#define O_RAW           _O_BINARY
#define O_TEMPORARY     _O_TEMPORARY
#define O_NOINHERIT     _O_NOINHERIT
#define O_SEQUENTIAL    _O_SEQUENTIAL
#define O_RANDOM        _O_RANDOM
#endif  /* __STDC__ */


typedef int intptr_t;
typedef	unsigned long _fsize_t;


/*
 * The maximum length of a file name. You should use GetVolumeInformation
 * instead of this constant. But hey, this works.
 * Also defined in stdio.h.
 */
#ifndef FILENAME_MAX
#define	FILENAME_MAX	(260)
#endif

#ifdef	__cplusplus
extern "C" {
#endif

 int     chdir (const char*);
 char*   getcwd (char*, int);
 int     mkdir (const char*);
 char*   mktemp (char*);
 int     rmdir (const char*);
 int     chmod (const char*, int);

#ifdef	__cplusplus
}
#endif

/* TODO: Maximum number of open handles has not been tested, I just set
 * it the same as FOPEN_MAX. */
#define	HANDLE_MAX	FOPEN_MAX

/* Some defines for _access nAccessMode (MS doesn't define them, but
 * it doesn't seem to hurt to add them). */
#define	F_OK	0	/* Check for file existence */
/* Well maybe it does hurt.  On newer versions of MSVCRT, an access mode
   of 1 causes invalid parameter error. */   
#define	X_OK	1	/* MS access() doesn't check for execute permission. */
#define	W_OK	2	/* Check for write permission */
#define	R_OK	4	/* Check for read permission */


#ifdef	__cplusplus
extern "C" {
#endif

 int  	 remove (const char*);
 int  	 rename (const char*, const char*);
 int     access (const char*, int);
 int     chsize (int, long );
 int     close (int);
 int     creat (const char*, int);
 int     dup (int);
 int     dup2 (int, int);
 int     eof (int);
 long    filelength (int);
 int     isatty (int);
 long    lseek (int, long, int);
 int     open (const char*, int, ...);
 int     read (int, void*, unsigned int);
 int     setmode (int, int);
 int     sopen (const char*, int, int, ...);
 long    tell (int);
 int     umask (int);
 int     unlink (const char*);
 int     write (int, const void*, unsigned int);

 typedef void FILE;

 FILE*   fopen(const char *, const char *);
 int     fclose(FILE *); 
 size_t  fwrite(const void *, size_t, size_t, FILE *);
 size_t  fread(void *, size_t, size_t, FILE*); 
 int     fseek(FILE *, long, int);
 long    ftell(FILE *);

 //stdin,stdout,stderr definition.
#define stdin  NULL
#define stdout NULL
#define stderr NULL

#ifdef	__cplusplus
}
#endif

#endif	//__IO_H__
