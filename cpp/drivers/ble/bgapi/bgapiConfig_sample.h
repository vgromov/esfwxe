#ifndef _bgapi_config_h_
#define _bgapi_config_h_

// Copy this to your project directory
// Tuneup defines, and rename to 
// bgapiConfig.h
//

// BLEBGAPI engine configuration
//

// Common defines, do not change, or do it at your own risc
#define ESE_BLEBG_IO_RETRIES_MAX          8
#define ESE_BLEBG_IO_RETRIES_MIN          1 
#define ESE_BLEBG_READ_TMO_MIN            50

//----------------------------------------------------------------------------
// User configurable part start

// Define ESE_BLEBG_SECTION to place BGAPI code, data to specific section
#define ESE_BLEBG_SECTION

// Define ESE_BLEBG_USE_MEMINTF to use EseMemoryIntf to manage dynamic memory manipulation
// Otherwise, standard memory functions memalloc, free, etc will be used
#define ESE_BLEBG_USE_MEMINTF

// Define ESE_BLEBG_USE_OSINTF to use EseOsIntf abstraction to create and access OS objects
// Otherwise, direct RTTOS API and objects will be used
#define ESE_BLEBG_USE_OSINTF

// Define ESE_BLEBG_USE_DBGINTF to use EseDebugIntf abstraction for trace and assert
#define ESE_BLEBG_USE_DBGINTF

// Define to utilize BGAPI in packet mode
#define ESE_BLEBG_PACKET_MODE

// Define BGAPI event queue length
#define ESE_BLEBG_EVT_QUEUE_LEN           4

// Define BGAPI wakeup state timeout, ms
// After this timeout is expired, API allows BG SOC to enter sleep state
#define ESE_BLEBG_SLEEP_ALLOW_TMO         500

// Define BGAPI channel read timeout, ms
#define ESE_BLEBG_READ_TMO                500

// Define BGAPI command send maximum retries count
#define ESE_BLEBG_IO_RETRIES              3

// User configurable part end
//----------------------------------------------------------------------------

// Finalize and cleanup defines
#if !defined(ESE_BLEBG_READ_TMO) || ESE_BLEBG_READ_TMO < ESE_BLEBG_READ_TMO_MIN
# undef ESE_BLEBG_READ_TMO
# define ESE_BLEBG_READ_TMO               ESE_BLEBG_READ_TMO_MIN
#endif
  
#if !defined(ESE_BLEBG_SLEEP_ALLOW_TMO) || ESE_BLEBG_SLEEP_ALLOW_TMO < ESE_BLEBG_READ_TMO
# undef ESE_BLEBG_SLEEP_ALLOW_TMO
# define ESE_BLEBG_SLEEP_ALLOW_TMO        (2*ESE_BLEBG_READ_TMO)
#endif
    
#if !defined(ESE_BLEBG_IO_RETRIES) || ESE_BLEBG_IO_RETRIES < ESE_BLEBG_IO_RETRIES_MIN || ESE_BLEBG_IO_RETRIES > ESE_BLEBG_IO_RETRIES_MAX
# undef ESE_BLEBG_IO_RETRIES
# define ESE_BLEBG_IO_RETRIES             (ESE_BLEBG_IO_RETRIES_MAX/2)
#endif

#endif //< _bgapi_config_h_
