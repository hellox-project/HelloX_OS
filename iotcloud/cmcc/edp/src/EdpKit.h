#ifndef __EDP_KIT_H__
#define __EDP_KIT_H__

#ifdef EDPKIT_EXPORTS
	#define EDPKIT_DLL __declspec(dllexport)
#else
	#define EDPKIT_DLL
#endif
 
#include "Common.h"
#include "cJSON.h"
/*
 * history
 * 2015-06-01 v1.0.1 wululu fix bug: malloc for string, MUST memset to 0
 * 2015-07-10 v1.1.0 wusongwei add UnpackCmdReq() and PacketCmdResp()
 * 2015-07-13 v1.1.1 wululu 增加封装json的接口, windows版本dll
 * 2015-07-13 v1.1.2 wululu 支持double和string类型的打包函数和解包函数
 * 2015-07-15 v1.1.3 wusongwei 添加SAVEACK响应
 * 2015-07-20 v1.1.4 wusongwei 添加/修改SAVEDATA消息的打包/解包函数
 */

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
#define MOSQ_MSB(A)         (uint8)((A & 0xFF00) >> 8)
#define MOSQ_LSB(A)         (uint8)(A & 0x00FF)
#define BUFFER_SIZE         1024//(0x01<<20) 
#define PROTOCOL_NAME       "EDP"
#define PROTOCOL_VERSION    1
/*----------------------------错误码-----------------------------------------*/
#define ERR_UNPACK_CONNRESP_REMAIN              -1000
#define ERR_UNPACK_CONNRESP_FLAG                -1001
#define ERR_UNPACK_CONNRESP_RTN                 -1002
#define ERR_UNPACK_PUSHD_REMAIN                 -1010
#define ERR_UNPACK_PUSHD_DEVID                  -1011
#define ERR_UNPACK_PUSHD_DATA                   -1012
#define ERR_UNPACK_SAVED_REMAIN                 -1020
#define ERR_UNPACK_SAVED_TANSFLAG               -1021
#define ERR_UNPACK_SAVED_DEVID                  -1022
#define ERR_UNPACK_SAVED_DATAFLAG               -1023
#define ERR_UNPACK_SAVED_JSON                   -1024
#define ERR_UNPACK_SAVED_PARSEJSON              -1025
#define ERR_UNPACK_SAVED_BIN_DESC               -1026
#define ERR_UNPACK_SAVED_PARSEDESC              -1027
#define ERR_UNPACK_SAVED_BINLEN                 -1028
#define ERR_UNPACK_SAVED_BINDATA                -1029
#define ERR_UNPACK_PING_REMAIN                  -1030
#define ERR_UNPACK_CMDREQ                       -1031
#define ERR_UNPACK_ENCRYPT_RESP                 -1032
#define ERR_UNPACK_SAVEDATA_ACK                 -1033

/*----------------------------消息类型---------------------------------------*/
/* 连接请求 */
#define CONNREQ             0x10
/* 连接响应 */
#define CONNRESP            0x20
/* 转发(透传)数据 */
#define PUSHDATA            0x30
/* 存储(转发)数据 */
#define SAVEDATA            0x80
/* 存储确认 */
#define SAVEACK             0x90
/* 命令请求 */
#define CMDREQ              0xA0
/* 命令响应 */
#define CMDRESP             0xB0
/* 心跳请求 */
#define PINGREQ             0xC0
/* 心跳响应 */
#define PINGRESP            0xD0
/* 加密请求 */
#define ENCRYPTREQ          0xE0
/* 加密响应 */
#define ENCRYPTRESP         0xF0

/* SAVEDATA消息支持的格式类型 */
typedef enum {
    kTypeFullJson = 0x01,
    kTypeBin = 0x02,
    kTypeSimpleJsonWithoutTime = 0x03,
    kTypeSimpleJsonWithTime = 0x04,
    kTypeString = 0x05,
}SaveDataType;

