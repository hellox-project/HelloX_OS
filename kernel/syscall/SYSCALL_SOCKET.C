//***********************************************************************/
//    Author                    : Garry,Tywind Huang.
//    Original Date             : Mar 13,2009
//    Module Name               : syscall_socket.c
//    Module Funciton           : 
//                                System call's implementation code,for socket
//                                API part.This is written by Tywind Huang,and
//                                refined by Garry.Xin,in 2018's spring festival.
//                                MEMO:
//                                It's a new year again,and also in the first
//                                day of spring festival,time flies so fast,I'm
//                                getting old,with white hair appear on head,with
//                                weight more and more...
//                                Life is a journey of soul,the journey is passing
//                                as quick as river,and never repeat again.Where is
//                                the destination of this journey?and what's the
//                                purpose of this journey?and what's the scene after
//                                this journey on earth?more and more frequently of
//                                these questions raise on my head.Wish a small,even
//                                little,step will advance in the arrived new year.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "netmgr.h"
#include "netglob.h"
#include "syscall.h"

/*
 * Macros to refer the parameter in system 
 * call parameter block.
 * Just to simplify the programming of system 
 * call's implementation.
 */
#define PARAM(idx) (pspb->param_##idx.param)
#define SYSCALL_RET (*pspb->ret_ptr.param)
#define EXT_PARAM(idx) (pspb->param_4.pExtBlock ? \
	pspb->param_4.pExtBlock->ext_param##idx : 0)

static void SC_Socket(__SYSCALL_PARAM_BLOCK* pspb)
{
	/* Just delegates to lwip_socket. */
	SYSCALL_RET = 
		(uint32_t)lwip_socket((int)PARAM(0), 
		(int)PARAM(1), (int)PARAM(2));
}

static void SC_Bind(__SYSCALL_PARAM_BLOCK* pspb)
{
	const struct sockaddr* dest_name = NULL;
	uint32_t ret = -1;

	if (0 == PARAM(2) || NULL == (void*)PARAM(1))
	{
		goto __TERMINAL;
	}
	if (sizeof(struct sockaddr) != PARAM(2))
	{
		goto __TERMINAL;
	}

	/* map user memory to kernel. */
	dest_name = (const struct sockaddr*)map_to_kernel((void*)PARAM(1), 
		sizeof(struct sockaddr), __IN);
	if (NULL == dest_name)
	{
		goto __TERMINAL;
	}
	/* Invoke the corresponding routine in kernel. */
	ret = (uint32_t)lwip_bind((int)PARAM(0), dest_name, (socklen_t)PARAM(2));
	/* map to user again. */
	map_to_user((void*)PARAM(1), sizeof(struct sockaddr), __IN, (void*)dest_name);

__TERMINAL:
	SYSCALL_RET = ret;
}

static void SC_Connect(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	const struct sockaddr* addr = NULL;

	/* Check parameters. */
	if ((NULL == (const struct sockaddr*)PARAM(1)) || (PARAM(2) != sizeof(struct sockaddr)))
	{
		goto __TERMINAL;
	}
	/* map to kernel. */
	addr = (const struct sockaddr*)map_to_kernel((void*)PARAM(1), sizeof(struct sockaddr), __IN);
	if (NULL == addr)
	{
		goto __TERMINAL;
	}
	/* Invoke the corresponding routine in kernel. */
	ret = (uint32_t)lwip_connect((int)PARAM(0), (const struct sockaddr *)PARAM(1),
		(socklen_t)PARAM(2));

__TERMINAL:
	if (addr)
	{
		map_to_user((void*)PARAM(1), sizeof(struct sockaddr), __IN, (void*)addr);
	}
	SYSCALL_RET = ret;
}

static void SC_Recv(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	void* buffer = NULL;

	if ((NULL == (void*)PARAM(1)) || (0 == PARAM(2)))
	{
		goto __TERMINAL;
	}
	buffer = map_to_kernel((void*)PARAM(1), PARAM(2), __OUT);
	if (NULL == buffer)
	{
		goto __TERMINAL;
	}
	/* Invoke the kernel routine. */
	ret = (uint32_t)lwip_recv((int)PARAM(0), 
		buffer, //(void*)PARAM(1),
		(size_t)PARAM(2),
		(int)PARAM(3));

__TERMINAL:
	if (buffer)
	{
		map_to_user((void*)PARAM(1), PARAM(2), __IN, buffer);
	}
	SYSCALL_RET = ret;
}

