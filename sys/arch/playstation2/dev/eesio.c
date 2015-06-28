#include <sys/bus.h>
#include <sys/device.h>
#include <sys/param.h>
#include <dev/cons.h>
#include <mips/cpuregs.h>

#include "../dev/eesio.h"

static int eesio_pool;

static void eesiocninit(struct consdev *);
static int  eesiocngetc(dev_t);
static void eesiocnputc(dev_t, int);
static void eesiocnpollc(dev_t, int);

struct consdev eesiocons = {
	NULL, eesiocninit, eesiocngetc, eesiocnputc, eesiocnpollc, NULL, NULL, NULL,
	NODEV, CN_NORMAL
};

cons_decl(eesio);

void eesiocninit(struct consdev *c)
{
	/*
	 * The real Playstation 2 serial port is initialized by the firmware at
	 * 38400 8N1.
	 * Just purge the RX FIFO.
	 */
	/* Reset receive FIFO */
	*(volatile uint32_t*)(SIO_BASE+SIO_FCR) = SIO_FCR_FRSTE|SIO_FCR_RFRST;
	/* Enable receive FIFO */
	*(volatile uint32_t*)(SIO_BASE+SIO_FCR) = 0;
}

int eesiocngetc(dev_t d)
{
	int c;

	while (1) {
		if (*(volatile uint32_t*)(SIO_BASE+SIO_ISR) & 0xf00) {
			c = *(volatile int8_t*)(SIO_BASE+SIO_RXFIFO);
			*(volatile uint32_t*)(SIO_BASE+SIO_ISR) = 7;
			return c;
		}

		if (eesio_pool == 0)
			break;
	}

	return -1;
}

void eesiocnputc(dev_t d, int c)
{
	while ((*(volatile uint32_t*)(SIO_BASE+SIO_ISR) & 0xf000) == 0x8000);
		*(volatile int8_t*)(SIO_BASE+SIO_TXFIFO) = (int8_t)c;
}

void eesiocnpollc(dev_t d, int c)
{
	eesio_pool = c;
}
