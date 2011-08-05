#ifndef __common_TYPE_H__
#define __common_TYPE_H__

#ifdef __cplusplus
	extern "C" {
#endif

typedef char									ES_ASCII_CHAR;
typedef const ES_ASCII_CHAR*	ES_ASCII_CSTR;
typedef ES_ASCII_CHAR*				ES_ASCII_STR;

// basic typedefs. these _must_ be in one word, without spaces, it's strongly required for 
// the remote reflection engine implementation
#ifndef _BASETSD_H_
	typedef signed char					INT8;
	typedef unsigned char				UINT8;
	typedef signed short 				INT16;
	typedef unsigned short			UINT16;
	typedef signed long					INT32;
	typedef unsigned long				UINT32;
#endif

typedef long long			 			INT64;
typedef unsigned long long	UINT64;

#ifndef _WINDEF_
	typedef unsigned int   			BOOL;
#endif

typedef INT64								DATETIME;
typedef INT64								TIMESPAN;
typedef float								FLOAT;
typedef double							DOUBLE;

// more human - friendly types
#define VOID								void

#ifndef _WINDEF_
#	define CHAR									INT8
#	define BYTE									UINT8
#	define WORD									UINT16
#	define DWORD								UINT32
#	define QWORD								UINT64
	typedef int									INT;
	typedef unsigned int				UINT;
#endif

#define BCDBYTE							UINT8
typedef volatile void*			busHANDLE;	

// special bytearray type
typedef struct {
	DWORD size;
	BYTE*	data;

} BYTEARRAY;

enum {
	NO_ID = 0xFFFF, // special id meaning "no id is found", "no device", etc.
	// power status masks
	powerBattery = 0x01,
	powerMains = 0x02,
	powerCharging = 0x03,
};

#pragma pack(push, 1)
// power status structure, returned
// from power status rpc call
//
typedef struct {
	WORD minCode;	 // device - dependent power codes
	WORD curCode;
	WORD maxCode;
	BYTE flags;		// power flags mask (optional, may be 0)

} PowerStatus;

// memory space units
enum {
	msuByte,
	msuKbyte,
	msuMbyte,
	msuGbyte,
	// special value - must go last
	msuCount
};

typedef struct {
	WORD count;
	WORD frac;
	BYTE unit; // = msuXXX
	
} EsMemSpace;

// file system result
typedef WORD FsResult;

// file system item struct. used in rpc
// directory listing, rpc FS functions
//
typedef struct {
	DWORD			fsize;			// file size
	DATETIME	fts;				// file time stamp
	BYTE			attrs;			// file attributes
	char			fname[13];	// file name (short)

} FsItem;

// file system enumeration|search result struct
//
typedef struct {
	FsResult	result;		// file operation result
	FsItem		item;

} FsItemResult;



// file system space info
//
typedef struct {
	FsResult	result;		// file operation result
	EsMemSpace		total;
	EsMemSpace		free;

} FsSpaceInfo;

// file read request result
//
typedef struct {
	FsResult 	result; 	// file existence check result
	DWORD			chunks;		// esimate of file chunks count needed to complete file reading

} FsFileReadRequest;

#pragma pack(pop)

// misc helper values and masks
enum {
	// file attributes
	FsAttrReadOnly				= 0x01,
	FsAttrHidden					= 0x02,
	FsAttrSystem					= 0x04,
	FsAttrVolumeLabel			= 0x08,
	FsAttrDirectory				= 0x10,
	FsAttrArchive					= 0x20,
	// struct sizes
	PowerStatus_SZE				= sizeof(PowerStatus),	
	FsItem_SZE						= sizeof(FsItem),
	FsResult_SZE					= sizeof(FsResult),
	FsItemResult_SZE			= sizeof(FsItemResult),
	EsMemSpace_SZE				= sizeof(EsMemSpace),
	FsSpaceInfo_SZE				= sizeof(FsSpaceInfo),
	FsFileReadRequest_SZE = sizeof(FsFileReadRequest),
};

// special values
#ifndef NULL
	#define NULL							(0)
#endif

#ifndef FALSE
	#define FALSE   					((BOOL)0)
#endif

#ifndef TRUE
	#define TRUE    					((BOOL)1)
#endif

#ifndef INVALID_HANDLE
	#define INVALID_HANDLE 		NULL
#endif

// cover possible different int 64 suffix compiler conventions
#define IMMEDIATE_INT64( Immediate )	(Immediate ## LL)
#define IMMEDIATE_UINT64( Immediate )	(Immediate ## ULL)

// define port handle type
#define DEF_VOLATILE_HANDLE(Name)	typedef volatile void* Name
// special cast for port handles and structures
#define CAST_PORT_HANDLE(PortStructType, PortHandle)	((volatile PortStructType*)(PortHandle))
// define port struct pointer var
#define DEF_PORT_STRUCT_VAR(PortStructType, PortHandle) volatile PortStructType* ps = CAST_PORT_HANDLE(PortStructType, PortHandle)

// define unused procedure parameter, to avoid compiler warnings
#ifdef __cplusplus
#	define UNUSED(x)
#else
#	define UNUSED(x)	x
#endif

#ifdef __cplusplus
	}
#endif

#endif  // __common_TYPE_H__

