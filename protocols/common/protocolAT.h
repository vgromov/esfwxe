#ifndef _protcol_at_h_
#define _protcol_at_h_

/// File: protocolAT.h
/// "AT commands" protocol implementation
///

#ifdef __cplusplus
  extern "C" {
#endif

/// AT protocol result codes
///
enum {
  atFail = -1,        ///< Command|response could not be received (may be due to timeout)
  atOk,               ///< Command was properly sent|Response line waas read OK
  atNotEnoughBuff,    ///< Response buffer was overflown
};

/// Send AT command over a channel. Command is terminated with
/// specified command terminator sequence (usually <CR>).
/// If command was sent successfully, atOk is returned, atFail otherwise.
/// If term is NULL, the default command terminator is used (<CR>)
///
int atCommandSend(EsChannelIo* chnl, const char* cmd, esU32 cmdLen,
  const char* term, esU32 termLen);

/// Repeat recently received AT command.
/// Returned is atOk if repeat was sent, atFail otherwise.
///
int atCommandRepeat(EsChannelIo* chnl);

/// Read AT command response line with timeout in ms.
/// 'out' [out] is buffer to receive response, 'outLen' [in, out] - upon call, it
/// specifies the length of the out buffer, upon return, it is set the amount of
/// chars actually received. 'sep' is the line separator sequence used in response
/// (usually <CR><LF>), 'sepLen' is its length.
/// Returned is atOk if response line received, atFail, if response was not
/// properly received in tmo time span, or atNotEnoughBuff,
/// if incoming data were longer, than supplied out buffer.
///
int atResponseLineRead(EsChannelIo* chnl, char* out, esU32* outLen,
  const char* sep, esU32 sepLen, esU32 tmo);

#ifdef __cplusplus
  }
#endif

#endif // _protcol_at_h_
