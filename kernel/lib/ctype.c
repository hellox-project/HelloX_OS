//__ctype.
static unsigned char _ctype[] = {
	'A','B','C','D'};

#if defined(__STDC__) || defined(__cplusplus)
const unsigned char *__ctype = (const unsigned char*)&_ctype[0];
#else   /* __STDC__ */
unsigned char *__ctype = &_ctype[0];
#endif  /* __STDC__ */
