#ifndef _ese_math_val_stat_h_
#define _ese_math_val_stat_h_

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
  
} EseMathValStat;

/// Start statistics calculation on specific value
void eseMathValStatReset(EseMathValStat* stat);

/// Append value to minimax-average calculator
void eseMathValStatValAppend(EseMathValStat* stat, esF val);

/// Complete minimax-average calculation
void eseMathValStatAvgComplete(EseMathValStat* stat);

/// Append value to Ua calculator
void eseMathValStatUaValAppend(EseMathValStat* stat, esF val);

/// Complete stat calculation. Device error is either relative (ratio),
/// if errIsRel = TRUE, or absolute, if errIsRel = FALSE
///
void eseMathValStatComplete(EseMathValStat* stat, esF deviceErr, esBL errIsRel);

#ifdef __cplusplus
  } //< extern "C"
#endif //< __cplusplus

#endif // _ese_math_val_stat_h_
