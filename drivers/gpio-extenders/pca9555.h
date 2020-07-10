#ifndef _pca_9555_h_
#define _pca_9555_h_

// 16 bit i2c gpio extender driver pca9555
//

#ifdef __cplusplus
    extern "C" {
#endif

// driver api
//
esBL pca9555DirGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* dir);
esBL pca9555DirSet(i2cHANDLE i2c, esU8 cfgAddr, esU16 dir);
esBL pca9555InvGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* inv);
esBL pca9555InvSet(i2cHANDLE i2c, esU8 cfgAddr, esU16 inv);
esBL pca9555OutGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* state);
esBL pca9555OutSet(i2cHANDLE i2c, esU8 cfgAddr, esU16 state);
esBL pca9555InputGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* input);

#ifdef __cplusplus
    }
#endif

#endif
