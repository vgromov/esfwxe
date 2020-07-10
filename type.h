#ifndef __esfwxe_type_h__
#define __esfwxe_type_h__

// concatenation stuff for preprocessor
#define esCONCAT_HELPER(text, line)   text ## line
#define esCONCAT2(text, line)         esCONCAT_HELPER(text, line)
#define esCONCAT3(x1, x2, x3)         esCONCAT2(esCONCAT2(x1, x2), x3)
#define esCONCAT4(x1, x2, x3, x4)     esCONCAT2(esCONCAT3(x1, x2, x3), x4)
#define esCONCAT5(x1, x2, x3, x4, x5) esCONCAT2(esCONCAT4(x1, x2, x3, x4), x5)
// stringizer stuff for preprocessor
#define esSTRINGIZE_HELPER(x)         #x
#define esSTRINGIZE(x)                esSTRINGIZE_HELPER(x)

// Former absacc.h stuff
#ifndef USE_EMULATOR
# ifndef __section
#    define __section(s) __attribute__(( section(s) ))
# endif

# ifndef __at
#    define __at(a)  __attribute__(( at(a) ))
# endif
#endif // USE_EMULATOR

#include <stdint.h>

#ifdef __cplusplus
  extern "C" {
#endif

typedef char                        ESE_CHAR;
typedef const ESE_CHAR*             ESE_CSTR;
typedef ESE_CHAR*                   ESE_STR;

// Basic _sized_ typedefs. these _must_ be in one word, without spaces, it's strongly required for
// the remote reflection engine implementation
//
typedef int8_t                      esI8;
typedef uint8_t                     esU8;
typedef int16_t                     esI16;
typedef uint16_t                    esU16;
typedef int32_t                     esI32;
typedef uint32_t                    esU32;
typedef int64_t                     esI64;
typedef uint64_t                    esU64;

typedef esI64                       esDT;
typedef esI64                       esTS;
typedef float                       esF;
typedef double                      esD;

// more human - friendly types
typedef unsigned int                esBL;

typedef esU8                        esBCD;
typedef void*                       busHANDLE;

// special bytearray type
typedef struct {
  esU32 size;
  esU8*  data;

} esBA;

enum {
  NO_ID = 0xFFFF, // special id meaning "no id is found", "no device", etc.
  // power status masks
  powerBattery                    = 0x01,
  powerMains                      = 0x02,
  powerCharging                   = 0x04,
  powerFastCharging               = 0x08
};

// memory space units
enum {
  msuByte,
  msuKbyte,
  msuMbyte,
  msuGbyte,
  // special value - must go last
  msuCount
};

#pragma pack(push, 1)
// power status structure, returned
// from power status rpc call
//
typedef struct _tagEsePowerStatus {
  esU16 minCode;   // device - dependent power codes
  esU16 curCode;
  esU16 maxCode;
  esU8  flags;    // power flags mask (optional, may be 0)

} EsePowerStatus;

typedef struct _tagEsMemSpaceInfo {
  esU16 count;
  esU16 frac;
  esU8 unit; // = msuXXX
  esU8 dummy;

} EsMemSpaceInfo;

// file system result
typedef esU16 FsResult;

// file system item struct. used in rpc
// directory listing, rpc FS functions
//
typedef struct {
  esU32     fsize;      // file size
  esDT      fts;        // file time stamp
  esU8      attrs;      // file attributes
  char      fname[13];  // file name (short)

} FsItem;

// file system enumeration|search result struct
//
typedef struct {
  FsResult  result;    // file operation result
  FsItem    item;

} FsItemResult;

// file system space info
//
typedef struct {
  FsResult       result;    // file operation result
  EsMemSpaceInfo total;
  EsMemSpaceInfo free;

} FsSpaceInfo;

// file read request result
//
typedef struct {
  FsResult   result;   // file existence check result
  esU32    chunks;    // esimate of file chunks count needed to complete file reading

} FsFileReadRequest;

#pragma pack(pop)

/// Misc helper values and masks
enum {
  /// File attributes
  FsAttrReadOnly         = 0x01,
  FsAttrHidden           = 0x02,
  FsAttrSystem           = 0x04,
  FsAttrVolumeLabel      = 0x08,
  FsAttrDirectory        = 0x10,
  FsAttrArchive          = 0x20,

  /// Standard HW config info flags
  HwCfgStdBletooth       = 0x0001,
  HwCfgStdBletoothLE     = 0x0002,
  HwCfgStdWIFI           = 0x0004,
  HwCfgStdUSBslave       = 0x0008,
  HwCfgStdUSBhost        = 0x0010,
  HwCfgStdAccumulator    = 0x0020,

  /// Struct sizes
  PowerStatus_SZE        = sizeof(EsePowerStatus),
  FsItem_SZE             = sizeof(FsItem),
  FsResult_SZE           = sizeof(FsResult),
  FsItemResult_SZE       = sizeof(FsItemResult),
  EsMemSpaceInfo_SZE     = sizeof(EsMemSpaceInfo),
  FsSpaceInfo_SZE        = sizeof(FsSpaceInfo),
  FsFileReadRequest_SZE  = sizeof(FsFileReadRequest)
};

// special values
#ifndef NULL
  #define NULL              (0)
#endif

#ifndef FALSE
  #define FALSE             ((esBL)0)
#endif

#ifndef TRUE
  #define TRUE              ((esBL)1)
#endif

#ifndef INVALID_HANDLE
  #define INVALID_HANDLE     NULL
#endif

// cover possible different int 64 suffix compiler conventions
#define IMMEDIATE_INT64( Immediate )  (Immediate ## LL)
#define IMMEDIATE_UINT64( Immediate )  (Immediate ## ULL)

// define port handle type
#define DEF_VOLATILE_HANDLE(Name)  typedef volatile void* Name
// special cast for port handles and structures
#define CAST_PORT_HANDLE(PortStructType, PortHandle)  ((volatile PortStructType*)(PortHandle))
// define port struct pointer var
#define DEF_PORT_STRUCT_VAR(PortStructType, PortHandle) volatile PortStructType* ps = CAST_PORT_HANDLE(PortStructType, PortHandle)

// define unused procedure parameter, to avoid compiler warnings
#define ESE_UNUSED(x)

// keil core libraries define their own short-cuts for types
// so handle these typedefs as well
//
#ifndef U32
# define U32                esU32
#endif

#ifndef U16
# define U16                esU16
#endif

#ifndef U8
# define U8                 esU8
#endif

#ifndef __TRUE
# define __TRUE             TRUE
#endif

#ifndef __FALSE
# define __FALSE            FALSE
#endif

#ifdef __cplusplus
  }
