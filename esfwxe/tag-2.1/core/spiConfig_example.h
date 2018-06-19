#ifndef _spi_Config_h_
#define _spi_Config_h_

#define USE_SPI_PORT0				// use spi 0

// #define SPI_USE_INTERRUPT

#ifdef SPI_USE_INTERRUPT
	#define SPI_INTERRUPT_PRIORITY	(HIGHEST_PRIORITY)
#endif

typedef enum {
#ifdef USE_SPI_PORT0
	spi0,
#endif

#ifdef USE_SPI_PORT1
	spi1,
#endif

	spiPortCnt

} spiPort;

#endif // _spi_Config_h_
