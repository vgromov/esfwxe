#ifndef _ese_crc_intf_h_
#define _ese_crc_intf_h_

/// Generalized CRC calculator interface
///
class ESE_ABSTRACT EseCrcIntf
{
public:
  virtual void destroy() ESE_NOTHROW = 0;

  virtual void reset(uint32_t val) ESE_NOTHROW = 0;

  virtual uint32_t update(uint8_t val) ESE_NOTHROW = 0;
  virtual uint32_t update(uint16_t val) ESE_NOTHROW = 0;
  virtual uint32_t update(uint32_t val) ESE_NOTHROW = 0;
  virtual uint32_t update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW = 0;

  virtual uint32_t get() const ESE_NOTHROW = 0;
  
  template <typename DataT>
  uint32_t update(const DataT& data) ESE_NOTHROW { return update( reinterpret_cast<const uint8_t*>(&data), sizeof(DataT) ); }
};

#endif //< _ese_crc_intf_h_
