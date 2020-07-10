#ifndef _es_device_hw_info_h_
#define _es_device_hw_info_h_

#ifdef __cplusplus
  extern "C" {
#endif

// device hardware information blocks support structs and macros
//
enum 
{
  hwMCU      = 0,
  hwUART     = 1,
  hwSPI      = 2,
  hwI2C      = 3,
  hwRTC      = 4,
  hwADC      = 5,
  hwDAC      = 6,
  hwRAM      = 7,
  hwNVRAM    = 8
};

typedef struct
{
  esU16 size; // total entry size
  esU16 id;
  esU16 locationEntry;
  esU16 order;
  // optional device name length, limited to 16 bytes
  esU8 nameLen[16];

} EsDeviceHwInfoEntryHdr;

typedef struct 
{
  EsDeviceHwInfoEntryHdr hdr;

  esU32 errorCode;

} EsDeviceHwInfoBusEntry;
    
typedef struct
{
  EsDeviceHwInfoEntryHdr hdr;

  EsMemSpaceInfo total;
  EsMemSpaceInfo free;

} EsDeviceHwInfoStorageEntry;

// hardware information block
typedef struct 
{
  esU32 health;   // latched errors, as in standard health info
  esU16 ver;      // hardware version
  esU16 entryCnt; // hardware info entry count

} EsDeviceHwInfoHdr;

enum {
  EsDeviceHwInfoEntryHdr_SZE = sizeof(EsDeviceHwInfoEntryHdr),
  EsDeviceHwInfoBusEntry_SZE = sizeof(EsDeviceHwInfoBusEntry),
  EsDeviceHwInfoStorageEntry_SZE = sizeof(EsDeviceHwInfoStorageEntry),
  EsDeviceHwInfoHdr_SZE = sizeof(EsDeviceHwInfoHdr),
};

#ifdef __cplusplus
  }
#endif

#endif // _es_device_hw_info_h_
