#ifndef _ese_os_task_intf_impl_h_
#define _ese_os_task_intf_impl_h_

/// EseOsTaskIntf interface implementation
///
class EseOsTaskIntfImpl : public EseOsTaskIntf
{
private:
  class Impl : public EseTask
  {
  public:
    Impl(EseOsTaskIntfImpl& owner) ESE_NOTHROW;
  
  protected:
    virtual bool checkForStopping(uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE;
    virtual void onStopping() ESE_NOTHROW ESE_OVERRIDE;
    virtual void onBeforeSuspend() ESE_NOTHROW ESE_OVERRIDE;
    virtual void onAfterSuspend() ESE_NOTHROW ESE_OVERRIDE;
    virtual void onBeforeResume() ESE_NOTHROW ESE_OVERRIDE;
    virtual void onEnter() ESE_OVERRIDE;
    virtual void execute() ESE_OVERRIDE;
    virtual void onExit() ESE_OVERRIDE;
    
#ifdef DEBUG
    virtual const char* dbgNameGet() const ESE_NOTHROW ESE_OVERRIDE;
#endif
  
  protected:
    EseOsTaskIntfImpl& m_owner;
    
    ESE_NONCOPYABLE(Impl);
    friend class EseOsTaskIntfImpl;
  };
  friend class Impl;

protected:
  EseOsTaskIntfImpl(int id, EseOsTaskIntf::WorkerT worker, EseOsTaskIntf::StopCheckT stopCheck, void* data) ESE_NOTHROW ESE_KEEP;
  
public:
  virtual ~EseOsTaskIntfImpl() ESE_NOTHROW ESE_KEEP;

  /// EseOsTaskIntf implementation
  ///
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int idGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_id; }
  virtual void* dataGet() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_data; }
  virtual uint32_t tickCountGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t priorityGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void prioritySet( uint32_t prio ) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void start(uint32_t priority, uint32_t stackDepth, void* data, WorkerT worker, StopCheckT stopCheck) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void stop() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool completionWait(uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void suspend() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void resume() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void sleep(uint32_t ms) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void sleepUntil(uint32_t& prevTicks, uint32_t ms) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void yield() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool checkForStopping(uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
protected:
  Impl m_impl;
  WorkerT m_worker;
  StopCheckT m_stopCheck;
  void* m_data;
  int m_id;

#ifdef DEBUG
  char m_name[configMAX_TASK_NAME_LEN];
#endif  
  
  ESE_NODEFAULT_CTOR(EseOsTaskIntfImpl);
  ESE_NONCOPYABLE(EseOsTaskIntfImpl);
  
  friend class EseOsIntfImpl;
};

#endif //< _ese_os_task_intf_impl_h_
