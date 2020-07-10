#ifndef _ese_hw_id_utils_fmt_flags_h_
#define _ese_hw_id_utils_fmt_flags_h_

#ifdef ES_USE_FWID_FORMATTERS

/// ApplianceFwID standard formatting flags
///
enum {
  APPL_ID_FMT_DESCR_SHORT = 0x0001,
  APPL_ID_FMT_DESCR_LONG  = 0x0002,
  APPL_ID_FMT_SERIAL      = 0x0004,
  APPL_ID_FMT_FW          = 0x0008,
  APPL_ID_FMT_REGION      = 0x0010,
  APPL_ID_FMT_HWINFO      = 0x0020,
  APPL_ID_FMT_SERIAL_SHORT= 0x0040, ///< Shortened serial number version OOOOO-YYMMDD
  
  /// Specific formatting masks
  ///

  /// Perform full formatting
  ///

  /// Use short device description
  APPL_ID_FMT_ALL_SHORT   = APPL_ID_FMT_DESCR_SHORT|
                            APPL_ID_FMT_SERIAL|
                            APPL_ID_FMT_FW|
                            APPL_ID_FMT_REGION|
                            APPL_ID_FMT_HWINFO,
                            
  /// Use long device description
  APPL_ID_FMT_ALL_LONG    = APPL_ID_FMT_DESCR_LONG|
                            APPL_ID_FMT_SERIAL|
                            APPL_ID_FMT_FW|
                            APPL_ID_FMT_REGION|
                            APPL_ID_FMT_HWINFO,
  
  /// Perform formatting using only essential parts, like description and serial
  ///

  /// Use short device description
  APPL_ID_FMT_MAIN_SHORT  = APPL_ID_FMT_DESCR_SHORT|
                            APPL_ID_FMT_SERIAL,
              
  /// Use long device description
  APPL_ID_FMT_MAIN_LONG   = APPL_ID_FMT_DESCR_LONG|
                            APPL_ID_FMT_SERIAL
  
};

#endif //< ES_USE_FWID_FORMATTERS

#endif //< _ese_hw_id_utils_fmt_flags_h_