#endif

#ifdef __cplusplus

# ifdef ES_MODERN_CPP
#   ifdef ESE_MODERN_CPP
#     undef ESE_MODERN_CPP
#   endif
#   define ESE_MODERN_CPP
# endif

# ifndef ESE_MODERN_CPP
#   if __cplusplus >= 201103L
#     define ESE_MODERN_CPP
#   endif
# endif

# ifdef USE_CPP_EXCEPTIONS
#   ifdef ESE_MODERN_CPP
#     define ESE_NOTHROW    noexcept
#   else
#     define ESE_NOTHROW    throw()
#   endif
# else
#   define ESE_NOTHROW
# endif

# ifdef ESE_MODERN_CPP
#   define ESE_OVERRIDE     override
#   define ESE_REMOVE_DECL  = delete
#   ifndef NULL
#     define NULL           nullptr
#   endif
# else
#   define nullptr          NULL
#   define ESE_OVERRIDE
#   define ESE_REMOVE_DECL
# endif
# if defined(__GNUC__) || defined(__ARMCC_VERSION)
#   define ESE_ABSTRACT
#   define ESE_KEEP         __attribute__((used))
# else
#   define ESE_ABSTRACT     __declspec(novtable)
#   define ESE_KEEP
# endif

# define ESE_NODEFAULT_CTOR(T) \
  private: \
    T() ESE_NOTHROW ESE_REMOVE_DECL

# define ESE_NONCOPYABLE(T) \
  private: \
    T(const T&) ESE_NOTHROW ESE_REMOVE_DECL; \
    T& operator=(const T&) ESE_NOTHROW ESE_REMOVE_DECL

#else // __cplusplus

# if defined(__GNUC__) || defined(__ARMCC_VERSION)
#   define ESE_ABSTRACT
#   define ESE_KEEP         __attribute__((used))
# else
#   define ESE_ABSTRACT     __declspec(novtable)
#   define ESE_KEEP
# endif

#endif

#if defined(__ARMCC_VERSION) && __ARMCC_VERSION < 6000000
# define ESE_ANON_UNION _Pragma("anon_unions")
#elif defined(__GNUC__)
# define ESE_ANON_UNION
#else
# define ESE_ANON_UNION
#endif

/// UTF-8 literals (where supported)
#define eseU8(str)   str

#endif  // __esfwxe_type_h__

