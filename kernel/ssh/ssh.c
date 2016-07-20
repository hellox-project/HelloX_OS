

//#ifdef WIN32
//	#include <Winsock2.h>
//#endif

#ifndef __STDAFX_H__
#include <StdAfx.h>
#endif


#include "ssh_def.h"
#include "ssh/ssh.h"

#define  MEM_HEAD     4

//#define  MEMORY_CHECK

__KERNEL_THREAD_OBJECT*  s_pSsh_Recv_ThreadObj = NULL;

#ifdef MEMORY_CHECK
#define  MEM_ALOC_COUNT        1024
static  uint32  s_obj_cnt        = MEM_ALOC_COUNT;  
static  uint32  s_obj[MEM_ALOC_COUNT]      = {0};
static  uint32  s_obj_setp[MEM_ALOC_COUNT] = {0};
extern int  s_cur_step;
void  reg_mem_obj(void*  p,uint32 len)
{
	uint32 addr = (uint32)p;
	int i;

	for(i=0;i<s_obj_cnt;i++)
	{
		if(s_obj[i] == 0)
		{
			s_obj[i] = addr;
			s_obj_setp[i] = len;
			break;
		}
	}
}
void  unreg_mem_obj(void*  p)
{
	uint32 addr = (uint32)p;
	int i;

	for(i=0;i<s_obj_cnt;i++)
	{
		if(s_obj[i] == addr)
		{
			s_obj[i] = 0;
			break;
		}
	}
}
void  show_mem_obj()
{
int i;
	for(i=0;i<s_obj_cnt;i++)
	{
		if(s_obj[i] != 0)
		{
		_hx_printf("memobj left =%X,len=%X\r\n",s_obj[i],s_obj_setp[i]);
		}
	}

}

#endif //MEMORY_CHECK

void*  ssh_new(int size)
{
	uint32*   p = (uint32*)_hx_malloc(size+MEM_HEAD);
	
#ifdef MEMORY_CHECK
	reg_mem_obj(p,size);
#endif //MEMORY_CHECK

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
		#ifdef MEMORY_CHECK
			unreg_mem_obj(op);
		#endif //MEMORY_CHECK

		
		_hx_free(op);		
	}
}


DWORD  ssh_recv_thread(LPVOID p)
{
	__KERNEL_THREAD_MESSAGE msg = {0};
	ssh_session*  ssh_obj   = (ssh_session*)p;
	ssh_callbk    pcall     = (ssh_callbk)ssh_obj->pcb;
	uint8*        recvbuf   = NULL;
	int           ret       = S_OK;
	
	
	recvbuf = (uint8*)ssh_new(SSH_NETBUF_LEN);
	while(1)
	{	 		
		int  recvlen = recv(ssh_obj->sd,recvbuf,SSH_NETBUF_LEN,0);
		if(recvlen == 0) 
		{			
			continue;
		}

		if(recvlen < 0)
		{
			ssh_obj->error_id = SSH_ERROR_NETWORK;
			pcall(SSH_NETWORK_ERROR,NULL,0,ssh_obj->wp);			
			break;
		}
		else
		{			
			ret = ssh_msg_analyze(ssh_obj,(uint8*)recvbuf,recvlen);		
			if(ret != S_OK && ret != S_NEED_MORE_DATA) 
			{				
				break;
			}
			
		}	
	}

	if(ret == S_OTHER_ERR)
	{
		ssh_callbk    pcall  = (ssh_callbk)ssh_obj->pcb;

		pcall(SSH_OTHER_ERROR,NULL,0,ssh_obj->wp);	
	}

	//notify shell  exit ssh
	closesocket(ssh_obj->sd);
	msg.wCommand = KERNEL_MESSAGE_AKEYDOWN;
	KernelThreadManager.SendMessage((__COMMON_OBJECT*)g_lpShellThread,&msg);

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
	
	
	ssh_obj->sd           = (int)socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family        = AF_INET;
	sin.sin_port          = htons(ssh_obj->port);
	sin.sin_addr.s_addr   = inet_addr(ssh_obj->address);
	if(connect(ssh_obj->sd, (struct sockaddr *)&sin, len) == -1) 
	{		
		closesocket(ssh_obj->sd);
		ssh_obj->error_id = SSH_ERROR_NETWORK;
		return FALSE;
	}

	//setsockopt(ssh_obj->sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ot, sizeof(ot));
	//setsockopt(ssh_obj->sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ot, sizeof(ot));
	
	s_pSsh_Recv_ThreadObj = KernelThreadManager.CreateKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
											0,
											KERNEL_THREAD_STATUS_READY,
											PRIORITY_LEVEL_HIGH,
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

		if (ssh_obj->sc_cipher_ctx)
		{
			ssh_obj->sccipher->free_context(ssh_obj->sc_cipher_ctx);
		}

		if (ssh_obj->cs_cipher_ctx)
		{
			ssh_obj->cscipher->free_context(ssh_obj->cs_cipher_ctx);
		}		
		if(ssh_obj->cs_mac_ctx)
		{
			ssh_obj->csmac->free_context(ssh_obj->cs_mac_ctx);
		}

		if(ssh_obj->cs_comp_ctx)
		{
			ssh_obj->cscomp->compress_cleanup(ssh_obj->cs_comp_ctx);
		}
		if (ssh_obj->sc_mac_ctx)
		{
			ssh_obj->scmac->free_context(ssh_obj->sc_mac_ctx);
		}
		if (ssh_obj->sc_comp_ctx)
		{
			ssh_obj->sccomp->decompress_cleanup(ssh_obj->sc_comp_ctx);
		}

		ssh_free(ssh_obj->hostkey_str);
		ssh_free(ssh_obj->deferred_send_data);		
		ssh_free(ssh_obj->mainchan);
		ssh_free(ssh_obj->v_s);
		ssh_free(ssh_obj->v_c );				
		ssh_free(ssh_obj->trans_state);			
		ssh_free(ssh_obj);		


		if(s_pSsh_Recv_ThreadObj)
		{
			s_pSsh_Recv_ThreadObj->WaitForThisObject((__COMMON_OBJECT*)s_pSsh_Recv_ThreadObj);

			KernelThreadManager.DestroyKernelThread((__COMMON_OBJECT*)&KernelThreadManager,
				(__COMMON_OBJECT*)s_pSsh_Recv_ThreadObj);
			s_pSsh_Recv_ThreadObj = NULL;	
		}
			

		#ifdef MEMORY_CHECK
			show_mem_obj();
		#endif 
		
	}
}