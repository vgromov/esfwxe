// lpc uart driver implementation
// this file is explicitly included from ../uart.c
// do not include it in the project directly
//
#ifndef _inc_uart_c_
#define _inc_uart_c_

#pragma ARM

#define IER_RBR		0x01
#define IER_THRE	0x02
#define IER_RLS		0x04

#define IIR_PEND	0x01
#define IIR_RLS		0x03
#define IIR_RDA		0x02
#define IIR_CTI		0x06
#define IIR_THRE	0x01

#define LSR_RDR		0x01
#define LSR_OE		0x02
#define LSR_PE		0x04
#define LSR_FE		0x08
#define LSR_BI		0x10
#define LSR_THRE	0x20
#define LSR_TEMT	0x40
#define LSR_RXFE	0x80

#define LCR_DLAB	0x80

// UART abstraction macros
//
#define UART_ENABLE_PWR(portNum)			PCONP |= PCUART ## portNum
#define UART_DISABLE_PWR(portNum)			PCONP &= ~PCUART ## portNum

#define UART_ENABLE_INT(portNum) 			U ## portNum ## IER = IER_RBR | IER_THRE | IER_RLS
#define UART_DISABLE_INT(portNum) 		U ## portNum ## IER = 0

#define UART_PUT_BYTE(portNum, Byte) 	U ## portNum ## THR = (Byte)
#define UART_GET_BYTE(portNum, Byte) 	(Byte) = U ## portNum ## RBR

// configure MCU pins for port 0
#define UART_SELECT_PINS0	\
	PINSEL0 = 0x00000050

// configure MCU pins for port 1
#if EA_BOARD_LPC24XX
	#define UART_SELECT_PINS1 PINSEL7 |= 0x0000000F
#else						
	#define UART_SELECT_PINS1 \
		PINSEL0 |= 0x40000000; \
		PINSEL1 |= 0x00000001
#endif

// configure uart0 clock selection
#define UART_SELECT_CLK0	\
	PCLKSEL0 |= (UART0div << 6);

// configure uart1 clock selection
#define UART_SELECT_CLK1	\
	PCLKSEL0 |= (UART1div << 8);

// configure uart2 clock selection
#define UART_SELECT_CLK2	\
	PCLKSEL1 |= (UART2div << 16);

// configure uart3 clock selection
#define UART_SELECT_CLK3	\
	PCLKSEL1 |= (UART3div << 18);

#define UART_CONFIG(portNum, dcb) \
		UART_SELECT_CLK ## portNum; \
		U ## portNum ## LCR = 0; \
		if( dcb->parity != uartNO_PARITY ) \
			U ## portNum ## LCR = 0x08 | (dcb->parity << 4); \
    U ## portNum ## LCR |= LCR_DLAB |	(dcb->stopBits << 2) | dcb->bits; \
    U ## portNum ## DLM = (( UART ## portNum ## clk / 16 ) / dcb->baud) / 256;	\
    U ## portNum ## DLL = (( UART ## portNum ## clk / 16 ) / dcb->baud) % 256; \
		U ## portNum ## LCR &= ~LCR_DLAB; \
    U ## portNum ## FCR = 0x07

