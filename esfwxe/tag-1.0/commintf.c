#include <common/target.h>
#include <string.h>
#pragma hdrstop

#include <common/commintf.h>

// basic channel initialization (must necesserily be called first from
// the code which initializes channel with specific bus impl)
//
void chnlInit(CommChannel* channel, busHANDLE bus)
{
	channel->setConfig = 0;
	channel->getConfig = 0;
	channel->lock = 0;
	channel->unlock = 0;
	channel->connect = 0;
	channel->disconnect = 0;
	channel->isConnected = 0;
	channel->putBytes = 0;
	channel->getBytes = 0;
	channel->resetIo = 0;
	channel->waitTxEmpty = 0;
	channel->isRateSupported = 0;
	channel->getRate = 0;
	channel->setRate = 0;
	channel->sendTimeEstimateGet = 0;
	channel->getError = 0;
	channel->m_type = CHNL_UNDEFINED;
	channel->m_bus = bus;
}

// channel methods
//
// configuration
BOOL chnlSetConfig(CommChannel* chnl, const void* config)
{
	if( chnl && config && chnl->setConfig )
		return chnl->setConfig(chnl->m_bus, config);

	return FALSE;
}

void chnlGetConfig(CommChannel* chnl, void* config)
{
	if( chnl && config && chnl->getConfig )
		chnl->getConfig(chnl->m_bus, config);
}

// locking | unlocking
BOOL chnlLock(CommChannel* chnl, DWORD timeout)
{
	if( chnl && chnl->lock )
		return chnl->lock(chnl->m_bus, timeout);

	return FALSE;
}

void chnlUnlock(CommChannel* chnl)
{
	if( chnl && chnl->unlock )
		chnl->unlock(chnl->m_bus);
}

// connect-disconnect, check connection status
BOOL chnlConnect(CommChannel* chnl)
{
	if( chnl && chnl->connect )
		return chnl->connect( chnl->m_bus );

	return FALSE;
}

void chnlDisconnect(CommChannel* chnl)
{
	if( chnl && chnl->disconnect )
		chnl->disconnect( chnl->m_bus );
}

BOOL chnlIsConnected(CommChannel* chnl)
{
	if( chnl && chnl->isConnected )
		return chnl->isConnected( chnl->m_bus );

	return FALSE;
}

// data io
DWORD chnlPutBytes(CommChannel* chnl, const BYTE* data, DWORD count, CommChannelBreak* brk)
{
	if( chnl && 
			chnl->putBytes && 
			!chnlIsBreaking(brk) )
		return chnl->putBytes(chnl->m_bus, data, count, brk);
		
	return 0;		
}

DWORD chnlGetBytes(CommChannel* chnl, BYTE* data, DWORD count, DWORD timeout, CommChannelBreak* brk)
{
	if( chnl && 
			chnl->getBytes && 
			!chnlIsBreaking(brk) )
		return chnl->getBytes(chnl->m_bus, data, count, timeout, brk);

	return 0;
}

void chnlResetIo(CommChannel* chnl, DWORD timeout)
{
	if( chnl && chnl->resetIo )
		chnl->resetIo(chnl->m_bus, timeout);	
}

BOOL chnlWaitTxEmpty(CommChannel* chnl, CommChannelBreak* brk)
{
	if( chnl &&	chnl->waitTxEmpty ) 
		return chnl->waitTxEmpty(chnl->m_bus, brk);
	
	return FALSE;
}

// rate support|change
BOOL chnlIsRateSupported(CommChannel* chnl, DWORD rate)
{
	return chnl && 
		chnl->isRateSupported &&
		chnl->isRateSupported(chnl->m_bus, rate);
}

DWORD chnlGetRate(CommChannel* chnl)
{
	if( chnl && chnl->getRate )
		return chnl->getRate(chnl->m_bus);

	return 0;
}

BOOL chnlSetRate(CommChannel* chnl, DWORD rate)
{
	return chnl && chnl->setRate &&
		chnl->setRate(chnl->m_bus, rate);
}

// error report
int chnlGetError(CommChannel* chnl)
{
	if( chnl && chnl->getError )
		return chnl->getError(chnl->m_bus);

	return 0;
}

// return ms estimate needed to send len bytes over the channel
// 0 is returned if implementation is unknown
DWORD chnlSendTimeEstimateGet(CommChannel* chnl, DWORD len)
{
	if( chnl && chnl->sendTimeEstimateGet )
		return chnl->sendTimeEstimateGet(chnl->m_bus, len);
	
	return 0;
}
