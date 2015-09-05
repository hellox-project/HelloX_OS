#include "../KAPI/K_SOCKET.H"
#include "../KAPI/K_KERNEL.H"
#include "../KAPI/K_IO.H"
#include "EdpKit.h"

#ifdef _ENCRYPT
#include "Openssl.h"
#endif

#define SRCDEV_ID       "151403" //设备ID
#define DSTDEV_ID       "145053" //设备ID
#define API_KEY         "bereqz67cnlTh7kN5wOkQ8zlmq8A"//"mgDiVsQ7E8bPUwfBDtTy4K8yMtMA" //APIKey
#define SERVER_ADDR      0x2728E6B7 //"jjfaedp.hedevice.com"    //OneNet EDP 服务器地址
#define SERVER_PORT      0x6c03    //876            //OneNet EDP 服务器端口

#define  PRT_FMT         "print test=%d"
/*
 * [说明]
 * Main.c 是为了测试EdpKit而写的, 也是给客户展示如何使用EdpKit
 * Main.c 使用的是c & linux socket
 *
 * 测试包含了：
 *      打包EDP包并发送：连接设备云请求, 心跳请求, 转发数据, 存储json数据, 存储bin数据
 *      接收并解析EDP包：连接设备云请求响应, 心跳请求响应, 转发数据, 存储json数据, 存储bin数据
 *
 * [注意]
 * Main.c不属于我们EDP SDK的一部分, 客户程序应该根据自己的系统写类似Main.c的代码
 * 客户程序应该只包含Common.h, cJSON.* 和 EdpKit.*, 而不应该包含Main.c
 * 
 * 加解密是利用openssl库实现的，如果有openssl库，则可以直接利用Openssl.*文件中提供
 * 的函数实现加解密。否则应该自己实现Openssl.h中的函数。
 * 如果需要加密功能，请参考Makefile中的说明，取消相应行的注释。
 */

/*----------------------------错误码-----------------------------------------*/
#define ERR_CREATE_SOCKET   -1 
#define ERR_HOSTBYNAME      -2 
#define ERR_CONNECT         -3 
#define ERR_SEND            -4
#define ERR_TIMEOUT         -5
#define ERR_RECV            -6
/*---------------统一linux和windows上的Socket api----------------------------*/
#ifndef htonll
#ifdef _BIG_ENDIAN
#define htonll(x)   (x)
#define ntohll(x)   (x)
#else
#define htonll(x)   ((((uint64)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x)   ((((uint64)ntohl(x)) << 32) + ntohl(x >> 32))
#endif
#endif

#define Socket(a,b,c)          socket(a,b,c)
#define Connect(a,b,c)         connect(a,b,c)
#define Close(a)               close(a)
#define Read(a,b,c)            read(a,b,c)
#define Recv(a,b,c,d)          recv(a, (void *)b, c, d)
#define Select(a,b,c,d,e)      select(a,b,c,d,e)
#define Send(a,b,c,d)          send(a, (const int8 *)b, c, d)
#define Write(a,b,c)           write(a,b,c)
#define GetSockopt(a,b,c,d,e)  getsockopt((int)a,(int)b,(int)c,(void *)d,(socklen_t *)e)
#define SetSockopt(a,b,c,d,e)  setsockopt((int)a,(int)b,(int)c,(const void *)d,(int)e)
#define GetHostByName(a)       gethostbyname((const char *)a)

/* 
 * 函数名:  Open
 * 功能:    创建socket套接字并连接服务端
 * 参数:    addr    ip地址
 *          protno  端口号
 * 说明:    这里只是给出一个创建socket连接服务端的例子, 其他方式请查询相关socket api
 * 相关socket api:  
 *          socket, gethostbyname, connect
 * 返回值:  类型 (int32)
 *          <=0     创建socket失败
 *          >0      socket描述符
 */
int32 Open(const uint8 *addr, int16 portno)
{
    int32 sockfd   = -1;
    struct sockaddr_in serv_addr;
    struct hostent* server;

    /* 创建socket套接字 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
	{
        _hx_printf("ERROR opening socket\n");
        return ERR_CREATE_SOCKET; 
    }

    /*server = GetHostByName(addr);
    if (server == NULL) 
	{
        f_hx_printf(stderr, "ERROR, no such host\n");
        return ERR_HOSTBYNAME;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr,   (char *)&serv_addr.sin_addr.s_addr, server->h_length);*/

	//测试，因为GetHostByName接口不支持，所以目前暂时设定一个测试IP地址和端口
	//183.230.40.39 0xB7E62827 0x2728E6B7
	//
	mymemset(&serv_addr, 0,sizeof(serv_addr));

	serv_addr.sin_addr.s_addr = SERVER_ADDR;	
	serv_addr.sin_family      = AF_INET;
    serv_addr.sin_port        = SERVER_PORT;//SERVER_PORT;//htons(portno);    

	/* 客户端 建立与TCP服务器的连接 */
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
    {        
        return ERR_CONNECT;
    }
	
    return sockfd;
}

