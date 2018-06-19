// lpc11xx uart driver implementation
// this file is explicitly included from ../uart.c
// do not include it in the project directly
//
#ifndef _inc_uart_cc_
#define _inc_uart_cc_

#ifndef UART_CLKDIV
# define UART_CLKDIV 1
#endif

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
#define LCR_BREAK	0x40

// baud rate calculation table [frx1000, divaddval, mulval]
typedef struct {
	esU16 fr;
	esU8	divaddval;
	esU8 mulval;

} uartBaudTblEntry;
static const uartBaudTblEntry sc_baudCalcTbl[] = {
{1000, 0, 1},
{1067, 1, 15}, 	
{1071, 1, 14}, 	
{1077, 1, 13}, 	
{1083, 1, 12}, 	
{1091, 1, 11}, 	
{1100, 1, 10}, 	
{1111, 1, 9}, 	
{1125, 1,	8},		
{1133, 2, 15},	
{1143, 1,	7},		
{1154, 2, 13}, 	
{1167, 1, 6}, 	
{1182, 2, 11},	
{1200, 1, 5}, 	
{1214, 3, 14}, 	
{1222, 2, 9}, 	
{1231, 3, 13},
{1250, 1, 4}, 
{1267, 4, 15},
{1273, 3, 11},
{1286, 2, 7}, 
{1300, 3, 10},
{1308, 4, 13},
{1333, 1, 3}, 
{1357, 5, 14},
{1364, 4, 11},
{1375, 3, 8},	
{1385, 5, 13},
{1400, 2, 5},	
{1417, 5, 12},
{1429, 3, 7},	
{1444, 4, 9},	
{1455, 5, 11},
{1462, 6, 13},
{1467, 7, 15},
{1500, 1, 2}, 	
{1533, 8, 15}, 
{1538, 7, 13}, 
{1545, 6, 11}, 
{1556, 5, 9}, 	
{1571, 4, 7}, 	
{1583, 7, 12}, 
{1600, 3, 5},		
{1615, 8, 13},	
{1625, 5,	8},		
{1636, 7, 11},	
{1643, 9, 14},	
{1667, 2, 3},		
{1692, 9, 13},	
{1700, 7, 10},	
{1714, 5, 7}, 	
{1727, 8, 11}, 
{1733, 11, 15},
{1750, 3, 4},
{1769, 10, 13},
{1778, 7, 9},
{1786, 11, 14},
{1800, 4, 5},
{1818, 9, 11},
{1833, 5, 6},
{1846, 11, 13},
{1857, 6, 7},
{1867, 13, 15},
{1875, 7, 8},
{1889, 8, 9},
{1900, 9, 10},
{1909, 10, 11},
{1917, 11, 12},
{1923, 12, 13},
{1929, 13, 14},
{1933, 14, 15}
};

// UART baud rate configuration algorithm implementation taken from lpc11xx datasheet
#define baudCalcAccuracyMul ((esU32)1000)
#define accurateMul(clk, baud, div) \
	(((esU64)(clk))*((esU64)baudCalcAccuracyMul)) / (((esU64)(baud))*((esU64)(div))*((esU64)16))

static void uartBaudConfig(esU32 baud)
{
	esU32 uartClk = LPC_SYSCON->UARTCLKDIV ? Fpclk	/ LPC_SYSCON->UARTCLKDIV : Fpclk;
	esU32 tblIdx = 0;
	esU32 FRest = sc_baudCalcTbl[tblIdx].fr;
	esU64 DLest = accurateMul(uartClk*baudCalcAccuracyMul, baud, FRest);
	
	LPC_UART->LCR |= LCR_DLAB;

	// is DLest an integer ?
	if( DLest % baudCalcAccuracyMul )
	{
		++tblIdx;

		do
		{
			FRest = sc_baudCalcTbl[tblIdx].fr;
			DLest = accurateMul(uartClk, baud, FRest);
			FRest = accurateMul(uartClk, baud, DLest);
      
			if( 1067 <= FRest && 1933 >= FRest )
			{
				esU32 idx = 0;
				// lookup entry in baud calc table
				while(idx < CONST_ARRAY_COUNT(sc_baudCalcTbl) &&
							sc_baudCalcTbl[idx].fr < FRest )
				{
					++idx;
				}
				
				if( idx >= CONST_ARRAY_COUNT(sc_baudCalcTbl) )
					tblIdx = idx-1;
				else
					tblIdx = idx;

				break;
			}
		
		}	while( ++tblIdx < CONST_ARRAY_COUNT(sc_baudCalcTbl) );
	}
  else
    DLest /= baudCalcAccuracyMul;
  
	LPC_UART->DLM = (DLest >> 8);
 	LPC_UART->DLL = DLest & 0xFF;
	LPC_UART->FDR = ((sc_baudCalcTbl[tblIdx].mulval & 0xF) << 4) | 
		(sc_baudCalcTbl[tblIdx].divaddval & 0xF);

	LPC_UART->LCR &= ~LCR_DLAB;
}

