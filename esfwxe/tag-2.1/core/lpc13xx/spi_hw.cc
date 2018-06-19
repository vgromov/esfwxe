// lpc13xx spi driver implementation
// this file is implicitly included from ../spi.c
// do not include it in the project directly
//
#ifndef _spi_lpc13xx_cc_
#define _spi_lpc13xx_cc_

// SPI service macros
//

// status bits
#define SSP_TFE		0x01	// TFE Transmit FIFO Empty. This bit is 1 is the Transmit FIFO is empty, 
#define SSP_TNF		0x02	// TNF Transmit FIFO Not Full. This bit is 0 if the Tx FIFO is full, 1 if not.
#define SSP_RNE		0x04 	// RNE Receive FIFO Not Empty. This bit is 0 if the Receive FIFO is empty, 1 if not.
#define SSP_RFF		0x08 	// RFF Receive FIFO Full. This bit is 1 if the Receive FIFO is full, 0 if not.
#define SSP_BSY		0x10 	// Busy. This bit is 0 if the SSP controller is idle, or 1 if it is 
												// currently sending/receiving a frame and/or the Tx FIFO is not empty.

#define SPI_CHECK_FOR_SENT_RETRIES	0x00007FFF

#ifdef USE_SPI_PORT0
// specific SCK0 locations
#	define LPC13XX_SCK0_P0_10	1
#	define LPC13XX_SCK0_P0_6	2
#	define LPC13XX_SCK0_P2_11	3

// port 0.2 SPI SSEL0,  
// port 0.8 MISO, port 0.9 MOSI0
#	define SPI0_PINS_CONFIG \
		{ LPC_SYSCON->SYSAHBCLKCTRL |= IOCON_CLOCK_CONNECT; \
		LPC_IOCON->PIO0_2 =	0x00000011; \
		LPC_IOCON->PIO0_8 =	0x00000011; \
		LPC_IOCON->PIO0_9 =	0x00000011;	\
		switch( USE_SPI0_LPC13XX_SCK0 ) { \
		case LPC13XX_SCK0_P0_10: LPC_IOCON->SCK_LOC = 0; LPC_IOCON->SWCLK_PIO0_10 = 0x00000012; break; \
		case LPC13XX_SCK0_P0_6: LPC_IOCON->SCK_LOC = 2; LPC_IOCON->PIO0_6 = 0x00000012; break; \
		case LPC13XX_SCK0_P2_11: LPC_IOCON->SCK_LOC = 1; LPC_IOCON->PIO2_11 = 0x00000011; break; \
		} \
		LPC_SYSCON->SYSAHBCLKCTRL &= ~IOCON_CLOCK_CONNECT; }

#	define SPI0_ENABLE_PWR \
	{ LPC_SYSCON->SYSAHBCLKCTRL |= 0x00000800; \
	LPC_SYSCON->SSP0CLKDIV = 1; \
	LPC_SYSCON->PRESETCTRL |= 1; }
	
#	define SPI0_DISABLE_PWR \
	{ LPC_SYSCON->SYSAHBCLKCTRL &= ~0x00000800; \
	LPC_SYSCON->SSP0CLKDIV = 0; \
	LPC_SYSCON->PRESETCTRL &= ~1; }

#define SPI0_CONFIG(pdcb) \
	{ esU32 r, presc; \
	SPI0_PINS_CONFIG; \
	LPC_SSP0->CR0 = 0; LPC_SSP0->CR1 = 0; \
	switch(pdcb->bits) { \
	case spi8_BITS: LPC_SSP0->CR0 |= 7; break; case spi9_BITS: LPC_SSP0->CR0 |= 8; break; case spi10_BITS: LPC_SSP0->CR0 |= 9; break;	case spi11_BITS: LPC_SSP0->CR0 |= 10; break; \
	case spi12_BITS: LPC_SSP0->CR0 |= 11; break; case spi13_BITS: LPC_SSP0->CR0 |= 12; break; case spi14_BITS: LPC_SSP0->CR0 |= 13; break; case spi15_BITS: LPC_SSP0->CR0 |= 14; break; \
	case spi16_BITS: LPC_SSP0->CR0 |= 15; break; } \
	switch(pdcb->latchMode) { \
	case spiCPOL0_CPHA0: break; case spiCPOL0_CPHA1: LPC_SSP0->CR0 |= 0x80; break; case spiCPOL1_CPHA0: LPC_SSP0->CR0 |= 0x40; break; case spiCPOL1_CPHA1: LPC_SSP0->CR0 |= 0xC0; break; } \
	r = (esU32)Fpclk / pdcb->freqHz; \
	presc = r; if( presc % 2 ) ++presc; if( presc < 2 ) presc = 2; else if( presc > 254 ) presc = 254; \
	LPC_SSP0->CPSR = presc; r = r/presc; if( r > 0 ) --r;	if( r > 255 ) r = 255; LPC_SSP0->CR0 |= (r << 8); \
	if(	spiSlave == pdcb->mode ) LPC_SSP0->CR1 |= 0x04; LPC_SSP0->CR1 |= 0x02; }

#define SPI0_CHECK_FOR_SENT 		(1)
#define SPI0_SEND_WORD(word) 		while( !(LPC_SSP0->SR & SSP_TNF) ); LPC_SSP0->DR = (word)
#define SPI0_RECEIVE_WORD(word) while( !(LPC_SSP0->SR & SSP_RNE) ); (word) = LPC_SSP0->DR

#endif

// macros generalization
//
#define SPI_CONFIG(PortNum, pdcb)				SPI## PortNum ##_CONFIG(pdcb)
#define SPI_SEND_WORD(PortNum, word) 		SPI## PortNum ##_SEND_WORD(word)
#define SPI_RECEIVE_WORD(PortNum, word)	SPI## PortNum ##_RECEIVE_WORD(word)
#define SPI_CHECK_FOR_SENT(PortNum)			SPI## PortNum ##_CHECK_FOR_SENT

#endif // _spi_lpc13xx_cc_
