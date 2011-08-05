/*----------------------------------------------------------------------------
 *      R T L  -  K e r n e l
 *----------------------------------------------------------------------------
 *      Name:    RTX_CONFIG.C
 *      Purpose: Configuration of RTX Kernel for Philips LPC23xx
 *      Rev.:    V3.20
 *----------------------------------------------------------------------------
 *      This code is part of the RealView Run-Time Library.
 *      Copyright (c) 2004-2008 KEIL - An ARM Company. All rights reserved.
 *---------------------------------------------------------------------------*/

#include <targetConfig.h>
#include <rtxRTOSConfig.h>          /* RTX user configuration header    */
#include <RTX_Config.h>							// rtx types and os defines
#include <rtl.h>

/*----------------------------------------------------------------------------
 *      RTX User configuration part BEGIN
 *---------------------------------------------------------------------------*/

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h>Task Definitions
// ===================
//
//   <o>Number of concurrent running tasks <0-250>
//   <i> Define max. number of tasks that will run at the same time.
//   <i> Default: 6
#ifndef OS_TASKCNT
 #define OS_TASKCNT     6
#endif

//   <o>Number of tasks with user-provided stack <0-250>
//   <i> Define the number of tasks that will use a bigger stack.
//   <i> The memory space for the stack is provided by the user.
//   <i> Default: 0
#ifndef OS_PRIVCNT
 #define OS_PRIVCNT     0
#endif

//   <o>Task stack size [bytes] <20-4096:8><#/4>
//   <i> Set the stack size for tasks which is assigned by the system.
//   <i> Default: 200
#ifndef OS_STKSIZE
 #define OS_STKSIZE     50
#endif

// <q>Check for the stack overflow
// ===============================
// <i> Include the stack checking code for a stack overflow.
// <i> Note that additional code reduces the Kernel performance.
#ifndef OS_STKCHECK
 #define OS_STKCHECK    1
#endif

//   <o>Number of user timers <0-250>
//   <i> Define max. number of user timers that will run at the same time.
//   <i> Default: 0  (User timers disabled)
#ifndef OS_TIMERCNT
 #define OS_TIMERCNT    0
#endif

// </h>
// <h>System Timer Configuration
// =============================
//   <o>RTX Kernel timer number <0=> Timer 0 <1=> Timer 1
//                              <2=> Timer 2 <3=> Timer 3
//   <i> Define the ARM timer used as a system tick timer.
//   <i> Default: Timer 0
#ifndef OS_TIMER
 #define OS_TIMER       0
#endif

//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Set the timer clock value for selected timer.
//   <i> Default: 15000000  (15MHz at 60MHz CCLK and VPBDIV = 4)
#ifndef OS_CLOCK
 #define OS_CLOCK       12000000
#endif

//   <o>Timer tick value [us] <1-1000000>
//   <i> Set the timer tick value for selected timer.
//   <i> Default: 10000  (10ms)
#ifndef OS_TICK
 #define OS_TICK        10000
#endif

// </h>
// <e>Round-Robin Task switching
// =============================
// <i> Enable Round-Robin Task switching.
#ifndef OS_ROBIN
 #define OS_ROBIN       1
#endif

//   <o>Round-Robin Timeout [ticks] <1-1000>
//   <i> Define how long a task will execute before a task switch.
//   <i> Default: 5
#ifndef OS_ROBINTOUT
 #define OS_ROBINTOUT   5
#endif

// </e>

//------------- <<< end of configuration section >>> -----------------------

/*----------------------------------------------------------------------------
 *      RTX User configuration part END
 *---------------------------------------------------------------------------*/

#if   (OS_TIMER == 0)                                   /* Timer 0          */
  #define OS_TID_       4                               /*  Timer ID        */
  #define PCON_VAL      1
  #define TIMx(reg)     T0##reg
  #define VICVectAddrX  VICVectAddr4
#elif (OS_TIMER == 1)                                   /* Timer 1          */
  #define OS_TID_       5                               /*  Timer ID        */
  #define PCON_VAL      2
  #define TIMx(reg)     T1##reg
  #define VICVectAddrX  VICVectAddr5
#elif (OS_TIMER == 2)                                   /* Timer 2          */
  #define OS_TID_       26                              /*  Timer ID        */
  #define PCON_VAL      22
  #define TIMx(reg)     T2##reg
  #define VICVectAddrX  VICVectAddr26
#elif (OS_TIMER == 3)                                   /* Timer 3          */
  #define OS_TID_       27                              /*  Timer ID        */
  #define PCON_VAL      23
  #define TIMx(reg)     T3##reg
  #define VICVectAddrX  VICVectAddr27
