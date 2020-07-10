#ifndef _ese_hw_user_app_header_h_
#define _ese_hw_user_app_header_h_

#ifdef __cplusplus
extern "C" {
#endif

/// AQ application ROM header
///
typedef struct 
{
  uint32_t m_size;                        ///< Size of this header+size of application firmware that follows
  uint32_t m_crc;
  uint16_t m_type;
  uint16_t m_countryCode;
  uint8_t m_coreVerMajor;
  uint8_t m_coreVerMinor;
  uint8_t m_verMajor;
  uint8_t m_verMinor;
  
} EseUserAppHeader;

#ifdef __cplusplus
}
#endif

#endif //< _ese_hw_user_app_header_h_
