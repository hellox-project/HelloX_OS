# =====================================================================
# Makefile for porting NASA cFS to HelloX OS
# =====================================================================
# Compiles the whole cFS (cFE + OSAL + PSP + apps) plus hxlib and the
# adapt layer into a single PE32 (.exe) executable for HelloX.
#
# Toolchain : clang (i686-w64-windows-gnu) + ld.lld, cross from Linux.
# Output    : PE32 (i386) console executable, entry _hx_main.
#
# Directory layout (relative to this Makefile):
#   hxlib/     HelloX user-mode library (managed separately, copied in)
#   cfs/       NASA cFS source (unchanged, only adds osal/psp "hellox" dirs)
#   adapt/     hxlib gap-fill layer (clock_settime/getpid/sysconf/... )
#   config/    generated config headers (osconfig.h, cfe_*_cfg.h)
#   build/     object files
#   bin/       output PE32 exe
# =====================================================================

CC      := clang
LD      := ld.lld
TARGET  := i686-w64-windows-gnu

CFS_HOME := $(abspath .)

OBJDIR    := build
BINDIR    := bin
EXE       := $(BINDIR)/cfs.exe

# ---------------------------------------------------------------------
# Include paths
# ---------------------------------------------------------------------
INCLUDES := \
    -Ihxlib -Ihxlib/sys -Ihxlib/ptmalloc -Ihxlib/cJSON -Ihxlib/sockets \
    -Ihxlib/regexp -Ihxlib/wildcard -Ihxlib/gnutls -Ihxlib/ftp_srv \
    -Iadapt -Iconfig -Icfs/build/inc -Icfs/build/native/default_cpu1/inc \
    -Icfs/cfe/cmake/target/inc \
    $(addprefix -I, $(wildcard cfs/cfe/modules/*/fsw/inc)) \
    $(addprefix -I, $(wildcard cfs/cfe/modules/*/fsw/platform_inc)) \
    $(addprefix -I, $(wildcard cfs/cfe/modules/*/fsw/mission_inc)) \
    $(addprefix -I, $(wildcard cfs/cfe/modules/*/fsw/src)) \
    $(addprefix -I, $(wildcard cfs/cfe/modules/*/option_inc)) \
    $(addprefix -I, $(wildcard cfs/cfe/modules/*/config)) \
    -Icfs/osal/src/os/inc \
    -Icfs/osal/src/os/shared/inc \
    -Icfs/osal/src/os/hellox/inc \
    -Icfs/osal/src/os/posix/inc \
    -Icfs/osal/src/bsp/shared/inc \
    -Icfs/osal/src/bsp/hellox/inc \
    -Icfs/psp/fsw/inc \
    -Icfs/psp/fsw/shared/inc \
    -Icfs/psp/fsw/hellox/inc \
    $(addprefix -I, $(wildcard cfs/psp/fsw/modules/*/inc)) \
    $(addprefix -I, $(wildcard cfs/apps/*/fsw/inc cfs/apps/*/fsw/platform_inc cfs/apps/*/fsw/mission_inc)) \
    $(addprefix -I, $(wildcard cfs/libs/*/fsw/inc cfs/libs/*/fsw/public_inc))

# ---------------------------------------------------------------------
# Ground station (Yamcs) downlink target.  Override per-satellite via:
#   make CFG_GROUND_IP=192.168.169.8 CFG_GROUND_PORT=1236
CFG_GROUND_IP   ?= 192.168.169.8
CFG_GROUND_PORT ?= 1235

# Compiler / linker flags (freestanding, no host libc)
# ---------------------------------------------------------------------
CFLAGS := --target=$(TARGET) \
          -ffreestanding -nostdlib -nostdinc -fno-builtin \
          -fno-stack-protector -fno-pic -mno-stack-arg-probe \
          -D__i386__ -D_POSIX_ \
          -DOSAL_CONFIG_INCLUDE_STATIC_LOADER \
          -D_CFE_PSP_MODULE_ \
          -DCFG_GROUND_IP=\"$(CFG_GROUND_IP)\" \
          -DCFG_GROUND_PORT=$(CFG_GROUND_PORT) \
          -include compat.h \
          $(INCLUDES) \
          -Wall -Wno-visibility -Wno-unused-function \
          -Wno-implicit-function-declaration -Wno-return-type \
          -Wno-macro-redefined

LDFLAGS := -m i386pe --entry=__hx_main --subsystem=console

