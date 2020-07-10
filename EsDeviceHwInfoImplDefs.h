// macros for hw info blocks definition. must be included just before the corresponding hwinfo.cc file
//

#ifndef ES_HW_INFO_NAMESPACE
#  define ES_HW_INFO_NAMESPACE
#endif

#define ES_HW_INFO_BEGIN(_ver, _cnt) \
static esCONCAT2(ES_HW_INFO_NAMESPACE, HardwareInfo) esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo) = { \
  { 0, (_ver), (_cnt) }
  
#define ES_HW_INFO_ENTRY_DEVICE(_id, _location, _order, _name) \
  , { EsDeviceHwInfoEntryHdr_SZE, (_id), (_location), (_order), esSTRINGIZE(_name)}

#define ES_HW_INFO_ENTRY_MCU(_location, _order, _name) \
  , { EsDeviceHwInfoEntryHdr_SZE, hwMCU, (_location), (_order), esSTRINGIZE(_name)}

#define ES_HW_INFO_ENTRY_BUS(_id, _location, _order, _name) \
  , { { EsDeviceHwInfoBusEntry_SZE, (_id), (_location), (_order), esSTRINGIZE(_name)}, 0 }

#define ES_HW_INFO_ENTRY_RAM(_location, _order, _name) \
  , { { EsDeviceHwInfoStorageEntry_SZE, hwRAM, (_location), (_order), esSTRINGIZE(_name)}, \
      {  0, 0, msuByte, 0}, {  0, 0, msuByte, 0} }

#define ES_HW_INFO_ENTRY_NVRAM(_location, _order, _name) \
  , { { EsDeviceHwInfoStorageEntry_SZE, hwNVRAM, (_location), (_order), esSTRINGIZE(_name)}, \
      {  0, 0, msuByte, 0}, {  0, 0, msuByte, 0} }

#define ES_HW_INFO_END \
  }; \
esU32 esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoHealthGet)(void) \
{ return esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo).hdr.health; } \
void esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoHealthSet)(esU32 health) \
{ esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo).hdr.health = health; } \
esU16 esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoVerGet)(void) \
{ return esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo).hdr.ver; } \
esU16 esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoEntriesCountGet)(void) \
{ return esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo).hdr.entryCnt; } \
const EsDeviceHwInfoEntryHdr* esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoEntryGet)(esU16 idx) \
{ if( idx < esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo).hdr.entryCnt ) { \
    const EsDeviceHwInfoEntryHdr* entry = (const EsDeviceHwInfoEntryHdr*)((const esU8*)&esCONCAT3(s_, ES_HW_INFO_NAMESPACE, hardwareInfo) + EsDeviceHwInfoHdr_SZE); \
    while(idx--) entry = (const EsDeviceHwInfoEntryHdr*)((const esU8*)entry + entry->size); \
    return entry; \
  } \
return 0; }
