#ifndef _comm_intf_h_
#define _comm_intf_h_

// communication abstraction interface 
//
#ifndef USE_COMMINTF
#	define USE_COMMINTF
#endif

#ifdef __cplusplus
	extern "C" {
#endif
		
struct tagEseChannelIo;
struct tagEseChannelIoBreaker;

// channel abstraction
//
typedef enum {
	CHNL_UNDEFINED,
	CHNL_UART,
	CHNL_SPI,
	CHNL_I2C,
	CHNL_1W,
	CHNL_ETHERNET,
	CHNL_MEM,
  CHNL_BLUETOOTH

} EseChannelIoType;

// channel io operation breaker interface
//
typedef struct tagEseChannelIoBreaker {
	esBL (*isBreaking)(struct tagEseChannelIoBreaker* This);
  busHANDLE m_bus;

} EseChannelIoBreaker;


typedef esBL (*ChnlSetConfig)(struct tagEseChannelIo*, const void* config);
typedef void (*ChnlGetConfig)(struct tagEseChannelIo*, void* config);

typedef struct tagEseChannelIo {
	// channel configuration
	ChnlSetConfig setConfig;
	ChnlGetConfig getConfig;
	// locking | unlocking
	esBL (*lock)(struct tagEseChannelIo*, esU32 timeout);
	void (*unlock)(struct tagEseChannelIo*);
	// connect-disconnect, check connection status
	esBL (*connect)(struct tagEseChannelIo*);
	void (*disconnect)(struct tagEseChannelIo*);
	esBL (*isConnected)(struct tagEseChannelIo*);
	// data io
	esU32 (*bytesPut)(struct tagEseChannelIo*, const esU8* data, esU32 count);
	esU32 (*bytesGet)(struct tagEseChannelIo*, esU8* data, esU32 count, esU32 timeout);
	void (*resetIo)(struct tagEseChannelIo*);
	esBL (*waitTxEmpty)(struct tagEseChannelIo*);
  // Packet channel services (optional)
  esBL (*txBatchBegin)(struct tagEseChannelIo*);
  void (*txBatchEnd)(struct tagEseChannelIo*, esBL ok);
	// rate support|change
	esBL (*isRateSupported)(struct tagEseChannelIo*, esU32 rate);
	esU32 (*rateGet)(struct tagEseChannelIo*);
	esBL (*rateSet)(struct tagEseChannelIo*, esU32 rate);
	// packet send estimate in ms
	esU32 (*sendTimeEstimateGet)(struct tagEseChannelIo*, esU32 len);
	// error report
	int (*errorGet)(struct tagEseChannelIo*);

	// channel type
	EseChannelIoType m_type;
	// hardware bus handle
	busHANDLE m_bus;
	// channel breaker
	EseChannelIoBreaker* m_breaker;

} EseChannelIo;

// basic channel initialization (must necesserily be called first from
// the code which initializes channel with specific bus impl)
//
void chnlInit(EseChannelIo* channel, busHANDLE bus);

// channel methods
//
// configuration
esBL chnlSetConfig(EseChannelIo* chnl, const void* config);
void chnlGetConfig(EseChannelIo* chnl, void* config);
// check if channel should break io
void chnlBreakerSet(EseChannelIo* chnl, EseChannelIoBreaker* brk);
esBL chnlIsBreaking(EseChannelIo* chnl);
// locking | unlocking
esBL chnlLock(EseChannelIo* chnl, esU32 timeout);
void chnlUnlock(EseChannelIo* chnl);
// connect-disconnect, check connection status
esBL chnlConnect(EseChannelIo* chnl);
void chnlDisconnect(EseChannelIo* chnl);
esBL chnlIsConnected(EseChannelIo* chnl);
// data io
esU32 chnlPutBytes(EseChannelIo* chnl, const esU8* data, esU32 count);
esU32 chnlGetBytes(EseChannelIo* chnl, esU8* data, esU32 count, esU32 timeout);
void chnlResetIo(EseChannelIo* chnl);
esBL chnlWaitTxEmpty(EseChannelIo* chnl);
// Batch TX
esBL chnlTxBatchBegin(EseChannelIo* chnl);
void chnlTxBatchEnd(EseChannelIo* chnl, esBL ok);
// rate support|change
esBL chnlIsRateSupported(EseChannelIo* chnl, esU32 rate);
esU32 chnlGetRate(EseChannelIo* chnl);
esBL chnlSetRate(EseChannelIo* chnl, esU32 rate);
// error report
int chnlGetError(EseChannelIo* chnl);
// return ms estimate needed to send len bytes over the channel
// 0 is returned if implementation is unknown
esU32 chnlSendTimeEstimateGet(EseChannelIo* chnl, esU32 len);

// try to receive binary pattern from channel
// try to receive specific byte from comm channel, in specified count of retries,
esBL chnlSpecificByteReceive(EseChannelIo* chnl, esU8 b, esU32 retries);
esBL chnlBinaryPatternReceive(EseChannelIo* chnl, const esU8* pattern, esU32 patternLen, esU32 retries);

#ifdef __cplusplus
	}
#endif

#endif // _comm_intf_h_