# ---------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------
# hxlib (all .c except test/ and the standalone test programs)
HXLIB_SRCS := $(filter-out %/test/% %/stattest.c %/timetest.c, \
    $(wildcard hxlib/*.c hxlib/cJSON/*.c hxlib/ptmalloc/*.c hxlib/sockets/*.c \
              hxlib/regexp/*.c hxlib/wildcard/*.c hxlib/gnutls/*.c hxlib/ftp_srv/*.c))

# adapt layer
ADAPT_SRCS := $(wildcard adapt/*.c)

# cFE core modules (excluding alternative/option implementations)
CFE_SRCS := $(filter-out %/cfe_msg_initdefaulthdr_pri.c %/cfe_msg_msgid_v2.c %/cfe_sbr_map_hash.c, \
    $(wildcard cfs/cfe/modules/*/fsw/src/*.c))

# cFE CMake-generated source files (config map, version, build env)
CFE_GEN_SRCS := cfs/build/src/cfe_config_map.c \
                cfs/build/src/cfe_build_env_table.c \
                cfs/build/src/cfe_mission_strings.c \
                cfs/build/src/cfe_module_version_table.c \
                cfs/cfe/cmake/target/src/target_config.c \
                config/cfe_static_module_list.c \
                config/cfe_static_symbol_list.c \
                config/hellox_module_list.c

# OSAL: shared + portable + hellox impl
OSAL_SHARED_SRCS    := $(wildcard cfs/osal/src/os/shared/src/*.c)
OSAL_PORTABLE_SRCS  := $(wildcard cfs/osal/src/os/portable/os-impl-bsd-sockets.c \
                               cfs/osal/src/os/portable/os-impl-bsd-select.c \
                               cfs/osal/src/os/portable/os-impl-console-bsp.c \
                               cfs/osal/src/os/portable/os-impl-posix-dirs.c \
                               cfs/osal/src/os/portable/os-impl-posix-files.c \
                               cfs/osal/src/os/portable/os-impl-posix-io.c \
                               cfs/osal/src/os/portable/os-impl-posix-gettime.c \
                               cfs/osal/src/os/portable/os-impl-posix-network.c \
                               cfs/osal/src/os/portable/os-impl-no-loader.c \
                               cfs/osal/src/os/portable/os-impl-no-shell.c \
                               cfs/osal/src/os/portable/os-impl-no-symtab.c)
OSAL_HELLOX_SRCS    := $(filter-out %/os-impl-loader.c %/os-impl-shell.c, \
                        $(wildcard cfs/osal/src/os/hellox/src/*.c))
OSAL_BSP_SRCS       := $(filter-out %/bsp_default_app_startup.c %/bsp_default_app_run.c %/bsp_default_symtab.c, \
                        $(wildcard cfs/osal/src/bsp/shared/src/*.c cfs/osal/src/bsp/hellox/src/*.c))

# PSP: shared + modules + hellox platform
PSP_SHARED_SRCS   := $(wildcard cfs/psp/fsw/shared/src/*.c)
PSP_MODULE_SRCS   := $(wildcard cfs/psp/fsw/modules/soft_timebase/*.c \
                            cfs/psp/fsw/modules/timebase_posix_clock/*.c \
                            cfs/psp/fsw/modules/iodriver/src/*.c \
                            cfs/psp/fsw/modules/eeprom_notimpl/*.c \
                            cfs/psp/fsw/modules/ram_notimpl/*.c \
                            cfs/psp/fsw/modules/port_notimpl/*.c)
PSP_HELLOX_SRCS   := $(wildcard cfs/psp/fsw/hellox/src/*.c)

# apps + libs
APP_SRCS := $(wildcard cfs/apps/*/fsw/src/*.c)
LIB_SRCS := $(wildcard cfs/libs/*/fsw/src/*.c)

SRCS := $(HXLIB_SRCS) $(ADAPT_SRCS) $(CFE_SRCS) $(CFE_GEN_SRCS) \
        $(OSAL_SHARED_SRCS) $(OSAL_PORTABLE_SRCS) $(OSAL_HELLOX_SRCS) $(OSAL_BSP_SRCS) \
        $(PSP_SHARED_SRCS) $(PSP_MODULE_SRCS) $(PSP_HELLOX_SRCS) \
        $(APP_SRCS) $(LIB_SRCS)

OBJS := $(patsubst %.c,$(OBJDIR)/%.obj,$(SRCS))

# ---------------------------------------------------------------------
# Targets
# ---------------------------------------------------------------------
.PHONY: all clean list

all: $(EXE)

$(EXE): $(OBJS)
	@mkdir -p $(BINDIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "Built $@"

$(OBJDIR)/%.obj: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

list:
	@echo "hxlib:     $(words $(HXLIB_SRCS)) files"
	@echo "adapt:     $(words $(ADAPT_SRCS)) files"
	@echo "cfe:       $(words $(CFE_SRCS)) files"
	@echo "osal shared: $(words $(OSAL_SHARED_SRCS)) files"
	@echo "osal portable: $(words $(OSAL_PORTABLE_SRCS)) files"
	@echo "osal hellox: $(words $(OSAL_HELLOX_SRCS)) files"
	@echo "osal bsp:  $(words $(OSAL_BSP_SRCS)) files"
	@echo "psp:       $(words $(PSP_SHARED_SRCS) $(PSP_MODULE_SRCS) $(PSP_HELLOX_SRCS)) files"
	@echo "apps+libs: $(words $(APP_SRCS) $(LIB_SRCS)) files"
	@echo "TOTAL:     $(words $(SRCS)) files"

clean:
	rm -rf $(OBJDIR) $(BINDIR)
