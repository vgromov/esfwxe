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

// channel abstraction
//
typedef enum {
	CHNL_UNDEFINED,
	CHNL_UART,
	CHNL_SPI,
	CHNL_I2C,
	CHNL_1W,
	CHNL_ETHERNET,
	CHNL_MEM	

} ChannelType;

// channel io operation break handler
//
typedef struct
{
	BOOL (*isBreaking)(void* This);

} CommChannelBreak;

typedef BOOL (*ChnlSetConfig)(busHANDLE, const void* config);
typedef void (*ChnlGetConfig)(busHANDLE, void* config);

typedef struct {
	// channel configuration
	ChnlSetConfig setConfig;
	ChnlGetConfig getConfig;
	// locking | unlocking
	BOOL (*lock)(busHANDLE, DWORD timeout);
	void (*unlock)(busHANDLE);
	// connect-disconnect, check connection status
	BOOL (*connect)(busHANDLE);
	void (*disconnect)(busHANDLE);
	BOOL (*isConnected)(busHANDLE);
	// data io
	DWORD (*putBytes)(busHANDLE, const BYTE* data, DWORD count, CommChannelBreak* brk);
	DWORD (*getBytes)(busHANDLE, BYTE* data, DWORD count, DWORD timeout, CommChannelBreak* brk);
	void (*resetIo)(busHANDLE, DWORD timeout);
	BOOL (*waitTxEmpty)(busHANDLE, CommChannelBreak* brk);
	// rate support|change
	BOOL (*isRateSupported)(busHANDLE, DWORD rate);
	DWORD (*getRate)(busHANDLE);
	BOOL (*setRate)(busHANDLE, DWORD rate);
	// packet send estimate in ms
	DWORD (*sendTimeEstimateGet)(busHANDLE, DWORD len);
	// error report
	int (*getError)(busHANDLE);

	// hardware bus handle
	busHANDLE m_bus;
	// channel type
	ChannelType m_type;

} CommChannel;

// basic channel initialization (must necesserily be called first from
// the code which initializes channel with specific bus impl)
//
void chnlInit(CommChannel* channel, busHANDLE bus);

// channel methods
//
// configuration
BOOL chnlSetConfig(CommChannel* chnl, const void* config);
void chnlGetConfig(CommChannel* chnl, void* config);
// locking | unlocking
BOOL chnlLock(CommChannel* chnl, DWORD timeout);
void chnlUnlock(CommChannel* chnl);
// connect-disconnect, check connection status
BOOL chnlConnect(CommChannel* chnl);
void chnlDisconnect(CommChannel* chnl);
BOOL chnlIsConnected(CommChannel* chnl);
// data io
DWORD chnlPutBytes(CommChannel* chnl, const BYTE* data, DWORD count, CommChannelBreak* brk);
DWORD chnlGetBytes(CommChannel* chnl, BYTE* data, DWORD count, DWORD timeout, CommChannelBreak* brk);
void chnlResetIo(CommChannel* chnl, DWORD timeout);
BOOL chnlWaitTxEmpty(CommChannel* chnl, CommChannelBreak* brk);
// rate support|change
BOOL chnlIsRateSupported(CommChannel* chnl, DWORD rate);
DWORD chnlGetRate(CommChannel* chnl);
BOOL chnlSetRate(CommChannel* chnl, DWORD rate);
// error report
int chnlGetError(CommChannel* chnl);
// return ms estimate needed to send len bytes over the channel
// 0 is returned if implementation is unknown
DWORD chnlSendTimeEstimateGet(CommChannel* chnl, DWORD len);

// inline checking of channel break
BOOL __inline chnlIsBreaking( void* obj )
{
	CommChannelBreak* This = (CommChannelBreak*)obj;

	if( This &&
			This->isBreaking )
 		return This->isBreaking(This);
 			
	return FALSE;
}

#ifdef __cplusplus
	}
#endif

#endif // _comm_intf_h_
