// macros for hw info blocks declaration. must be included just before the corresponding hwinfo.cc file
//
#ifndef ES_HW_INFO_NAMESPACE
#	define ES_HW_INFO_NAMESPACE
#endif

#define ES_HW_INFO_BEGIN(_ver, _cnt) \
typedef struct { \
	EsDeviceHwInfoHdr hdr;
	
#define ES_HW_INFO_ENTRY_DEVICE(_id, _location, _order, _name) \
	EsDeviceHwInfoEntryHdr esCONCAT3(_name, _, _order);

#define ES_HW_INFO_ENTRY_MCU(_location, _order, _name) \
	EsDeviceHwInfoEntryHdr esCONCAT3(_name, _, _order);

#define ES_HW_INFO_ENTRY_BUS(_id, _location, _order, _name) \
	EsDeviceHwInfoBusEntry esCONCAT3(_name, _, _order);

#define ES_HW_INFO_ENTRY_RAM(_location, _order, _name) \
	EsDeviceHwInfoStorageEntry esCONCAT3(_name, _, _order);

#define ES_HW_INFO_ENTRY_NVRAM(_location, _order, _name) \
	EsDeviceHwInfoStorageEntry esCONCAT3(_name, _, _order);

#define ES_HW_INFO_END \
	} esCONCAT2(ES_HW_INFO_NAMESPACE, HardwareInfo); \
esU32 esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoHealthGet)(void); \
void esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoHealthSet)(esU32 health); \
esU16 esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoVerGet)(void); \
esU16 esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoEntriesCountGet)(void); \
const EsDeviceHwInfoEntryHdr* esCONCAT2(ES_HW_INFO_NAMESPACE, hardwareInfoEntryGet)(esU16 idx);
