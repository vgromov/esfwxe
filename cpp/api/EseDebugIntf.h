#ifndef _ese_debug_intf_h_
#define _ese_debug_intf_h_

/// Tracing-Debugging interface abstraction
///
class ESE_ABSTRACT EseDebugIntf
{
public:
  /// Assert service
  virtual void assertTrue(bool cond, const char* file, int line) const = 0;

  /// Debug trace service, AKA debug printf
  virtual void trace(const char* fmt, ...) const ESE_NOTHROW = 0;
  
  /// Debug pin trace services
  virtual size_t pinTraceCntGet() const ESE_NOTHROW = 0;
  virtual void pinTraceSet(size_t pin, bool set) const ESE_NOTHROW = 0;
  virtual void pinTraceToggle(size_t pin) const ESE_NOTHROW = 0;
};

#endif //< _ese_debug_intf_h_
