#ifndef _protocol_octava_h_
#define _protocol_octava_h_

#ifdef __cplusplus
  extern "C" {
#endif

// octava proprietary protocol implementation
//
// frame magic
extern const esU8 c_octavaMagic[4];

#pragma pack(push, 2)

// octava device id
typedef struct {
   esU16 typeData;
  esU16 typeAppliance;
  esU16 vin[2];

}  OctavaDeviceId;

// common data packet header
typedef struct {
  esU16 dataByteLen;  
  OctavaDeviceId id;
  
} OctavaDataHeader;

// device - specific data contents
//
// p3-80e contents 
typedef struct {
  esU16 dummy0[38];
  esU32 msrLen;       // measurement length in sec
  esI32 dummy1;
  esI32 calibration;  // calibration value
  esI32 refLevel;     // reference level
  esI32 sensitivity;   // sensor sensitivity value
  esI32 dummy2[3];
  esU16 runFlag;
  esU16 inputOverloadFlag;
  esI32 edb;
  esI32 edbMin;
  esI32 edbMax;
  
} OctavaP3_80Econtents;

// p3-81-1|2 contents 
typedef struct {
  esI32 mx;
  esI32 my;
  esI32 mz;
  esI32 mxyz;
  esI32 mxy;
  esI32 mdb;
  esI32 mdbMin;
  esI32 mdbMax;
  
} OctavaP3_81_1_2node;

typedef struct {
  esU16 dummy0[38];
  esU32 msrLen;       // measurement length in sec
  esI32 dummy1;
  esI32 calibration;  // calibration value
  esI32 refLevel;     // reference level
  esI32 sensitivity;   // sensor sensitivity value
  esI32 dummy2[3];
  esU16 runFlag;
  esU16 inputOverloadFlag;
  esI32 dummy3;
  OctavaP3_81_1_2node  m0;
  OctavaP3_81_1_2node  m50;
  esU16 vin[2];
  OctavaP3_81_1_2node  m25;

} OctavaP3_81_1_2contents;

// sizes
enum {
  octavaDeviceId_SZE = sizeof(OctavaDeviceId),
  octavaDataHeader_SZE = sizeof(OctavaDataHeader),
  octavaMaxFrame_SZE = 512,
  octavaMaxDataSpace_SZE = octavaMaxFrame_SZE-octavaDataHeader_SZE-2,
  octavaP3_80Econtents_SZE = sizeof(OctavaP3_80Econtents),
  cctavaP3_81_1_2contents_SZE = sizeof(OctavaP3_81_1_2contents),
};

// protocol frame header buffer
typedef struct {
  OctavaDataHeader hdr;
  
  union {
    esU8 raw[octavaMaxFrame_SZE-octavaDataHeader_SZE];
    OctavaP3_80Econtents p380e;
    OctavaP3_81_1_2contents p381_1_2;
    
  } data;
  
} OctavaFrameBuffer;

#pragma pack(pop)

// octava data frame reading|interpretation api
//
esBL octavaProtocolFrameRead(OctavaFrameBuffer* buff, EsChannelIo* chnl);
esBL octavaProtocolFrameWrite(const esU8* data, esU32 dataLen, EsChannelIo* chnl);

#ifdef __cplusplus
  }
#endif

#endif  // _protocol_octava_h_
