/*
 * sshbn.h: the assorted conditional definitions of BignumInt and
 * multiply/divide macros used throughout the bignum code to treat
 * numbers as arrays of the most conveniently sized word for the
 * target machine. Exported so that other code (e.g. poly1305) can use
 * it too.
 */

/*
 * Usage notes:
 *  * Do not call the DIVMOD_WORD macro with expressions such as array
 *    subscripts, as some implementations object to this (see below).
 *  * Note that none of the division methods below will cope if the
 *    quotient won't fit into BIGNUM_INT_BITS. Callers should be careful
 *    to avoid this case.
 *    If this condition occurs, in the case of the x86 DIV instruction,
 *    an overflow exception will occur, which (according to a correspondent)
 *    will manifest on Windows as something like
 *      0xC0000095: Integer overflow
 *    The C variant won't give the right answer, either.
 */

//typedef unsigned __int32 BignumInt;
//typedef unsigned __int64 BignumDblInt;
#define BIGNUM_INT_MASK  0xFFFFFFFFUL
#define BIGNUM_TOP_BIT   0x80000000UL
#define BIGNUM_INT_BITS  32
#define MUL_WORD(w1, w2) ((BignumDblInt)w1 * w2)
#define DIVMOD_WORD(q, r, hi, lo, w) do { \
	__asm mov edx, hi \
	__asm mov eax, lo \
	__asm div w \
	__asm mov r, edx \
	__asm mov q, eax \
} while(0)

#define BIGNUM_INT_BYTES (BIGNUM_INT_BITS / 8)
