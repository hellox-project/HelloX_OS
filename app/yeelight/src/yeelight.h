/* Header file of yeelight controller. */
#ifndef __YEELIGHT_H__
#define __YEELIGHT_H__

/* For struct sockaddr_in. */
#include <socket/sockets.h>

/* Version number. */
#define __MAJOR_VER  1
#define __MINNOR_VER 2

/* Constants. */
#define SSDP_DEST_ADDR "239.255.255.250"
#define SSDP_DEST_PORT 1982
#define SSDP_WAIT_TIMEOUT 2000 /* Wait at most for 2s for incoming data. */
#define SSDP_LAN_ADDR "192.168.169.1"

/* Controller server listening port. */
#define YLIGHT_SERVER_PORT 2049

/* Waiting timeout value between controller and bulb. */
#define YLIGHT_WAIT_TIMEOUT 5000

/* Timer ID of yeelight periodic searching. */
#define YLIGHT_PERIODIC_ID 1000
/* Periodic time of yeelight searching,in ms. */
#define SSDP_PERIODIC_TIME 3000

/* Search request message. */
#define SSDP_REQ_CMD "M-SEARCH * HTTP/1.1\r\n\
HOST: 239.255.255.250:1982\r\n\
MAN: \"ssdp:discover\"\r\n\
ST: wifi_bulb"

/* Toggle command message. */
#define YLIGHT_TOGGLE_CMD "{\"id\":\"%s\",\"method\":\"toggle\",\"params\":[]}\r\n"

/* Light ID's length. */
#define LIGHT_ID_LENGTH 20

/* structure to hold yeelight object. */
struct yeelight_object{
	char id[LIGHT_ID_LENGTH + 1];
	struct sockaddr_in socket;
	char* pOriginalResp;
	int sock; /* Socket that opened to the light. */
	struct yeelight_object* pNext; /* Pointer of link list. */
};

/* Global bulb list contains all yeelight object(s) found. */
extern struct yeelight_object* pLightObject;

/* Entry point of yeelight server. */
DWORD ylight_server(LPVOID);

/* Entry point of yeelight controller. */
DWORD ylight_controller(LPVOID);

/* Messages that yeelight controller can handle. */
#define YLIGHT_MSG_TOGGLE (KERNEL_MESSAGE_USER + 1)
#define YLIGHT_MSG_SETRGB (KERNEL_MESSAGE_USER + 2)

#endif //__YEELIGHT_H__
