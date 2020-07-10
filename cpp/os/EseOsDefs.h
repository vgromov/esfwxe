#ifndef _ese_os_defs_h_
#define _ese_os_defs_h_

#define rtosMaxDelay ((esU32)0xFFFFFFFF)

/// RTOS operation status codes
///
enum rtosStatus {
  rtosOK,                           ///< No error
  rtosError,                        ///< Generic RTOS error
  rtosErrorTimeout,                 ///< Operation failed due to timeout
  rtosAlreadyLocked,                ///< Resource already locked
  rtosAlreadyUnlocked,              ///< Resource already unlocked
  rtosTaskNotCreated,               ///< Task was not created
  rtosTaskAlreadyRunning,           ///< Task is already created, and is executing
  rtosErrorResource,                ///< Invalid resource handle
  rtosErrorQueueFull,               ///< Queue container is full
  rtosErrorQueueEmpty,              ///< Queue container is empty
  rtosErrorParameterUnknown,        ///< Unknown request parameter
  rtosErrorParameterNotSupported,   ///< Request parameter is not supported
  rtosErrorParameterValueInvalid,   ///< Paramter value is invalid|out of range
  rtosErrorRxOverrun                ///< RX queue encounters overrun condition
};

#endif // _ese_os_defs_h_
