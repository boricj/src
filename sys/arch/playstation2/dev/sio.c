#include "sio.h"

cons_decl(sio);

void    siocnprobe(struct consdev *cp) {
	cp->cn_pri = CN_NORMAL;
}

void	siocninit(struct consdev *c) {
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

int	siocngetc(dev_t d) {
	int c;
	if (*(volatile uint32_t*)(SIO_BASE+SIO_ISR) & 0xf00) {
		c = *(volatile int8_t*)(SIO_BASE+SIO_RXFIFO);
		*(volatile uint32_t*)(SIO_BASE+SIO_ISR) = 7;
		return c;
	}
	
	return -1;
}

void	siocnputc(dev_t d, int c) {
	while ((*(volatile uint32_t*)(SIO_BASE+SIO_ISR) & 0xf000) == 0x8000);
	*(volatile int8_t*)(SIO_BASE+SIO_TXFIFO) = (int8_t)c;
}

void	siocnhalt(dev_t d) {
	return;
}

void	siocnflush(dev_t d) {
	while ((*(volatile uint32_t*)(SIO_BASE+SIO_ISR) & 0xf000) == 0x8000);
	return;
}

void sio_puts(const char *str) {
	if (!str)
		return;
	
	while(*str != 0) {
		siocnputc(0, *str++);
	}
	siocnputc(0, '\n');
}