// UART abstraction macros
//
#define UART_ENABLE_PWR(portNum)	\
	LPC_SYSCON->SYSAHBCLKCTRL |= 0x00001000

#define UART_DISABLE_PWR(portNum)	\
	LPC_SYSCON->SYSAHBCLKCTRL &= 0x00001000

#define UART_PUT_BYTE(portNum, Byte) 	LPC_UART->THR = (Byte)
#define UART_GET_BYTE(portNum, Byte) 	(Byte) = LPC_UART->RBR

// configure MCU pins for port 0
#define UART_SELECT_PINS0	\
	LPC_SYSCON->SYSAHBCLKCTRL |= IOCON_CLOCK_CONNECT; \
	LPC_IOCON->PIO1_6 =	0x00000011;	\
	LPC_IOCON->PIO1_7 =	0x00000011; \
	LPC_SYSCON->SYSAHBCLKCTRL &= ~IOCON_CLOCK_CONNECT

#define UART_CONFIG(portNum, dcb) \
	LPC_SYSCON->UARTCLKDIV = UART_CLKDIV; \
	LPC_UART->LCR = 0; \
	if( dcb->parity != uartNO_PARITY ) \
		LPC_UART->LCR = 0x08 | (dcb->parity << 4); \
  LPC_UART->LCR |= (dcb->stopBits << 2) | dcb->bits; \
	uartBaudConfig(dcb->baud); \
	LPC_UART->FCR = 0x07

#define UART_BREAK_SET(portNum) \
	LPC_UART->LCR |= LCR_BREAK

#define UART_BREAK_UNSET(portNum) \
	LPC_UART->LCR &= ~LCR_BREAK

static esBL uartIrqInit(void)
{
	NVIC_ClearPendingIRQ(UART_IRQn);
	NVIC_EnableIRQ(UART_IRQn);
	LPC_UART->IER = IER_RBR | IER_THRE | IER_RLS;

	return TRUE;
}

static void uartIrqUninit(void)
{
	LPC_UART->IER = 0;
	NVIC_ClearPendingIRQ(UART_IRQn);
	NVIC_DisableIRQ(UART_IRQn);
}

#define UART_ENABLE_INT(portNum) 	uartIrqInit() 		
#define UART_DISABLE_INT(portNum) uartIrqUninit() 		

#define UART_IRQ_INIT(portNum) 		uartIrqInit()
#define UART_IRQ_UNINIT(portNum) 	uartIrqUninit()

#if defined( USE_FREE_RTOS )
	// dummy asm to include context switch macros in our driver
	__asm void uartPortmacro(void) 
	{
		INCLUDE c:/FreeRTOS/Source/portable/RealView/ARM7/portmacro.inc
	}
#endif

#ifdef USE_UART_PORT0

#if defined( USE_FREE_RTOS )
void uartISRWorker0(void)
#else
void UART_IRQHandler(void) __irq
#endif
{
  esU8 dummy = 0;
	DEF_PORT_STRUCT_VAR(uartPortStruct, &ports[uart0]);
#ifdef USE_FREE_RTOS
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;	
#endif	

	switch( (LPC_UART->IIR & 0x0F) >> 1 )
	{
	case IIR_RLS:		// Receive Line Status
		dummy = LPC_UART->LSR;
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
		dummy = LPC_UART->LSR;		// Check status in the LSR to see if valid data in U1THR or not
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
#if defined( USE_FREE_RTOS	)
				xSemaphoreGiveFromISR(ps->uartTxReady, &xHigherPriorityTaskWoken);
#else
				ps->dcr |= uartTxReady;
#endif
		}
		break;
  }


#ifdef USE_FREE_RTOS
  // we can switch context if necessary
  portEXIT_SWITCHING_ISR( xHigherPriorityTaskWoken );
#endif
}

#ifdef USE_FREE_RTOS
__asm void UART_IRQHandler(void)
{
	PRESERVE8
	ARM

	portSAVE_CONTEXT

; ---------- call ISR worker
	IMPORT uartISRWorker0
	BL 		 uartISRWorker0
	
	portRESTORE_CONTEXT	
}
#endif // USE_FREE_RTOS
#endif // USE_UART_PORT0

#endif // _inc_uart_cc_
