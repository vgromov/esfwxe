#ifndef _inc_spi_hw_cc_
#define _inc_spi_hw_cc_

// lpc spi driver implementation
// this file is explicitly included from ../spi.c
// do not include it in the project directly
//

// SPI service macros
//
#define SPI_CHECK_FOR_SENT_RETRIES	0x00007FFF																																 	

// interrupt status
#define ABRT			0x08
#define MODF			0x10
#define ROVR			0x20
#define WCOL			0x40
#define SPIF			0x80

// SPI control register bits
#define SPI_BE		0x0004
#define SPI_CPHA	0x0008
#define SPI_CPOL	0x0010
#define SPI_MSTR	0x0020
#define SPI_LSBF	0x0040
#define SPI_SPIE	0x0080

#ifdef USE_SPI_PORT0

// Port 0.4 SPI SCK,  
// port0.5 MISO, port0.6 MOSI
#define SPI0_PINS_CONFIG \
  PINSEL0 |= 0x00001500

// enable|disable SPI power
#define SPI0_ENABLE_PWR 	PCONP |= PCSPI0
#define SPI0_DISABLE_PWR 	PCONP &= ~PCSPI0

#define SPI0_CONFIG(pdcb) \
	{ DWORD freq = ((DWORD)Fpclk / (2 * pdcb->freqHz)) * 2; \
	SPI0_PINS_CONFIG; \
	S0SPCR = 0;	\
	if( freq < 8 ) freq = 8;	\
	if( freq > 255 ) freq = 254; \
  S0SPCCR = freq; \
	S0SPCR |= SPI_BE | ((pdcb->bits == spi16_BITS ? 0 : pdcb->bits + 8) << 8); \
	if( pdcb->mode == spiMaster )	\
 		S0SPCR |= SPI_MSTR; \
	else \
		S0SPCR &= ~SPI_MSTR; \
	if( pdcb->order == spiMSB ) \
		S0SPCR &= ~SPI_LSBF; \
	else \
		S0SPCR |= SPI_LSBF; \
	switch(pdcb->latchMode) \
	{ \
	case spiCPOL0_CPHA0: \
		S0SPCR &= ~(SPI_CPHA | SPI_CPOL); \
	break; \
	case spiCPOL0_CPHA1: \
		S0SPCR &= ~SPI_CPOL; \
		S0SPCR |= SPI_CPHA; \
	break; \
	case spiCPOL1_CPHA0: \
		S0SPCR |= SPI_CPOL; \
		S0SPCR &= ~SPI_CPHA; \
	break; \
	case spiCPOL1_CPHA1: \
		S0SPCR |= (SPI_CPOL | SPI_CPHA); \
	break; }	}

#define SPI0_CHECK_FOR_SENT 		(S0SPSR & SPIF)
#define SPI0_SEND_WORD(word) 		S0SPDR = (word)
#define SPI0_RECEIVE_WORD(word) (word) = S0SPDR

#endif // USE_SPI_PORT0

#ifdef USE_SPI_PORT1

// configuration bits
#define SSP_MSTR	0x04
#define SSP_EN		0x02
#define SSP_CPHA	0x0080
#define SSP_CPOL	0x0040

// status bits
#define SSP_TFE		0x01	// TFE Transmit FIFO Empty. This bit is 1 is the Transmit FIFO is empty, 
#define SSP_TNF		0x02	// TNF Transmit FIFO Not Full. This bit is 0 if the Tx FIFO is full, 1 if not.
#define SSP_RNE		0x04 	// RNE Receive FIFO Not Empty. This bit is 0 if the Receive FIFO is empty, 1 if not.
#define SSP_RFF		0x08 	// RFF Receive FIFO Full. This bit is 1 if the Receive FIFO is full, 0 if not.
#define SSP_BSY		0x10 	// Busy. This bit is 0 if the SSP controller is idle, or 1 if it is 
												// currently sending/receiving a frame and/or the Tx FIFO is not empty.

// Port 0.14 SPI SCK,  
// port0.19 MISO, port0.20 MOSI
#define SPI1_PINS_CONFIG \
	PINSEL0 |= 0x20000000; \
  PINSEL1 |= 0x00000140

// manage power state
#define SPI1_ENABLE_PWR 	PCONP |= PCSSP
#define SPI1_DISABLE_PWR 	PCONP &= ~PCSSP

#define SPI1_CONFIG(pdcb) \
	{ BYTE div = pdcb->mode == spiMaster ? 2 : 12; \
	DWORD rate = ((DWORD)Fpclk / ((DWORD)pdcb->freqHz * (DWORD)div)); \
	if( rate > 0 ) --rate;  \
	SPI1_PINS_CONFIG; \
	SSPCR0 = 0;	\
	SSPCR1 = 0; \
  SSPCR0 = (pdcb->bits + 7) | (rate << 8); \
	SSPCPSR = div; \
	switch(pdcb->latchMode) \
	{ \
	case spiCPOL0_CPHA0: \
		SSPCR0 &= ~(SSP_CPHA | SSP_CPOL); \
	break; \
	case spiCPOL0_CPHA1: \
		SSPCR0 &= ~SSP_CPOL; \
		SSPCR0 |= SSP_CPHA; \
	break; \
	case spiCPOL1_CPHA0: \
		SSPCR0 |= SSP_CPOL; \
		SSPCR0 &= ~SSP_CPHA; \
	break; \
	case spiCPOL1_CPHA1: \
		SSPCR0 |= (SSP_CPOL | SSP_CPHA); \
	break; \
	}	 \
	if( pdcb->mode == spiMaster )	\
 		SSPCR1 &= ~SSP_MSTR; \
	else \
		SSPCR1 |= SSP_MSTR; \
	SSPCR1 |= SSP_EN; }

#define SPI1_CHECK_FOR_SENT 		!(SSPSR & SSP_BSY)
#define SPI1_SEND_WORD(word) 		SSPDR = (word)
#define SPI1_RECEIVE_WORD(word) (word) = SSPDR

#endif // USE_SPI_PORT1

// macros generalization
//
#define SPI_CONFIG(PortNum, pdcb)				SPI## PortNum ##_CONFIG(pdcb)
#define SPI_SEND_WORD(PortNum, word) 		SPI## PortNum ##_SEND_WORD(word)
#define SPI_RECEIVE_WORD(PortNum, word)	SPI## PortNum ##_RECEIVE_WORD(word)
#define SPI_CHECK_FOR_SENT(PortNum)			SPI## PortNum ##_CHECK_FOR_SENT

#endif // _inc_spi_hw_cc_
