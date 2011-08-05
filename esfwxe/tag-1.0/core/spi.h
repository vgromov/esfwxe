#ifndef _spi_h_
#define _spi_h_

#ifdef __cplusplus
	extern "C" {
#endif

// SPI control enums
//
typedef enum
{
	spiMaster,
	spiSlave

} spiMode;

typedef enum
{
	spi8_BITS,
	spi9_BITS,
	spi10_BITS,
	spi11_BITS,
	spi12_BITS,
	spi13_BITS,
	spi14_BITS,
	spi15_BITS,
	spi16_BITS 

} spiDataBits;

typedef enum
{
	spiMSB,
	spiLSB

} spiOrder;

typedef enum
{
	spiCPOL0_CPHA0,
	spiCPOL0_CPHA1,
	spiCPOL1_CPHA0,
	spiCPOL1_CPHA1
	 
} spiLatchMode;

typedef enum
{
	spiInvalidHandle = -1,
	spiOK,
	spiSlaveAbort,
	spiModeFault,
	spiReadOverrun,
	spiWriteCollision	

} spiErrorCode;

// SPI DCB
//
typedef struct
{
	spiMode				mode;
	spiDataBits 	bits;
	spiOrder			order;
	spiLatchMode	latchMode;
	DWORD					freqHz;

} spiDCB;

typedef volatile void* spiHANDLE;

#include "spiConfig.h"

typedef enum {
#ifdef USE_SPI_PORT0
	spi0,
#endif

#ifdef USE_SPI_PORT1
	spi1,
#endif

	spiPortCnt

} spiPort;

// try to mutually exclusive acquire port resource
// timeout is in millisecond
BOOL spiLockPort(spiHANDLE hport, DWORD timeout);
void spiUnlockPort(spiHANDLE hport);

void spiGetDCB(spiHANDLE hport, spiDCB* dcb);
BOOL spiSetDCB(spiHANDLE hport, const spiDCB* dcb);
spiHANDLE spiInit(spiPort port, const spiDCB* dcb);

void spiPowerUp( spiPort port );
void spiPowerDown( spiPort port );

BOOL spiIsOpen(spiHANDLE hport);
BOOL spiOpen(spiHANDLE hport);
void spiClose(spiHANDLE hport);

spiErrorCode spiGetErrorCode(spiHANDLE hport);

DWORD spiPutBytes( spiHANDLE hport, const BYTE* Bytes, DWORD length );
DWORD spiGetBytes( spiHANDLE hport, BYTE* Bytes, DWORD length );

#ifdef __cplusplus
	};
#endif

#endif //	_spi_h_
