#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# AC_PREREQ([2.0])
AC_INIT([HelloX], [1.78], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([-Wall -Wno-portability foreign subdir-objects])


AC_SUBST(os, "HelloX")
AC_SUBST(hx_arch, $hx_arch)
AC_SUBST(hx_drivers, $hx_drivers)
AC_SUBST(cflags, $cflags)

##AM_CPPFLAGS =-nostdlib -m32  -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/config  -I$(top_srcdir)/kernel/lib -I$(top_srcdir)/kernel/lib/sys -D_TIME_T_DEFINED -D_M_IX86
#HX_CFLAGS = -nostdlib -m32 -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/lib -I$(top_srcdir)/kernel/lib/sys
#AC_SUBST(HX_FLAGS, "-nostdlib -m32 -I$(top_srcdir)/kernel/config -I$(top_srcdir)/kernel -I$(top_srcdir)/kernel/include -I$(top_srcdir)/kernel/lib -I$(top_srcdir)/kernel/lib/sys");
#AC_DEFINE(_GCC_, [true], "GCC")

AC_CONFIG_SRCDIR([gui/CTRL/bmpbtn.cpp])
AC_CONFIG_HEADERS([kernel/config.h])

AC_USE_SYSTEM_EXTENSIONS


# AUTOMAKE_OPTIONS([subdir-objects])


# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_AS
AC_PROG_LN_S
AC_PROG_LIBTOOL

AC_USE_SYSTEM_EXTENSIONS

AM_CONDITIONAL([COMPILE_TIME_RELOC_CHECKS],[AC_COMPILE_IFELSE], [true])





# Checks for libraries.

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h limits.h locale.h malloc.h memory.h stddef.h stdint.h stdlib.h string.h sys/param.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_CHECK_MEMBERS([struct stat.st_rdev])
AC_TYPE_UID_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_FUNC_REALLOC
AC_FUNC_STRERROR_R
AC_FUNC_STRNLEN
AC_CHECK_FUNCS([dup2 floor getcwd getpagesize gettimeofday inet_ntoa memchr memmove memset mkdir modf munmap rmdir setlocale strchr strrchr strstr strtol strtoumax uname])

				#kernel/jvm/Makefile
                 #kernel/jvm/arch/Makefile
                 #kernel/jvm/interp/Makefile
                 #kernel/jvm/interp/engine/Makefile
                 #kernel/jvm/lib/Makefile
                 #kernel/jvm/lib/gnu/Makefile
                 #kernel/jvm/lib/gnu/classpath/Makefile
                 #kernel/jvm/lib/jamvm/Makefile
                 #kernel/jvm/lib/jamvm/java/Makefile
                 #kernel/jvm/lib/jamvm/java/lang/Makefile
                 #kernel/jvm/lib/java/Makefile
                 #kernel/jvm/lib/java/lang/Makefile
                 #kernel/jvm/lib/java/lang/reflect/Makefile
                 #kernel/jvm/lib/java/security/Makefile
                 #kernel/jvm/lib/sun/Makefile
                 #kernel/jvm/lib/sun/reflect/Makefile
                 #kernel/jvm/lib/sun/reflect/annotation/Makefile
                 #kernel/jvm/os/Makefile
                 #kernel/jvm/os/bsd/Makefile
                 #kernel/jvm/os/bsd/arm/Makefile
                 #kernel/jvm/os/bsd/i386/Makefile
                 #kernel/jvm/os/bsd/powerpc/Makefile
                 #kernel/jvm/os/bsd/sparc/Makefile
                 #kernel/jvm/os/bsd/x86_64/Makefile
                 #kernel/jvm/os/darwin/Makefile
                 #kernel/jvm/os/darwin/arm/Makefile
                 #kernel/jvm/os/darwin/i386/Makefile
                 #kernel/jvm/os/darwin/powerpc/Makefile
                 #kernel/jvm/os/linux/Makefile
                 #kernel/jvm/os/linux/arm/Makefile
                 #kernel/jvm/os/linux/i386/Makefile
                 #kernel/jvm/os/linux/mips/Makefile
                 #kernel/jvm/os/linux/parisc/Makefile
                 #kernel/jvm/os/linux/powerpc/Makefile
                 #kernel/jvm/os/linux/x86_64/Makefile
                 #kernel/jvm/os/solaris/Makefile
                 #kernel/jvm/os/solaris/x86/Makefile
                 #kernel/jvm/os/HelloX/Makefile
                 #kernel/jvm/os/HelloX/i386/Makefile
                 
AC_CONFIG_FILES([kernel/kernel/Makefile
                 kernel/fs/Makefile
                 kernel/kapi/Makefile
                 kernel/kthread/Makefile
                 kernel/network/Makefile
                 kernel/osentry/Makefile
                 kernel/shell/Makefile
                 kernel/lib/Makefile
                 kernel/user/Makefile
                 kernel/drivers/x86/Makefile
                 kernel/arch/x86/Makefile
                 kernel/Makefile])
AC_OUTPUT