static void SC_Send(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	const void* buffer = NULL;

	if ((NULL == (const void*)PARAM(1)) || (0 == PARAM(2)))
	{
		goto __TERMINAL;
	}
	/* map to kernel space. */
	buffer = (const void*)map_to_kernel((void*)PARAM(1), PARAM(2), __IN);
	if (NULL == buffer)
	{
		goto __TERMINAL;
	}

	/* Invoke kernel mode routine. */
	ret = (uint32_t)lwip_send((int)PARAM(0),
		buffer, //(const void*)PARAM(1),
		(size_t)PARAM(2),
		(int)PARAM(3));

__TERMINAL:
	if (buffer)
	{
		map_to_user((void*)PARAM(1), PARAM(2), __IN, (void*)buffer);
	}
	SYSCALL_RET = ret;
}

static void SC_RecvFrom(__SYSCALL_PARAM_BLOCK* pspb)
{
	char* recv_mem = NULL;
	uint32_t ret = -1;
	struct sockaddr* from = NULL;
	socklen_t from_len = 0;

	/* Map to kernel. */
	if (EXT_PARAM(1) && EXT_PARAM(2))
	{
		recv_mem = (char*)map_to_kernel((void*)EXT_PARAM(1), EXT_PARAM(2), __OUT);
		if (NULL == recv_mem)
		{
			goto __TERMINAL;
		}
	}
	if (EXT_PARAM(5))
	{
		from_len = *(socklen_t*)EXT_PARAM(5);
		if (from_len != sizeof(struct sockaddr))
		{
			goto __TERMINAL;
		}
	}
	if (EXT_PARAM(4))
	{
		/* 
		 * The from addr maybe set by caller so it must 
		 * be transfer to recvfrom routine, use __INOUT
		 * indicator to copy it to kernel.
		 */
		from = (struct sockaddr*)map_to_kernel((void*)EXT_PARAM(4), 
			sizeof(struct sockaddr), __INOUT);
		if (NULL == from)
		{
			goto __TERMINAL;
		}
	}

	ret = (uint32_t)lwip_recvfrom(
		(int)EXT_PARAM(0),
		recv_mem, //(void*)EXT_PARAM(1),
		(size_t)EXT_PARAM(2),
		(int)EXT_PARAM(3),
		from, //(struct sockaddr*)EXT_PARAM(4),
		&from_len //(socklen_t *)EXT_PARAM(5));
	);
__TERMINAL:
	if (recv_mem)
	{
		map_to_user((void*)EXT_PARAM(1), EXT_PARAM(2), __OUT, recv_mem);
	}
	if (from)
	{
		map_to_user((void*)EXT_PARAM(4), from_len, __OUT, from);
	}
	if (EXT_PARAM(5))
	{
		*(socklen_t*)EXT_PARAM(5) = from_len;
	}
	SYSCALL_RET = ret;
}

static void SC_SendTo(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	char* send_mem = NULL;
	const struct sockaddr* to = NULL;

	if ((NULL == (void*)EXT_PARAM(1)) || (0 == EXT_PARAM(2)))
	{
		goto __TERMINAL;
	}
	send_mem = (char*)map_to_kernel((void*)EXT_PARAM(1), EXT_PARAM(2), __IN);
	if (NULL == send_mem)
	{
		goto __TERMINAL;
	}
	if (EXT_PARAM(4))
	{
		if (0 == EXT_PARAM(5))
		{
			/* Invalid parameters combination. */
			goto __TERMINAL;
		}
		/* Map to kernel. */
		to = (const struct sockaddr*)map_to_kernel((void*)EXT_PARAM(4), EXT_PARAM(5), __IN);
		if (NULL == to)
		{
			goto __TERMINAL;
		}
	}

	ret = (uint32_t)lwip_sendto(
		(int)EXT_PARAM(0),
		send_mem, //(const void*)EXT_PARAM(1),
		(size_t)EXT_PARAM(2),
		(INT)EXT_PARAM(3),
		to, //(const struct sockaddr*)EXT_PARAM(4),
		(socklen_t)EXT_PARAM(5));

__TERMINAL:
	if (send_mem)
	{
		map_to_user((void*)EXT_PARAM(1), EXT_PARAM(2), __IN, send_mem);
	}
	if (to)
	{
		map_to_user((void*)EXT_PARAM(4), EXT_PARAM(5), __IN, (void*)to);
	}
	SYSCALL_RET = ret;
}