#else
  #error OS_TIMER invalid
#endif

#define OS_TIM_         (1 << OS_TID_)                  /*  Interrupt Mask  */
#define OS_TRV          ((U32)(((double)OS_CLOCK*(double)OS_TICK)/1E6)-1)
#define OS_TVAL         TIMx(TC)                        /*  Timer Value     */
#define OS_TOVF         (TIMx(IR) & 1)                  /*  Overflow Flag   */
#define OS_TREL()       ;                               /*  Timer Reload    */
#define OS_TFIRQ()      VICSoftInt   |= OS_TIM_;        /*  Force Interrupt */
#define OS_TIACK()      TIMx(IR) = 1;                   /*  Interrupt Ack   */ \
                        VICSoftIntClr = OS_TIM_;                               \
                        VICVectAddr   = 0;
#define OS_TINIT()      PCONP |= (1<<PCON_VAL);         /*  Initialization  */ \
                        TIMx(MR0) = OS_TRV;                                    \
                        TIMx(MCR) = 3;                                         \
                        TIMx(TCR) = 1;                                         \
                        VICVectAddrX = (U32)os_clock_interrupt;

#define OS_IACK()       VICVectAddr   = 0;              /* Interrupt Ack    */

#define OS_LOCK()       VICIntEnClr   = OS_TIM_;        /* Task Lock        */
#define OS_UNLOCK()     VICIntEnable |= OS_TIM_;        /* Task Unlock      */

/* WARNING ! Do not use IDLE mode if you are using a JTAG interface  */
/*           for debugging your application.                         */
#define _idle_()        PCON |= 1;
#define INITIAL_CPSR    0x40000010
#define MAGIC_WORD      0xE25A2EA5

/*----------------------------------------------------------------------------
 *      Global Variables
 *---------------------------------------------------------------------------*/

extern P_TCB os_runtask;
extern struct OS_XCB os_rdy;
extern struct OS_TCB os_clock_TCB;
extern U16 os_time;
#pragma push
#pragma O0
U16 const os_maxtaskrun = OS_TASKCNT;
/* Export following defines to uVision debugger. */
U32 const os_stackinfo = (OS_STKCHECK<<24) | (OS_PRIVCNT<<16) | (OS_STKSIZE*4);
U32 const os_clockrate = OS_TICK;
U32 const os_timernum  = (OS_TIMER << 16) | OS_TIMERCNT;
U32 const os_rrobin    = (OS_ROBIN << 16) | OS_ROBINTOUT;
#pragma pop

/*----------------------------------------------------------------------------
 *      Local Variables
 *---------------------------------------------------------------------------*/
/* Memory pool for TCB allocation    */
_declare_box (static m_tcb, sizeof(struct OS_TCB), OS_TASKCNT);

/* Memory pool for System stack allocation. Need to allocate 2 additional  */
/* entries for 'os_clock_demon()' and 'os_idle_demon()'.                   */
_declare_box8 (static m_stk, OS_STKSIZE*4, OS_TASKCNT-OS_PRIVCNT+2);

/* An array of Active task pointers.                                       */
P_TCB os_active_TCB[OS_TASKCNT];

#if (OS_ROBIN == 1)
  static U16  os_robin_time;
  static P_TCB os_tsk_robin;
#endif

#if (OS_TIMERCNT != 0)
  /* Memory pool for User Timer allocation                                 */
  _declare_box (static m_tmr, sizeof(struct OS_TMR), OS_TIMERCNT);
#endif

#if (OS_STKCHECK == 1)
  static BIT os_del_flag;
#endif

/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

extern void os_clock_interrupt (void);

/*--------------------------- os_idle_demon ---------------------------------*/

void os_idle_demon (void) __task 
{
	/* The idle demon is a system task. It is running when no other task is   */
	/* ready to run (idle situation). It must not terminate. Therefore it     */
	/* should contain at least an endless loop.                               */
	
	for(;;) 
	{
		// enter low-power mode in idle mode
#ifndef JTAG_DEBUG
		_idle_(); 	
#endif
	}
} /* end of os_idle_demon */


/*--------------------------- os_tmr_call -----------------------------------*/

void os_tmr_call (U16 info) {
   /* This function is called when the user timer has expired.               */
   /* Parameter "info" is the parameter defined when the timer was created.  */
   /* HERE: include here optional user code to be executed on timeout.       */
   info = info;
} /* end of os_tmr_call */

/*--------------------------- os_stk_overflow -------------------------------*/

