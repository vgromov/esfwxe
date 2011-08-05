/******************************************************************************
 *   irq.h:  Interrupt related Header file for NXP LPC23xx/24xx Family 
 *   Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.01  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __IRQ_H 
#define __IRQ_H

#define HIGHEST_PRIORITY	0x01
#define LOWEST_PRIORITY		0x0F

#define I_Bit			0x80
#define F_Bit			0x40

#define SYS32Mode		0x1F
#define IRQ32Mode		0x12
#define FIQ32Mode		0x11

#define	WDT_INT			0
#define ARM_CORE0_INT	2
#define	ARM_CORE1_INT	3
#define	TIMER0_INT		4
#define TIMER1_INT		5
#define UART0_INT		6
#define	UART1_INT		7
#define I2C0_INT		9
#define SPI0_INT		10
#define	SPI1_INT		11
#define	PLL_INT			12
#define RTC_INT			13
#define EINT0_INT		14
#define EINT1_INT		15
#define EINT2_INT		16
#define	ADC0_INT		18
#define I2C1_INT		19
#define TIMER2_INT	26
#define TIMER3_INT	27

#define IRQ_SLOT_EN	0x20	/* bit 5 in Vector control register */
#define VIC_SIZE		16

#define VECT_ADDR_INDEX	0x100
#define VECT_CNTL_INDEX 0x200

#endif /* end __IRQ_H */

/******************************************************************************
**                            End Of File
******************************************************************************/
