/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
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
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Conny Ohult <conny@sm.luth.se>
 */

#ifndef __LWBT_MEMP_H__
#define __LWBT_MEMP_H__

typedef enum {
  MEMP_HCI_PCB,
  MEMP_HCI_LINK,
  MEMP_HCI_INQ,
  MEMP_L2CAP_PCB,
  MEMP_L2CAP_PCB_LISTEN,
  MEMP_L2CAP_SIG,
  MEMP_L2CAP_SEG,
  MEMP_SDP_PCB,
  MEMP_SDP_RECORD,
  MEMP_RFCOMM_PCB,
  MEMP_RFCOMM_PCB_LISTEN,
  MEMP_PPP_PCB,
  MEMP_PPP_REQ,

  MEMP_LWBT_MAX
} lwbt_memp_t;

void lwbt_memp_init(void);

void *lwbt_memp_malloc(lwbt_memp_t type);
void lwbt_memp_free(lwbt_memp_t type, void *mem);

#endif /* __LWBT_MEMP_H__  */
