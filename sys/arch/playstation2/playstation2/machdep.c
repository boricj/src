/*	$NetBSD: machdep.c,v 1.30 2014/03/31 11:25:49 martin Exp $	*/

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
__KERNEL_RCSID(0, "$NetBSD: machdep.c,v 1.30 2014/03/31 11:25:49 martin Exp $");

#include "opt_ddb.h"
#include "opt_kloader.h"
#include "opt_kloader_kernel_path.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/buf.h>
#include <sys/reboot.h>
#include <sys/mount.h>
#include <sys/kcore.h>
#include <sys/boot_flag.h>
#include <sys/device.h>

#include <uvm/uvm_extern.h>

#ifdef DDB
#include <machine/db_machdep.h>
#include <ddb/db_sym.h>
#include <ddb/db_extern.h>
#ifndef DB_ELFSIZE
#error Must define DB_ELFSIZE!
#endif
#define ELFSIZE		DB_ELFSIZE
#include <sys/exec_elf.h>
#endif

#include <dev/cons.h>	/* cntab access (cpu_reboot) */
#include <machine/bootinfo.h>
#include <machine/psl.h>
#include <machine/intr.h>/* hardintr_init */
#include <playstation2/playstation2/sifbios.h>
#include <playstation2/playstation2/interrupt.h>
#include <playstation2/ee/timervar.h>
#include "../dev/sio.h"

#include <mips/cache.h>
#include <mips/locore.h>
#include <mips/cpuregs.h>

#if defined KLOADER_KERNEL_PATH && !defined KLOADER
#error "define KLOADER"
#endif
#ifdef KLOADER
#include <machine/kloader.h>
#endif

struct vm_map *mb_map;
struct vm_map *phys_map;
phys_ram_seg_t mem_clusters[VM_PHYSSEG_MAX];
int mem_cluster_cnt;
paddr_t avail_start = 0, avail_end = PS2_MEMORY_SIZE - BOOTINFO_BLOCK_SIZE;
void mach_init(void);

/*
 * Do all the stuff that locore normally does before calling main().
 */

void
mach_init(void)
{
	extern char kernel_text[], edata[], end[];
	void *kernend;
	size_t memstart, memsize;

	/*
	 * Clear the BSS segment.
	 */
	kernend = (void *)mips_round_page(end);
	memset(edata, 0, (char*)kernend - (char*)edata);

	/* disable all interrupt */
	//interrupt_init_bootstrap();

	/* enable SIF BIOS */
	//sifbios_init();

	consinit();

	printf("kernel_text=%p edata=%p end=%p\n", kernel_text, edata, end);
#ifdef DEBUG
	bootinfo_dump();
#endif
	/*
	 * Copy exception-dispatch code down to exception vector.
	 * Initialize locore-function vector.
	 * Clear out the I and D caches.
	 */
	cpu_setmodel("SONY PlayStation 2");
	mips_vector_init(NULL, false);
	uvm_setpagesize();
	physmem = btoc(PS2_MEMORY_SIZE);

	/*
	 * Load the rest of the available pages into the VM system.
	 */
	memstart = PS2_MEMORY_EE_RESERVED_SIZE;
	memsize = PS2_MEMORY_SIZE - PS2_MEMORY_EE_RESERVED_SIZE -
	    BOOTINFO_BLOCK_SIZE;

	/* kernel itself */
	mem_clusters[0].start = memstart;
	mem_clusters[0].size = memsize;
	mem_cluster_cnt = 1;

	mips_page_physload(MIPS_KSEG0_START, (vaddr_t)kernend,
	    mem_clusters, mem_cluster_cnt, NULL, 0);

	/*
	 * Initialize message buffer (at end of core).
	 */
	mips_init_msgbuf();

	/*
	 * Initialize the virtual memory system.
	 */
	pmap_bootstrap();

	/*
	 * Allocate uarea page for lwp0 and set it.
	 */
	mips_init_lwp0_uarea();

	/*
	 * Initialize debuggers, and break into them, if appropriate.
	 */
#ifdef DDB
	if (boothowto & RB_KDB)
		Debugger();
#endif
}

void consinit(void) {
	// Force sio for now
	cn_tab = &constab[0];
}

/*
 * Allocate memory for variable-sized tables,
 */

#include <sys/rnd.h>

extern rndsave_t *boot_rsp;
rndsave_t boot_rsp_data;

void
cpu_startup(void)
{
	/*
	 * Do the common startup items.
	 */
	cpu_startup_common();

	//interrupt_init();
	splsched();

	/* XXX:Pretend that the early startup code is a source of randomness */
	boot_rsp = &boot_rsp_data;
	boot_rsp_data.entropy=128;
	memcpy(&boot_rsp_data.data, (char*)0x80100000, sizeof(boot_rsp_data.data));
}

void
cpu_reboot(int howto, char *bootstr)
{
#ifdef KLOADER
	struct kloader_bootinfo kbi;
#endif
	static int waittime = -1;

	/* Take a snapshot before clobbering any registers. */
	if (curlwp)
		savectx(curpcb);

	if (cold) {
		howto |= RB_HALT;
		goto haltsys;
	}

	/* If "always halt" was specified as a boot flag, obey. */
	if (boothowto & RB_HALT) {
		howto |= RB_HALT;
	}

#ifdef KLOADER
	/* No bootinfo is required. */
	kloader_bootinfo_set(&kbi, 0, NULL, NULL, true);
#ifndef KLOADER_KERNEL_PATH
#define	KLOADER_KERNEL_PATH	"/netbsd"
#endif
/*	if ((howto & RB_HALT) == 0)
		kloader_reboot_setup(KLOADER_KERNEL_PATH);*/
#endif

	boothowto = howto;
	if ((howto & RB_NOSYNC) == 0 && (waittime < 0)) {
		waittime = 0;
		vfs_shutdown();

		/*
		 * If we've been adjusting the clock, the todr
		 * will be out of synch; adjust it now.
		 */
		resettodr();
	}

	splhigh();

	if (howto & RB_DUMP)
		dumpsys();

 haltsys:
	doshutdownhooks();

	pmf_system_shutdown(boothowto);

	if ((howto & RB_POWERDOWN) == RB_POWERDOWN) {
		///sifbios_halt(0); /* power down */
	}
	else if (howto & RB_HALT) {
		//sifbios_halt(1); /* halt */
	}
	else {
#ifdef KLOADER
		kloader_reboot();
		/* NOTREACHED */
#endif
		//sifbios_halt(2); /* reset */
	}

	while (1)
		;
	/* NOTREACHED */
}

#ifdef DEBUG
void
bootinfo_dump(void)
{
	printf("devconf=%#x, option=%#x, rtc=%#x, pcmcia_type=%#x,"
	    "sysconf=%#x\n",
	    BOOTINFO_REF(BOOTINFO_DEVCONF),
	    BOOTINFO_REF(BOOTINFO_OPTION_PTR),
	    BOOTINFO_REF(BOOTINFO_RTC),
	    BOOTINFO_REF(BOOTINFO_PCMCIA_TYPE),	
	    BOOTINFO_REF(BOOTINFO_SYSCONF));
}
#endif /* DEBUG */
