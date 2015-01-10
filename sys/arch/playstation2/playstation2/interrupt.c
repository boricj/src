/*	$NetBSD: interrupt.c,v 1.11 2014/03/31 11:25:49 martin Exp $	*/

/*-
 * Copyright (c) 2001 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: interrupt.c,v 1.11 2014/03/31 11:25:49 martin Exp $");

#include "debug_playstation2.h"
#if defined INTR_DEBUG && !defined GSFB_DEBUG_MONITOR
#error "add option GSFB_DEBUG_MONITOR"
#endif

#include <sys/param.h>
#include <sys/malloc.h>

#include <uvm/uvm_extern.h>	/* uvmexp.intrs */

#include <machine/locore.h>	/* mips3_cp0_*() */

#include <playstation2/playstation2/interrupt.h>

#include <playstation2/ee/eevar.h>
#include <playstation2/ee/intcvar.h>
#include <playstation2/ee/intcreg.h>
#include <playstation2/ee/dmacreg.h>
#include <playstation2/ee/dmacvar.h>
#include <playstation2/ee/timervar.h>

#ifdef INTR_DEBUG
#include <playstation2/ee/gsvar.h>	/* debug monitor */
#endif

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

struct _playstation2_evcnt _playstation2_evcnt = {
	.clock	= EVCNT_INITIALIZER(EVCNT_TYPE_INTR, NULL, "hard", "clock"),
	.sbus	= EVCNT_INITIALIZER(EVCNT_TYPE_INTR, NULL, "hard", "sbus"),
	.dmac	= EVCNT_INITIALIZER(EVCNT_TYPE_INTR, NULL, "hard", "dmac"),
};

/*
STATIC struct {
	u_int32_t sr, imask;
} _sif_call_env;
*/

struct clockframe playstation2_clockframe;
//struct playstation2_soft_intr playstation2_soft_intrs[_IPL_NSOFT];
struct playstation2_soft_intrhand *softnet_intrhand;

u_int32_t __icu_mask[_IPL_N];	/* interrupt mask of DMAC/INTC */
volatile u_int32_t md_imask;

#ifdef INTR_DEBUG
void _debug_print_ipl(void);
void _debug_print_intr(const char *);
#endif /* INTR_DEBUG */

void
interrupt_init_bootstrap(void)
{
	int i;

	/* initialize interrupt mask (masked all) */
	for (i = 0; i < _IPL_N; i++)
		__icu_mask[i] = 0xffffffff;
#if 0
	/* intialize EE embeded device */
	timer_init();

	/* clear all pending interrupt and disable all */
	intc_init(); /* INT0 */
	dmac_init(); /* INT1 */
#endif
}

void
interrupt_init(void)
{
#if 0
	struct playstation2_soft_intr *asi;
	int i;

	evcnt_attach_static(&_playstation2_evcnt.clock);
	evcnt_attach_static(&_playstation2_evcnt.sbus);
	evcnt_attach_static(&_playstation2_evcnt.dmac);

	/* install software interrupt handler */
	intc_intr_establish(I_CH10_TIMER1, IPL_SOFT, timer1_intr, 0);
	intc_intr_establish(I_CH11_TIMER2, IPL_SOFTCLOCK, timer2_intr, 0);

	/* IPL_SOFTNET and IPL_SOFTSERIAL are shared interrupt. */
	intc_intr_establish(I_CH12_TIMER3, IPL_SOFTNET, timer3_intr, 0);

	/* enable SIF BIOS access */
	md_imask = ~D_STAT_CIM_BIT(D_CH5_SIF0);
	mips_cp0_status_write(0x00010801);
#endif
}

/*
 *  Hardware interrupt support
 */
void
cpu_intr(int ppl, vaddr_t pc, uint32_t status)
{
	struct cpu_info *ci;

#if 0
	_debug_print_intr(__func__);
#endif

	ci = curcpu();
	ci->ci_idepth++;
	//uvmexp.intrs++;

	//playstation2_clockframe.ppl = md_imask;
	playstation2_clockframe.sr = status;
	playstation2_clockframe.pc = pc;

	/*if (ipending & MIPS_INT_MASK_0) {
		intc_intr(md_imask);
	}
	
	if (ipending & MIPS_INT_MASK_1) {
		_playstation2_evcnt.dmac.ev_count++;
		dmac_intr(md_imask);
	}*/
	ci->ci_idepth--;
}