static void SC_Write(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	const void* buffer = NULL;

	if ((NULL == (const void*)PARAM(1)) || (0 == PARAM(2)))
	{
		goto __TERMINAL;
	}
	/* map to kernel space. */
	buffer = (const void*)map_to_kernel((void*)PARAM(1), PARAM(2), __IN);
	if (NULL == buffer)
	{
		goto __TERMINAL;
	}

	/* Invoke kernel mode routine. */
	ret = (uint32_t)lwip_write((int)PARAM(0),
		buffer, //(const void*)PARAM(1),
		(size_t)PARAM(2));

__TERMINAL:
	if (buffer)
	{
		map_to_user((void*)PARAM(1), PARAM(2), __IN, (void*)buffer);
	}
	SYSCALL_RET = ret;
}

static void SC_Select(__SYSCALL_PARAM_BLOCK* pspb)
{
	fd_set* readset = NULL, *writeset = NULL, *excepset = NULL;
	struct timeval* time = NULL;
	uint32_t ret = -1;

	if ((NULL == (void*)PARAM(1)) && (NULL == (void*)PARAM(2)) && (NULL == (void*)PARAM(3)))
	{
		goto __TERMINAL;
	}
	/* Map user memory space to kernel. */
#define __LOCAL_MAP_TO_KERNEL(id, set) \
	if (PARAM(id)) \
	{ \
		set = (fd_set*)map_to_kernel((void*)PARAM(id), sizeof(fd_set), __IN); \
		if (NULL == set) \
		{ \
			goto __TERMINAL; \
		} \
	}
	__LOCAL_MAP_TO_KERNEL(1, readset);
	__LOCAL_MAP_TO_KERNEL(2, writeset);
	__LOCAL_MAP_TO_KERNEL(3, excepset);
#undef __LOCAL_MAP_TO_KERNEL
	
	if (PARAM(4))
	{
		time = (struct timeval*)map_to_kernel((void*)PARAM(4), sizeof(struct timeval), __INOUT);
		if (NULL == time)
		{
			goto __TERMINAL;
		}
	}

	/* Invoke the corresponding kernel routine. */
	ret = (uint32_t)lwip_select(
		(int)PARAM(0),
		readset, //(fd_set*)PARAM(1),
		writeset, //(fd_set*)PARAM(2),
		excepset, //(fd_set*)PARAM(3),
		(struct timeval*)PARAM(4));

__TERMINAL:
#define __LOCAL_MAP_TO_USER(id, set) \
	if (set) \
	{ \
		map_to_user((void*)PARAM(id), sizeof(fd_set), __IN, set); \
	}
	__LOCAL_MAP_TO_USER(1, readset);
	__LOCAL_MAP_TO_USER(2, writeset);
	__LOCAL_MAP_TO_USER(3, excepset);
#undef __LOCAL_MAP_TO_USER

	if (time)
	{
		map_to_user((void*)PARAM(4), sizeof(struct timeval), __INOUT, time);
	}

	SYSCALL_RET = ret;
}

static void SC_SetSocketOpt(__SYSCALL_PARAM_BLOCK* pspb)
{
	void* opt_val = NULL;
	uint32_t ret = -1;

	/* Map user memory to kernel. */
	opt_val = map_to_kernel((void*)PARAM(3), PARAM(4), __IN);
	if (NULL == opt_val)
	{
		goto __TERMINAL;
	}

	ret = (uint32_t)lwip_setsockopt(
		(int)PARAM(0),
		(int)PARAM(1),
		(int)PARAM(2),
		(const void*)opt_val,
		//(const void*)PARAM(3),
		(socklen_t)PARAM(4));

	/* Map back to user. */
	map_to_user((void*)PARAM(3), PARAM(4), __IN, opt_val);

__TERMINAL:
	SYSCALL_RET = (uint32_t)ret;
}

