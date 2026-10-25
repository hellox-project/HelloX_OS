/* arpa/inet.h compatibility shim: maps to hxlib's inet.h, which provides
 * inet_addr/inet_aton/inet_ntoa and the byte-order helpers.
 * (inet_pton/inet_ntop are provided by the adapt layer.) */
#ifndef __ARPA_INET_H__
#define __ARPA_INET_H__

#include "inet.h"
#include "adapt.h"

#endif /* __ARPA_INET_H__ */
