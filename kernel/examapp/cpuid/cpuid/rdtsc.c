/*
 * Copyright 2008  Veselin Georgiev,
 * anrieffNOSPAM @ mgail_DOT.com (convert to gmail)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "libcpuid.h"
#include "libcpuid_util.h"
#include "asm-bits.h"
#include "rdtsc.h"

/* out = a - b */
static void mark_t_subtract(struct cpu_mark_t* a, struct cpu_mark_t* b, struct cpu_mark_t *out)
{
	out->tsc = a->tsc - b->tsc;
	out->sys_clock = a->sys_clock - b->sys_clock;
}

int cpu_clock_by_mark(struct cpu_mark_t* mark)
{
	uint64_t result;

	/* Check if some subtraction resulted in a negative number: */
	if ((mark->tsc >> 63) != 0 || (mark->sys_clock >> 63) != 0) return -1;

	/* Divide-by-zero check: */
	if (mark->sys_clock == 0) return -1;

	/* Check if the result fits in 32bits */
	result = mark->tsc / mark->sys_clock;
	if (result > (uint64_t) 0x7fffffff) return -1;
	return (int) result;
}

/* Emulate doing useful CPU intensive work */
static int busy_loop(int amount)
{
	int i, j, k, s = 0;
	static volatile int data[42] = {32, 12, -1, 5, 23, 0 };
	for (i = 0; i < amount; i++)
		for (j = 0; j < 65536; j++)
			for (k = 0; k < 42; k++)
				s += data[k];
	return s;
}

static void adjust_march_ic_multiplier(const struct cpu_id_t* id, int* numerator, int* denom)
{
	/*
	 * for cpu_clock_by_ic: we need to know how many clocks does a typical ADDPS instruction
	 * take, when issued in rapid succesion without dependencies. The whole idea of
	 * cpu_clock_by_ic was that this is easy to determine, at least it was back in 2010. Now
	 * it's getting progressively more hairy, but here are the current measurements:
	 *
	 * 1. For CPUs with  64-bit SSE units, ADDPS issue rate is 0.5 IPC (one insn in 2 clocks)
	 * 2. For CPUs with 128-bit SSE units, issue rate is exactly 1.0 IPC
	 * 3. For Bulldozer and later, it is 1.4 IPC (we multiply by 5/7)
	 * 4. For Skylake and later, it is 1.6 IPC (we multiply by 5/8)
	 */
	//
	if (id->sse_size < 128) {
		debugf(1, "SSE execution path is 64-bit\n");
		// on a CPU with half SSE unit length, SSE instructions execute at 0.5 IPC;
		// the resulting value must be multiplied by 2:
		*numerator = 2;
	} else {
		debugf(1, "SSE execution path is 128-bit\n");
	}
	//
	// Bulldozer or later: assume 1.4 IPC
	if ((id->vendor == VENDOR_AMD && id->ext_family >= 21) || (id->vendor == VENDOR_HYGON)) {
		debugf(1, "cpu_clock_by_ic: Bulldozer (or later) detected, dividing result by 1.4\n");
		*numerator = 5;
		*denom = 7; // multiply by 5/7, to divide by 1.4
	}
	//
	// Skylake or later: assume 1.6 IPC
	if (id->vendor == VENDOR_INTEL && id->ext_model >= 94) {
		debugf(1, "cpu_clock_by_ic: Skylake (or later) detected, dividing result by 1.6\n");
		*numerator = 5;
		*denom = 8; // to divide by 1.6, multiply by 5/8
	}
}
