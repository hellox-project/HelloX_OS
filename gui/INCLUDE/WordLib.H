#ifndef WordLib_h__
#define WordLib_h__

//Base address of ASCII code map library.
#define ASCII_LIB_BASE   0x00190000
//Base address of chinese character image library.
#define CHNCHAR_LIB_BASE 0x001A0000

//Display chinese character.
VOID DispHZK16(int x, int y, unsigned char *pHZ  , __COLOR color);
//Display ASCII code.
VOID DispASC16(int x, int y, unsigned char *pXZ  , __COLOR color);
//Display a chinese and ASCII mixed string.
VOID TextOut(int x,int y,char *pStr,__COLOR color);

#endif // WordLib_h__