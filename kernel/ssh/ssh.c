

//#ifdef WIN32
//	#include <Winsock2.h>
//#endif

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif


#include "ssh_def.h"
#include "ssh/ssh.h"

#define  MEM_HEAD   4

void*  ssh_new(int size)
{
	uint32*   p = (uint32*)_hx_malloc(size+MEM_HEAD);
	
	memset(p,0,size+MEM_HEAD);
	*p = size;
	 p ++; 

	return p;
	
}

void*  ssh_rnew(void* p,int nsize)
{
	uint32*   np = NULL;

	np = ssh_new(nsize);
	if(p )
	{
		uint32*   op     = (uint32*)p;
		int    cpsize = 0;

		op --; cpsize  = *op;
				
		if(cpsize > nsize) 	cpsize = nsize;
		
		memcpy(np,p,cpsize);
		ssh_free(p);
	}

	return np;
}


void  ssh_free(void* p)
{
	uint32*   op = (uint32*)p;

	if(op)
	{
		op--;
		_hx_free(op);	
	}
}


DWORD  ssh_recv_thread(LPVOID p)
{
	ssh_session*  ssh_obj   = (ssh_session*)p;
	ssh_callbk    pcall     = (ssh_callbk)ssh_obj->pcb;
	uint8*        recvbuf   = NULL;
	
	recvbuf = (uint8*)ssh_new(SSH_NETBUF_LEN);
	while(1)
	{	 		
		int  recvlen      = 0;
		int  ret          = S_OK;

		recvlen = recv(ssh_obj->sd,recvbuf,SSH_NETBUF_LEN,0);
		if(recvlen == 0) 
		{
			//PrintLine("recv is null");
			continue;
		}

		if(recvlen < 0)
		{
			ssh_obj->error_id = SSH_ERROR_NETWORK;
			_hx_printf("SSH2:recv thread exit\r\n",recvlen);
			break;
		}
		else
		{			
			ret = ssh_msg_analyze(ssh_obj,(uint8*)recvbuf,recvlen);			
			switch(ret)
			{
				case S_FALSE: 
					{
					//exec error
					_hx_printf("SSH2:ssh_msg_analyze faild r\n");
					pcall(SSH_ERROR,NULL,0,ssh_obj->wp);
					goto _EXIT;
					}
					break;
				case S_OK:
					{
					}
					break;
				case S_PKT_ERR:
					{					
					}
					break;
				case S_LOGOUT:
					{					
					goto _EXIT;
					}
					break;			
			}
			
		}	
	}

_EXIT:

	ssh_free(recvbuf);
	
	return S_OK;
}

void* ssh_new_seesion(char* server,int port,ssh_callbk pcall,unsigned int wparam)
{
	ssh_session*  ssh_obj;

	ssh_obj = ssh_new(sizeof(ssh_session));

	ssh_obj->pcb = pcall;
	ssh_obj->wp  = wparam;

	strncpy(ssh_obj->address,server,sizeof(ssh_obj->address));
	ssh_obj->port = port;
	
	return ssh_obj;
}

int   ssh_set_terminal(void* p,int rows,int cols)
{
	ssh_session*  ssh_obj = (ssh_session*)p;

	ssh_obj->term_rows = rows;
	ssh_obj->term_cols = cols;

	return S_OK;
}

int ssh_start_seesion(void* p)
{
	ssh_session*  ssh_obj = (ssh_session*)p;
	struct sockaddr_in sin;		
	int          len = sizeof(sin);
	int          ot  = 5000;
	
	
	ssh_obj->sd       = (int)socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family    = AF_INET;
	sin.sin_port       = htons(ssh_obj->port);
	sin.sin_addr.s_addr   = inet_addr(ssh_obj->address);
	if(connect(ssh_obj->sd, (struct sockaddr *)&sin, len) == -1) 
	{		
		ssh_obj->error_id = SSH_ERROR_NETWORK;
		return FALSE;
	}

	//setsockopt(ssh_obj->sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ot, sizeof(ot));
	//setsockopt(ssh_obj->sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ot, sizeof(ot));
	
	KernelThreadManager.CreateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
											0,
											KERNEL_THREAD_STATUS_READY,PRIORITY_LEVEL_HIGH,
											ssh_recv_thread,p,
											NULL,"SSH");
	

	
	return  TRUE;
}

int   ssh_set_account(void* p,char* user,char* passwd)
{
	ssh_session*  ssh_obj = (ssh_session*)p;

	strncpy(ssh_obj->username,user,sizeof(ssh_obj->username));
	strncpy(ssh_obj->password,passwd,sizeof(ssh_obj->password));
	
	return TRUE;
}

int   ssh_send_msg(void* p,const char* buf,int len)
{
	ssh_session*  ssh_obj = (ssh_session*)p;

	if(ssh_obj->error_id != SSH_ERROR_NO)
	{
		return S_FALSE;
	}

	if(ssh_obj )
	{
		ssh_send_text(ssh_obj,buf,len);
	}
	
	return  S_OK;
}

int   ssh_get_errid(void* p)
{
	ssh_session*  ssh_obj = (ssh_session*)p;

	if(ssh_obj)
	{
		return ssh_obj->error_id;
	}
	
	return 0;
}

void  ssh_free_seesion(void* p)
{
	ssh_session*  ssh_obj = (ssh_session*)p;

	if(ssh_obj)
	{		
		//关闭会出错，目前不知道原因
		//closesocket(ssh_obj->sd);		
		ssh_free(ssh_obj);			
	}
}