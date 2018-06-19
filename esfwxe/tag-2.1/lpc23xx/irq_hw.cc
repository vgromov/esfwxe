/*****************************************************************************
 *   irq.c: Interrupt handler C file for NXP LPC23xx/24xx Family Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.07.13  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/

/* Initialize the interrupt controller */
/******************************************************************************
** Function name:		vicInit
**
** Descriptions:		Initialize VIC interrupt controller.
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void vicInit(void) 
{
  esU32 i = 0;
  esU32 *vect_addr, *vect_prio;
 	
  /* initialize VIC*/
  VICIntEnClr = 0xffffffff;
  VICVectAddr = 0;
  VICIntSelect = 0;

  /* set all the vector and vector control register to 0 */
  for ( i = 0; i < VIC_SIZE; i++ )
  {
		vect_addr = (esU32 *)(VIC_BASE_ADDR + VECT_ADDR_INDEX + i*4);
		vect_prio = (esU32 *)(VIC_BASE_ADDR + VECT_PRIO_INDEX + i*4);
		*vect_addr = 0x0;	
		*vect_prio = 0xF;
  }
}

/******************************************************************************
** Function name:		irqInstall
**
** Descriptions:		Install interrupt handler
** parameters:			Interrupt number, interrupt handler address, 
**						interrupt priority
** Returned value:		true or false, return false if IntNum is out of range
** 
******************************************************************************/
esBL irqInstall( esU32 IntNumber, void *HandlerAddr, esU32 Priority )
{
  VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
  if ( IntNumber >= VIC_SIZE )
		return FALSE;
  else
  {
		/* find first un-assigned VIC address for the handler */
		esU32* vect_addr = (esU32*)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber*4);
		esU32* vect_prio = (esU32*)(VIC_BASE_ADDR + VECT_PRIO_INDEX + IntNumber*4);
		*vect_addr = (esU32)HandlerAddr;	/* set interrupt vector */
		*vect_prio = Priority;
		VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */
		return TRUE;
  }
}


/******************************************************************************
** Function name:		irqUninstall
**
** Descriptions:		Uninstall interrupt handler
**				Find the interrupt handler installed in the VIC
**				based on the interrupt number, set the location
**				back to NULL to uninstall it.
** parameters:			Interrupt number
** Returned value:		true or false, when the interrupt number is not found, 
**				return false
** 
******************************************************************************/
esBL irqUninstall( esU32 IntNumber )
{
 	VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
  if ( IntNumber >= VIC_SIZE )
		return FALSE;
  else
  {
		/* find first un-assigned VIC address for the handler */
		esU32* vect_addr = (esU32*)(VIC_BASE_ADDR + VECT_ADDR_INDEX + IntNumber*4);
		esU32* vect_prio = (esU32*)(VIC_BASE_ADDR + VECT_PRIO_INDEX + IntNumber*4);
		*vect_addr = 0;	/* remove interrupt vector */
		*vect_prio = 0;
		VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */
		return TRUE;
  }
}
/******************************************************************************
**                            End Of File
******************************************************************************/
