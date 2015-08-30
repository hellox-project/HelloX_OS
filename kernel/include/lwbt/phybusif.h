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

#ifndef __LWBT_PHYBUSIF_H__
#define __LWBT_PHYBUSIF_H__

#include "lwip/pbuf.h"
#include "lwip/err.h"

struct phybusif_cb;

/* Application program's interface: */
void phybusif_init(const char * port); /* Must be called first to initialize the physical bus interface */
err_t phybusif_reset(struct phybusif_cb *cb);
err_t phybusif_input(struct phybusif_cb *cb);

/* Upper layer interface: */
void phybusif_output(struct pbuf *p, u16_t len);

enum phybusif_state {
  W4_PACKET_TYPE, W4_EVENT_HDR, W4_EVENT_PARAM, W4_ACL_HDR, W4_ACL_DATA
};

/* The physical bus interface control block */
struct phybusif_cb {
  enum phybusif_state state;

  struct pbuf *p;
  struct pbuf *q;

  struct hci_event_hdr *evhdr;
  struct hci_acl_hdr *aclhdr;
  
  unsigned int tot_recvd;
  unsigned int recvd;
};

#endif /* __LWBT_PHYBUSIF_H__ */
