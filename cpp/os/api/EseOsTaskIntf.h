#ifndef _ese_os_task_intf_h_
#define _ese_os_task_intf_h_

/// EseTaskIntf interface
///
class ESE_ABSTRACT EseOsTaskIntf
{
public:
  enum Stage
  {
    stageEntering,        //< This stage is executed in context of an executing task
    stageWorking,         //< This stage is executed in context of an executing task
    stageExiting,         //< This stage is executed in context of an executing task
    stageStopping,        //< This stage is executed in context of the task called 'stop'
    stageBeforeSuspend,   //< This stage is executed in context of the task called 'suspend'
    stageAfterSuspend,    //< This stage is executed in context of the task called 'suspend'
    stageBeforeResume     //< This stage is executed in context of the task called 'resume'
  };

  /// Staged worker type
  typedef void (*WorkerT)(EseOsTaskIntf& intf, void* data, Stage stage);
  
  /// Optional stopping check type. NB! stop checking is executed in task context
  typedef bool (*StopCheckT)(EseOsTaskIntf& intf, void* data);

public:
  /// Dynamic interface deallocation
  virtual void destroy() ESE_NOTHROW = 0;
  
  /// Return task ID
  virtual int idGet() const ESE_NOTHROW = 0;
  
  /// Return task's data
  virtual void* dataGet() ESE_NOTHROW = 0;
  
  /// Return true if underlying task object was created
  virtual bool isOk() const ESE_NOTHROW = 0;

  /// Return current value of OS kernel tick counter
  virtual uint32_t tickCountGet() const ESE_NOTHROW = 0;

  /// Return task priority
  virtual uint32_t priorityGet() const ESE_NOTHROW = 0;

  /// Assign priority to the task
  virtual void prioritySet( uint32_t prio ) ESE_NOTHROW = 0;

  /// Start execution of the task code
  virtual void start(uint32_t priority, uint32_t stackDepth = 0, void* data = NULL, WorkerT worker = NULL, StopCheckT stopCheck = NULL ) ESE_NOTHROW = 0;
  
  /// Stop execution of the task code, and delete underlying task object
  virtual void stop() ESE_NOTHROW = 0;

  /// Wait for task to complete execution. Return true if wait was a success.
  /// False otherwise. A false condition may occur if task is not executing at all, 
  /// or a wait timeout is expired.
  ///
  virtual bool completionWait(uint32_t tmo = rtosMaxDelay) ESE_NOTHROW = 0;

  /// Suspend task execution
  virtual void suspend() ESE_NOTHROW = 0;

  /// Resume task execution
  virtual void resume() ESE_NOTHROW = 0;

  /// Temporarily stop execution of the task, by @ms milliseconds
  virtual void sleep(uint32_t ms) const ESE_NOTHROW = 0;
  
  /// Temporarily stop execution of the task, until @ms milliseconds expire since @prevTicks stamp
  virtual void sleepUntil(uint32_t& prevTicks, uint32_t ms) const ESE_NOTHROW = 0;
  
  /// Pass execution to the other pending tasks
  virtual void yield() const ESE_NOTHROW = 0;

  // Should be called periodically from task code, to allow graceful task shutdown
  virtual bool checkForStopping(uint32_t tmo = 0) ESE_NOTHROW = 0;
};

#endif //< _ese_os_task_intf_h_
