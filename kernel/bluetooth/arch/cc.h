#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>

//system types
typedef uint8_t		u8_t;
#define U8_F		"d"
typedef int8_t		s8_t;
#define S8_F		"d"
typedef uint16_t	u16_t;
#define U16_F		"04x"
typedef int16_t		s16_t;
#define S16_F		"04x"
typedef uint32_t	u32_t;
#define U32_F		"08x"
typedef int32_t		s32_t;
#define S32_F		"08x"
typedef int			mem_ptr_t;

#define PACK_STRUCT_STRUCT __attribute__((__packed__))

#define LWIP_PLATFORM_DIAG(x) printf x
#define LWIP_PLATFORM_ASSERT(x) printf(x)//; raise(SIGTRAP)
