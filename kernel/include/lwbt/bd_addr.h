/*
 * Copyright (c) 2003 EISLAB, Lulea University of Technology.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwBT Bluetooth stack.
 * 
 * Author: Conny Ohult <conny@sm.luth.se>
 *
 */

#ifndef __LWBT_BD_ADDR_H__
#define __LWBT_BD_ADDR_H__

struct bd_addr {
  u8_t addr[6];
};

#define BD_ADDR_LEN 6

#define BD_ADDR(bdaddr, a, b, c, d, e, f) do{ \
                                        bdaddr->addr[0] = a; \
				        bdaddr ->addr[1] = b; \
				        bdaddr->addr[2] = c; \
				        bdaddr->addr[3] = d; \
				        bdaddr->addr[4] = e; \
				       bdaddr->addr[5] = f; }while(0)
//TODO: USE memcmp????
#define bd_addr_cmp(addr1, addr2) (((addr1)->addr[0] == (addr2)->addr[0]) && \
				   ((addr1)->addr[1] == (addr2)->addr[1]) && \
				   ((addr1)->addr[2] == (addr2)->addr[2]) && \
				   ((addr1)->addr[3] == (addr2)->addr[3]) && \
				   ((addr1)->addr[4] == (addr2)->addr[4]) && \
				   ((addr1)->addr[5] == (addr2)->addr[5]))
//TODO: USE memcpy????
#define bd_addr_set(addr1, addr2) do { \
                                   (addr1)->addr[0] = (addr2)->addr[0]; \
				   (addr1)->addr[1] = (addr2)->addr[1]; \
				   (addr1)->addr[2] = (addr2)->addr[2]; \
				   (addr1)->addr[3] = (addr2)->addr[3]; \
				   (addr1)->addr[4] = (addr2)->addr[4]; \
				   (addr1)->addr[5] = (addr2)->addr[5]; }while(0)

#define bd_addr_debug_print(bdaddr) LWIP_DEBUGF(LWBT_DEBUG, ("%02x:%02x:%02x:%02x:%02x:%02x\n", \
		    (bdaddr)->addr[0], \
		    (bdaddr)->addr[1], \
		    (bdaddr)->addr[2], \
		    (bdaddr)->addr[3], \
		    (bdaddr)->addr[4], \
		    (bdaddr)->addr[5]))


#endif /* __LWBT_BD_ADDR_H__ */