static void SC_GetSocketOpt(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	socklen_t opt_len = 0, old_len = 0;
	void* optval = NULL;

	/* check parameters. */
	if ((NULL == (void*)PARAM(3)) || (NULL == (socklen_t*)PARAM(4)))
	{
		goto __TERMINAL;
	}
	old_len = *(socklen_t*)PARAM(4);
	opt_len = old_len;
	if (0 == old_len)
	{
		goto __TERMINAL;
	}
	/* map opt value from user to kernel. */
	optval = map_to_kernel((void*)PARAM(3), old_len, __OUT);
	if (NULL == optval)
	{
		goto __TERMINAL;
	}

	/* invoke */
	ret = (uint32_t)lwip_getsockopt((int)PARAM(0),
		(int)PARAM(1),
		(int)PARAM(2),
		optval, //(void*)PARAM(3),
		&opt_len); // (socklen_t*)PARAM(4));

__TERMINAL:
	if (optval)
	{
		map_to_user((void*)PARAM(3), old_len, __OUT, optval);
		*(socklen_t*)PARAM(4) = opt_len;
	}
	SYSCALL_RET = ret;
}

static void SC_GetHostByName(__SYSCALL_PARAM_BLOCK* pspb)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
}

static void SC_Ioctl(__SYSCALL_PARAM_BLOCK* pspb)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
}

static void SC_Fcntl(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)lwip_fcntl((int)PARAM(0),
		(int)PARAM(1),
		(int)PARAM(2));
}

static void SC_Accept(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	struct sockaddr* addr = NULL;
	socklen_t addr_len = 0, old_len = 0;

	/* Validates parameters. */
	if ((NULL == (struct sockaddr*)PARAM(1)) || (NULL == (socklen_t*)PARAM(2)))
	{
		goto __TERMINAL;
	}
	addr_len = old_len = *(socklen_t*)PARAM(2);
	if (0 == addr_len)
	{
		goto __TERMINAL;
	}
	addr = (struct sockaddr*)map_to_kernel((void*)PARAM(1), addr_len, __INOUT);
	if (NULL == addr)
	{
		goto __TERMINAL;
	}

	ret = (uint32_t)lwip_accept((int)PARAM(0),
		addr, //(struct sockaddr *)PARAM(1),
		&addr_len); // (socklen_t*)PARAM(2));

__TERMINAL:
	if (addr)
	{
		map_to_user((void*)PARAM(1), old_len, __INOUT, addr);
		*(socklen_t*)PARAM(2) = addr_len;
	}
	SYSCALL_RET = ret;
}

static void SC_Listen(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)lwip_listen((int)PARAM(0), (int)PARAM(1));
}

static void SC_CloseSocket(__SYSCALL_PARAM_BLOCK* pspb)
{
	SYSCALL_RET = (uint32_t)lwip_close((int)PARAM(0));
}

/* Return global network's information. */
static void SC_GetNetworkInfo(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = 0; /* BOOL */
	__SYSTEM_NETWORK_INFO* pSysinfo = NULL;

	if (NULL == (__SYSTEM_NETWORK_INFO*)PARAM(0))
	{
		goto __TERMINAL;
	}
	pSysinfo = (__SYSTEM_NETWORK_INFO*)map_to_kernel((void*)PARAM(0), 
		sizeof(__SYSTEM_NETWORK_INFO), __OUT);
	if (NULL == pSysinfo)
	{
		goto __TERMINAL;
	}

	ret = (uint32_t)NetworkGlobal.GetNetworkInfo(pSysinfo);

__TERMINAL:
	if (pSysinfo)
	{
		map_to_user((void*)PARAM(0), sizeof(__SYSTEM_NETWORK_INFO), __OUT, pSysinfo);
	}
	SYSCALL_RET = ret;
}

/* Return all genifs in system. */
static void SC_GetGenifInfo(__SYSCALL_PARAM_BLOCK* pspb)
{
	unsigned long ulFlags = PARAM(2);
	__GENERIC_NETIF* pNetif = NULL;
	unsigned long buff_req = 0;
	unsigned long old_req = 0;
	uint32_t ret = -1;

	if ((NULL == (__GENERIC_NETIF*)PARAM(0)) || (NULL == (unsigned long*)PARAM(1)))
	{
		/* Bad parameters. */
		goto __TERMINAL;
	}
	buff_req = *(unsigned long*)PARAM(1);
	/* Save the original buff length since kernel may change it. */
	old_req = buff_req;
	pNetif = (__GENERIC_NETIF*)map_to_kernel((void*)PARAM(0), buff_req, __OUT);
	if (NULL == pNetif)
	{
		goto __TERMINAL;
	}

	/* 
	 * ulFlags controls which kind of information of the genif 
	 * should be returned to user.
	 * Not supported yet.
	 */
	ret = (uint32_t)NetworkManager.GetGenifInfo(pNetif, &buff_req);

__TERMINAL:
	if (PARAM(1))
	{
		*(unsigned long*)PARAM(1) = buff_req;
	}
	if (pNetif)
	{
		map_to_user((void*)PARAM(0), old_req, __OUT, pNetif);
	}
	SYSCALL_RET = ret;
}