/* 
 * 函数名:  DoSend
 * 功能:    将buffer中的len字节内容写入(发送)socket描述符sockfd, 成功时返回写的(发送的)字节数.
 * 参数:    sockfd  socket描述符 
 *          buffer  需发送的字节
 *          len     需发送的长度
 * 说明:    这里只是给出了一个发送数据的例子, 其他方式请查询相关socket api
 *          一般来说, 发送都需要循环发送, 是因为需要发送的字节数 > socket的写缓存区时, 一次send是发送不完的.
 * 相关socket api:  
 *          send
 * 返回值:  类型 (int32)
 *          <=0     发送失败
 *          >0      成功发送的字节数
 */
int   getch()
{
	MSG     Msg = {0};
	int     key = 0; 

	while(1)
	{
		GetMessage(&Msg);
		if(Msg.wCommand == MSG_KEY_DOWN || Msg.wCommand == MSG_VK_KEY_DOWN)
		{
			key = Msg.dwParam;
			if(key != 0x0D)
			{
				break;
			}			
		}
	}
	
	return key;
}

int32 DoSend(int32 sockfd, const char* buffer, int32 len)
{
    int32 total  = 0;
    int32 n      = 0;
	int   flage  = 0;

	//_hx_printf("DoSend: Try to send to server %d bytes...\r\n", len);

    while (len != total)
    {
        /* 试着发送len - total个字节的数据 */
        n = send(sockfd,buffer + total,len - total,flage);
        if (n <= 0)
        {
            _hx_printf("ERROR writing to socket:addr=0x%X,len=%d",buffer,len);
            return n;
        }
        /* 成功发送了n个字节的数据 */
        total += n;
    }

	//_hx_printf("DoSend: Send %d bytes to server.\r\n", len);
    /* wululu test print send bytes */
    //hexdump(buffer, len);

    return total;
}

