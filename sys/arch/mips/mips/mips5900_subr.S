/*	$NetBSD: mips5900_subr.S,v 1.1 2002/03/05 15:51:00 simonb Exp $	*/

/* Toshiba R5900 specific functions */

#include <mips/asm.h>
#include <mips/cpuregs.h>

	.set	noreorder
	.set	mips3
	.text
	.align 6	/* align cache line size (64B) */

/*
 * _intr_resume, _intr_suspend:
 *	Fast interrupt locking functions.
 */
LEAF_NOPROFILE(_intr_resume)
	beqz	a0, 1f
	nop
	.word 0x42000039	/* ei */
1:
	j	ra
	nop
END(_intr_resume)

LEAF_NOPROFILE(_intr_suspend)
	mfc0	v0, MIPS_COP_0_STATUS
	lui	t0, 0x1		/* EIE bit */
	and	v0, v0, t0
1:	.word 0x42000039	/* di */
	sync.p
	mfc0	t1, MIPS_COP_0_STATUS
	and	t2, t0, t1
	beq	t2, t0, 1b
	nop
	j	ra
	nop
END(_intr_suspend)
