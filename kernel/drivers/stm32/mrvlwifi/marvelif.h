//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 26,2014
//    Module Name               : marvel.h
//    Module Funciton           : 
//                                This module countains Marvell WiFi ethernet
//                                driver's low level function definitions.
//                                The hardware driver only need implement several
//                                low level routines and link them into HelloX's
//                                ethernet skeleton.
//
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __MARVELIF_H__
#define __MARVELIF_H__

//Ethernet interface's name,please be aware that no space character is allowed in name
//string since HelloX's shell can not support string contains space.
#define MARVEL_ETH_NAME "Marvel_WLAN_Int"

//Initializer or the Marvel Ethernet Driver,it's a global function and is called
//by Ethernet Manager in process of initialization.
BOOL Marvel_Initialize(LPVOID pData);

//Default WiFi hot spot SSID to associate when startup,and also the default
//SSID when HelloX running in AdHoc mode.
#define WIFI_DEFAULT_SSID "HelloX_HGW"

//Default key when associate with the default SSID,or the default key when
//run in AdHoc mode.
#define WIFI_DEFAULT_KEY  "0123456789012"

//Running mode of the HelloX.
#define WIFI_MODE_INFRA   '0'
#define WIFI_MODE_ADHOC   '1'

#endif //__MARVELIF_H__
