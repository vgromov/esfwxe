#ifndef _user_SWI_h_
#define _user_SWI_h_

// swi mappings have to be described in separate swiImpl.h file in form
// DEF_SWI_HANDLER(swiNumber, swiHandlerName)
//

#ifdef __cplusplus
 extern "C" {
#endif

#define DEF_SWI_HANDLER(swiNumber, swiHandlerName) \
	extern void swiHandlerName (void) __svc( swiNumber );
#define IMPL_SWI_HANDLER

#include <swiImpl.cc>

#ifdef __cplusplus
 }
#endif

// swi handler implementations must be placed in the same swiImpl.cc
// in #ifdef | #endif IMPL_SWI section using IMPL_SWI_HANDLER( swiHandlerName ) as implementation entry point

#endif //	_user_SWI_h_
