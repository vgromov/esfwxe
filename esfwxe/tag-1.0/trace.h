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
#	define TRACE_BOOLSTR(b)	((b) ? "true" : "false")
# ifndef USE_EMULATOR
#		define TRACE	printf
#		define TRACE_FMT_STR "%s"
#		define TRACE_EOL	"\n\r"
#	else
		void TRACE(ES_ASCII_CSTR s, ...);
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

#endif // _trace_h_
