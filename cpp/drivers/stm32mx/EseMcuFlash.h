#ifndef _ese_mcu_flash_h_
#define _ese_mcu_flash_h_

class EseMcuFlash
{
public:
  EseMcuFlash() ESE_NOTHROW ESE_KEEP;
  
  void dataWriteAreaSet(uint32_t addr, uint32_t size) ESE_NOTHROW ESE_KEEP;
  void dataAddressReset() ESE_NOTHROW ESE_KEEP;
  
  uint32_t dataAddressStartGet() const ESE_NOTHROW ESE_KEEP;
  uint32_t dataAddressCurrentGet() const ESE_NOTHROW ESE_KEEP;
  void dataAddressSet(uint32_t addr) ESE_NOTHROW ESE_KEEP;
  
  uint32_t dataWrite(uint32_t len, const esU8* data) ESE_NOTHROW ESE_KEEP;
  
  template <typename DataT>
  static const DataT* dataPtrAccess(uint32_t addr) ESE_NOTHROW
  {
    return reinterpret_cast<const DataT*>( addr );
  }

  /// Write-access lock-unlock
  static bool lock() ESE_NOTHROW ESE_KEEP;
  static bool unlock() ESE_NOTHROW ESE_KEEP;

  static uint32_t pageFromAddr(uint32_t addr) ESE_NOTHROW ESE_KEEP;
  static uint32_t pagesCountFromSize(uint32_t size) ESE_NOTHROW ESE_KEEP;
 
  bool pagesErase() ESE_NOTHROW ESE_KEEP;

protected:
  uint32_t m_addr;
  uint32_t m_addrEnd;
  uint32_t m_addrCur;
  
  ESE_NONCOPYABLE(EseMcuFlash);
};

#endif //< _ese_mcu_flash_h_
