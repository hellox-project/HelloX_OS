//***********************************************************************/
//    Author                    : twind
//    Original Date             : oct,21 2015
//    Module Name               : k_socket.CPP
//    Module Funciton           : 
//                                All socket in kernel module are wrapped
//                                in this file.
//
//    Lines number              :
//***********************************************************************/


#include "K_SOCKET.H"


int accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
	SYSCALL_PARAM_3(SYSCALL_ACCEPT,s,addr,	addrlen);

}

int bind(int s, const struct sockaddr *name, socklen_t namelen)
{
	SYSCALL_PARAM_3(SYSCALL_BIND,s,name,	namelen);
}

int getsockopt (int s, int level, int optname, void *optval, socklen_t *optlen)
{
	SYSCALL_PARAM_5(SYSCALL_GETSOCKET,s,level,	optname,optval,optlen);
}

int setsockopt (int s, int level, int optname, const void *optval, socklen_t optlen)
{
	SYSCALL_PARAM_5(SYSCALL_SETSOCKET,s,level,	optname,optval,optlen);
}

int close(int s)
{
	SYSCALL_PARAM_1(SYSCALL_CLOSESOCKET,s);
}

int connect(int s, const struct sockaddr *name, socklen_t namelen)
{
	SYSCALL_PARAM_3(SYSCALL_CONNECT,s,name,namelen);
}

int listen(int s, int backlog)
{
	SYSCALL_PARAM_2(SYSCALL_LISTEN,s,backlog);
}

int recv(int s, void *mem, size_t len, int flags)
{
	SYSCALL_PARAM_4(SYSCALL_RECV,s,mem,len,flags);
}

int read(int s, void *mem, size_t len)
{
	SYSCALL_PARAM_3(SYSCALL_RECV,s,mem,len,0);
}

int recvfrom(int s, void *mem, size_t len, int flags,struct sockaddr *from, socklen_t *fromlen)
{
	SYSCALL_PARAM_6(SYSCALL_RECVFROM,s,mem,len,flags,from,fromlen);
}

int send(int s, const void *dataptr, size_t datasize,int flags)
{
	SYSCALL_PARAM_4(SYSCALL_SEND,s,dataptr,datasize,flags);
}

int sendto(int s, const void *dataptr, size_t datasize, int flags,	const struct sockaddr *to, socklen_t tolen)
{
	SYSCALL_PARAM_6(SYSCALL_SENDTO,s,dataptr,datasize,flags,to,tolen);
}

int socket(int domain, int ntype, int protocol)
{
	SYSCALL_PARAM_3(SYSCALL_SOCKET,domain,ntype,protocol);	
	
}

int write(int s, const void *dataptr, size_t datasize)
{
	//SYSCALL_PARAM_4(SYSCALL_SEND,s,dataptr,datasize,1);
}

int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset,struct timeval *timeout)
{
	SYSCALL_PARAM_5(SYSCALL_SELECT,maxfdp1,readset,writeset,exceptset,timeout);
}

int ioctl(int s, long cmd, void *argp)
{
	SYSCALL_PARAM_3(SYSCALL_IOCONTROL,s,cmd,argp);
}

int fcntl(int s, int cmd, int val)
{
	return 0;
	//SYSCALL_PARAM_3(SYSCALL_FCN,s,cmd,val);
}