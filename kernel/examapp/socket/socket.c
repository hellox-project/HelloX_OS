//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 11,2020
//    Module Name               : socket.c
//    Module Funciton           : 
//                                User land agent for socket and network APIs.
//
//    Last modified Author      : 
//    Last modified Date        : 
//    Last modified Content     : 
//                                
//    Lines number              :
//***********************************************************************/

#include "../hellox.h"
#include "../stdint.h"
#include "../stddef.h"
#include "../stdio.h"

#include "genif.h"
#include "sockets.h"

/* Macros to simplify the programming. */
#define __PARAM_0 edi
#define __PARAM_1 esi
#define __PARAM_2 edx
#define __PARAM_3 ecx
#define __PARAM_4 ebx

/* New a socket and return the index. */
int lwip_socket(int _domain, int _type, int _protocol)
{
	int ret_val = 0;
	int* pRet = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push ebp
		push eax
		mov __PARAM_0, _domain
		mov __PARAM_1, _type
		mov __PARAM_2, _protocol
		mov ebp, pRet
		mov eax, SYSCALL_SOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Close a socket. */
int lwip_close(int s)
{
	int ret_val = 0;
	int* pRet = &ret_val;

	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, s
		mov ebp, pRet
		mov eax, SYSCALL_CLOSESOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return ret_val;
}

/* Bind a socket. */
int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret_val = 0;
	int* pRet = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push ebp
		push eax
		mov __PARAM_0, s
		mov __PARAM_1, name
		mov __PARAM_2, namelen
		mov ebp, pRet
		mov eax, SYSCALL_BIND
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Socket selection. */
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
	struct timeval *timeout)
{
	int ret_val = 0;
	int* pRetPtr = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, maxfdp1
		mov __PARAM_1, readset
		mov __PARAM_2, writeset
		mov __PARAM_3, exceptset
		mov __PARAM_4, timeout
		mov eax, pRetPtr
		mov ebp, eax
		mov eax, SYSCALL_SELECT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Set socket option. */
int lwip_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	int ret_val = 0;
	int* pRetPtr = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, s
		mov __PARAM_1, level
		mov __PARAM_2, optname
		mov __PARAM_3, optval
		mov __PARAM_4, optlen
		mov eax, pRetPtr
		mov ebp, eax
		mov eax, SYSCALL_SETSOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Get socket option. */
int lwip_getsockopt(int s, int level, int optname, const void *optval, socklen_t* optlen)
{
	int ret_val = 0;
	int* pRetPtr = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, s
		mov __PARAM_1, level
		mov __PARAM_2, optname
		mov __PARAM_3, optval
		mov __PARAM_4, optlen
		mov eax, pRetPtr
		mov ebp, eax
		mov eax, SYSCALL_GETSOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Send data to a target socket. */
int lwip_sendto(int s, const void *dataptr, size_t size, int flags,
	const struct sockaddr *to, socklen_t tolen)
{
	__SYSCALL_PARAM_EXTENSION_BLOCK ext_block;
	__SYSCALL_PARAM_EXTENSION_BLOCK* pext_block = &ext_block;
	int ret = -1;
	int* pret = &ret;

	/* Init extension param block. */
	ext_block.ext_param0 = (uint32_t)s;
	ext_block.ext_param1 = (uint32_t)dataptr;
	ext_block.ext_param2 = (uint32_t)size;
	ext_block.ext_param3 = (uint32_t)flags;
	ext_block.ext_param4 = (uint32_t)to;
	ext_block.ext_param5 = (uint32_t)tolen;

	/* Launch the system call. */
	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, 0
		mov __PARAM_1, 0
		mov __PARAM_2, 0
		mov __PARAM_3, 0
		mov __PARAM_4, pext_block
		mov eax, pret
		mov ebp, eax
		mov eax, SYSCALL_SENDTO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret;
}

/* Receive data from a target socket. */
int lwip_recvfrom(int s, void *mem, size_t len, int flags,
	struct sockaddr *from, socklen_t *fromlen)
{
	__SYSCALL_PARAM_EXTENSION_BLOCK ext_block;
	__SYSCALL_PARAM_EXTENSION_BLOCK* pext_block = &ext_block;
	int ret = -1;
	int* pret = &ret;

	/* Init extension param block. */
	ext_block.ext_param0 = (uint32_t)s;
	ext_block.ext_param1 = (uint32_t)mem;
	ext_block.ext_param2 = (uint32_t)len;
	ext_block.ext_param3 = (uint32_t)flags;
	ext_block.ext_param4 = (uint32_t)from;
	ext_block.ext_param5 = (uint32_t)fromlen;

	/* Launch the system call. */
	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, 0
		mov __PARAM_1, 0
		mov __PARAM_2, 0
		mov __PARAM_3, 0
		mov __PARAM_4, pext_block
		mov eax, pret
		mov ebp, eax
		mov eax, SYSCALL_RECVFROM
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret;
}

/* Get all genif's information from system. */
int GetGenifInfo(__GENERIC_NETIF* pGenifBlock, unsigned long* buf_req,
	unsigned long ulFlags)
{
	int ret_val = 0;
	int* pRet = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push ebp
		push eax
		mov __PARAM_0, pGenifBlock
		mov __PARAM_1, buf_req
		mov __PARAM_2, ulFlags
		mov ebp, pRet
		mov eax, SYSCALL_GETGENIFINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Add one network address to a genif. */
int AddGenifAddress(unsigned long genif_index,
	unsigned long protocol,
	__COMMON_NETWORK_ADDRESS* comm_addr,
	int addr_num,
	BOOL bSecondary)
{
	int ret_val = 0;
	int* pRetPtr = &ret_val;

	__asm {
		push __PARAM_0
		push __PARAM_1
		push __PARAM_2
		push __PARAM_3
		push __PARAM_4
		push ebp
		push eax
		mov __PARAM_0, genif_index
		mov __PARAM_1, protocol
		mov __PARAM_2, comm_addr
		mov __PARAM_3, addr_num
		mov __PARAM_4, bSecondary
		mov eax, pRetPtr
		mov ebp, eax
		mov eax, SYSCALL_ADDGENIFADDRESS
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_4
		pop __PARAM_3
		pop __PARAM_2
		pop __PARAM_1
		pop __PARAM_0
	}
	return ret_val;
}

/* Get network info from system. */
BOOL GetNetworkInfo(__SYSTEM_NETWORK_INFO* pNetInfo)
{
	BOOL bRet = FALSE;
	BOOL* pRet = &bRet;
	__asm {
		push __PARAM_0
		push ebp
		push eax
		mov __PARAM_0, pNetInfo
		mov eax, pRet
		mov ebp, eax
		mov eax, SYSCALL_GETNETWORKINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop __PARAM_0
	}
	return bRet;
}

#undef __PARAM_0
#undef __PARAM_1
#undef __PARAM_2
#undef __PARAM_3
#undef __PARAM_4
