/*
  * stdarg.h
  *
  * Provides facilities for stepping through a list of function arguments of
  * an unknown number and type.
  *
  * NOTE: Gcc should provide stdarg.h, and I believe their version will work
  *       with crtdll. If necessary I think you can replace this with the GCC
  *       stdarg.h.
  *
  * Note that the type used in va_arg is supposed to match the actual type
  * *after default promotions*. Thus, va_arg (..., short) is not valid.
  *
  * This file is part of the Mingw32 package.
  *
  * Contributors:
  *  Created by Colin Peters <colin@bird.fu.is.saga-u.ac.jp>
  *
  *  THIS SOFTWARE IS NOT COPYRIGHTED
  *
  *  This source code is offered for use in the public domain. You may
  *  use, modify or distribute it freely.
  *
  *  This code is distributed in the hope that it will be useful but
  *  WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
  *  DISCLAMED. This includes but is not limited to warranties of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  *
  * $Revision: 1.2 $
  * $Author: noer $
  * $Date: 1998/10/10 00:51:16 $
  *
  */

#ifndef __STDARG_H__
#define __STDARG_H__

#include "__hxcomm.h"

/*
 * Don't do any of this stuff for the resource compiler.
 */
#ifndef RC_INVOKED

 /*
  * I was told that Win NT likes this.
  */
#ifndef _VA_LIST_DEFINED
#define _VA_LIST_DEFINED
#endif

#ifndef __VA_LIST_DEFINED__
#define __VA_LIST_DEFINED__
typedef char* va_list;
#endif

/*
 * Amount of space required in an argument list (ie. the stack) for an
 * argument of type t.
 */
#define __va_argsiz(t)    \
    (((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

/*
 * Start variable argument list processing by setting AP to point to the
 * argument after pN.
 */
#ifdef    __GNUC__
/*
 * In GNU the stack is not necessarily arranged very neatly in order to
 * pack shorts and such into a smaller argument list. Fortunately a
 * neatly arranged version is available through the use of __builtin_next_arg.
 */
#ifndef __VA_START_DEFINED__
#define __VA_START_DEFINED__
#define va_start(ap, pN)    \
    ((ap) = ((va_list) __builtin_next_arg(pN)))
#endif //__VA_START_DEFINED__
#else
/*
 * For a simple minded compiler this should work (it works in GNU too for
 * vararg lists that don't follow shorts and such).
 */
#ifndef __VA_START_DEFINED__
#define __VA_START_DEFINED__
#define va_start(ap, pN)    \
    ((ap) = ((va_list) (&pN) + __va_argsiz(pN)))
#endif // __VA_START_DEFINED__
#endif //__GNUC__

/*
 * End processing of variable argument list. In this case we do nothing.
 */
#ifndef __VA_END_DEFINED__
#define __VA_END_DEFINED__
#define va_end(ap)    ((void)0)
#endif // __VA_END_DEFINED__

/*
 * Increment ap to the next argument in the list while returing a
 * pointer to what ap pointed to first, which is of type t.
 *
 * We cast to void* and then to t* because this avoids a warning about
 * increasing the alignment requirement.
 */
#ifndef __VA_ARG_DEFINED__
#define __VA_ARG_DEFINED__
#define va_arg(ap, t)                    \
     (((ap) = (ap) + __va_argsiz(t)),        \
     *((t*) (void*) ((ap) - __va_argsiz(t))))
#endif // __VA_ARG_DEFINED__

#endif /* Not RC_INVOKED */
#endif // __STDARG_H__