/*-------------发送buffer, 接收buffer, EDP包结构定义-------------------------*/
EDPKIT_DLL 
typedef struct Buffer
{
    uint8*  _data;          /* buffer数据 */
    uint32  _write_pos;     /* buffer写入位置 */
    uint32  _read_pos;      /* buffer读取位置 */
    uint32  _capacity;      /* buffer容量 */
}Buffer, SendBuffer, RecvBuffer, EdpPacket;
/*-----------------------------操作Buffer的接口------------------------------*/
/* 
 * 函数名:  NewBuffer
 * 功能:    生成Buffer
 * 说明:    一般情况下, NewBuffer和DeleteBuffer应该成对出现
 * 参数:    无
 * 返回值:  类型 (Buffer*)
 *          返回值非空 生成Buffer成功, 返回这个Buffer的指针
 *          返回值为空 生成Buffer失败, 内存不够
 */
EDPKIT_DLL Buffer* NewBuffer();
/* 
 * 函数名:  DeleteBuffer
 * 功能:    销毁Buffer
 * 说明:    一般情况下, NewBuffer和DeleteBuffer应该成对出现
 * 参数:    buf     一个Buffer的指针的指针
 * 返回值:  无
 */
EDPKIT_DLL void DeleteBuffer(Buffer** buf);
/* 
 * 函数名:  CheckCapacity
 * 功能:    检查Buffer是否能够写入长度为len的字节流, 
 *          如果Buffer的容量不够, 自动成倍扩展Buffer的容量(不影响Buffer数据)
 * 参数:    buf     需要写入Buffer的指针
 *          len     期望写入的长度
 * 返回值:  类型 (int32)
 *          <0      失败, 内存不够
 *          =0      成功
 */
EDPKIT_DLL int32 CheckCapacity(Buffer* buf, uint32 len);

/*------------------------读取EDP包数据的接口-------------------------------*/
/* 
 * 函数名:  ReadByte
 * 功能:    按EDP协议, 从Buffer(包)中读取一个字节数据
 * 参数:    pkg     EDP包
 *          val     数据(一个字节)
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 ReadByte(EdpPacket* pkg, uint8* val);
/* 
 * 函数名:  ReadBytes
 * 功能:    按EDP协议, 从Buffer(包)中读取count个字节数据
 * 说明:    val是malloc出来的, 需要客户端自己free 
 * 参数:    pkg     EDP包
 *          val     数据(count个字节)
 *          count   字节数
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 ReadBytes(EdpPacket* pkg, uint8** val, uint32 count);
/* 
 * 函数名:  ReadUint16
 * 功能:    按EDP协议, 从Buffer(包)中读取uint16值
 * 参数:    pkg     EDP包
 *          val     uint16值
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 ReadUint16(EdpPacket* pkg, uint16* val);
/* 
 * 函数名:  ReadUint32
 * 功能:    按EDP协议, 从Buffer(包)中读取uint32值
 * 参数:    pkg     EDP包
 *          val     uint32值
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 ReadUint32(EdpPacket* pkg, uint32* val);
/* 
 * 函数名:  ReadStr
 * 功能:    按EDP协议, 从Buffer(包)中读取字符串, 以\0结尾
 * 参数:    pkg     EDP包
 *          val     字符串
 * 说明:    val是malloc出来的, 需要客户端自己free 
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 ReadStr(EdpPacket* pkg, char** val);
/* 
 * 函数名:  ReadRemainlen
 * 功能:    按EDP协议, 从Buffer(包)中remainlen
 * 说明:    remainlen是EDP协议中的概念, 是一个EDP包身的长度
 * 参数:    pkg     EDP包
 *          len_val remainlen
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 ReadRemainlen(EdpPacket* pkg, uint32* len_val);

/*------------------------数据写入EDP包的接口-------------------------------*/
/*
 * 说明:    目前不支持一个包即在写入又在读取, 因此, 只有对于_read_pos为0的包才能被写入
 */
