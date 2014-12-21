/*	$NetBSD: mainbus.c,v 1.12 2014/03/31 11:25:49 martin Exp $	*/

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
__KERNEL_RCSID(0, "$NetBSD: mainbus.c,v 1.12 2014/03/31 11:25:49 martin Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/bus.h>

#include <machine/autoconf.h>

static int	mainbus_match(device_t, cfdata_t, void *);
static void	mainbus_attach(device_t, device_t, void *);
static int	mainbus_submatch(device_t, cfdata_t, const int *, void *);
static int	mainbus_print(void *, const char *);

CFATTACH_DECL_NEW(mainbus, 0,
    mainbus_match, mainbus_attach, NULL, NULL);

/* There can be only one. */
bool mainbus_found;

struct mainbusdev {
	const char *md_name;
	bus_addr_t md_addr;
	int md_intr;
};

const struct mainbusdev mainbusdevs[] = {
	{ "cpu",		-1,			-1 },
	{ NULL,			0,			0 },
};

static int
mainbus_match(device_t parent, cfdata_t match, void *aux)
{
	if (mainbus_found)
		return (0);

	return (1);
}

static void
mainbus_attach(device_t parent, device_t self, void *aux)
{
	struct mainbus_attach_args ma;
	const struct mainbusdev *md;
	for (md = mainbusdevs; md->md_name != NULL; md++) {
		ma.ma_name = md->md_name;
		ma.ma_addr = md->md_addr;
		ma.ma_intr = md->md_intr;
		(void) config_found_sm_loc(self, "mainbus", NULL, &ma,
		    mainbus_print, mainbus_submatch);
	}
}

static int
mainbus_submatch(device_t parent, cfdata_t cf,
		 const int *ldesc, void *aux)
{
	struct mainbus_attach_args *ma = aux;

	return (config_match(parent, cf, ma));
}

static int
mainbus_print(void *aux, const char *pnp)
{
	struct mainbus_attach_args *ma = aux;

	if (pnp != 0)
		return QUIET;

	if (pnp)
		aprint_normal("%s at %s", ma->ma_name, pnp);
	//if (ma->ma_addr != MAINBUSCF_ADDR_DEFAULT)
		aprint_normal(" addr 0x%lx", ma->ma_addr);

	return (UNCONF);
}