DWORD DoRecv(int sockfd)
{    
    int error = 0;
    int n, rtn,total;
    uint8 mtype, jsonorbin;
    char buffer[1024];
    RecvBuffer* recv_buf = NewBuffer();
    EdpPacket* pkg;
    
    char* src_devid;
    char* push_data;
    uint32 push_datalen;

    cJSON* save_json;
    char* save_json_str;

    cJSON* desc_json;
    char* desc_json_str;
    char* save_bin; 
    uint32 save_binlen;
    char* json_ack;

    char* cmdid;
    uint16 cmdid_len;
    char*  cmd_req;
    uint32 cmd_req_len;
    EdpPacket* send_pkg;
    char* ds_id;
    double dValue = 0;
    int iValue = 0;
    char* cValue = NULL;
	int    i = 0;
	int    r = 0;

_START_RECV:
	/* 试着接收1024个字节的数据 */
	_hx_printf("DoRecv: Try to recv %d bytes from server...\r\n", 1024);
	n = Recv(sockfd, buffer, 1024, 0);
	if (n <= 0)   
	{
		_hx_printf("recv from server error!");
		goto _END;
	}
	
	_hx_printf("DoRecv: Received %d bytes from server.\r\n",n);
	/* 成功接收了n个字节的数据 */
    if(WriteBytes(recv_buf, buffer, n) == -1)
	{
		PrintLine("write buf over");
		return 0;
		
	}
	
	while (1)
	{
		/* 获取一个完成的EDP包 */
		if ((pkg = GetEdpPacket(recv_buf)) == 0)
		{
			break;
		}
				
            /* 获取这个EDP包的消息类型 */
            mtype = EdpPacketType(pkg);
			_hx_printf("EdpPacketType :0x%X",mtype);

            /* 根据这个EDP包的消息类型, 分别做EDP包解析 */
            switch(mtype)
            {
                case CONNRESP:
                    /* 解析EDP包 - 连接响应 */
                    rtn = UnpackConnectResp(pkg);
					_hx_printf("recv connect resp, rtn: %d", rtn);
                    break;
                case PUSHDATA:
                    /* 解析EDP包 - 数据转发 */
                    UnpackPushdata(pkg, &src_devid, &push_data, &push_datalen);
                    _hx_printf("recv push data, src_devid: %s, push_data: %s, len: %d", 
                            src_devid, push_data, push_datalen);
                    hx_free(src_devid);
                    hx_free(push_data);
                    break;
				case SAVEDATA:
                    /* 解析EDP包 - 数据存储 */
                    if (UnpackSavedata(pkg, &src_devid, &jsonorbin) == 0)
                    {
                        if (jsonorbin == kTypeFullJson  || jsonorbin == kTypeSimpleJsonWithoutTime || jsonorbin == kTypeSimpleJsonWithTime) 
						{
						_hx_printf("json type is %d", jsonorbin);
			
						UnpackSavedataDouble(jsonorbin, pkg, &ds_id, &dValue);
						
						_hx_printf("src_devid=%s,ds_id = %s ",src_devid, ds_id);		
						//don't sport float 
						//_hx_printf("ds_id = %s value = %f", ds_id, dValue);		
						//IsPkgComplete();

						hx_free(src_devid);
						hx_free(ds_id);

						//has a  next Packet;
						goto _START_RECV;
						}
						else if (jsonorbin == kTypeBin)
						{
						 /* 解析EDP包 - bin数据存储 */
                            UnpackSavedataBin(pkg, &desc_json, (uint8**)&save_bin, &save_binlen);
                            desc_json_str=cJSON_Print(desc_json);
                            _hx_printf("recv save data bin, src_devid: %s, desc json: %s, bin: %s, binlen: %d", 
                                    src_devid, desc_json_str, save_bin, save_binlen);
                            hx_free(desc_json_str);
                            cJSON_Delete(desc_json);
                            hx_free(save_bin);
                        }
					else if (jsonorbin == kTypeString)
					{
						char* simple_str = NULL;
						UnpackSavedataSimpleString(pkg, &simple_str);
			    		_hx_printf("%s", simple_str);
						hx_free(simple_str);
					}
                     hx_free(src_devid);
                    }
					else
					{
					_hx_printf("error");
					}
                    break;
				case SAVEACK:
					{
					json_ack = NULL;
					UnpackSavedataAck(pkg, &json_ack);
					_hx_printf("save json ack = %s", json_ack);
					hx_free(json_ack);
					}
					break;
				case CMDREQ:
					if (UnpackCmdReq(pkg, &cmdid, &cmdid_len, &cmd_req, &cmd_req_len) == 0)
					{
					/* 
					 * 用户按照自己的需求处理并返回，响应消息体可以为空，此处假设返回2个字符"ok"。
					 * 处理完后需要释放
					 */
					char cmd_resp[] = "ok";
					unsigned cmd_resp_len = strlen(cmd_resp);

					send_pkg = PacketCmdResp(cmdid, cmdid_len,cmd_resp, cmd_resp_len);
					DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
					DeleteBuffer(&send_pkg);
		    
					hx_free(cmdid);
					hx_free(cmd_req);
					}
					break;
                case PINGRESP:
                    /* 解析EDP包 - 心跳响应 */
                    {
					UnpackPingResp(pkg); 
                    _hx_printf("recv ping resp");
					}
                    break;
                default:
                    /* 未知消息类型 */
                    {
					error = 1;
                    _hx_printf("recv failed...");
					}
                    break;
            }
            DeleteBuffer(&pkg);

		//	goto _START_RECV;
		//break;		
        }

_END:
    DeleteBuffer(&recv_buf);
    PrintLine("recv Data complete");
	return 0;
}


