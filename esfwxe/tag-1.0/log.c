#include <common/target.h>
#pragma hdrstop

#include <common/log.h>
#ifdef USE_RTOS
#	include <common/core/rtos_intf.h>	
#endif

// log chain entries
static int s_loggerCnt;
static Logger s_loggers[MAX_LOGGER_COUNT];

#ifdef USE_RTOS
	// rtos stuff
	static rtosMUTEX_HANDLE s_lock;
#endif

// log level strings
ES_ASCII_CSTR c_logLvlStrings[lvlCount] = {
	"CRIT",
	"ERR",
	"OK",
	"WARN",
	"INFO"
};

// log registrar API
void logInit(void)
{
	s_loggerCnt = 0;	
#ifdef USE_RTOS
	s_lock = rtosMutexCreate();
#endif
}

#ifdef USE_RTOS
static __inline BOOL logLock(void)
{
	if( s_lock )
		return rtosMutexLock(s_lock, rtosMAX_DELAY);

	return TRUE;
}

static __inline void logUnlock(void)
{
	if( s_lock )
		rtosMutexUnlock(s_lock);
}
#endif

// simple search for registered logger
// return -1 if logger is not registered
static int logFindLogger(Logger logger)
{
	int idx;
	for( idx = 0; idx < s_loggerCnt; ++idx )
		if( s_loggers[idx] == logger )
			return idx;

	return -1;
}

BOOL logRegister(Logger logger)
{
	BOOL result = FALSE;
#ifdef USE_RTOS
	if( logLock() )
	{
#endif	
		
		if( s_loggerCnt < MAX_LOGGER_COUNT && -1 == logFindLogger(logger) )
		{
			s_loggers[s_loggerCnt++] = logger;
			result = TRUE;
		}
		
#ifdef USE_RTOS		
		logUnlock();
	}
#endif

	return result;
}

BOOL logUnregister(Logger logger)
{
	BOOL result = FALSE;
#ifdef USE_RTOS
	if( logLock() )
	{
#endif
		if( s_loggerCnt < MAX_LOGGER_COUNT )
		{
			int idx = logFindLogger(logger);
			if( idx != -1 )
			{
				--s_loggerCnt;
				for(; idx < s_loggerCnt; ++idx)
					s_loggers[idx] = s_loggers[idx+1];

				result = TRUE;
			}
		}
#ifdef USE_RTOS
		logUnlock();
	}
#endif
	return result;
}

// process log message
void logPush( LogLvl lvl, ES_ASCII_CSTR msg )
{
#ifdef USE_RTOS
	if( logLock() )
	{
#endif
		int idx;
		for(idx = 0; idx < s_loggerCnt; ++idx )
			s_loggers[idx](lvl, msg);
#ifdef USE_RTOS
		logUnlock();
	}
#endif
}
