//------------------------------------------------------------------------
// The following code is copied from ping.c in lwIP contribution directory,
// and modified by Garry Xin to fit HelloX operating system's requirement.
//------------------------------------------------------------------------

/** 
 * This is an example of a "ping" sender (with raw API and socket API).
 * It can be used as a start point to maintain opened a network connection, or
 * like a network "watchdog" for your device.
 *
 */

#include "lwip/opt.h"

#if LWIP_RAW /* don't build if not configured for use in lwipopts.h */

#include "lwip/mem.h"
#include "lwip/raw.h"
#include "lwip/icmp.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/timers.h"
#include "lwip/inet_chksum.h"

#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "stdio.h"
#include "network.h"

/**
 * PING_DEBUG: Enable debugging for PING.
 */
#ifndef PING_DEBUG
#define PING_DEBUG     LWIP_DBG_ON
#endif

/** ping target - should be a "ip_addr_t" */
#ifndef PING_TARGET
#define PING_TARGET   (netif_default?netif_default->gw:ip_addr_any)
#endif

/** ping receive timeout - in milliseconds */
//#ifndef PING_RCV_TIMEO
#define PING_RCV_TIMEO 1000
//#endif

/** ping delay - in milliseconds */
#ifndef PING_DELAY
#define PING_DELAY     100
#endif

/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif

/** ping result action - no default action */
#ifndef PING_RESULT
#define PING_RESULT(ping_ok)
#endif

/* ping variables */
static u16_t ping_seq_num;
static u32_t ping_time;
static u32_t ping_pkt_seq = 0;
static u32_t ping_succ    = 0;

#if !PING_USE_SOCKETS
static struct raw_pcb *ping_pcb;
#endif /* PING_USE_SOCKETS */

/** Prepare a echo ICMP request */
static void
ping_prepare_echo( struct icmp_echo_hdr *iecho, u16_t len)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = htons(++ping_seq_num);

  /* fill the additional data buffer with some data */
  for(i = 0; i < data_len; i++) {
    ((char*)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char)i;
  }

  iecho->chksum = inet_chksum(iecho, len);
}

/* Ping using the socket ip */
static err_t
ping_send(int s, ip_addr_t *addr,int size)
{
  int err;
  struct icmp_echo_hdr *iecho;
  struct sockaddr_in to;
  size_t ping_size = 0;

  //Set ping packet's size.
  if(size < 16)
  {
	  size = 16;
  }
  if(size >= 65500)
  {
	  size = 65500;
  }
  ping_size = sizeof(struct icmp_echo_hdr) + size;

  LWIP_ASSERT("ping_size is too big", ping_size <= 0xffff);

  iecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
  if (!iecho) {
    return ERR_MEM;
  }

  ping_prepare_echo(iecho, (u16_t)ping_size);

  to.sin_len = sizeof(to);
  to.sin_family = AF_INET;
  inet_addr_from_ipaddr(&to.sin_addr, addr);

  err = lwip_sendto(s, iecho, ping_size, 0, (struct sockaddr*)&to, sizeof(to));

  mem_free(iecho);

  return (err ? ERR_OK : ERR_VAL);
}

static void
ping_recv(int s)
{
  char*     buf            = NULL;
  int       fromlen, len;
  struct    sockaddr_in from;
  struct    ip_hdr *iphdr;
  struct    icmp_echo_hdr *iecho;
  int       ms;
  BOOL      bResult        = FALSE;

  //Allocate a buffer to contain the received data.
  buf = (char*)KMemAlloc(1500,KMEM_SIZE_TYPE_ANY);
  if(NULL == buf)
  {
	  return;
  }
  while((len = lwip_recvfrom(s, buf, 1500, 0, (struct sockaddr*)&from, (socklen_t*)&fromlen)) > 0)
  {
		if(len >= (int)(sizeof(struct ip_hdr)+sizeof(struct icmp_echo_hdr)))
		{
			ip_addr_t fromaddr;
      inet_addr_to_ipaddr(&fromaddr, &from.sin_addr);
			//Get times between sent and receive.
			ms = sys_now() - ping_time;
			ms *= SYSTEM_TIME_SLICE;

      iphdr = (struct ip_hdr *)buf;
      iecho = (struct icmp_echo_hdr *)(buf + (IPH_HL(iphdr) * 4));
      if (((iecho->id == PING_ID) && (iecho->seqno == htons(ping_seq_num)) && iecho->type == ICMP_ER))
			{
				len = len - sizeof(struct ip_hdr) - sizeof(struct icmp_echo_hdr);  //Adjust received data's length,since it
		                                                                       //includes IP and ICMP headers.
				_hx_printf("  [%d] Reply from %s,size = %d,time = %d(ms)\r\n",ping_pkt_seq,inet_ntoa(fromaddr),len,ms);
				ping_succ ++;
				bResult = TRUE;
      }
	    else
	    {
		    //printf("  ping : Received invalid replay,drop it.\r\n");
      }
    }
  }

  if (!bResult)
  {
		_hx_printf("  [%d] Request time out.\r\n",ping_pkt_seq);
  }

  if(buf)  //Release it.
  {
	  KMemFree(buf,KMEM_SIZE_TYPE_ANY,0);
  }
}

//Entry point of ping application.
void ping_Entry(void *arg)
{
  int s;
  int timeout = PING_RCV_TIMEO;
  __PING_PARAM* pParam = (__PING_PARAM*)arg;

  ping_pkt_seq = 0;  //Reset ping sequence number.
	ping_succ    = 0;

  if((s = lwip_socket(AF_INET, SOCK_RAW, IP_PROTO_ICMP)) < 0)
  {
		PrintLine("  ping : Create raw socket failed,quit.");
    return;
  }

  lwip_setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  _hx_printf("\r\n  Ping %s with %d bytes packet:\r\n",inet_ntoa(pParam->targetAddr),pParam->size);
  while (1)
	{
		//ping_target = PING_TARGET; //ping gw
		//IP4_ADDR(&ping_target, 127,0,0,1); //ping loopback.
    if (ping_send(s, &pParam->targetAddr,pParam->size) == ERR_OK)
		{
			//printf(" ping_Entry : Send out packet,addr = %s,size = %d\r\n",inet_ntoa(pParam->targetAddr),pParam->size);
      ping_time = sys_now();
      ping_recv(s);
	    ping_pkt_seq ++;
    }
	  else
	  {
	    PrintLine("   ping : Send out packet failed.");
    }
    //sys_msleep(PING_DELAY);

	  //Try the specified times.
	  pParam->count --;
	  if(0 == pParam->count)
	  {
		  break;
	  }
  }
	//Show ping statistics.
	_hx_printf("\r\n");
	_hx_printf("  ping statistics: total send = %d,received = %d,%d loss.\r\n",
	  ping_pkt_seq,ping_succ,(ping_pkt_seq - ping_succ));
  //Close socket.
  lwip_close(s);
}

#endif /* LWIP_RAW */
