#ifndef _stat_math_h_
#define _stat_math_h_

#ifdef __cplusplus
  extern "C" {
#endif

/// Statistics flags
///
enum {
  eseValStatInvalidAvg  = 0x00000001, ///< average was not calculated yet
  eseValStatInvalidA    = 0x00000002, ///< ua was not calculated yet
  eseValStatInvalidB    = 0x00000004, ///< ub was not calculated yet
  eseValStatInvalidC    = 0x00000008, ///< uc was not calculated yet
  eseValStatInvalidU    = 0x00000010, ///< U was not calculated yet
  eseValStatInvalid     = eseValStatInvalidAvg| ///< Invlaid statistics. Either calculation is incomplete, or too low value count for proper calculation
                          eseValStatInvalidA|
                          eseValStatInvalidB|
                          eseValStatInvalidC|
                          eseValStatInvalidU,
  eseValStatDoingAvg    = 0x00000020  ///< Temporary avg calculation flag
};

/// Calculated Statistics on specific value
///
typedef struct {
  esF   m_min;    ///< Minimum value over calculation range
  esF   m_max;    ///< Maximum value over calculation range
  esF   m_avg;    ///< Average value over calculation range
  esF   m_ua;     ///< Calculated type A error
  esF   m_ub;     ///< Calculated type B error
  esF   m_uc;     ///< Calculated type C error
  esF   m_U;      
  esU32 m_cnt;
  esU32 m_flags;
  
} EseValStat;

/// Start statistics calculation on specific value
void valStatReset(EseValStat* stat);

/// Append value to minimax-average calculator
void valStatValAppend(EseValStat* stat, esF val);

/// Complete minimax-average calculation
void valStatAvgComplete(EseValStat* stat);

/// Append value to Ua calculator
void valStatUaValAppend(EseValStat* stat, esF val);

/// Complete stat calculation. Device error is either relative (ratio),
/// if errIsRel = TRUE, or absolute, if errIsRel = FALSE
///
void valStatComplete(EseValStat* stat, esF deviceErr, esBL errIsRel);

#ifdef __cplusplus
  }
#endif

#endif // _stat_math_h_
