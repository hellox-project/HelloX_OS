
#ifndef	__SSH_CLIENT_H_
#define __SSH_CLIENT_H_

#define  SSH_ERROR_NO			 0
#define  SSH_ERROR_NETWORK		 1
#define  SSH_ERROR_STEP1		 1
#define  SSH_ERROR_STEP2         2
#define  SSH_ERROR_STEP3         3
#define  SSH_ERROR_STEP4         4
#define  SSH_ERROR_STEP5         5
#define  SSH_ERROR_AUTH          6  //username or password error
#define  SSH_ERROR_STEP7         7
#define  SSH_ERROR_STEP8         8

#define  SSH_ERROR               -1
#define  SSH_SVR_TEXT            100
#define  SSH_USER_LOGOUT         101
#define  SSH_REQ_ACCOUNT         102
#define  SSH_NETWORK_ERROR       103
#define  SSH_AUTH_ERROR          104
#define  SSH_OTHER_ERROR         105

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	
	typedef int (* ssh_callbk)(unsigned int code,const char* txtbuf,unsigned int len,unsigned int wp);

	void* ssh_new_seesion(char* server,int port,ssh_callbk pcall,unsigned int wparam);
	
	int   ssh_set_terminal(void* p,int rows,int cols);

	int   ssh_start_seesion(void* p);

	int   ssh_set_account(void* p,char* user,char* passwd);

	int   ssh_get_errid(void* p);
		
	void  ssh_free_seesion(void* p);

	int   ssh_send_msg(void* p,const char* buf,int len);

	int   ssh_recv_msg(void* p,char* buf,int len);
	

#ifdef __cplusplus
}
#endif


#endif //
