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
//#define __PARAM_0 edi
//#define __PARAM_1 esi
//#define __PARAM_2 edx
//#define __PARAM_3 ecx
//#define __PARAM_4 ebx

/*
 * Disable of showing out the C4731 warning.
 * Inline assembly is used in syscall's source
 * code, and EBP is modified but properly saved,
 * so supress the warning is clear.
 */
#if defined(_MSC_VER) && !defined(__clang__)
#pragma warning(disable:4731)
#endif

 /* New a socket and return the index. */
int lwip_socket(int _domain, int _type, int _protocol)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, _domain
		mov esi, _type
		mov edx, _protocol
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (_domain), "m" (_type), "m" (_protocol), "i" (SYSCALL_SOCKET), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
	return ret_val;
}

/* Close a socket. */
int lwip_close(int s)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push ebp
		push eax
		mov edi, s
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CLOSESOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "i" (SYSCALL_CLOSESOCKET), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "ebp"
	);
#endif
	return ret_val;
}

/* Bind a socket. */
int lwip_bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, s
		mov esi, name
		mov edx, namelen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_BIND
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (name), "m" (namelen), "i" (SYSCALL_BIND), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
	return ret_val;
}

/* Connect to server. */
int lwip_connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, s
		mov esi, name
		mov edx, namelen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_CONNECT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (name), "m" (namelen), "i" (SYSCALL_CONNECT), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
	return ret_val;
}

/* Socket selection. */
int lwip_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,
	struct timeval *timeout)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, maxfdp1
		mov esi, readset
		mov edx, writeset
		mov ecx, exceptset
		mov ebx, timeout
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SELECT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"movl %5, %%ebx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %6, %%eax\n\t"
		"int %7\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (maxfdp1), "m" (readset), "m" (writeset), "m" (exceptset), "m" (timeout), "i" (SYSCALL_SELECT), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebx", "ebp"
	);
#endif
	return ret_val;
}

/* Set socket option. */
int lwip_setsockopt(int s, int level, int optname, const void *optval, socklen_t optlen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, s
		mov esi, level
		mov edx, optname
		mov ecx, optval
		mov ebx, optlen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SETSOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"movl %5, %%ebx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %6, %%eax\n\t"
		"int %7\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (level), "m" (optname), "m" (optval), "m" (optlen), "i" (SYSCALL_SETSOCKET), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebx", "ebp"
	);
#endif
	return ret_val;
}

/* Get socket option. */
int lwip_getsockopt(int s, int level, int optname, void *optval, socklen_t* optlen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, s
		mov esi, level
		mov edx, optname
		mov ecx, optval
		mov ebx, optlen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_GETSOCKET
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"movl %5, %%ebx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %6, %%eax\n\t"
		"int %7\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (level), "m" (optname), "m" (optval), "m" (optlen), "i" (SYSCALL_GETSOCKET), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebx", "ebp"
	);
#endif
	return ret_val;
}

/* Receive from socket. */
int lwip_recv(int s, void *mem, size_t len, int flags)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebp
		push eax
		mov edi, s
		mov esi, mem
		mov edx, len
		mov ecx, flags
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_RECV
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %5, %%eax\n\t"
		"int %6\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (mem), "m" (len), "m" (flags), "i" (SYSCALL_RECV), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebp"
	);
#endif
	return ret_val;
}

/* Alias of recv without flags. */
int lwip_read(int s, void *mem, size_t len)
{
	return lwip_recv(s, mem, len, 0);
}

/* Send to socket. */
int lwip_send(int s, const void *dataptr, size_t len, int flags)
{
#if defined(_MSC_VER) && !defined(__clang__)
	int ret_val = 0;
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebp
		push eax
		mov edi, s
		mov esi, dataptr
		mov edx, len
		mov ecx, flags
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SEND
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ecx
		pop edx
		pop esi
		pop edi
	}
	return ret_val;
#elif defined(__clang__) || defined(__GNUC__)
	int ret_val = 0;
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %5, %%eax\n\t"
		"int %6\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (dataptr), "m" (len), "m" (flags), "i" (SYSCALL_SEND), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebp"
	);
	return ret_val;
#endif
}

/* Alias of send without flags. */
int lwip_write(int s, const void *dataptr, size_t size)
{
	return lwip_send(s, dataptr, size, 0);
}

/* Shutdown socket. */
int lwip_shutdown(int s, int how)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, s
		mov esi, how
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SHUTDOWN
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}

#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (how), "i" (SYSCALL_SHUTDOWN), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "ebp"
	);
#endif

	return ret_val;
}

int lwip_listen(int s, int backlog)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push ebp
		push eax
		mov edi, s
		mov esi, backlog
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_LISTEN
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop esi
		pop edi
	}

#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %3, %%eax\n\t"
		"int %4\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (backlog), "i" (SYSCALL_LISTEN), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "ebp"
	);
#endif

	return ret_val;
}

int lwip_accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, s
		mov esi, addr
		mov edx, addrlen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_ACCEPT
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (addr), "m" (addrlen), "i" (SYSCALL_ACCEPT), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
	return ret_val;
}

/* Get socket local addr. */
int lwip_getsockname(int s, struct sockaddr *name, socklen_t *namelen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, s
		mov esi, name
		mov edx, namelen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_GETSOCKNAME
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (name), "m" (namelen), "i" (SYSCALL_GETSOCKNAME), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
	return ret_val;
}

/* Get socket peer addr. */
int lwip_getpeername(int s, struct sockaddr *name, socklen_t *namelen)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, s
		mov esi, name
		mov edx, namelen
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_GETPEERNAME
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (s), "m" (name), "m" (namelen), "i" (SYSCALL_GETPEERNAME), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
	return ret_val;
}

