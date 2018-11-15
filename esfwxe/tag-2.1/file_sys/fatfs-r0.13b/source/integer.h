/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef FF_INTEGER
#define FF_INTEGER

#ifdef _WIN32    /* FatFs development platform */

#include <windows.h>
typedef unsigned __int64 QWORD;

#elif defined(__esfwxe_type_h__)

typedef esI32           INT;
typedef esU32           UINT;
typedef esU8            BYTE;

/* These types MUST be 16-bit */
typedef esI16           SHORT;
typedef esU16           WORD;
typedef esU16           WCHAR;

/* These types MUST be 32-bit */
typedef esI32           LONG;
typedef esU32           DWORD;

/* This type MUST be 64-bit (Remove this for ANSI C (C89) compatibility) */
typedef esU64           QWORD;

#else            /* Embedded platform */

/* These types MUST be 16-bit or 32-bit */
typedef int             INT;
typedef unsigned int    UINT;

/* This type MUST be 8-bit */
typedef unsigned char   BYTE;

/* These types MUST be 16-bit */
typedef short           SHORT;
typedef unsigned short  WORD;
typedef unsigned short  WCHAR;

/* These types MUST be 32-bit */
typedef long            LONG;
typedef unsigned long   DWORD;

/* This type MUST be 64-bit (Remove this for ANSI C (C89) compatibility) */
typedef unsigned long long QWORD;

#endif

#endif
