# Makefile for HelloX v190_lib
# Compiles all source files into a PE/EXE executable loadable by HelloX.
# Entry point: _hx_main
# Toolchain: clang (no external libraries or headers)

CC      := clang
LD      := clang

# Direct ld invocation for PE linking (bypasses clang's flag filtering)
LD_PE   := ld -m i386pe
CFLAGS  := -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
           -fno-pic -fno-PIC -fno-pie \
           -D__GNUC__=4 -D__i386__ -D_POSIX_ \
           -DWOLFSSL_USER_SETTINGS \
           -I. -I./sys -I./ptmalloc -I./cJSON -I./sockets \
           -I./regexp -I./wildcard -I./gnutls -I./ftp_srv \
           -I./wolfssl \
           -Wall -Wno-implicit-function-declaration \
           -Wno-incompatible-library-redeclaration \
           -Wno-deprecated-declarations -Wno-undefined-inline \
           -Wno-typedef-redefinition -Wno-macro-redefined \
           -Wno-cpp -Wno-unknown-warning-option

# Target: PE/EXE executable for HelloX
TARGET := sysbrain.exe

# Root source files (all .c in top directory)
SRCS_ROOT := $(wildcard *.c)

# Subdirectory source files
SRCS_cJSON    := $(wildcard cJSON/*.c)
SRCS_ftp_srv  := $(wildcard ftp_srv/*.c)
SRCS_gnutls   := $(wildcard gnutls/*.c)
SRCS_ptmalloc := $(wildcard ptmalloc/*.c)
SRCS_regexp   := $(wildcard regexp/*.c)
SRCS_sockets  := $(wildcard sockets/*.c)
SRCS_wildcard := $(wildcard wildcard/*.c)

# wolfSSL source files
SRCS_wolfssl  := $(wildcard wolfssl/src/*.c)
SRCS_wolfcrypt := $(wildcard wolfssl/wolfcrypt/src/*.c)

# All object files (same basename as .c files, in corresponding subdirs)
OBJS := $(SRCS_ROOT:.c=.o) \
        $(SRCS_cJSON:.c=.o) \
        $(SRCS_ftp_srv:.c=.o) \
        $(SRCS_gnutls:.c=.o) \
        $(SRCS_ptmalloc:.c=.o) \
        $(SRCS_regexp:.c=.o) \
        $(SRCS_sockets:.c=.o) \
        $(SRCS_wildcard:.c=.o) \
        $(SRCS_wolfssl:.c=.o) \
        $(SRCS_wolfcrypt:.c=.o)

# Linker flags for PE/COFF direct ld invocation.
# We bypass clang because clang filters some ld PE-specific options
# like --enable-reloc-section. Instead we use ld -m i386pe directly.
LDFLAGS := --enable-reloc-section \
           --entry=_hx_main \
           -T hellox.ld \
           -o $(TARGET)

# ld command for building with .reloc section (i386pe emulation)
LD_RELOC := ld -m i386pe --subsystem native:0 --entry _hx_main \
             --image-base 0x00400000 --section-alignment 4096 \
             -Map=sysbrain.map \
             --file-alignment 512 -T hellox.ld

LDFLAGS_LAST := $(OBJS)

.PHONY: all clean

all: $(TARGET)

#---------------------------------------------------------------
# Compilation rules
#---------------------------------------------------------------

# Top-level .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Subdirectory .c -> .o (generic pattern)
cJSON/%.o: cJSON/%.c
	$(CC) $(CFLAGS) -c $< -o $@

ftp_srv/%.o: ftp_srv/%.c
	$(CC) $(CFLAGS) -c $< -o $@

gnutls/%.o: gnutls/%.c
	$(CC) $(CFLAGS) -c $< -o $@

ptmalloc/%.o: ptmalloc/%.c
	$(CC) $(CFLAGS) -c $< -o $@

regexp/%.o: regexp/%.c
	$(CC) $(CFLAGS) -c $< -o $@

sockets/%.o: sockets/%.c
	$(CC) $(CFLAGS) -c $< -o $@

wildcard/%.o: wildcard/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# wolfSSL compilation rules
wolfssl/src/%.o: wolfssl/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

wolfssl/wolfcrypt/src/%.o: wolfssl/wolfcrypt/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#---------------------------------------------------------------
# Link step: produce PE/EXE
#---------------------------------------------------------------
$(TARGET): $(OBJS) hellox.ld
	$(LD_PE) $(LDFLAGS) $(LDFLAGS_LAST)

#---------------------------------------------------------------
# Build with .reloc section (base relocation table)
#---------------------------------------------------------------
reloc: $(OBJS) hellox.ld
	$(LD_RELOC) -o $(TARGET) $(OBJS)

#---------------------------------------------------------------
# Clean
#---------------------------------------------------------------
clean:
	rm -f $(OBJS) $(TARGET)