#if (OS_STKCHECK == 1)
static void os_stk_overflow (void) {
   /* This function is called when a stack overflow is detected.             */
   /*  'os_runtask' points to a TCB of a task which has a stack overflow     */
   /*  'task_id'    holds a task id for this task                            */
   /* HERE: include here optional code to be executed on stack overflow.     */
   static volatile OS_TID task_id;

   /* Get a task identification for a task with stack problem to 'task_id'.*/
   task_id = os_get_TID (os_runtask);
   /* Use a uVision 'RTX Kernel' debug dialog page 'Active Tasks' to      */
   /* check which task has got a stack overflow and needs a bigger stack. */
   for (;;);
}
#endif

/*--------------------------- os_clock_interrupt0 ---------------------------*/

U32 os_clock_interrupt0 (U32 stack) {
   /* Do task switch to clock demon: entered upon a clock interrupt. Saving */
   /* and restoring context is written in assembly (module: Irq_RTX.s)      */
   OS_TIACK();
   tsk_lock ();
   os_runtask->tsk_stack = stack;
   os_runtask->full_ctx = __TRUE;
   os_runtask->state  = READY;
   os_put_rdy_first (os_runtask);
   os_runtask = &os_clock_TCB;
   os_clock_TCB.state = RUNNING;
   return (os_runtask->tsk_stack);
} /* end of os_clock_interrupt0 */

/*--------------------------- os_def_interrupt ------------------------------*/

static void os_def_interrupt (void) __irq  {
   /* Default Interrupt Function: may be called when timer ISR is disabled */
   OS_IACK();
}

/*--------------------------- os_tmr_init -----------------------------------*/

void os_tmr_init (void) {
   /* Initialize hardware timer as system tick timer. This function is     */
   /* called at the system startup.                                        */
   OS_TINIT();
#if (OS_ROBIN == 1)
   os_robin_time = OS_ROBINTOUT;
#endif
} /* end of os_tmr_init */

/*--------------------------- os_tmr_reload ---------------------------------*/

void os_tmr_reload (void) {
   /* Reload system timer for next period if a timer requires reload.        */
   OS_TREL();
} /* end of os_tmr_reload */

/*--------------------------- os_tmr_force_irq ------------------------------*/

void os_tmr_force_irq (void) {
   /* Force a timer interrupt.                                               */
   OS_TFIRQ();
} /* end of os_tmr_force_irq */

/*--------------------------- os_tmr_inspect_cnt ----------------------------*/

U32 os_tmr_inspect_cnt (void) {
   /* Inspect current value of rtx timer.                                    */
   return (OS_TVAL);
} /* end of os_tmr_inspect_cnt */

/*--------------------------- os_tmr_inspect_ovf ----------------------------*/

BOOL os_tmr_inspect_ovf (void) {
   /* Inspect current state of timer overflow flag.                          */
   return (OS_TOVF);
} /* end of os_tmr_inspect_ovf */

/*--------------------------- tsk_lock --------------------------------------*/

void tsk_lock (void) {
   /* Lock out tasks: prevents task switching by locking out scheduler       */
   /* activation on interrupt.                                            .  */
   OS_LOCK();
} /* end of tsk_lock */

/*--------------------------- tsk_unlock ------------------------------------*/

void tsk_unlock (void) {
   /* Enable AR System Tick Timer Interrupts.                                */
   OS_UNLOCK();
} /* end of tsk_unlock */

/*--------------------------- os_init_mem -----------------------------------*/

void os_init_mem (void) {
   U32 i;

   for (i = 0; i < OS_TASKCNT; i++) {
      os_active_TCB[i] = NULL;
   }
   _init_box (&m_tcb, sizeof(m_tcb), sizeof(struct OS_TCB));
   _init_box8 (&m_stk, sizeof(m_stk), OS_STKSIZE*4);
#if (OS_TIMERCNT != 0)
   _init_box (&m_tmr, sizeof(m_tmr), sizeof(struct OS_TMR));
#endif
} /* end of os_init_mem */

/*--------------------------- os_alloc_TCB ----------------------------------*/

P_TCB os_alloc_TCB () {
   return (_alloc_box (m_tcb));
} /* end of os_alloc_TCB */

/*--------------------------- os_free_TCB -----------------------------------*/

void os_free_TCB (P_TCB p_TCB) {
   /* Free allocated memory resources for the task "p_TCB" */
   _free_box (m_stk, p_TCB->stack);
   _free_box (m_tcb, p_TCB);
#if (OS_STKCHECK == 1)
   if (os_runtask == p_TCB) {
      /* os_tsk_delete_self() called. */
      os_del_flag = __TRUE;
   }
#endif
} /* end of os_free_TCB */

/*--------------------------- os_alloc_TMR ----------------------------------*/

