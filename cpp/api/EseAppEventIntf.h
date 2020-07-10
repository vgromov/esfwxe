#ifndef _ese_app_event_intf_h_
#define _ese_app_event_intf_h_

/// Generic application event interface
///
struct ESE_ABSTRACT EseAppEventIntf
{
  /// Dynamic deallocation
  virtual void destroy() ESE_NOTHROW = 0;

  /// Return event ID
  virtual uint32_t idGet() const ESE_NOTHROW = 0;

  /// Assign new event ID
  virtual void idSet(uint32_t id) ESE_NOTHROW = 0;

  /// Return true if event is empty
  virtual bool isNull() const ESE_NOTHROW = 0;

  /// Return true if event contains dynamic data
  virtual bool haveDynamicData() const ESE_NOTHROW = 0;

  /// Cleanup data which might be contained in event.
  /// Must be called after event handling is done.
  ///
  virtual void cleanup() ESE_NOTHROW = 0;
  
  /// Release ownership of data which might be contained in event.
  virtual void release() ESE_NOTHROW = 0;
  
  /// Access opaque event payload
  virtual void payloadSet(const uint8_t* src, size_t size) ESE_NOTHROW = 0;
  virtual void payloadGet(uint8_t* dest, size_t size) const ESE_NOTHROW = 0;
  
  /// Typed payload access
  template <typename PayloadT>
  void dataSetAs(const PayloadT& src) { payloadSet(reinterpret_cast<const uint8_t*>(&src), sizeof(PayloadT)); }
  
  template <typename PayloadT>
  void dataGetAs(PayloadT& dest) const { payloadGet(reinterpret_cast<uint8_t*>(&dest), sizeof(PayloadT)); }

  template <typename PayloadT>
  PayloadT dataGetAs() const { PayloadT ret; dataGetAs(ret); return ret; }
  
  /// Event implementor class type casting helper. Use with caution, you should know exactly what you're doing
  /// It may be used to raw-cast interface pointer directly to the underlying implementor object pointer
  ///
  template <typename EvtImplT>
  EvtImplT& as() ESE_NOTHROW { return *reinterpret_cast<EvtImplT*>(this); }

  template <typename EvtImplT>
  const EvtImplT& as() const ESE_NOTHROW { return *reinterpret_cast<const EvtImplT*>(this); }
};

#endif //< _ese_app_event_intf_h_
