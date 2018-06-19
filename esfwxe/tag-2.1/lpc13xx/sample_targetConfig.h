#ifndef _target_config_h_
#define _target_config_h_

//#define _DEBUG

// target configuration file for lpc13xx project
//

#if defined(_DEBUG) || defined(DEBUG)
#	ifndef DEBUG
#		define DEBUG
#	endif
#	ifndef _DEBUG
#		define _DEBUG
#	endif
#endif 

#if defined(_DEBUG) || defined(DEBUG)
// use jtag debug (no WDT)
// remove in production code
#	define JTAG_DEBUG
#	undef  NDEBUG
// use trace facility
#	define USE_TRACE
#elif !defined(DEBUG) && !defined(_DEBUG)
#	ifndef NDEBUG
#		define NDEBUG
#	endif
// define USE_CODE_READ_PROTECTION to produce final code not-readable from flash
#	define USE_CODE_READ_PROTECTION
#endif

// frequency configuration data
//
// external Xtal frequency
#define Fosc						12000000UL	


/*
//-------- <<< Use Configuration Wizard in Context Menu >>> ------------------
*/

/*--------------------- Clock Configuration ----------------------------------
//
// <e> Clock Configuration
//   <h> System Oscillator Control Register (SYSOSCCTRL)
//     <o1.0>      BYPASS: System Oscillator Bypass Enable
//                     <i> If enabled then PLL input (sys_osc_clk) is fed
//                     <i> directly from XTALIN and XTALOUT pins.
//     <o1.9>      FREQRANGE: System Oscillator Frequency Range
//                     <i> Determines frequency range for Low-power oscillator.
//                   <0=> 1 - 20 MHz
//                   <1=> 15 - 25 MHz
//   </h>
//
//   <h> Watchdog Oscillator Control Register (WDTOSCCTRL)
//     <o2.0..4>   DIVSEL: Select Divider for Fclkana
//                     <i> wdt_osc_clk = Fclkana/ (2 Ч (1 + DIVSEL))
//                   <0-31>
//     <o2.5..8>   FREQSEL: Select Watchdog Oscillator Analog Output Frequency (Fclkana)
//                   <0=> Undefined
//                   <1=> 0.5 MHz
//                   <2=> 0.8 MHz
//                   <3=> 1.1 MHz
//                   <4=> 1.4 MHz
//                   <5=> 1.6 MHz
//                   <6=> 1.8 MHz
//                   <7=> 2.0 MHz
//                   <8=> 2.2 MHz
//                   <9=> 2.4 MHz
//                   <10=> 2.6 MHz
//                   <11=> 2.7 MHz
//                   <12=> 2.9 MHz
//                   <13=> 3.1 MHz
//                   <14=> 3.2 MHz
//                   <15=> 3.4 MHz
//   </h>
//
//   <h> System PLL Control Register (SYSPLLCTRL)
//                   <i> F_clkout = M * F_clkin = F_CCO / (2 * P)
//                   <i> F_clkin must be in the range of  10 MHz to  25 MHz
//                   <i> F_CCO   must be in the range of 156 MHz to 320 MHz
//     <o3.0..4>   MSEL: Feedback Divider Selection
//                     <i> M = MSEL + 1
//                   <0-31>
//     <o3.5..6>   PSEL: Post Divider Selection
//                   <0=> P = 1
//                   <1=> P = 2
//                   <2=> P = 4
//                   <3=> P = 8
//   </h>
//
//   <h> System PLL Clock Source Select Register (SYSPLLCLKSEL)
//     <o4.0..1>   SEL: System PLL Clock Source
//                   <0=> IRC Oscillator
//                   <1=> System Oscillator
//                   <2=> Reserved
//                   <3=> Reserved
//   </h>
//
//   <h> Main Clock Source Select Register (MAINCLKSEL)
//     <o5.0..1>   SEL: Clock Source for Main Clock
//                   <0=> IRC Oscillator
//                   <1=> Input Clock to System PLL
//                   <2=> WDT Oscillator
//                   <3=> System PLL Clock Out
//   </h>
//
//   <h> System AHB Clock Divider Register (SYSAHBCLKDIV)
//     <o6.0..7>   DIV: System AHB Clock Divider
//                     <i> Divides main clock to provide system clock to core, memories, and peripherals.
//                     <i> 0 = is disabled
//                   <0-255>
//   </h>
//
//   <h> USB PLL Control Register (USBPLLCTRL)
//                   <i> F_clkout = M * F_clkin = F_CCO / (2 * P)
//                   <i> F_clkin must be in the range of  10 MHz to  25 MHz
//                   <i> F_CCO   must be in the range of 156 MHz to 320 MHz
//     <o7.0..4>   MSEL: Feedback Divider Selection
//                     <i> M = MSEL + 1
//                   <0-31>
//     <o7.5..6>   PSEL: Post Divider Selection
//                   <0=> P = 1
//                   <1=> P = 2
//                   <2=> P = 4
//                   <3=> P = 8
//   </h>
//
//   <h> USB PLL Clock Source Select Register (USBPLLCLKSEL)
//     <o8.0..1>   SEL: USB PLL Clock Source
//                     <i> USB PLL clock source must be switched to System Oscillator for correct USB operation
//                   <0=> IRC Oscillator
//                   <1=> System Oscillator
//                   <2=> Reserved
//                   <3=> Reserved
//   </h>
//
//   <h> USB Clock Source Select Register (USBCLKSEL)
//     <o9.0..1>   SEL: System PLL Clock Source
//                   <0=> USB PLL out
//                   <1=> Main clock
//                   <2=> Reserved
//                   <3=> Reserved
//   </h>
//
//   <h> USB Clock Divider Register (USBCLKDIV)
//     <o10.0..7>  DIV: USB Clock Divider
//                     <i> Divides USB clock to 48 MHz.
//                     <i> 0 = is disabled
//                   <0-255>
//   </h>
// </e>
*/
#define CLOCK_SETUP           1
#define SYSOSCCTRL_Val        0x00000000              // Reset: 0x000
#define WDTOSCCTRL_Val        0x00000000              // Reset: 0x000
#define SYSPLLCTRL_Val        0x00000025              // Reset: 0x000
#define SYSPLLCLKSEL_Val      0x00000001              // Reset: 0x000
#define MAINCLKSEL_Val        0x00000003              // Reset: 0x000
#define SYSAHBCLKDIV_Val      0x00000001              // Reset: 0x001
#define USBPLLCTRL_Val        0x00000023              // Reset: 0x000
#define USBPLLCLKSEL_Val      0x00000001              // Reset: 0x000
#define USBCLKSEL_Val         0x00000000              // Reset: 0x000
#define USBCLKDIV_Val         0x00000001              // Reset: 0x001

