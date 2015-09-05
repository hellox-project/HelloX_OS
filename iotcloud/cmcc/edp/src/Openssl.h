/*
 * 【说明】
 * 此文件利用openssl库实现加解密，
 * 如果需要加密功能又没有openssl库，则需要自己实现
 */
 
#ifndef _OPENSSL_20150710_69toglgn2b_H_
#define _OPENSSL_20150710_69toglgn2b_H_

#ifdef _ENCRYPT

#include <stdlib.h>
#include "EdpKit.h"
#include <openssl/rsa.h>
#include <openssl/aes.h>

typedef enum{
    kTypeAes = 1
}EncryptAlgType;

/* 
 * 函数名:  PacketEncryptReq
 * 功能:    打包 由设备到设备云的EDP协议包, 加密请求
 * 说明:    返回的EDP包发送给设备云后, 需要客户程序删除该包
 *          设备云会回复加密响应给设备
 * 相关函数:UnpackEncryptResp
 * 参数:    type 对称加密算法类型，目前只支持AES，参数为kTypeAes

 * 返回值:  类型 (EdpPacket*) 
 *          非空        EDP协议包
 *          为空        EDP协议包生成失败 
 */
EDPKIT_DLL EdpPacket* PacketEncryptReq(EncryptAlgType type);

/* 
 * 函数名:  UnpackEncryptResp
 * 功能:    解包,根据指定的对称加密算法类型初始化相应
 *          对称加密算法
 *
 * 相关函数:PacketEncryptReq
 * 参数:    pkg         EDP包, 必须是连接响应包
 * 返回值:  类型 (int32) 
 *          =0          连接成功
 *          >0          连接失败, 具体失败原因见<OneNet接入方案与接口.docx>
 *          <0          解析失败, 具体失败原因见本h文件的错误码
 */
EDPKIT_DLL int32 UnpackEncryptResp(EdpPacket* pkg);

/* 
 * 函数名:  SymmEncrypt
 * 功能:    将pkg的remain_len之后听数据加密，
 *          加密之后的数据依然存放于pkg指示的空间内
 *
 * 相关函数:SymmDecrypt
 * 参数:    pkg         EDP包
 * 返回值:  类型 (int32) 
 *          >=0         需要加密的数据加密后数据长度
 *          <0          加密失败
 */
EDPKIT_DLL int SymmEncrypt(EdpPacket* pkg);

/* 
 * 函数名:  UnpackEncryptResp
 * 功能:    解包,根据指定的对称加密算法类型初始化相应
 *          对称加密算法的key
 *
 * 相关函数:PacketEncryptReq
 * 参数:    pkg         EDP包

 * 返回值:  类型 (int32) 
 *          >=0         需要解密的数据解密后的长度
 *          <0          解密失败
 */
EDPKIT_DLL int SymmDecrypt(EdpPacket* pkg);

#endif
#endif