P_TMR os_alloc_TMR () {
#if (OS_TIMERCNT != 0)
   return (_alloc_box (m_tmr));
#else
   return (NULL);
#endif
} /* end of os_alloc_TMR */

/*--------------------------- os_free_TMR -----------------------------------*/

void os_free_TMR (P_TMR timer) {
   /* Free allocated memory resources for user timer 'timer' */
#if (OS_TIMERCNT != 0)
   _free_box (m_tmr, timer);
#else
   timer = timer;
#endif
} /* end of os_free_TMR */

/*--------------------------- os_init_context -------------------------------*/

void os_init_context (P_TCB p_TCB, U8 priority,
                      FUNCP task_body, U8 full_context) {
   /* Prepare TCB and saved context for a first time start of a task         */
   /* "p_TCB" points to TCB to be initialised. "priority" indicates desired  */
   /* execution priority. "task_body" is the start address of the task.      */
   /* "full_context" identifies context type.                                */
   U32 *stk,i;

   /* Initialize general part of TCB */
   p_TCB->cb_type = TCB;
   p_TCB->state   = READY;
   p_TCB->prio    = priority;
   p_TCB->p_lnk   = NULL;
   p_TCB->p_rlnk  = NULL;
   p_TCB->p_dlnk  = NULL;
   p_TCB->p_blnk  = NULL;
   p_TCB->delta_time    = 0;
   p_TCB->interval_time = 0;
   p_TCB->events  = 0;
   p_TCB->waits   = 0;

   /* Initialize ARM specific part of TCB */
   p_TCB->full_ctx = full_context;

   /* Prepare a complete interrupt frame for first task start */
   if (p_TCB->priv_stack != 0) {
      /* User has provided a memory space for the stack. */
      stk = &p_TCB->stack[p_TCB->priv_stack>>2];
   }
   else {
      /* Allocate the memory space for the stack. */
      p_TCB->stack = _alloc_box (m_stk);
      /* Write to the top of stack. */
      stk = &p_TCB->stack[OS_STKSIZE];
   }

   /* Initial PC and default CPSR */
   *--stk = (U32)task_body;
   i      = INITIAL_CPSR;

   /* If a task in THUMB mode, set T-bit. */
   if ((U32)task_body & 1) {
      i |= 0x00000020;
   }
   *--stk = i;

   /* Write initial registers. */
   for (i = full_context ? 13 : 8; i; i--) {
      *--stk = 0;
   }

   /* For "full_context" assign a void pointer to R0. */
   if (full_context) {
      *--stk = (U32)p_TCB->p_msg;
   }

   /* Initial Task stack pointer. */
   p_TCB->tsk_stack = (U32)stk;

   /* Task entry point. */
   p_TCB->ptask = task_body;
#if (OS_STKCHECK == 1)
   /* Set a magic word for checking of stack overflow. */
   p_TCB->stack[0] = MAGIC_WORD;
#endif
} /* end of os_init_context */


/*--------------------------- os_switch_tasks -------------------------------*/

void __swi(0) os_switch_tasks (P_TCB p_new);
void __SWI_0                  (P_TCB p_new) {
   /* Switch to next task (identified by "p_new"). Saving old and restoring */
   /* new context is written in assembly (module: Swi_RTX.s)                */

#if (OS_STKCHECK == 1)
   if (tstclrb (&os_del_flag) == __FALSE) {
      /* Do not check if task has deleted itself. */
      if ((os_runtask->tsk_stack < (U32)os_runtask->stack) || 
          (os_runtask->stack[0] != MAGIC_WORD )) {
         os_stk_overflow ();
      }
   }
#endif
   os_runtask->full_ctx = __FALSE;
   os_runtask = p_new;
   p_new->state = RUNNING;
#if (OS_ROBIN == 1)
   if (p_new->full_ctx == __TRUE) {
      os_tsk_robin = p_new;
   }
#endif
   /* Tsk_Unlock */
   OS_UNLOCK();
} /* end of os_switch_tasks */


/*--------------------------- os_chk_robin ----------------------------------*/

void os_chk_robin (void) {
   /* Check if Round Robin timeout expired and switch to the next ready task.*/
   /* This function is called from the "os_clock_demon()" task scheduler.    */
#if (OS_ROBIN == 1)
   P_TCB p_new;

   if (os_rdy.p_lnk != os_tsk_robin) {
      os_robin_time = os_time + OS_ROBINTOUT;
      return;
      }
   if (os_robin_time == os_time) {
      /* Round Robin timeout has expired. */
      os_robin_time += OS_ROBINTOUT;
      p_new = os_get_first (&os_rdy);
      os_put_prio ((P_XCB)&os_rdy, p_new);
      }
#endif
} /* end of os_chk_robin */

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/