/*--------------------- USB Configuration ----------------------------------
// <h> USB Configuration
//   <o0> USB Power
//        <i> Default Power Setting
//        <0=> Bus-powered
//        <1=> Self-powered
//   <o1> Max Number of Interfaces <1-256>
//   <o2> Max Number of Endpoints  <1-10>
//   <o3> Max Endpoint 0 Packet Size
//        <64=> 64 Bytes
// </h>
*/

#define USB_POWER           0
#define USB_IF_NUM          1
#define USB_EP_NUM          10
#define USB_MAX_PACKET0     64


/*
// <h> USB Event Handlers
//   <h> Device Events
//     <o0.0> Power Event
//     <o1.0> Reset Event
//     <o2.0> Suspend Event
//     <o3.0> Resume Event
//     <o4.0> Remote Wakeup Event
//     <o5.0> Start of Frame Event
//     <o6.0> Error Event
//   </h>
//   <h> Endpoint Events
//     <o7.0>  Endpoint 0 Event
//     <o7.1>  Endpoint 1 Event
//     <o7.2>  Endpoint 2 Event
//     <o7.3>  Endpoint 3 Event
//     <o7.4>  Endpoint 4 Event
//   </h>
//   <h> USB Core Events
//     <o8.0>  Set Configuration Event
//     <o9.0>  Set Interface Event
//     <o10.0> Set/Clear Feature Event
//   </h>
// </h>
*/

#define USB_POWER_EVENT     0
#define USB_RESET_EVENT     1
#define USB_SUSPEND_EVENT   0
#define USB_RESUME_EVENT    0
#define USB_WAKEUP_EVENT    0
#define USB_SOF_EVENT       0
#define USB_ERROR_EVENT     0
#define USB_EP_EVENT        0x0003
#define USB_CONFIGURE_EVENT 1
#define USB_INTERFACE_EVENT 0
#define USB_FEATURE_EVENT   0


/*
// <e0> USB Class Support
//   <i> enables USB Class specific Requests
//   <e1> Human Interface Device (HID)
//     <o2> Interface Number <0-255>
//   </e>
//   <e3> Mass Storage
//     <o4> Interface Number <0-255>
//   </e>
//   <e5> Audio Device
//     <o6> Control Interface Number <0-255>
//     <o7> Streaming Interface 1 Number <0-255>
//     <o8> Streaming Interface 2 Number <0-255>
//   </e>
//   <e9> Communication Device
//     <o10> Control Interface Number <0-255>
//     <o11> Bulk Interface Number <0-255>
//     <o12> Max Communication Device Buffer Size
//        <64=> 64 Bytes 
//   </e>
// </e>
*/

#define USB_CLASS           1
#define USB_HID             1
#define USB_HID_IF_NUM      0
#define USB_MSC             0
#define USB_MSC_IF_NUM      0
#define USB_AUDIO           0
#define USB_ADC_CIF_NUM     0
#define USB_ADC_SIF1_NUM    1
#define USB_ADC_SIF2_NUM    2
#define USB_CDC  			0
#define USB_CDC_CIF_NUM     0
#define USB_CDC_DIF_NUM     1
#define USB_CDC_BUFSIZE     64

/*
// <e0> USB Vendor Support
//   <i> enables USB Vendor specific Requests
// </e>
*/
#define USB_VENDOR          0

/*
// <h> LPC13xx Specific Configuration
//   <e0>USB Device FIQ Select
//     <i>Setting all bits to ‘1’ at the same time is not allowed.
//     <o1.0>FRAME: Route FRAME interrupt to HighPriority interrupt line
//     <o1.1>BULKOUT: Route Bulk OUT EP3 IRQ to HighPriority interrupt line
//     <o1.2>BULKIN: Route Bukk IN EP3 IRQ to HighPriority interrupt line
//   </e>
// </h>
*/
#define USB_LPC13XX_FIQ       0
#define USB_LPC13XX_FIQ_VAL   0

/*
//-------- <<< end of configuration section >>> ------------------------------
*/

#define Fmcu	SystemCoreClock
#define Fpclk SystemCoreClock

// project-specific defines
//
// use communication channel abstraction
//#define USE_COMMINTF

// use custom implementations of 
// void usDelay(esU32 useconds) and
// void nsDelay(esU32 nseconds)
//
//#define USE_CUSTOM_DELAY

#endif  // _target_config_h_
