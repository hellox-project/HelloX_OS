//***********************************************************************/
//    Author                    : tywind
//    Original Date             : 15 MAY,2016
//    Module Name               : telnet2.c
//    Module Funciton           : 
//    Description               : Implementation code of telnet application.
//    Last modified Author      :
//    Last modified Date        : 
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//    Extra comment             : 
//***********************************************************************/

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"

#include "telnet2.h"



static const struct Opt o_naws =
{ WILL, WONT, DO, DONT, TELOPT_NAWS, OPTINDEX_NAWS, REQUESTED };
static const struct Opt o_tspeed =
{ WILL, WONT, DO, DONT, TELOPT_TSPEED, OPTINDEX_TSPEED, REQUESTED };
static const struct Opt o_ttype =
{ WILL, WONT, DO, DONT, TELOPT_TTYPE, OPTINDEX_TTYPE, REQUESTED };
static const struct Opt o_oenv =
{ WILL, WONT, DO, DONT, TELOPT_OLD_ENVIRON, OPTINDEX_OENV, INACTIVE };
static const struct Opt o_nenv =
{ WILL, WONT, DO, DONT, TELOPT_NEW_ENVIRON, OPTINDEX_NENV, REQUESTED };
static const struct Opt o_echo =
{ DO, DONT, WILL, WONT, TELOPT_ECHO, OPTINDEX_ECHO, REQUESTED };
static const struct Opt o_we_sga =
{ WILL, WONT, DO, DONT, TELOPT_SGA, OPTINDEX_WE_SGA, REQUESTED };
static const struct Opt o_they_sga =
{ DO, DONT, WILL, WONT, TELOPT_SGA, OPTINDEX_THEY_SGA, REQUESTED };
static const struct Opt o_we_bin =
{ WILL, WONT, DO, DONT, TELOPT_BINARY, OPTINDEX_WE_BIN, INACTIVE };
static const struct Opt o_they_bin =
{ DO, DONT, WILL, WONT, TELOPT_BINARY, OPTINDEX_THEY_BIN, INACTIVE };

static const struct Opt *const opts[] = {
	&o_naws, &o_tspeed, &o_ttype, &o_oenv, &o_nenv, &o_echo,
	&o_we_sga, &o_they_sga, &o_we_bin, &o_they_bin, NULL
};



void send_opt(int sock, int cmd, int option)
{
	unsigned char buf[3];

	buf[0] = IAC;
	buf[1] = cmd;
	buf[2] = option;
		
	send(sock,buf,sizeof(buf),0);
}

void proc_rec_opt(telnet_user*  ctl, int cmd, int option)
{
	const struct Opt *const *o;

	for (o = opts; *o; o++) 
	{
		if ((*o)->option == option && (*o)->ack == cmd) 
		{
			switch (ctl->opt_states[(*o)->index]) 

			{
			case REQUESTED:
				ctl->opt_states[(*o)->index] = ACTIVE;
				//activate_option(telnet, *o);
				break;
			case ACTIVE:
				break;
			case INACTIVE:
				ctl->opt_states[(*o)->index] = ACTIVE;
				send_opt(ctl->sd, (*o)->send, option);
				//activate_option(telnet, *o);
				break;
			case REALLY_INACTIVE:
				send_opt(ctl->sd, (*o)->nsend, option);
				break;
			}
			return;
		}
		else if ((*o)->option == option && (*o)->nak == cmd) 
		{
			switch (ctl->opt_states[(*o)->index]) 
			{
			case REQUESTED:
				ctl->opt_states[(*o)->index] = INACTIVE;
				//refused_option(telnet, *o);
				break;
			case ACTIVE:
				ctl->opt_states[(*o)->index] = INACTIVE;
				send_opt(ctl->sd, (*o)->nsend, option);
				//option_side_effects(telnet, *o, 0);
				break;
			case INACTIVE:
			case REALLY_INACTIVE:
				break;
			}
			return;
		}
	}
	/*
	* If we reach here, the option was one we weren't prepared to
	* cope with. If the request was positive (WILL or DO), we send
	* a negative ack to indicate refusal. If the request was
	* negative (WONT / DONT), we must do nothing.
	*/
	if (cmd == WILL || cmd == DO)
	{
		send_opt(ctl->sd, (cmd == WILL ? DONT : WONT), option);
	}
}