#if defined( USE_FREE_RTOS )

	#define UART_INIT(portNum, dcb) \
		if( install_irq( UART ## portNum ##_INT, (void*)&uartISRHandler ## portNum, UART_INTERRUPT_PRIORITY ) == TRUE ) \
    { \
			ports[uart## portNum].dcb = *dcb; \
			ports[uart## portNum].dcr.port = uart## portNum; \
			ports[uart## portNum].dcr.opened = FALSE; \
			ports[uart## portNum].uartTxQueue = xQueueCreate( UART_TX_BUFFLEN, sizeof(BYTE) ); \
			ports[uart## portNum].uartRxQueue = xQueueCreate( UART_RX_BUFFLEN, sizeof(BYTE) );	\
			ports[uart## portNum].mutex = xSemaphoreCreateMutex(); \
  	  return (uartHANDLE)&ports[uart## portNum]; \
		}

#else

	#define UART_INIT(portNum, dcb) \
		if( install_irq( UART ## portNum ##_INT, (void*)&uartISRWorker ## portNum, UART_INTERRUPT_PRIORITY ) == TRUE ) \
    { \
			ports[uart## portNum].dcb = *dcb; \
			ports[uart## portNum].dcr.port = uart## portNum; \
			ports[uart## portNum].dcr.opened = FALSE; \
			rbInit( (rbHANDLE)&ports[uart## portNum].uartTxQueue, (BYTE*)ports[uart## portNum].txData, UART_TX_BUFFLEN	); \
			rbInit( (rbHANDLE)&ports[uart## portNum].uartRxQueue, (BYTE*)ports[uart## portNum].rxData, UART_RX_BUFFLEN ); \
  	  return (uartHANDLE)&ports[uart## portNum]; \
		}

#endif

#if defined( USE_FREE_RTOS )
	// dummy asm to include context switch macros in our driver
	__asm void portmacro(void) 
	{
		INCLUDE c:/FreeRTOS/Source/portable/RealView/ARM7/portmacro.inc
	}

	#define UART_IRQ_INIT(portNum) \
		install_irq( UART ## portNum ##_INT, (void*)&uartISRHandler ## portNum, UART_INTERRUPT_PRIORITY )

#else

	#define UART_IRQ_INIT(portNum) \
		install_irq( UART ## portNum ##_INT, (void*)&uartISRWorker ## portNum, UART_INTERRUPT_PRIORITY )

#endif

#define UART_IRQ_UNINIT(portNum) \
	UART_DISABLE_INT(portNum); \
	uninstall_irq( UART ## portNum ##_INT )

#ifdef USE_UART_PORT0

#if defined( USE_FREE_RTOS )
void uartISRWorker0(void)
#else
static void uartISRWorker0(void) __irq
#endif
{
  BYTE dummy = 0;
	DEF_PORT_STRUCT_VAR(uartPortStruct, &ports[uart0]);
#ifdef USE_FREE_RTOS
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;	
#endif	

//  IENABLE;				// handles nested interrupt	
	switch( (U0IIR & 0x0F) >> 1 )
	{
	case IIR_RLS:		// Receive Line Status
		dummy = U0LSR;
		// Receive Line Status
		if( dummy & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
		  // There are errors or break interrupt
		  // Read LSR will clear the interrupt
		  ps->err = uartLineError;
			ps->lineStatus = dummy;
		}
		break;

  case IIR_RDA:
	case IIR_CTI:	// Character timeout indicator
		// Receive Data Available
		UART_GET_BYTE(0, dummy);  
#if defined( USE_FREE_RTOS )
	  if( xQueueSendFromISR(ps->uartRxQueue, 
				(void*)&dummy, 
				(portBASE_TYPE*)&xHigherPriorityTaskWoken) != pdTRUE)
#else
		if( !rbPushFromIsrB( &ps->uartRxQueue, dummy ) )
#endif
			ps->err = uartRXOverflow;		// buffer overflow
		break;

	case IIR_THRE:	// THRE, transmit holding register empty
		// THRE interrupt
		dummy = U0LSR;		// Check status in the LSR to see if valid data in U1THR or not
		if( dummy & LSR_THRE )
		{
#if defined( USE_FREE_RTOS	)		
			if(	xQueueReceiveFromISR(ps->uartTxQueue, 
				(void*)&dummy, 
				(portBASE_TYPE*)&xHigherPriorityTaskWoken) == pdTRUE )
#else
			if( rbPopFromIsrB(&ps->uartTxQueue, &dummy) )
#endif
			{
				UART_PUT_BYTE(0, dummy);
			}
			else
#if defined( USE_FREE_RTOS )
				xSemaphoreGiveFromISR(ps->uartTxReady, &xHigherPriorityTaskWoken);
#else
				ps->dcr |= uartTxReady;
#endif
		}
		break;
  }

//  IDISABLE;
  VICVectAddr = 0;		// aknowledge interrupt
#ifdef USE_FREE_RTOS
  // we can switch context if necessary
	portEXIT_SWITCHING_ISR( xHigherPriorityTaskWoken );
#endif
}

#ifdef USE_FREE_RTOS
__asm void uartISRHandler0(void)
{
	PRESERVE8
	ARM

	portSAVE_CONTEXT 	

; ---------- call ISR worker
	IMPORT 	uartISRWorker0
	BL			uartISRWorker0

	portRESTORE_CONTEXT
}
#endif // USE_FREE_RTOS
#endif // USE_UART_PORT0

#ifdef USE_UART_PORT1

#ifdef USE_FREE_RTOS
void uartISRWorker1(void)
#else
static void uartISRWorker1(void) __irq
#endif
{
  BYTE dummy = 0;
	DEF_PORT_STRUCT_VAR(uartPortStruct, &ports[uart1]);
#ifdef USE_FREE_RTOS
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;	
#endif	

//  IENABLE;				// handles nested interrupt	
	switch( (U1IIR & 0x0F) >> 1 )
	{
	case IIR_RLS:		// Receive Line Status
		dummy = U1LSR;
		// Receive Line Status
		if( dummy & (LSR_OE|LSR_PE|LSR_FE|LSR_RXFE|LSR_BI) )
		{
		  // There are errors or break interrupt
		  // Read LSR will clear the interrupt
		  ps->err = uartLineError;
			ps->lineStatus = dummy;
		}
		break;

  case IIR_RDA:
	case IIR_CTI:	// Character timeout indicator
		// Receive Data Available
		UART_GET_BYTE(1, dummy);  
#if defined( USE_FREE_RTOS )
	  if( xQueueSendFromISR(ps->uartRxQueue, 
				(void*)&dummy, 
				(portBASE_TYPE*)&xHigherPriorityTaskWoken) != pdTRUE)
#else
		if( !rbPushFromIsrB( &ps->uartRxQueue, dummy ) )
#endif
			ps->err = uartRXOverflow;		// buffer overflow
		break;

	case IIR_THRE:	// THRE, transmit holding register empty
		// THRE interrupt
		dummy = U1LSR;		// Check status in the LSR to see if valid data in U1THR or not
		if( dummy & LSR_THRE )
		{
#if defined( USE_FREE_RTOS	)		
			if(	xQueueReceiveFromISR(ps->uartTxQueue, 
				(void*)&dummy, 
				(portBASE_TYPE*)&xHigherPriorityTaskWoken) == pdTRUE )
#else
			if( rbPopFromIsrB(&ps->uartTxQueue, &dummy) )
#endif
			{
				UART_PUT_BYTE(1, dummy);
			}
			else
#if defined( USE_FREE_RTOS )
				xSemaphoreGiveFromISR(ps->uartTxReady, &xHigherPriorityTaskWoken);
#else
				ps->dcr |= uartTxReady;
#endif
		}
		break;
  }

//  IDISABLE;
  VICVectAddr = 0;		// aknowledge interrupt
#ifdef USE_FREE_RTOS
  // we can switch context if necessary
	portEXIT_SWITCHING_ISR( xHigherPriorityTaskWoken );
#endif
}

#ifdef USE_FREE_RTOS
__asm void uartISRHandler1(void)
{
	PRESERVE8
	ARM

	portSAVE_CONTEXT

; ---------- call ISR worker
	IMPORT uartISRWorker1
	BL 		 uartISRWorker1
	
	portRESTORE_CONTEXT
}
#endif // USE_FREE_RTOS
#endif // USE_UART_PORT1

#endif // _inc_uart_c_