/* Send data to a target socket. */
int lwip_sendto(int s, const void *dataptr, size_t size, int flags,
	const struct sockaddr *to, socklen_t tolen)
{
	__SYSCALL_PARAM_EXTENSION_BLOCK ext_block;
	__SYSCALL_PARAM_EXTENSION_BLOCK* pext_block = &ext_block;
	int ret_val = -1;

	/* Init extension param block. */
	ext_block.ext_param0 = (uint32_t)s;
	ext_block.ext_param1 = (uint32_t)dataptr;
	ext_block.ext_param2 = (uint32_t)size;
	ext_block.ext_param3 = (uint32_t)flags;
	ext_block.ext_param4 = (uint32_t)to;
	ext_block.ext_param5 = (uint32_t)tolen;

	/* Launch the system call. */
#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, 0
		mov esi, 0
		mov edx, 0
		mov ecx, 0
		mov ebx, pext_block
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_SENDTO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl $0, %%edi\n\t"
		"movl $0, %%esi\n\t"
		"movl $0, %%edx\n\t"
		"movl $0, %%ecx\n\t"
		"movl %1, %%ebx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (pext_block), "i" (SYSCALL_SENDTO), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebx", "ebp"
	);
#endif
	return ret_val;
}

/* Receive data from a target socket. */
int lwip_recvfrom(int s, void *mem, size_t len, int flags,
	struct sockaddr *from, socklen_t *fromlen)
{
	__SYSCALL_PARAM_EXTENSION_BLOCK ext_block;
	__SYSCALL_PARAM_EXTENSION_BLOCK* pext_block = &ext_block;
	int ret_val = -1;

	/* Init extension param block. */
	ext_block.ext_param0 = (uint32_t)s;
	ext_block.ext_param1 = (uint32_t)mem;
	ext_block.ext_param2 = (uint32_t)len;
	ext_block.ext_param3 = (uint32_t)flags;
	ext_block.ext_param4 = (uint32_t)from;
	ext_block.ext_param5 = (uint32_t)fromlen;

	/* Launch the system call. */
#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, 0
		mov esi, 0
		mov edx, 0
		mov ecx, 0
		mov ebx, pext_block
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_RECVFROM
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl $0, %%edi\n\t"
		"movl $0, %%esi\n\t"
		"movl $0, %%edx\n\t"
		"movl $0, %%ecx\n\t"
		"movl %1, %%ebx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (pext_block), "i" (SYSCALL_RECVFROM), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebx", "ebp"
	);
#endif
	return ret_val;
}

/* Get all genif's information from system. */
int GetGenifInfo(__GENERIC_NETIF* pGenifBlock, unsigned long* buf_req,
	unsigned long ulFlags)
{
	int ret_val = 0;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ebp
		push eax
		mov edi, pGenifBlock
		mov esi, buf_req
		mov edx, ulFlags
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_GETGENIFINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %4, %%eax\n\t"
		"int %5\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (pGenifBlock), "m" (buf_req), "m" (ulFlags), "i" (SYSCALL_GETGENIFINFO), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ebp"
	);
#endif
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

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push ebp
		push eax
		mov edi, genif_index
		mov esi, protocol
		mov edx, comm_addr
		mov ecx, addr_num
		mov ebx, bSecondary
		lea eax, ret_val
		mov ebp, eax
		mov eax, SYSCALL_ADDGENIFADDRESS
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%esi\n\t"
		"pushl %%edx\n\t"
		"pushl %%ecx\n\t"
		"pushl %%ebx\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"movl %2, %%esi\n\t"
		"movl %3, %%edx\n\t"
		"movl %4, %%ecx\n\t"
		"movl %5, %%ebx\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %6, %%eax\n\t"
		"int %7\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%ebx\n\t"
		"popl %%ecx\n\t"
		"popl %%edx\n\t"
		"popl %%esi\n\t"
		"popl %%edi\n\t"
		: "+m" (ret_val)
		: "m" (genif_index), "m" (protocol), "m" (comm_addr), "m" (addr_num), "m" (bSecondary), "i" (SYSCALL_ADDGENIFADDRESS), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "esi", "edx", "ecx", "ebx", "ebp"
	);
#endif
	return ret_val;
}

/* Get network info from system. */
BOOL GetNetworkInfo(__SYSTEM_NETWORK_INFO* pNetInfo)
{
	BOOL bRet = FALSE;

#if defined(_MSC_VER) && !defined(__clang__)
	__asm {
		push edi
		push ebp
		push eax
		mov edi, pNetInfo
		lea eax, bRet
		mov ebp, eax
		mov eax, SYSCALL_GETNETWORKINFO
		int SYSCALL_VECTOR
		pop eax
		pop ebp
		pop edi
	}
#elif defined(__clang__) || defined(__GNUC__)
	__asm__ __volatile__(
		"pushl %%edi\n\t"
		"pushl %%ebp\n\t"
		"pushl %%eax\n\t"
		"movl %1, %%edi\n\t"
		"leal %0, %%eax\n\t"
		"movl %%eax, %%ebp\n\t"
		"movl %2, %%eax\n\t"
		"int %3\n\t"
		"popl %%eax\n\t"
		"popl %%ebp\n\t"
		"popl %%edi\n\t"
		: "+m" (bRet)
		: "m" (pNetInfo), "i" (SYSCALL_GETNETWORKINFO), "i" (SYSCALL_VECTOR)
		: "memory", "eax", "edi", "ebp"
	);
#endif
	return bRet;
}
