/*

    Interface definitions for bget.c, the memory management package.

*/

#include <bgetConfig.h>

#ifdef __cplusplus
	extern "C" {
#endif

void	bpool(void *buffer, bufsize len);
void*	bget(bufsize size);
void*	bgetz(bufsize size);
void*	bgetr(void *buffer, bufsize newsize);
void	brel(void *buf);
void	bectl(int (*compact)(bufsize sizereq, int sequence),
					 					void *(*acquire)(bufsize size),
					 					void (*release)(void *buf), bufsize pool_incr);
void	bstats(bufsize *curalloc, bufsize *totfree, bufsize *maxfree,
					 					long *nget, long *nrel);
void	bstatse(bufsize *pool_incr, long *npool, long *npget,
					 					long *nprel, long *ndget, long *ndrel);
void	bufdump(void *buf);
void	bpoold(void *pool, int dumpalloc, int dumpfree);
int		bpoolv(void *pool);

#if defined(USE_TRACE) && defined(USE_MEM_TRACE)
	void traceMem(void);
	#define TRACE_MEM	traceMem();
#else
	#define TRACE_MEM
#endif

#ifdef __cplusplus
	}
#endif
