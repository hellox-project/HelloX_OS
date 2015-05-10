#include "stdafx.h"
#include "make_usb_boot.h"



INT APIENTRY GetStringInt(LPTSTR pString)
{
	LPTSTR	 pNumEnd     = StrStrI(pString,L" ");
	TCHAR    szNum[128]  = {0} ;

	if(pNumEnd)
	{
		StrCpyN(szNum,pString,pNumEnd-pString+1);
	}
	else
	{
		StrCpyN(szNum,pString,128);
	}

	return StrToInt(szNum);
}

INT APIENTRY GetModelPath(HMODULE hModul,LPTSTR pModelPath,INT nBufLen)
{	
	GetModuleFileName(hModul,pModelPath,nBufLen);

	LPTSTR Pos = StrRChr(pModelPath,NULL,'\\');
	if(Pos != NULL)
	{
		Pos++;*Pos = 0;
	}

	return S_OK;
}

TCHAR APIENTRY GetDriveFromMask(UINT nMask)  
{  
	TCHAR i;  

	for (i = 0; i < 26; ++i)  
	{  
		if (nMask & 0x1)  
			break;  
		nMask = nMask >> 1;  
	}  

	return (i + 'A');  
}  