//ff fa 27 00 ff f0
int telnet_send_subneg(telnet_user*  ctl)
{	 
	char buf[128] = {0};
	int  len      = 0;

	switch(ctl->sb_opt)
	{
	case TELOPT_NEW_ENVIRON:
		{
			buf[0] = IAC;
			buf[1] = SB;
			buf[2] = ctl->sb_opt;
			buf[3] = TELQUAL_IS;
			buf[4] = IAC;
			buf[5] = SE;
			len    = 6;				
		}
		break;
	case TELOPT_TTYPE:
		{
			buf[0] = IAC;
			buf[1] = SB;
			buf[2] = TELOPT_TTYPE;
			buf[3] = TELQUAL_IS;
			strcpy(&buf[4],"xterm");
			buf[9] = IAC;
			buf[10] = SE;
			len     = 11;				
		}
		break;
	}

	if(len > 0)
	{
		send(ctl->sd,buf,len,0);
	}

	return len;
}

int telnet_recv_msg(telnet_user*  ctl,char* outbuf,int len)
{	
	char   recvbuf[TBL]  = {0};
	char*  strpos        = NULL;
	int    recvlen       = 0;
	int    msglen        = 0;

	
	recvlen = recv(ctl->sd,recvbuf,sizeof(recvbuf),0);
	
	if(recvlen <= 0)
	{
		return -1;
	}
	strpos = recvbuf;

	while(recvlen > 0 )
	{	
		int c;

		c   = (unsigned char)*strpos;

		switch (ctl->state) 
		{
		case TOP_LEVEL:
		case SEENCR:
			{
				if (c == 0 && ctl->state == SEENCR)
				{
					ctl->state = TOP_LEVEL;
				}
				else if (c == IAC)
				{
					ctl->state = SEENIAC;
				}
				else if(msglen < len)
				{
					outbuf[msglen ++] = c;
				}
			}

			break;
		case SEENIAC:
			if (c == DO)
				ctl->state = SEENDO;
			else if (c == DONT)
				ctl->state = SEENDONT;
			else if (c == WILL)
				ctl->state = SEENWILL;
			else if (c == WONT)
				ctl->state = SEENWONT;
			else if (c == SB)
				ctl->state = SEENSB;
			else if (c == DM) 
			{				 
				ctl->state = TOP_LEVEL;
			}
			else
			{
				ctl->state = TOP_LEVEL;
			}
			break;
		case SEENWILL:
			proc_rec_opt(ctl, WILL, c);
			ctl->state = TOP_LEVEL;
			break;
		case SEENWONT:
			proc_rec_opt(ctl, WONT, c);
			ctl->state = TOP_LEVEL;
			break;
		case SEENDO:
			proc_rec_opt(ctl, DO, c);
			ctl->state = TOP_LEVEL;
			break;
		case SEENDONT:
			proc_rec_opt(ctl, DONT, c);
			ctl->state = TOP_LEVEL;
			break;
		case SEENSB:
			ctl->sb_opt = c;
			ctl->state  = SUBNEGOT;
			break;
		case SUBNEGOT:
			if (c == IAC)
			{
				ctl->state = SUBNEG_IAC;
			}
			break;
		case SUBNEG_IAC:
			if (c == SE)
			{				 
				telnet_send_subneg(ctl);
				ctl->state = TOP_LEVEL;
			}
			break;
		}

		strpos  ++;
		recvlen --;
	}

	return msglen;
}

int telnet_recv_server_answer(telnet_user*  ctl)
{
	int     ret = 0;

	while(1)
	{		 
		char   recvbuf[TBL+1] = {0};
		int    len            = 0;

		len = telnet_recv_msg(ctl,recvbuf,TBL);	
		if(len >= 0)
		{	
			 
			 ctl->cb(TELMSG_RESPONSE_OK,recvbuf);

			//选项交互已经通过，出来了输入提示符，可以开始交互了
			if(strstr(recvbuf,":"))
			{
				ret = 1;
				break;
			}
		}
		else
		{
			break;
		}
	}

	return ret;
}


