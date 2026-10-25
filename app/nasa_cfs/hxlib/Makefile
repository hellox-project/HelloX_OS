# Makefile for HelloX hxlib (user-mode development library)
# =====================================================================
# Compiles the hxlib sources plus a demo/test application into a single
# PE32 (.exe) executable that runs on HelloX.
#
# Toolchain: clang (LLVM) + ld.lld, cross-compiling from Linux.
# Output    : a Windows PE32 (i386) console executable, entry _hx_main.
#
# Design points
#   - Target is i686-w64-windows-gnu (MinGW). This uses clang's default
#     GNU/AT&T syntax style (__GNUC__/__clang__ are defined, _MSC_VER is
#     not), so the GNU inline-assembly paths in the sources are selected,
#     while still producing COFF objects and a PE32 executable with the
#     leading-underscore (cdecl) symbol decoration HelloX expects.
#   - Fully freestanding: -nostdlib -nostdinc -ffreestanding; nothing from
#     the host Linux toolchain or C library is linked in. The exe talks to
#     HelloX only through the int 0x7F system calls in hellox.c.
#   - -mno-stack-arg-probe: HelloX has no guard pages, so stack probing is
#     unnecessary; this avoids an unresolved __chkstk/__alloca symbol.
#   - comp_gcc_AI.c supplies the 64-bit division/remainder helpers
#     (__udivdi3/__divdi3/__umoddi3/__moddi3) that clang emits in MinGW mode.
#   - crt0.c supplies a no-op __main() that clang/MinGW emits a call to at
#     the start of main(); freestanding builds have no libmingw32 to provide it.
# =====================================================================

CC      := clang
LD      := ld.lld
TARGET  := i686-w64-windows-gnu

# ---------------------------------------------------------------------
# Directories
# ---------------------------------------------------------------------
OBJDIR     := build
RELEASEDIR := release
EXE        := $(RELEASEDIR)/hxlib.exe

# ---------------------------------------------------------------------
# Include paths (all hxlib headers; freestanding => no system headers)
# ---------------------------------------------------------------------
INCLUDES  := -I. -I./sys -I./ptmalloc -I./cJSON -I./sockets \
             -I./regexp -I./wildcard -I./gnutls -I./ftp_srv

# ---------------------------------------------------------------------
# Compiler flags
# ---------------------------------------------------------------------
CFLAGS    := --target=$(TARGET) \
             -ffreestanding -nostdlib -nostdinc -fno-builtin \
             -fno-stack-protector -fno-pic -mno-stack-arg-probe \
             -D__i386__ -D_POSIX_ \
             $(INCLUDES) \
             -Wall -Wno-visibility -Wno-unused-function \
             -Wno-implicit-function-declaration -Wno-return-type

# ---------------------------------------------------------------------
# Linker flags (MinGW PE linking)
# ---------------------------------------------------------------------
LDFLAGS   := -m i386pe --entry=__hx_main --subsystem=console

# ---------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------
# Core library (C runtime + HelloX OS API + utility libs + compiler shims)
LIB_SRCS  := acos.c asin.c atan.c atan2.c atox.c binsem.c ceil.c \
             cJSON/cJSON.c cleanup.c copysign.c ctype.c dirent.c dirlist.c \
             errno.c exp.c fabs.c finite.c getenv.c hellox.c io.c log.c \
             math.c memory.c nextafter.c pow.c pthread.c pthread_cond.c \
             pthread_mutex.c pthread_other.c pthread_sem.c ptmalloc/malloc.c \
             qsort.c rand.c regexp/regexp.c scalbn.c scanf.c sched.c setjmp.c \
             signal.c sockaddr.c sockets/hx_eth.c sockets/hx_inet.c \
             sockets/ip_addr.c sockets/socket.c sqrt.c stat.c stdio.c \
             strftime.c string.c strtod.c time.c trig.c wchar.c \
             wildcard/wildcard.c mq.c comp_gcc_AI.c crt0.c main.c

# Demo/test application (provides the classic main() entry point).
APP_SRCS  := test/mq_robust_test.c

OBJS      := $(patsubst %.c,$(OBJDIR)/%.obj,$(LIB_SRCS) $(APP_SRCS))

# ---------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------
.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJS)
	@mkdir -p $(RELEASEDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built $@"

$(OBJDIR)/%.obj: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(RELEASEDIR)
