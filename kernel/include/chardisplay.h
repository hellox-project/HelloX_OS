//***********************************************************************/
//    Author                    : tywind
//    Original Date             : oct,13 2014
//    Module Name               : chardisplay.h
//    Module Funciton           : char display api
//                                
//    Last modified Author      : tywind
//    Last modified Date        : oct,11 2014
//***********************************************************************/

#ifndef __CHARDISPLAY_H__
#define __CHARDISPLAY_H__

#ifndef __TYPES_H__
#include "TYPES.H"
#endif

#ifndef __CONFIG_H__
#include "..\config\config.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

//ɾ��ģʽ
#define  DISPLAY_DELCHAR_PREV        0      //ɾ�����ǰ��һ���ַ�
#define  DISPLAY_DELCHAR_CURR        1      //ɾ����굱ǰλ��һ���ַ�

//��ʼ����ʾ�豸
VOID CD_InitDisplay(INT nDisplayMode);

//������ʾģʽ
VOID CD_SetDisplayMode(INT nMode);

//�õ��к���
VOID CD_GetDisPlayRang(WORD* pLines,WORD* pColums);

//����
VOID CD_ChangeLine();

//�õ���ǰ���λ��
VOID  CD_GetCursorPos(WORD* pCursorX,WORD* pCursorY);

//���õ�ǰ���λ��
VOID  CD_SetCursorPos(WORD nCursorX,WORD nCursorY);

//��ӡ�ַ���,cl��ʾ�Ƿ���
VOID CD_PrintString(LPSTR pStr,BOOL cl);

//��ӡһ���ַ�
VOID CD_PrintChar(CHAR ch);

//��ָ��λ�õõ��ַ���
VOID  CD_GetString(WORD nCursorX,WORD nCursorY,LPSTR pString,INT nBufLen);

//ɾ���ַ���
VOID  CD_DelString(WORD nCursorX,WORD nCursorY,INT nDelLen);	

//ɾ���ַ�
VOID  CD_DelChar(INT nDelMode);	

//����
VOID  CD_Clear();

#ifdef __cplusplus
}
#endif

#endif    //cd.h
