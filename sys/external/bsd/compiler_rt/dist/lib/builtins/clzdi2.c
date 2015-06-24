/* ===-- clzdi2.c - Implement __clzdi2 -------------------------------------===
 *
 *               The LLVM Compiler Infrastructure
 *
 * This file is dual licensed under the MIT and the University of Illinois Open
 * Source Licenses. See LICENSE.TXT for details.
 *
 * ===----------------------------------------------------------------------===
 *
 * This file implements __clzdi2 for the compiler_rt library.
 *
 * ===----------------------------------------------------------------------===
 */

#include "int_lib.h"

COMPILER_RT_ABI si_int
__clzsi2(si_int a);

/* Returns: the number of leading 0-bits */

/* Precondition: a != 0 */

COMPILER_RT_ABI si_int
__clzdi2(di_int a)
{
    dwords x;
    x.all = a;
    const si_int f = -(x.s.high == 0);
    return __clzsi2((x.s.high & ~f) | (x.s.low & f)) +
           (f & ((si_int)(sizeof(si_int) * CHAR_BIT)));
}
