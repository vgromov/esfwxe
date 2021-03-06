#ifndef _ese_task_h_
#define _ese_task_h_

class EseTask
{
public:
  /// Task state flags
  enum {
    flagRunning       = 0x00000001,
    flagStopping      = 0x00000002,
    flagStopped       = 0x00000004,
    
    flagsAll          = flagRunning   |
                        flagStopping  |
                        flagStopped
  };

protected:
  /// Task scoped execution tracker helper class
  class Tracker
  {
  public:
    Tracker(EseTask& task) ESE_NOTHROW;
    ~Tracker() ESE_NOTHROW;
    
  protected:
    EseTask& m_task;
    
    ESE_NODEFAULT_CTOR(Tracker);
    ESE_NONCOPYABLE(Tracker);
  };
  friend class Tracker;

protected:
  EseTask() ESE_NOTHROW;

public:
  virtual ~EseTask() ESE_NOTHROW;

  /// Return true if underlying task object was created
  bool isOk() const ESE_NOTHROW { return 0 != m_h; }

  /// Return recently caught exception object caught during task execution
  const EseException& errorGet() const ESE_NOTHROW { return m_ex; }

  /// Static service to return current value of OS kernel tick counter
  static esU32 tickCountGet() ESE_NOTHROW;

  /// Return task priority
  esU32 priorityGet() const ESE_NOTHROW;

  /// Assign priority to the task
  void prioritySet( esU32 prio ) ESE_NOTHROW;

  /// Return priority of the task in which call is made
  static esU32 priorityGetThis() ESE_NOTHROW;

  /// Assign priority to the task in which call is made
  static void prioritySetThis( esU32 prio ) ESE_NOTHROW;

  /// Start execution of the task code. Implicitly create task object, if needed.
  void start(esU32 priority, esU32 stackDepth = 0);
  
  /// Stop execution of the task code, and delete underlying task object
  void stop() ESE_NOTHROW;
  
  /// Wait for task to complete execution
  bool completionWait(uint32_t tmo = rtosMaxDelay) ESE_NOTHROW;

  /// Suspend task execution
  void suspend() ESE_NOTHROW;

  /// Resume task execution
  void resume() ESE_NOTHROW;

  /// Suspend execution of the task, in which call is made
  static void suspendThis() ESE_NOTHROW;

  /// Temporarily stop execution of the task, by @ms milliseconds
  static void sleep(esU32 ms) ESE_NOTHROW;
  
  /// Temporarily stop execution of the task, until @ms milliseconds expire since @prevTicks stamp
  static void sleepUntil(esU32& prevTicks, esU32 ms) ESE_NOTHROW;
  
  /// Pass execution to the other pending tasks
  static void yield() ESE_NOTHROW;

protected:
  // Should be called periodically from task code, to allow graceful task shutdown
  virtual bool checkForStopping(esU32 tmo = 0) ESE_NOTHROW;
  
  // Perform additional optional activity on task stopping
  // NB! This activity is performed in context of the calling task
  virtual void onStopping() ESE_NOTHROW {}
  
  // Perform any additional activity before-after task is suspended, at the context & priority of the 'suspend' caller
  virtual void onBeforeSuspend() ESE_NOTHROW {}
  virtual void onAfterSuspend() ESE_NOTHROW {}
  
  // Perform any additional activity before task is resumed, at the context & priority of the 'resume' caller
  virtual void onBeforeResumeFromIsr() ESE_NOTHROW {}
  virtual void onBeforeResume() ESE_NOTHROW {}
  
  // Task execution handlers
  virtual void onEnter() {}
  virtual void execute() = 0;
  virtual void onExit() {}
  static void worker( void* params ) ESE_NOTHROW;

  // Debug stuff
#ifdef DEBUG
  virtual const char* dbgNameGet() const ESE_NOTHROW = 0;
#endif

protected:
  EseException m_ex;
  volatile void* m_h;
  volatile void* m_nf;

  ESE_NONCOPYABLE(EseTask);
};

#endif // _ese_task_h_
