/*****************************************************************************
 *   irq.c: Interrupt handler C file for NXP LPC2101-03 Microprocessors
******************************************************************************/
#pragma ARM
 
static void DefaultVICHandler(void) __irq
{
    /* if the IRQ is not installed into the VIC, and interrupt occurs, the
    default interrupt VIC address will be used. This could happen in a race 
    condition. For debugging, use this endless loop to trace back. */
    /* For more details, see Philips appnote AN10414 */
    VICVectAddr = 0;		/* Acknowledge Interrupt */ 
		while(1) ;
}

/* Initialize the interrupt controller */
/******************************************************************************
** Function name:		vicInit
**
** Descriptions:		Initialize VIC interrupt controller.
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
#if( VIC_BASE_ADDR==0 )
	#error "VIC_BASE_ADDR is not properly defined!"
#endif

void vicInit(void) 
{
	esU32 i = 0;
	esU32* vect_addr = (esU32*)(VIC_BASE_ADDR + VECT_ADDR_INDEX);
	esU32* vect_cntl = (esU32*)(VIC_BASE_ADDR + VECT_CNTL_INDEX);
	
 //Disable all interrupts
  VICIntEnClr = 0xFFFFFFFF;
  //Clear Software Interrupts
  VICSoftIntClr = 0xFFFFFFFF;
  //Write to VicVectAddr register
  VICVectAddr = 0;
  //Set all to IRQ
  VICIntSelect = 0;
	
	/* set all the vector and vector control register to 0 */
	for ( i = 0; i < VIC_SIZE; i++ )
	{
		vect_addr[i] = 0;	
		vect_cntl[i] = 0;
	}

  /* Install the default VIC handler here */
  VICDefVectAddr = (esU32)DefaultVICHandler;   
}

/******************************************************************************
** Function name:		irqInstall
**
** Descriptions:		Install interrupt handler
**				The max VIC size is 16, but, there are 32 interrupt
**				request inputs. Not all of them can be installed into
**				VIC table at the same time.
**				The order of the interrupt request installation is
**				first come first serve.
** parameters:			Interrupt number and interrupt handler address
** Returned value:		true or false, when the table is full, return false
** 
******************************************************************************/
// priority parameter is not used in some MCU cores
esBL irqInstall( esU32 IntNumber, void *HandlerAddr, esU32 priority )
{
	esU32 i;
	esU32* vect_addr = (esU32*)(VIC_BASE_ADDR + VECT_ADDR_INDEX);
	esU32* vect_cntl = (esU32*)(VIC_BASE_ADDR + VECT_CNTL_INDEX);
	  
	VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
	
	for ( i = 0; i < VIC_SIZE; ++i )
	{
		/* find first un-assigned VIC address for the handler */
		if( vect_addr[i] == (esU32)NULL )
		{
		    vect_addr[i] = (esU32)HandlerAddr;	/* set interrupt vector */
		    vect_cntl[i] = (esU32)(IRQ_SLOT_EN | IntNumber);
	    	VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */

				return TRUE;
		}
	}

	return FALSE;		/* fatal error, can't find empty vector slot */
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
	esU32 i;
	esU32* vect_addr = (esU32*)(VIC_BASE_ADDR + VECT_ADDR_INDEX);
	esU32* vect_cntl = (esU32*)(VIC_BASE_ADDR + VECT_CNTL_INDEX);
	  
	VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
	
	for( i = 0; i < VIC_SIZE; ++i )
	{
		/* find VIC assigned for the IntNumber */
		if( vect_cntl[i] == (IRQ_SLOT_EN|IntNumber) )
		{
		   vect_addr[i] = 0;	/* clear the VIC entry in the VIC table */
		   vect_cntl[i] = 0;	/* clear control */	
		   
			 return TRUE;
		}
	}

	return FALSE;
}
