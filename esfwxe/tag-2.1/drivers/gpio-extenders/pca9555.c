#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
#include <esfwxe/core/twi_i2c.h>
#include "pca9555.h"

enum { 
    // base device address
    pca9555BaseAddr = 0x40,
    pca9555AddrCfgMask = 0x0E,
    // commands
    pca9555In0 = 0, // Input port 0
    pca9555In1,            // Input port 1
    pca9555Out0,        // Output port 0
    pca9555Out1,         // Output port 1
    pca9555Inv0,        // Polarity Inversion port 0
    pca9555Inv1,        // Polarity Inversion port 1
    pca9555Dir0,        // Configuration port 0
    pca9555Dir1         // Configuration port 1
};

// internal api
//
#define pca9555MakeAddr(cfgAddr) (pca9555BaseAddr+(pca9555AddrCfgMask & (cfgAddr << 1)))

// register access
//
//static __inline esBL pca9555SetByte(i2cHANDLE handle, esU8 addr, esU8 reg, esU8 b)
//{
//    esU8 tmp[2];
//    tmp[0] = reg;
//    tmp[1] = b;
//    return i2cPutBytes(handle, addr, tmp, 2) == 2;
//}

static __inline esBL pca9555SetWord(i2cHANDLE handle, esU8 addr, esU8 reg, esU16 w)
{
    esU8 tmp[3];
    tmp[0] = reg;
    tmp[1] = LOBYTE(w);
    tmp[2] = HIBYTE(w);
    return i2cPutBytes(handle, addr, tmp, 3) == 3;
}

//static __inline esBL pca9555GetByte(i2cHANDLE handle, esU8 addr, esU8 reg, esU8* b)
//{
//    return i2cPutBytes(handle, addr, &reg, 1) == 1 &&
//        i2cGetBytes(handle, addr, b, 1) == 1;
//}

static __inline esBL pca9555GetWord(i2cHANDLE handle, esU8 addr, esU8 reg, esU16* w)
{
    return i2cPutBytes(handle, addr, &reg, 1) == 1 &&
        i2cGetBytes(handle, addr, (esU8*)w, 2) == 2;
}

// driver api
//
esBL pca9555DirGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* dir)
{
    return pca9555GetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555Dir0, dir);
}

esBL pca9555DirSet(i2cHANDLE i2c, esU8 cfgAddr, esU16 dir)
{
    return pca9555SetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555Dir0, dir);
}

esBL pca9555InvGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* inv)
{
    return pca9555GetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555Inv0, inv);
}

esBL pca9555InvSet(i2cHANDLE i2c, esU8 cfgAddr, esU16 inv)
{
    return pca9555SetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555Inv0, inv);
}

esBL pca9555OutGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* state)
{
    return pca9555GetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555Out0, state);
}

esBL pca9555OutSet(i2cHANDLE i2c, esU8 cfgAddr, esU16 state)
{
    return pca9555SetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555Out0, state);
}

esBL pca9555InputGet(i2cHANDLE i2c, esU8 cfgAddr, esU16* input)
{
    return pca9555GetWord(i2c, pca9555MakeAddr(cfgAddr), pca9555In0, input);
}