/* 
 * 函数名:  WriteByte
 * 功能:    按EDP协议, 将一个字节数据写入Buffer(包)中
 * 参数:    pkg     EDP包
 *          byte    数据(一个字节)
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 WriteByte(Buffer* buf, uint8 byte);
/* 
 * 函数名:  WriteBytes
 * 功能:    按EDP协议, 将count个字节数据写入Buffer(包)中
 * 参数:    pkg     EDP包
 *          bytes   数据
 *          count   字节数
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 WriteBytes(Buffer* buf, const void* bytes, uint32 count);
/* 
 * 函数名:  WriteUint16
 * 功能:    按EDP协议, 将uint16写入Buffer(包)中
 * 参数:    pkg     EDP包
 *          val     uint16数据
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 WriteUint16(Buffer* buf, uint16 val);
/* 
 * 函数名:  WriteUint32
 * 功能:    按EDP协议, 将uint32写入Buffer(包)中
 * 参数:    pkg     EDP包
 *          val     uint32数据
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 WriteUint32(Buffer* buf, uint32 val);
/* 
 * 函数名:  WriteStr
 * 功能:    按EDP协议, 将字符串写入Buffer(包)中
 * 参数:    pkg     EDP包
 *          val     字符串
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 WriteStr(Buffer* buf, const char *str);
/* 
 * 函数名:  WriteRemainlen 
 * 功能:    按EDP协议, 将remainlen写入Buffer(包)中
 * 说明:    remainlen是EDP协议中的概念, 是一个EDP包身的长度
 * 参数:    pkg     EDP包
 *          len_val remainlen
 * 返回值:  类型 (int32)
 *          <0      失败, pkg中无数据
 *          =0      成功
 */
EDPKIT_DLL int32 WriteRemainlen(Buffer* buf, uint32 len_val);
/* 
 * 函数名:  IsPkgComplete 
 * 功能:    判断接收到的Buffer, 是否为一个完整的EDP包
 * 参数:    buf     接收到的Buffer(二进制流)
 * 返回值:  类型 (int32)
 *          =0      数据还未收完, 需要继续接收
 *          >0      成功
 *          <0      数据错误, 不符合EDP协议
 */
EDPKIT_DLL int32 IsPkgComplete(RecvBuffer* buf);

/*-----------------------------客户端操作的接口------------------------------*/
/* 
 * 函数名:  GetEdpPacket 
 * 功能:    将接收到的二进制流, 分解成一个一个的EDP包
 * 说明:    返回的EDP包使用后, 需要删除
 * 相关函数:EdpPacketType, Unpack***类函数
 * 参数:    buf         接收缓存
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        无完整的EDP协议包
 */
EDPKIT_DLL EdpPacket* GetEdpPacket(RecvBuffer* buf);

/* 
 * 函数名:  EdpPacketType 
 * 功能:    获取一个EDP包的消息类型, 客户程序根据消息类型做不同的处理
 * 相关函数:Unpack***类函数
 * 参数:    pkg         EDP协议包
 * 返回值:  类型 (uint8) 
 *          值          消息类型(详细参见本h的消息类型定义)
 */
/* 例子:
 * ...
 * int8 mtype = EdpPacketType(pkg);
 * switch(mtype)
 * {
 *  case CONNRESP:
 *      UnpackConnectResp(pkg);
 *      break;
 *  case PUSHDATA:
 *      UnpackPushdata(pkg, src_devid, data, data_len);
 *      break;
 *  case SAVEDATA:
 *      UnpackSavedata(pkg, src_devid, flag, data);
 *      break;
 *  case PINGRESP:
 *      UnpackPingResp(pkg); 
 *      break;
 *  ...
 * }
 */
EDPKIT_DLL uint8 EdpPacketType(EdpPacket* pkg);

