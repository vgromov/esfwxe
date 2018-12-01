#ifndef _ring_buff_h_
#define _ring_buff_h_

#ifdef __cplusplus
  extern "C" {
#endif

// the following file defines which ringbuff 
// access implementation to use
#include <ringbuffConfig.h>

// ring buffer struct
//
typedef struct {
  void* data;  // data buffer
  esU32 mask; // buffer size mask
  esU32  in;    // first index of data in buffer
  esU32  out;  // last index of data in buffer
  esU32 count;
  
} RingBuff;
typedef volatile RingBuff* rbHANDLE;

// ring buffer initialization 
//
// size  is number of elements, not bytes
void rbInit(rbHANDLE buff, void* data, esU32 size);

// ring buffer check
//
// is empty
esBL rbIsEmpty(rbHANDLE buff);
// is full
esBL rbIsFull(rbHANDLE buff);
// elements count
esU32 rbGetCount(rbHANDLE buff);

// ring buffer access
//
// flush entire buffer
void rbFlush(rbHANDLE buff); 

// all 'pop' services below may have null pointer as input parameter
// therefore, no popped value will be returned
// otherwise, popped value is put into dereferenced ptr slot
//

// unguarded versions to use inside ISR
#ifdef USE_ISR_SAFE_RB
  esBL rbIsEmptyFromIsr(rbHANDLE buff);
  esBL rbIsFullFromIsr(rbHANDLE buff);
  esU32 rbGetCountFromIsr(rbHANDLE buff);
  void rbFlushFromIsr(rbHANDLE buff);  
#else  // USE_ISR_SAFE_RB
  #define rbIsEmptyFromIsr  rbIsEmpty
  #define rbIsFullFromIsr    rbIsFull
  #define rbGetCountFromIsr rbGetCount
  #define rbFlushFromIsr    rbFlush  
#endif // USE_ISR_SAFE_RB

// type-dependent part
//
// esU8 access
//
#ifdef USE_BYTE_RB
// append byte to the end of the buffer, return FALSE if fail, TRUE on success
esBL rbPushB(rbHANDLE buff, esU8 b);
// append byte to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutB(rbHANDLE buff, esU8 b, esU32 timeout);

// pop byte from front of the buffer
esBL rbPopB(rbHANDLE buff, esU8* b);
// pop byte from the buffer with timeout in ms
esBL rbPopTimeoutB(rbHANDLE buff, esU8* b, esU32 timeout);

#ifdef USE_ISR_SAFE_RB
  // append byte to the end of the buffer, return FALSE if fail, TRUE on success
  esBL rbPushFromIsrB(rbHANDLE buff, esU8 b);
  // pop byte from front of the buffer
  esBL rbPopFromIsrB(rbHANDLE buff, esU8* b);
#else // USE_ISR_SAFE_RB  
  #define rbPushFromIsrB rbPushB
  #define rbPopFromIsrB   rbPopB
#endif // USE_ISR_SAFE_RB

#endif // USE_BYTE_RB

// esU16 access
//
#ifdef USE_WORD_RB
// append byte to the end of the buffer, return FALSE if fail, TRUE on success
esBL rbPushW(rbHANDLE buff, esU16 w);
// append byte to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutW(rbHANDLE buff, esU16 w, esU32 timeout);
// pop byte from front of the buffer
esBL rbPopW(rbHANDLE buff, esU16* w);
// pop byte from the buffer with timeout in ms
esBL rbPopTimeoutW(rbHANDLE buff, esU16* w, esU32 timeout);

#ifdef USE_ISR_SAFE_RB
  // append byte to the end of the buffer, return FALSE if fail, TRUE on success
  esBL rbPushFromIsrW(rbHANDLE buff, esU16 w);
  // pop byte from front of the buffer
  esBL rbPopFromIsrW(rbHANDLE buff, esU16* w);  
#else  // USE_ISR_SAFE_RB
  #define rbPushFromIsrW rbPushW
  #define rbPopFromIsrW   rbPopW
#endif // USE_ISR_SAFE_RB

#endif // USE_WORD_RB

// esU32 access
//
#ifdef USE_DWORD_RB
// append byte to the end of the buffer, return FALSE if fail, TRUE on success
esBL rbPushDW(rbHANDLE buff, esU32 dw);
// append byte to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutDW(rbHANDLE buff, esU32 dw, esU32 timeout);
// pop byte from front of the buffer
esBL rbPopDW(rbHANDLE buff, esU32* dw);
// pop byte from the buffer with timeout in ms
esBL rbPopTimeoutDW(rbHANDLE buff, esU32* dw, esU32 timeout);

#ifdef USE_ISR_SAFE_RB
  // append byte to the end of the buffer, return FALSE if fail, TRUE on success
  esBL rbPushFromIsrDW(rbHANDLE buff, esU32 dw);
  // pop byte from front of the buffer
  esBL rbPopFromIsrDW(rbHANDLE buff, esU32* dw);  
#else  // USE_ISR_SAFE_RB
  #define rbPushFromIsrDW rbPushDW
  #define rbPopFromIsrDW  rbPopDW
#endif // USE_ISR_SAFE_RB

#endif // USE_DWORD_RB

// esF access
//
#ifdef USE_FLOAT_RB
// append float to the end of the buffer, return FALSE if fail, TRUE on success
esBL rbPushF(rbHANDLE buff, esF f);
// append float to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutF(rbHANDLE buff, esF f, esU32 timeout);
// pop float from front of the buffer
esBL rbPopF(rbHANDLE buff, esF* f);
// pop float from the buffer with timeout in ms
esBL rbPopTimeoutF(rbHANDLE buff, esF* f, esU32 timeout);

#ifdef USE_ISR_SAFE_RB
  // append float to the end of the buffer, return FALSE if fail, TRUE on success
  esBL rbPushFromIsrF(rbHANDLE buff, esF f);
  // pop float from front of the buffer
  esBL rbPopFromIsrF(rbHANDLE buff, esF* F);  
#else  // USE_ISR_SAFE_RB
  #define rbPushFromIsrF rbPushF
  #define rbPopFromIsrF  rbPopF
#endif // USE_ISR_SAFE_RB

#endif // USE_FLOAT_RB

#ifdef __cplusplus
  }
#endif

#endif
