/*
stdbool.h

-- Boolean type and values
(substitute for missing C99 standard header)

public-domain implementation from [EMAIL PROTECTED]

implements subclause 7.16 of ISO/IEC 9899:1999 (E)
*/

#ifndef __STDBOOL_H__
#define __STDBOOL_H__

#include "__hxcomm.h"

#ifndef __bool_true_false_are_defined
#define __bool_true_false_are_defined 1

/* program is allowed to contain its own definitions, so ... */
#if defined(bool)
#undef bool
#endif

#if defined(true)
#undef true
#endif

#if defined(false)
#undef false
#endif

#define bool int
#define true 1
#define false 0

#endif /* !defined(__bool_true_false_are_defined) */

#endif //__STDBOOL_H__