//A local helper routine used to write data to server.
static int write_func(int arg,char* data_stream,int value)
{
	unsigned int now = 1024;
	int sockfd = arg;
	EdpPacket* send_pkg;
	cJSON *save_json;
	int32 ret = 0;
	char text[25] = { 0 };
	char send_buf[256];

	/*产生JSON串，其中携带了要上传的用户数据*/
	send_buf[0] = 0;
	strcat(send_buf, "{\"datastreams\": [{");
	//strcat(send_buf, "\"id\": \"sys_time\",");
	_hx_sprintf(text, "\"id\": \"%s\",", data_stream);
	strcat(send_buf, text);
	strcat(send_buf, "\"datapoints\": [");
	strcat(send_buf, "{");
	_hx_sprintf(text, "\"value\": \"%d\"", value);
	strcat(send_buf, text);
	strcat(send_buf, "}]}]}");
	//_hx_printf("write_func: json str = %s.\r\n", send_buf);
	/*将JSON串封装成EDP数据包*/
	save_json = cJSON_Parse(send_buf);
	if (NULL == save_json)
	{
		_hx_printf("write_func: cJSON_Parse failed.\r\n");
		return -1;
	}
	send_pkg = PacketSavedataJson(SRCDEV_ID, save_json, kTypeFullJson);
	if (NULL == send_pkg)
	{
		_hx_printf("write_func: PacketSavedataJson failed.\r\n");
		return -1;
	}
	cJSON_Delete(save_json);
	/*发送EDP数据包上传数据*/
	ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
	DeleteBuffer(&send_pkg);
	return ret;
}

#define MAX_SEND_COUNTER 10000

int edp_main(int argc, char *argv[])
{
	int sockfd, n, ret;
	EdpPacket* send_pkg;
	cJSON *save_json;
	int send_count = MAX_SEND_COUNTER;
	__ETH_INTERFACE_STATE ethState;
	DWORD dwSendFrmNum = 0;
	DWORD dwSendByteNum = 0;
	DWORD dwRecvFrmNum = 0;
	DWORD dwRecvByteNum = 0;
	DWORD dwDeltSendFrm;
	DWORD dwDeltSendByte;
	DWORD dwDeltRecvFrm;
	DWORD dwDeltRecvByte;
	
	/* create a socket and connect to server */
	PrintLine("EDP client starts connecting...");
	sockfd = Open(NULL, 0);
	if (sockfd < 0)
	{
		PrintLine("Failed to connect to EDP server.");
		return 0;
	}

	PrintLine("EDP connects server successfully.");

	/* connect to server */
	send_pkg = PacketConnect1(SRCDEV_ID, API_KEY);

	/* 向设备云发送连接请求 */
	ret = DoSend(sockfd, send_pkg->_data, send_pkg->_write_pos);
	DoRecv(sockfd);
	DeleteBuffer(&send_pkg);

	//Begin to transfer data to server.
	_hx_printf("EDP Client: Begin to save data to server...\r\n");
	while (TRUE)
	{
		if (!GetEthernetInterfaceState(&ethState, 0, NULL))
		{
			_hx_printf("EDP client: Get ethernet interface state failed.\r\n");
			goto __EXIT;
		}
		dwDeltSendFrm = ethState.dwFrameSend - dwSendFrmNum;
		dwDeltSendByte = ethState.dwTotalSendSize - dwSendByteNum;
		dwDeltRecvFrm = ethState.dwFrameRecv - dwRecvFrmNum;
		dwDeltRecvByte = ethState.dwTotalRecvSize - dwRecvByteNum;
		if (write_func(sockfd,"sendfrm#",dwDeltSendFrm) < 0)
		{
			_hx_printf("EDP Client: Write data failed.\r\n");
			goto __EXIT;
		}
		if (write_func(sockfd, "sendbyte#", dwDeltSendByte) < 0)
		{
			_hx_printf("EDP Client: Write data failed.\r\n");
			goto __EXIT;
		}
		if (write_func(sockfd, "recvfrm#", dwDeltRecvFrm) < 0)
		{
			_hx_printf("EDP Client: Write data failed.\r\n");
			goto __EXIT;
		}
		if (write_func(sockfd, "recvbyte#", dwDeltRecvByte) < 0)
		{
			_hx_printf("EDP Client: Write data failed.\r\n");
			goto __EXIT;
		}
		_hx_printf("EDP Client: Save data [%d/%d/%d/%d,round = %d] OK.\r\n",
			dwDeltSendFrm,
			dwDeltSendByte,
			dwDeltRecvFrm,
			dwDeltRecvByte,
			send_count);
		dwSendFrmNum = ethState.dwFrameSend;
		dwSendByteNum = ethState.dwTotalSendSize;
		dwRecvFrmNum = ethState.dwFrameRecv;
		dwRecvByteNum = ethState.dwTotalRecvSize;
		send_count--;
		if (0 == send_count)
		{
			goto __EXIT;
		}
		Sleep(2000);  //Sleep 2s.
	}
	
__EXIT:
	PrintLine("Exit the EDP client application.");
	/* close socket */
	Close(sockfd);
	return 0;
}
