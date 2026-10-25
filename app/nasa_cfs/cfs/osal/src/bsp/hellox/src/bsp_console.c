/*
 * bsp_console.c - HelloX OSAL BSP console output.
 */

#include <hellox.h>

#include "hellox_bsp_internal.h"
#include "bsp-impl.h"

/* ---------------------------------------------------------
 * OS_BSP_ConsoleOutput_Impl - write raw bytes to the console.
 *
 * HelloX's PrintChar() syscall does NOT interpret control
 * characters: a '\n' would be emitted as a raw 0x0A glyph and
 * the cursor would simply advance one column instead of moving
 * to the start of the next line.  Match the behavior of hxlib's
 * own _hx_printf() (stdio.c) by translating:
 *     '\n' -> carriage-return + line-feed (GotoHome + ChangeLine)
 *     '\r' -> carriage-return (GotoHome)
 * --------------------------------------------------------- */
void OS_BSP_ConsoleOutput_Impl(const char *Str, size_t DataLen)
{
    size_t i;

    for (i = 0; i < DataLen; i++)
    {
        if (Str[i] == '\n')
        {
            GotoHome();
            ChangeLine();
        }
        else if (Str[i] == '\r')
        {
            GotoHome();
        }
        else
        {
            PrintChar(Str[i]);
        }
    }
}

/* ---------------------------------------------------------
 * OS_BSP_ConsoleSetMode_Impl - no-op (no terminal control on HelloX).
 * --------------------------------------------------------- */
void OS_BSP_ConsoleSetMode_Impl(uint32 ModeBits)
{
}
