#ifndef _mc_24xx256_h_
#define _mc_24xx256_h_

// microchip 24xx256 (where xx is aa, lc, or fc) driver interface
//
#ifdef __cplusplus
	extern "C" {
#endif

// base chip address
#define MC24XX256_BASE_ADDR	0xA0
// make chip address from base address and selectable bits
#define	MC24XX256_MAKE_ADDR( ChipAddr )	((BYTE)(MC24XX256_BASE_ADDR + ((ChipAddr) & 0x0E)))
// memory size
#define MC24XX256_SIZE	0x8000

// write bytes to the chip. only 3 lower bits are used from the chipAddr
// internally, paged operations are invoked to perform continuous writes
DWORD mc24xx256PutBytes(i2cHANDLE handle, BYTE chipAddr, WORD memAddr, const BYTE* data, DWORD dataLen);
// sequentially read bytes from memory chip
DWORD mc24xx256GetBytes(i2cHANDLE handle, BYTE chipAddr, WORD memAddr, BYTE* data, DWORD dataLen);

#ifdef __cplusplus
	}
#endif

#endif // _mc_24xx256_h_