/* 
 * 函数名:  PacketConnect1 
 * 功能:    打包 由设备到设备云的EDP协议包, 连接设备云的请求(登录认证方式1)
 * 说明:    返回的EDP包发送给设备云后, 需要客户程序删除该包
 *          设备云会回复连接响应给设备
 * 相关函数:UnpackConnectResp
 * 参数:    devid       设备ID, 申请设备时平台返回的ID
 *          auth_key    鉴权信息(api-key), 在平台申请的可以操作该设备的api-key字符串
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketConnect1(const char* devid, const char* auth_key);

/* 
 * 函数名:  PacketConnect2 
 * 功能:    打包 由设备到设备云的EDP协议包, 连接设备云的请求(登录认证方式2)
 * 说明:    返回的EDP包发送给设备云后, 需要客户程序删除该包
 *          设备云会回复连接响应给设备
 * 相关函数:UnpackConnectResp
 * 参数:    userid      用户ID, 在平台注册账号时平台返回的用户ID
 *          auth_info   鉴权信息, 在平台申请设备时填写设备的auth_info属性
 *                      (json对象字符串), 该属性需要具备唯一性
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketConnect2(const char* userid, const char* auth_info);

/* 
 * 函数名:  UnpackConnectResp
 * 功能:    解包 由设备云到设备的EDP协议包, 连接响应
 * 说明:    接收设备云发来的数据, 通过函数GetEdpPacket和EdpPacketType判断出是连接响应后, 
 *          将整个响应EDP包作为参数, 由该函数进行解析
 * 相关函数:PacketConnect1, PacketConnect2, GetEdpPacket, EdpPacketType
 * 参数:    pkg         EDP包, 必须是连接响应包
 * 返回值:  类型 (int32) 
 *          =0          连接成功
 *          >0          连接失败, 具体失败原因见<OneNet接入方案与接口.docx>
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackConnectResp(EdpPacket* pkg);

/* 
 * 函数名:  PacketPushdata
 * 功能:    打包 设备到设备云的EDP协议包, 设备与设备之间转发数据
 * 说明:    返回的EDP包发送给设备云后, 需要删除这个包
 * 相关函数:UnpackPushdata
 * 参数:    dst_devid   目的设备ID
 *          data        数据
 *          data_len    数据长度
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketPushdata(const char* dst_devid, 
        const char* data, uint32 data_len);

/* 
 * 函数名:  UnpackPushdata
 * 功能:    解包 由设备云到设备的EDP协议包, 设备与设备之间转发数据
 * 说明:    接收设备云发来的数据, 通过函数GetEdpPacket和EdpPacketType判断出是pushdata后, 
 *          将整个响应EDP包作为参数, 由该函数进行解析 
 *          返回的源设备ID(src_devid)和数据(data)都需要客户端释放
 * 相关函数:PacketPushdata, GetEdpPacket, EdpPacketType
 * 参数:    pkg         EDP包, 必须是pushdata包
 *          src_devid   源设备ID
 *          data        数据
 *          data_len    数据长度
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackPushdata(EdpPacket* pkg, char** src_devid, 
        char** data, uint32* data_len);

/* 
 * 函数名:  PacketSavedataJson
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(json格式数据)
 * 说明:    返回的EDP包发送给设备云后, 需要删除这个包
 * 相关函数:UnpackSavedata, UnpackSavedataJson
 * 参数:    dst_devid   目的设备ID
 *          json_obj    json数据
 *          type        json的类型         
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EdpPacket* PacketSavedataJson(const char* dst_devid, cJSON* json_obj, int type);

/* 
 * 函数名:  PacketSavedataInt
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(json格式数据)
 * 说明:    该函数适用于数据点为int类型的数据流
 *          它把参数封装成EDP协议规定的cJSON对象,
 *          type类型决定使用哪种JSON格式，具体格式说明见文档《设备终端接入协议2-EDP.docx》
 * 相关函数:UnPacketSavedataInt
 * 参数:    type        采用的JSON数据类型，可选类型为：kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          dst_devid   目的设备ID
 *          ds_id       数据流ID
 *          value       int型数据点
 *          at          如果设置为0，则采用系统当前时间，否则采用给定时间。
 *                      如果type选择为kTypeSimpleJsonWithoutTime，由于这种类型的JSON格式不带时间，
 *                      服务器端统一采用系统时间，此值将被忽略
 *          token       当type为kTypeFullJson时，将根据EDP协议封装token字段，
 *                      为其它类型时将被忽略。
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketSavedataInt(SaveDataType type, const char* dst_devid, 
					const char* ds_id, int value, 
					time_t at, const char* token);

/* 
 * 函数名:  PacketSavedataDouble
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(json格式数据)
 * 说明:    该函数适用于数据点为double类型的数据流
 *          它把参数封装成EDP协议规定的cJSON对象,
 *          type类型决定使用哪种JSON格式，具体格式说明见文档《设备终端接入协议2-EDP.docx》
 * 相关函数:UnPacketSavedataDouble
 * 参数:    type        采用的JSON数据类型，可选类型为：kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          dst_devid   目的设备ID
 *          ds_id       数据流ID
 *          value       double型数据点
 *          at          如果设置为0，则采用系统当前时间，否则采用给定时间。
 *                      如果type选择为kTypeSimpleJsonWithoutTime，由于这种类型的JSON格式不带时间，
 *                      服务器端统一采用系统时间，此值将被忽略
 *          token       当type为kTypeFullJson时，将根据EDP协议封装token字段，
 *                      为其它类型时将被忽略。
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketSavedataDouble(SaveDataType type, const char* dst_devid, 
				const char* ds_id, double value, 
				time_t at, const char* token);

/* 
 * 函数名:  PacketSavedataString
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(json格式数据)
 * 说明:    该函数适用于数据点为char*类型的数据流
 *          它把参数封装成EDP协议规定的cJSON对象,
 *          type类型决定使用哪种JSON格式，具体格式说明见文档《设备终端接入协议2-EDP.docx》
 * 相关函数:UnPacketSavedataString
 * 参数:    type        采用的JSON数据类型，可选类型为：kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          dst_devid   目的设备ID
 *          ds_id       数据流ID
 *          value       char*型数据点
 *          at          如果设置为0，则采用系统当前时间，否则采用给定时间。
 *                      如果type选择为kTypeSimpleJsonWithoutTime，由于这种类型的JSON格式不带时间，
 *                      服务器端统一采用系统时间，此值将被忽略
 *          token       当type为kTypeFullJson时，将根据EDP协议封装token字段，
 *                      为其它类型时将被忽略。
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketSavedataString(SaveDataType type, const char* dst_devid, 
				const char* ds_id, const char* value, 
				time_t at, const char* token);

/* 
 * 函数名:  UnpackSavedataInt
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据
 * 说明:    接收设备云发来的数据,将其中的数据流ID及值解析出来。
 *
 * 相关函数:PacketSavedataInt
 *          
 * 参数:    type        采用的JSON数据类型，可选类型为：kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          pkg         EDP包, 必须是savedata包
 *          ds_id       获取数据流ID，使用完后必须释放
 *          value       数据流对应的值
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, -1 type类型不合法，其它值见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedataInt(SaveDataType type, EdpPacket* pkg,
				     char** ds_id, int* value);

/* 
 * 函数名:  UnpackSavedataDouble
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据
 * 说明:    接收设备云发来的数据,将其中的数据流ID及值解析出来。
 *
 * 相关函数:PacketSavedataDouble
 *          
 * 参数:    type        采用的JSON数据类型，可选类型为：kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          pkg         EDP包, 必须是savedata包
 *          ds_id       获取数据流ID，使用完后必须释放
 *          value       数据流对应的值
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, -1 type类型不合法，其它值见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedataDouble(SaveDataType type, EdpPacket* pkg,
					char** ds_id, double* value);

/* 
 * 函数名:  UnpackSavedataString
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据
 * 说明:    接收设备云发来的数据,将其中的数据流ID及值解析出来。
 *
 * 相关函数:PacketSavedataString
 *          
 * 参数:    type        采用的JSON数据类型，可选类型为：kTypeFullJson, 
 *                      kTypeSimpleJsonWithoutTime, kTypeSimpleJsonWithTime
 *          pkg         EDP包, 必须是savedata包
 *          ds_id       获取数据流ID，使用完后需要释放
 *          value       数据流对应的值，使用完后需要释放
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, -1 type类型不合法，其它值见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedataString(SaveDataType type, EdpPacket* pkg,
					char** ds_id, char** value);


/* 
 * 函数名:  UnpackSavedataAck
 * 功能:    解包 由设备云到设备的EDP协议包, 存贮（转发）消息的响应
 * 说明:    当存贮（转发）消息带token时，平台会响应一个SAVE_ACK消息，
 *          用作存储消息的确认。
 * 相关函数: PacketSavedataDoubleWithToken PacketSavedataStringWithToken
 * 参数:    pkg         EDP包, 必须是连接响应包
 *          json_ack    获取响应的json字符串，使用完后需要释放
 * 返回值:  类型 (int32) 
 *          =0          心跳成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedataAck(EdpPacket* pkg, char** json_ack);

/* 
 * 函数名:  PacketSavedataSimpleString
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(以分号分隔的简单字符串形式)
 * 说明:    返回的EDP包发送给设备云后, 需要删除这个包
 * 相关函数:UnpackSavedataSimpleString
 * 参数:    dst_devid   目的设备ID
 *          input       以分号分隔的简单字符串形式，
 *                      详见《设备终端接入协议2-EDP.docx》
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketSavedataSimpleString(const char* dst_devid, const char* input);

/* 
 * 函数名:  UnpackSavedataSimpleString
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据
 * 说明:    接收设备云发来的数据, 通过函数GetEdpPacket和EdpPacketType判断出是savedata后,
 *          将整个响应EDP包作为参数, 由该函数进行解析,
 *          获取源端发送来的以分号作为分隔符的字符串。
 * 相关函数: PacketSavedataSimpleString
 *          
 * 参数:    pkg         EDP包, 必须是savedata包
 *          output      存储发送来的字符串
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedataSimpleString(EdpPacket* pkg, char** output);

/* 
 * 函数名:  PacketSavedataBin
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(bin格式数据)
 * 说明:    返回的EDP包发送给设备云后, 需要删除这个包
 * 相关函数:UnpackSavedata, UnpackSavedataBin
 * 参数:    dst_devid   目的设备ID
 *          desc_obj    数据描述 json格式
 *          bin_data    二进制数据
 *          bin_len     二进制数据长度
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EdpPacket* PacketSavedataBin(const char* dst_devid, 
        cJSON* desc_obj, uint8* bin_data, uint32 bin_len);
/* 
 * 函数名:  PacketSavedataBinStr
 * 功能:    打包 设备到设备云的EDP协议包, 存储数据(bin格式数据)
 * 说明:    返回的EDP包发送给设备云后, 需要删除这个包
 * 相关函数:UnpackSavedata, UnpackSavedataBin
 * 参数:    dst_devid   目的设备ID
 *          desc_obj    数据描述 字符串格式
 *          bin_data    二进制数据
 *          bin_len     二进制数据长度
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketSavedataBinStr(const char* dst_devid, 
        const char* desc_str, const uint8* bin_data, uint32 bin_len);

/* 
 * 函数名:  UnpackSavedata
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据
 * 说明:    接收设备云发来的数据, 通过函数GetEdpPacket和EdpPacketType判断出是savedata后,
 *          将整个响应EDP包作为参数, 由该函数进行解析 
 *          然后再根据json和bin的标识(jb_flag), 调用相应的解析函数
 *          返回的源设备ID(src_devid)需要客户端释放
 * 相关函数:PacketSavedataJson, PacketSavedataBin, GetEdpPacket, 
 *          UnpackSavedataJson, UnpackSavedataBin
 * 参数:    pkg         EDP包, 必须是savedata包
 *          src_devid   源设备ID
 *          jb_flag     json or bin数据, 1: json, 2: 二进制
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedata(EdpPacket* pkg, char** src_devid, uint8* jb_flag);

/* 
 * 函数名:  UnpackSavedataJson
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据(json格式数据)
 * 说明:    返回的json数据(json_obj)需要客户端释放
 * 相关函数:PacketSavedataJson, GetEdpPacket, EdpPacketType, UnpackSavedata
 * 参数:    pkg         EDP包, 必须是savedata包的json数据包
 *          json_obj    json数据 
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
int32 UnpackSavedataJson(EdpPacket* pkg, cJSON** json_obj);

/* 
 * 函数名:  UnpackSavedataBin
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据(bin格式数据)
 * 说明:    返回的数据描述(desc_obj)和bin数据(bin_data)需要客户端释放
 * 相关函数:PacketSavedataBin, GetEdpPacket, EdpPacketType, UnpackSavedata
 * 参数:    pkg         EDP包, 必须是savedata包的bin数据包
 *          desc_obj    数据描述 json格式
 *          bin_data    二进制数据
 *          bin_len     二进制数据长度
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
int32 UnpackSavedataBin(EdpPacket* pkg, cJSON** desc_obj, 
        uint8** bin_data, uint32* bin_len);
/* 
 * 函数名:  UnpackSavedataBinStr
 * 功能:    解包 由设备云到设备的EDP协议包, 存储数据(bin格式数据)
 * 说明:    返回的数据描述(desc_obj)和bin数据(bin_data)需要客户端释放
 * 相关函数:PacketSavedataBin, GetEdpPacket, EdpPacketType, UnpackSavedata
 * 参数:    pkg         EDP包, 必须是savedata包的bin数据包
 *          desc_obj    数据描述 string格式
 *          bin_data    二进制数据
 *          bin_len     二进制数据长度
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackSavedataBinStr(EdpPacket* pkg, char** desc_str, 
        uint8** bin_data, uint32* bin_len);
/* 
 * 函数名:  PacketCmdResp
 * 功能:    向接入机发送命令响应
 * 说明:    返回的EDP包发送给设备云后, 需要客户程序删除该包
 *          
 * 相关函数:UnpackCmdReq
 * 参数:    cmdid       命令id
 *          cmdid_len   命令id长度
 *          resp        响应的消息
 *          resp_len    响应消息长度
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketCmdResp(const char* cmdid, uint16 cmdid_len, 
        const char* resp, uint32 resp_len);

/* 
 * 函数名:  UnpackCmdReq
 * 功能:    解包 由设备云到设备的EDP协议包, 命令请求消息
 * 说明:    接收设备云发来的数据, 解析命令请求消息包
 *          获取的cmdid以及req需要在使用后释放。
 * 相关函数:PacketCmdResp
 * 参数:    pkg         EDP包
 *          cmdid       获取命令id
 *          cmdid_len   cmdid的长度
 *          req         用户命令的起始位置
 *          req_len     用户命令的长度
 * 返回值:  类型 (int32) 
 *          =0          解析成功
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackCmdReq(EdpPacket* pkg, char** cmdid, uint16* cmdid_len, 
			      char** req, uint32* req_len);

/* 
 * 函数名:  PacketPing
 * 功能:    打包 由设备到设备云的EDP协议包, 心跳
 * 说明:    返回的EDP包发送给设备云后, 需要客户程序删除该包
 *          设备云会回复心跳响应给设备
 * 相关函数:UnpackPingResp
 * 参数:    无
 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketPing(void);

/* 
 * 函数名:  UnpackPingResp
 * 功能:    解包 由设备云到设备的EDP协议包, 心跳响应
 * 说明:    接收设备云发来的数据, 通过函数GetEdpPacket和EdpPacketType判断出是连接响应后, 
 *          将整个响应EDP包作为参数, 由该函数进行解析
 * 相关函数:PacketPing, GetEdpPacket, EdpPacketType
 * 参数:    pkg         EDP包, 必须是连接响应包
 * 返回值:  类型 (int32) 
 *          =0          心跳成功
 *          >0          心跳失败, 具体失败原因见<OneNet接入方案与接口.docx>
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackPingResp(EdpPacket* pkg);

#ifdef __cplusplus
}
#endif

#endif /* __EDP_KIT_H__ */
