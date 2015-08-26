//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Mar 13,2009
//    Module Name               : SYSCALL.CPP
//    Module Funciton           : 
//                                System call implementation code.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "StdAfx.h"
#include "syscall.h"
#include "stdio.h"
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "devmgr.h"
#include "iomgr.h"
 
//int lwip_shutdown(int s, int how);
//int lwip_getpeername (int s, struct sockaddr *name, socklen_t *namelen);
//int lwip_getsockname (int s, struct sockaddr *name, socklen_t *namelen);


static void   SC_Socket(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_socket((INT)PARAM(0), (INT)PARAM(1),(INT)PARAM(2));

}

static void   SC_Bind(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_bind((INT)PARAM(0),(const struct sockaddr *)PARAM(1),(socklen_t)PARAM(2));
}

static void   SC_Connect(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_connect((INT)PARAM(0),(const struct sockaddr *)PARAM(1),(socklen_t)PARAM(2));
}

static void   SC_Recv(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_recv(
		                (INT)PARAM(0),
						(void*)PARAM(1),
						(size_t)PARAM(2),
						(INT)PARAM(3)
						);
}

static void   SC_Send(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_send(
		               (INT)PARAM(0),
		               (const void*)PARAM(1),
		               (size_t)PARAM(2),
					   (INT)PARAM(3)
		               );
}

static void   SC_RecvFrom(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_recvfrom(
		(INT)PARAM(0),
		(void*)PARAM(1),
		(size_t)PARAM(2),
		(INT)PARAM(3),
		(struct sockaddr*)PARAM(4),
		(socklen_t *)PARAM(5)
		);
}

static void   SC_SendTo(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_sendto(
		(INT)PARAM(0),
		(const void*)PARAM(1),
		(size_t)PARAM(2),
		 (INT)PARAM(3),
		(const struct sockaddr*)PARAM(4),
		(socklen_t)PARAM(5)
		);
}

static void   SC_Write(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_write(
		(INT)PARAM(0),
		(const void*)PARAM(1),
		(size_t)PARAM(2)
		);
}

static void   SC_Select(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_select(
		(INT)PARAM(0),
		(fd_set*)PARAM(1),
		(fd_set*)PARAM(2),
		(fd_set*)PARAM(3),
		(struct timeval*)PARAM(4)
		);
}


static void   SC_SetSocketOpt(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_setsockopt(
		(INT)PARAM(0),
		(int)PARAM(1),
		(int)PARAM(2),
		(const void*)PARAM(3),
		(socklen_t)PARAM(4)
		);
}

static void   SC_GetSocketOpt(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_getsockopt(
		(INT)PARAM(0),
		(int)PARAM(1),
		(int)PARAM(2),
		(void*)PARAM(3),
		(socklen_t*)PARAM(4)
		);
}

static void   SC_GetHostByName(__SYSCALL_PARAM_BLOCK*  pspb)
{
	
}


static void   SC_Ioctl(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_ioctl(
		                      (INT)PARAM(0),
		                      (long)PARAM(1),
		                      (void*)PARAM(2));
}
static void   SC_Fcntl(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_fcntl(
		                       (INT)PARAM(0),
		                       (INT)PARAM(1),
		                       (INT)PARAM(2));
}


static void   SC_Accept(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_accept((INT)PARAM(0),(struct sockaddr *)PARAM(1),(socklen_t*)PARAM(2));
}

static void   SC_Listen(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_listen((INT)PARAM(0),(INT)PARAM(1));
}

static void   SC_CloseSocket(__SYSCALL_PARAM_BLOCK*  pspb)
{
	pspb->lpRetValue = (LPVOID)lwip_close((INT)PARAM(0));
}


void  RegisterSocketEntry(SYSCALL_ENTRY* pSysCallEntry)
{
	pSysCallEntry[SYSCALL_SOCKET]          = SC_Socket;
	pSysCallEntry[SYSCALL_BIND]            = SC_Bind;
	pSysCallEntry[SYSCALL_ACCEPT]          = SC_Accept;
	pSysCallEntry[SYSCALL_CONNECT]         = SC_Connect;

	pSysCallEntry[SYSCALL_SETSOCKET]       = SC_SetSocketOpt;
	pSysCallEntry[SYSCALL_GETSOCKET]       = SC_GetSocketOpt;

	pSysCallEntry[SYSCALL_LISTEN]          = SC_Listen;
	pSysCallEntry[SYSCALL_SELECT]          = SC_Select;
	pSysCallEntry[SYSCALL_CLOSESOCKET]     = SC_CloseSocket;

	pSysCallEntry[SYSCALL_SEND]            = SC_Send;
	pSysCallEntry[SYSCALL_SENDTO]          = SC_SendTo;
	
	pSysCallEntry[SYSCALL_RECV]            = SC_Recv;
	pSysCallEntry[SYSCALL_RECVFROM]        = SC_RecvFrom;

}
