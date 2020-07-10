#ifndef _ese_crc_intf_impl_h_
#define _ese_crc_intf_impl_h_

#include <crcConfig.h>

/// CRC interface implementation base class
///
class EseCrcIntfImpl : public EseCrcIntf
{
public:
  EseCrcIntfImpl(uint32_t val) ESE_NOTHROW ESE_KEEP : m_crc(val) {}
  virtual ~EseCrcIntfImpl() ESE_NOTHROW ESE_KEEP {}

  /// EseCrcIntf partial implementation
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { delete this; }
  virtual void reset(uint32_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { m_crc = val; }
  virtual uint32_t get() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_crc; }

protected:
  uint32_t m_crc;
  
  ESE_NODEFAULT_CTOR(EseCrcIntfImpl);
  ESE_NONCOPYABLE(EseCrcIntfImpl);
};

/// Distinct CRC algorithm specializations
///

#ifdef USE_CRC7
class EseCrc7 : public EseCrcIntfImpl
{
public:
  EseCrc7(uint32_t val = 0) ESE_NOTHROW ESE_KEEP;

  /// CRC intf implementation
  ///
  virtual uint32_t update(uint8_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint16_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint32_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
};
#endif

#ifdef USE_CRC8
class EseCrc8 : public EseCrcIntfImpl
{
public:
  EseCrc8(uint32_t val = 0) ESE_NOTHROW ESE_KEEP;

  /// CRC intf implementation
  ///
  virtual uint32_t update(uint8_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint16_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint32_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
};
#endif

// calculate misc CRC16
//
#ifdef USE_CRC16_CCITT
class EseCrc16_CCIIT : public EseCrcIntfImpl
{
public:
  EseCrc16_CCIIT(uint32_t val = 0) ESE_NOTHROW ESE_KEEP;

  /// CRC intf implementation
  ///
  virtual uint32_t update(uint8_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint16_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint32_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
};
#endif

// calculate checksum 16 as of RFC1071
#ifdef USE_CHECKSUM16
class EseCrc16_RFC1071 : public EseCrcIntfImpl
{
public:
  EseCrc16_RFC1071(uint32_t val = 0) ESE_NOTHROW ESE_KEEP;

  /// CRC intf implementation
  ///
  virtual uint32_t update(uint8_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint16_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint32_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t get() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
};
#endif

// calculate misc CRC32
//
#ifdef USE_CRC32_IEEE_802_3
class EseCrc32_IEEE_802_3 : public EseCrcIntfImpl
{
public:
  EseCrc32_IEEE_802_3(uint32_t val = 0) ESE_NOTHROW ESE_KEEP;

  /// CRC intf implementation
  ///
  virtual uint32_t update(uint8_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint16_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(uint32_t val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
};
#endif

#endif //< _ese_crc_intf_impl_h_
