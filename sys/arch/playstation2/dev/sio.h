#ifndef _PLAYSTATION2_SIO_H_
#define _PLAYSTATION2_SIO_H_

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/device.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/time.h>
#include <sys/kernel.h>
#include <sys/syslog.h>
#include <sys/cpu.h>
#include <sys/intr.h>

#include <dev/cons.h>

#define SIO_BASE	MIPS_PHYS_TO_KSEG1(0x1000f100)

#define SIO_LCR	0x0 		/* Line Control Register. */
#define SIO_LCR_UMODE_8BIT 0x00 /* UART Mode. */
#define SIO_LCR_UMODE_7BIT 0x01
#define SIO_LCR_USBL_1BIT 0x00 	/* UART Stop Bit Length. */
#define SIO_LCR_USBL_2BITS 0x01
#define SIO_LCR_UPEN_OFF 0x00 	/* UART Parity Enable. */
#define SIO_LCR_UPEN_ON 0x01
#define SIO_LCR_UEPS_ODD 0x00 	/* UART Even Parity Select. */
#define SIO_LCR_UEPS_EVEN 0x01

#define SIO_LSR	0x10 		/* Line Status Register. */
#define SIO_LSR_DR 0x01 	/* Data Ready. (Not tested) */
#define SIO_LSR_OE 0x02 	/* Overrun Error. */
#define SIO_LSR_PE 0x04 	/* Parity Error. */
#define SIO_LSR_FE 0x08 	/* Framing Error. */

#define SIO_IER	0x20 		/* Interrupt Enable Register. */
#define SIO_IER_ERDAI	0x01 	/* Enable Received Data Available Interrupt */
#define SIO_IER_ELSI 0x04 	/* Enable Line Status Interrupt. */

#define SIO_ISR	0x30 		/* Interrupt Status Register (?). */
#define SIO_ISR_RX_DATA 0x01
#define SIO_ISR_TX_EMPTY 0x02
#define SIO_ISR_RX_ERROR 0x04

#define SIO_FCR	0x40 		/* FIFO Control Register. */
#define SIO_FCR_FRSTE 0x01 	/* FIFO Reset Enable. */
#define SIO_FCR_RFRST 0x02 	/* RX FIFO Reset. */
#define SIO_FCR_TFRST 0x04 	/* TX FIFO Reset. */

#define SIO_BGR	0x50 		/* Baud Rate Control Register. */

#define SIO_TXFIFO	0x80 	/* Transmit FIFO. */

#define SIO_RXFIFO	0xc0 	/* Receive FIFO. */

void	siocnprobe(struct consdev *cp);
void	siocninit(struct consdev *c);
int	siocngetc(dev_t d);
void	siocnputc(dev_t d, int c);
#define siocnpollc nullcnpollc
//void	siocnbell(dev_t, u_int, u_int, u_int);
void	siocnhalt(dev_t d);
void	siocnflush(dev_t d);

void sio_puts(const char *str);

extern struct consdev siocons;

#endif

