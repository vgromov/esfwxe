#ifndef _pca_9555_h_
#define _pca_9555_h_

// 16 bit i2c gpio extender driver pca9555
//

#ifdef __cplusplus
	extern "C" {
#endif

// driver api
//
BOOL pca9555DirGet(i2cHANDLE i2c, BYTE cfgAddr, UINT16* dir);
BOOL pca9555DirSet(i2cHANDLE i2c, BYTE cfgAddr, UINT16 dir);
BOOL pca9555InvGet(i2cHANDLE i2c, BYTE cfgAddr, UINT16* inv);
BOOL pca9555InvSet(i2cHANDLE i2c, BYTE cfgAddr, UINT16 inv);
BOOL pca9555OutGet(i2cHANDLE i2c, BYTE cfgAddr, UINT16* state);
BOOL pca9555OutSet(i2cHANDLE i2c, BYTE cfgAddr, UINT16 state);
BOOL pca9555InputGet(i2cHANDLE i2c, BYTE cfgAddr, UINT16* input);

#ifdef __cplusplus
	}
#endif

#endif
