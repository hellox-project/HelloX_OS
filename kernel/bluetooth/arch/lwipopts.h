#pragma once

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 4321
#endif

#define LWIP_DEBUG 1
#define NO_SYS 1
#define MEM_LIBC_MALLOC 1
#define MEM_ALIGNMENT 4
#define LWIP_TCP 0
#define LWIP_UDP 0
#define LWIP_IGMP 0
#define LWIP_RAW 0
#define IP_REASSEMBLY 0
#define LWIP_NETCONN 0
#define ARP_QUEUEING 0
//#define BYTE_ORDER LITTLE_ENDIAN
#define LWIP_PLATFORM_BYTESWAP 0

#define MEM_DEBUG LWIP_DBG_ON
#define MEM_STATS 1
