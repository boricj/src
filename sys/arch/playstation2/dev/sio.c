#include "sio.h"

struct consdev siocons = {
	NULL, siocninit, siocngetc, siocnputc, NULL, siocnhalt, siocnflush, NULL,
	NODEV, CN_NORMAL
};

void	siocninit(struct consdev *c) {
	/*
	 * The real Playstation 2 serial port is initialized by the firmware at
	 * 38400 8N1.
	 * PCSX2 doesn't care about initialization.
	 */
	return;
}

int	siocngetc(dev_t d) {
	return -1;
}

void	siocnputc(dev_t d, int c) {
	while ((*(volatile uint16_t*)(SIO_BASE+SIO_ISR) & 0xf000) == 0x8000);
	*(int8_t*)(SIO_BASE+SIO_TXFIFO) = (int8_t)c;
}

void	siocnhalt(dev_t d) {
	return;
}

void	siocnflush(dev_t d) {
	return;
}