/* Set address to a given genif. */
static void SC_AddGenifAddress(__SYSCALL_PARAM_BLOCK* pspb)
{
	uint32_t ret = -1;
	__COMMON_NETWORK_ADDRESS* addr = NULL;
	int addr_len = 0;

	if ((NULL == (__COMMON_NETWORK_ADDRESS*)PARAM(2)) || (0 == PARAM(3)))
	{
		/* At least 1 address must be specified. */
		goto __TERMINAL;
	}
	addr_len = sizeof(__COMMON_NETWORK_ADDRESS) * PARAM(3);
	addr = (__COMMON_NETWORK_ADDRESS*)map_to_kernel((void*)PARAM(2), addr_len, __IN);
	if (NULL == addr)
	{
		goto __TERMINAL;
	}

	ret = (uint32_t)NetworkManager.AddGenifAddress(
		(unsigned long)PARAM(0),
		(unsigned long)PARAM(1),
		addr, //(__COMMON_NETWORK_ADDRESS*)PARAM(2),
		(int)PARAM(3),
		(BOOL)PARAM(4));

__TERMINAL:
	if (addr)
	{
		/* map back to user from kernel. */
		map_to_user((void*)PARAM(2), addr_len, __IN, addr);
	}
	SYSCALL_RET = ret;
}

/* 
 * A test routine to test the function of xfering 
 * 6 or more parameters. 
 */
static void SC_TestParamXfer(__SYSCALL_PARAM_BLOCK* pspb)
{
	char* pszInfo0 = (char*)EXT_PARAM(0);
	char* pszInfo1 = (char*)EXT_PARAM(1);
	char* pszInfo2 = (char*)EXT_PARAM(2);
	int nInfo3 = (int)EXT_PARAM(3);
	unsigned int nInfo4 = (unsigned int)EXT_PARAM(4);
	char* pszInfo5 = (char*)EXT_PARAM(5);
	char* pszInfo6 = (char*)EXT_PARAM(6);
	char* pszInfo7 = (char*)EXT_PARAM(7);

	/* Show out all these information elements. */
	_hx_printf("%s %s %s %d %d %s %s %s\r\n",
		pszInfo0, pszInfo1, pszInfo2, nInfo3, nInfo4,
		pszInfo5, pszInfo6, pszInfo7);
	SYSCALL_RET = (uint32_t)TRUE;
}

void RegisterSocketEntry(SYSCALL_ENTRY* pSysCallEntry)
{
	pSysCallEntry[SYSCALL_SOCKET] = SC_Socket;
	pSysCallEntry[SYSCALL_BIND] = SC_Bind;
	pSysCallEntry[SYSCALL_ACCEPT] = SC_Accept;
	pSysCallEntry[SYSCALL_CONNECT] = SC_Connect;
	pSysCallEntry[SYSCALL_SETSOCKET] = SC_SetSocketOpt;
	pSysCallEntry[SYSCALL_GETSOCKET] = SC_GetSocketOpt;
	pSysCallEntry[SYSCALL_LISTEN] = SC_Listen;
	pSysCallEntry[SYSCALL_SELECT] = SC_Select;
	pSysCallEntry[SYSCALL_CLOSESOCKET] = SC_CloseSocket;
	pSysCallEntry[SYSCALL_SEND] = SC_Send;
	pSysCallEntry[SYSCALL_SENDTO] = SC_SendTo;
	pSysCallEntry[SYSCALL_RECV] = SC_Recv;
	pSysCallEntry[SYSCALL_RECVFROM] = SC_RecvFrom;
	pSysCallEntry[SYSCALL_GETGENIFINFO] = SC_GetGenifInfo;
	pSysCallEntry[SYSCALL_ADDGENIFADDRESS] = SC_AddGenifAddress;
	pSysCallEntry[SYSCALL_GETNETWORKINFO] = SC_GetNetworkInfo;
	
	/* Test routine. */
	pSysCallEntry[SYSCALL_TESTPARAMXFER] = SC_TestParamXfer;
}

#undef PARAM
#undef SYSCALL_RET
#undef EXT_PARAM
