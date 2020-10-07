/* $Id: scanf.c,v 1.2 2002/08/09 20:56:57 pefo Exp $ */

/*
 * Copyright (c) 2000-2002 Opsycon AB  (www.opsycon.se)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Opsycon AB.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "io.h"
#include "stdlib.h"

 /*
  * ** fscanf --\    sscanf --\
  * **          |                  |
  * **  scanf --+-- vfscanf ----- vsscanf
  * **
  * ** This not been very well tested.. it probably has bugs
  */
static int vsscanf(const char *, const char *, va_list);

#define ISSPACE " \t\n\r\f\v"

/* Local helpers. */
static int isspace(int x)
{
	if (x == ' ' || x == '\t' || x == '\n' || x == '\f' || x == '\b' || x == '\r')
		return 1;
	else
		return 0;
}

static int isdigit(int x)
{
	if (x <= '9'&&x >= '0')
		return 1;
	else
		return 0;
}

#if 0

/*
 *  scanf(fmt,va_alist)
 */
int
scanf(const char *fmt, ...)
{
	int count;
	va_list ap;

	va_start(ap, fmt);
	count = vfscanf(stdin, fmt, ap);
	va_end(ap);
	return (count);
}

/*
 *  fscanf(fp,fmt,va_alist)
 */
int
fscanf(FILE *fp, const char *fmt, ...)
{
	int             count;
	va_list ap;

	va_start(ap, fmt);
	count = vfscanf(fp, fmt, ap);
	va_end(ap);
	return (count);
}
#endif 

/*
 *  sscanf(buf,fmt,va_alist)
 */
int
sscanf(const char *buf, const char *fmt, ...)
{
	int             count;
	va_list ap;

	va_start(ap, fmt);
	count = vsscanf(buf, fmt, ap);
	va_end(ap);
	return (count);
}

#if 0

/*
 *  vfscanf(fp,fmt,ap)
 */
static int
vfscanf(FILE *fp, const char *fmt, va_list ap)
{
	int             count;
	char            buf[MAX_BUFFER_SIZE + 1];

	if (fgets(buf, MAX_BUFFER_SIZE, fp) == 0)
		return (-1);
	count = vsscanf(buf, fmt, ap);
	return (count);
}
#endif

/*
 *  vsscanf(buf,fmt,ap)
 */
static int
vsscanf(const char *buf, const char *s, va_list ap)
{
	int             count, noassign, width, base, lflag;
	const char     *tc;
	char           *t, tmp[MAX_BUFFER_SIZE];

	count = noassign = width = lflag = 0;
	while (*s && *buf) {
		while (isspace(*s))
			s++;
		if (*s == '%') {
			s++;
			for (; *s; s++) {
				if (strchr("dibouxcsefg%", *s))
					break;
				if (*s == '*')
					noassign = 1;
				else if (*s == 'l' || *s == 'L')
					lflag = 1;
				else if (*s >= '1' && *s <= '9') {
					for (tc = s; isdigit(*s); s++);
					strncpy(tmp, (char*)tc, s - tc);
					tmp[s - tc] = '\0';
					atob(&width, tmp, 10);
					s--;
				}
			}
			if (*s == 's') {
				while (isspace(*buf))
					buf++;
				if (!width)
					width = strcspn(buf, ISSPACE);
				if (!noassign) {
					strncpy(t = va_arg(ap, char *), (char*)buf, width);
					t[width] = '\0';
				}
				buf += width;
			}
			else if (*s == 'c') {
				if (!width)
					width = 1;
				if (!noassign) {
					strncpy(t = va_arg(ap, char *), (char*)buf, width);
					t[width] = '\0';
				}
				buf += width;
			}
			else if (strchr("dobxu", *s)) {
				while (isspace(*buf))
					buf++;
				if (*s == 'd' || *s == 'u')
					base = 10;
				else if (*s == 'x')
					base = 16;
				else if (*s == 'o')
					base = 8;
				else if (*s == 'b')
					base = 2;
				if (!width) {
					if (isspace(*(s + 1)) || *(s + 1) == 0)
						width = strcspn(buf, ISSPACE);
					else
						width = strchr(buf, *(s + 1)) - buf;
				}
				strncpy(tmp, (char*)buf, width);
				tmp[width] = '\0';
				buf += width;
				if (!noassign)
					atob(va_arg(ap, uint32_t *), tmp, base);
			}
			if (!noassign)
				count++;
			width = noassign = lflag = 0;
			s++;
		}
		else {
			while (isspace(*buf))
				buf++;
			if (*s != *buf)
				break;
			else
				s++, buf++;
		}
	}
	return (count);
}