/*int  telnet_recv_section(telnet_user*  ctl ,const char* p)
{
	int  ret = 0;

	while(1)
	{		 
		char   recvbuf[TELNET_BUF_LEN] = {0};
		int    len                     = 0;

		len = telnet_recv_msg(ctl,recvbuf);	
		if(len > 0)
		{						
			if(strstr(recvbuf,p))
			{
				ret = 1;

			}
			PrintTelnetStr(recvbuf,FALSE);
			break;
		}	

		if(len == -1)
		{
			continue;
		}

	}

	return ret;
}*/


int telnet_connect(telnet_user*  ctl)
{
	struct sockaddr_in sin;	
	const struct Opt *const *o;
	unsigned char   opt[64] = {0};
	int             pos    = 0;
	int          len = sizeof(sin);
	int          ret = 1;
	//int          ot  = 5000;
	//int          i,j;

	memset(&sin, 0,sizeof(sin));

	ctl->sd               = (int)socket(AF_INET, SOCK_STREAM, 0);
	sin.sin_family        = AF_INET;	
	sin.sin_addr.s_addr   = inet_addr(ctl->address);
	sin.sin_port          = htons(ctl->port);

	if(connect(ctl->sd, (struct sockaddr *)&sin, len) == -1) 
	{		
		ctl->cb(TELMSG_CONNECT_FAILD,NULL);
		return FALSE;
	}


	
	ctl->cb(TELMSG_CONNECT_OK,NULL);
	//setsockopt(ctl->sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&ot, sizeof(ot));
	//setsockopt(ctl->sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&ot, sizeof(ot));
	
	ctl->state = TOP_LEVEL;
	for (o = opts; *o; o++) 
	{
		ctl->opt_states[(*o)->index] = (*o)->initial_state;

		if (ctl->opt_states[(*o)->index] == REQUESTED)
		{
			//send_opt(ctl->sd, (*o)->send, (*o)->option);			
			opt[pos]   = IAC;
			opt[pos+1] = (unsigned char)(*o)->send;
			opt[pos+2] = (unsigned char)(*o)->option;

			pos += 3;
		}
	}

	if(send(ctl->sd,opt,pos,0) <= 0 )
	{
		
		ctl->cb(TELMSG_SETUP_FAILD,NULL);
		return FALSE;
	}

	ctl->cb(TELMSG_SETUP_OK,NULL);
	ctl->activated = TRUE;

	return TRUE;
}


//Telnet 初始化
int telnet_init(const char* logfile)
{	
	return S_OK;
}


void*  telnet_new_seesion(const char* server,int port,telnet_cb cb)
{
	telnet_user*    ctl = NULL;

	ctl  = _hx_calloc(sizeof(telnet_user),1);

	if(!ctl) return NULL;

	//pht   = gethostbyname(server);
	//strncpy(ctl->address, inet_ntoa (*(struct  in_addr  *)pht->h_addr_list[0]),sizeof(ctl->address));	
	strcpy(ctl->address, server);	
	ctl->port   = port;
	ctl-> cb    = cb;
	

	return ctl;
}


int  telnet_connect_seesion(void* p)
{
	telnet_user*   ctl = (telnet_user*)p;
	int            ret = 0;

	if(telnet_connect(ctl) == TRUE)
	{			
		ret = telnet_recv_server_answer(ctl);		
	}

	return ret;
}

int  telnet_send_input(void* p,const char* buf,int len)
{
	telnet_user*   ctl = (telnet_user*)p;	
	
	return send(ctl->sd,buf,len,0);
}

int  telnet_recv_ret(void* p,char* buf,int len)
{
	telnet_user*   ctl = (telnet_user*)p;	

	return telnet_recv_msg(ctl,buf,len);	
	
}

void  telnet_free_seesion(void* p)
{
	telnet_user*   ctl = (telnet_user*)p;	

	if(ctl)
	{
		closesocket(ctl->sd);		
		_hx_free(ctl);	
	}
	}
