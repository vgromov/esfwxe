#ifndef _trace_h_
#define _trace_h_

// debug tracing facility
//
// include stdio for printf prototype
#include <stdio.h>

// forward declarations. application should provide specific
// implementations of the following two prototypes to	allow 
// trace facility to function properly
#if !defined(NDEBUG) && defined(USE_TRACE)
	int TRACE_BEGIN(void);
	void TRACE_END(void);
#	define TRACE_esBLSTR(b)	((b) ? "true" : "false")
# ifndef USE_EMULATOR
#		define TRACE	printf
#		define TRACE_FMT_STR "%s"
#		define TRACE_EOL	"\n\r"
#	else
		void TRACE(ESE_CSTR s, ...);
#		define TRACE_EOL
#		ifdef UNICODE
#			define TRACE_FMT_STR	"%S"
#		else
#			define TRACE_FMT_STR	"%s"
#		endif
#	endif // USE_EMULATOR
#		define TRACE0(str) \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL); \
				TRACE_END();	}
#		define TRACE1(str, p1)  \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL, p1); \
				TRACE_END();	}
#		define TRACE2(str, p1, p2) \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL, p1, p2); \
				TRACE_END();	}
#		define TRACE3(str, p1, p2, p3) \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL, p1, p2, p3); \
				TRACE_END();	}
#		define TRACE4(str, p1, p2, p3, p4)  \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL, p1, p2, p3, p4); \
				TRACE_END();	}
#		define TRACE5(str, p1, p2, p3, p4, p5)   \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL, p1, p2, p3, p4, p5); \
				TRACE_END();	}
#		define TRACE6(str, p1, p2, p3, p4, p5, p6)   \
			if( TRACE_BEGIN() )	\
			{	TRACE(str TRACE_EOL, p1, p2, p3, p4, p5, p6); \
				TRACE_END();	}
#		if defined(USE_MEM_TRACE)
			void traceMem(void);
# 		define TRACE_MEM	traceMem();
#		else
#			define TRACE_MEM
#		endif
#else
#	define TRACE
#	define TRACE0(str)
#	define TRACE1(str, p1)
#	define TRACE2(str, p1, p2)
#	define TRACE3(str, p1, p2, p3)
#	define TRACE4(str, p1, p2, p3, p4)
#	define TRACE5(str, p1, p2, p3, p4, p5)
#	define TRACE_MEM
#endif

#ifdef USE_ITM_TRACE
# define ES_DEBUG_TRACE0(fmt) printf(fmt);
# define ES_DEBUG_TRACE1(fmt, _1) printf(fmt, (_1));
# define ES_DEBUG_TRACE2(fmt, _1, _2) printf(fmt, (_1), (_2));
# define ES_DEBUG_TRACE3(fmt, _1, _2, _3) printf(fmt, (_1), (_2), (_3));
# define ES_DEBUG_TRACE4(fmt, _1, _2, _3, _4) printf(fmt, (_1), (_2), (_3), (_4));
# define ES_DEBUG_TRACE5(fmt, _1, _2, _3, _4, _5) printf(fmt, (_1), (_2), (_3), (_4), (_5));
# define ES_DEBUG_TRACE6(fmt, _1, _2, _3, _4, _5, _6) printf(fmt, (_1), (_2), (_3), (_4), (_5), (_6));
# define ES_DEBUG_TRACE7(fmt, _1, _2, _3, _4, _5, _6, _7) printf(fmt, (_1), (_2), (_3), (_4), (_5), (_6), (_7));
#elif USE_EMULATOR
# define ES_DEBUG_TRACE0(fmt) ES_DEBUG_TRACE_RAW(fmt);
# define ES_DEBUG_TRACE1(fmt, _1) ES_DEBUG_TRACE_RAW(fmt, _1);
# define ES_DEBUG_TRACE2(fmt, _1, _2) ES_DEBUG_TRACE_RAW(fmt, _1, _2);
# define ES_DEBUG_TRACE3(fmt, _1, _2, _3) ES_DEBUG_TRACE_RAW(fmt, _1, _2, _3);
# define ES_DEBUG_TRACE4(fmt, _1, _2, _3, _4) ES_DEBUG_TRACE_RAW(fmt, _1, _2, _3, _4);
# define ES_DEBUG_TRACE5(fmt, _1, _2, _3, _4, _5) ES_DEBUG_TRACE_RAW(fmt, _1, _2, _3, _4, _5);
# define ES_DEBUG_TRACE6(fmt, _1, _2, _3, _4, _5, _6) ES_DEBUG_TRACE_RAW(fmt, _1, _2, _3, _4, _5, _6);
# define ES_DEBUG_TRACE7(fmt, _1, _2, _3, _4, _5, _6, _7) ES_DEBUG_TRACE_RAW(fmt, _1, _2, _3, _4, _5, _6, _7);
#else
# define ES_DEBUG_TRACE0(fmt)
# define ES_DEBUG_TRACE1(fmt, _1)
# define ES_DEBUG_TRACE2(fmt, _1, _2)
# define ES_DEBUG_TRACE3(fmt, _1, _2, _3)
# define ES_DEBUG_TRACE4(fmt, _1, _2, _3, _4)
# define ES_DEBUG_TRACE5(fmt, _1, _2, _3, _4, _5)
# define ES_DEBUG_TRACE6(fmt, _1, _2, _3, _4, _5, _6)
# define ES_DEBUG_TRACE7(fmt, _1, _2, _3, _4, _5, _6, _7)
#endif

#endif // _trace_h_
