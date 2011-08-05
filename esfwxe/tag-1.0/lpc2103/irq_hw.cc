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
** Function name:		init_VIC
**
** Descriptions:		Initialize VIC interrupt controller.
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
#if( VIC_BASE_ADDR==0 )
	#error "VIC_BASE_ADDR is not properly defined!"
#endif

void init_VIC(void) 
{
	DWORD i = 0;
	DWORD* vect_addr = (DWORD*)(VIC_BASE_ADDR + VECT_ADDR_INDEX);
	DWORD* vect_cntl = (DWORD*)(VIC_BASE_ADDR + VECT_CNTL_INDEX);
	
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
  VICDefVectAddr = (DWORD)DefaultVICHandler;   
}

/******************************************************************************
** Function name:		install_irq
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
BOOL install_irq( DWORD IntNumber, void *HandlerAddr, DWORD priority )
{
	DWORD i;
	DWORD* vect_addr = (DWORD*)(VIC_BASE_ADDR + VECT_ADDR_INDEX);
	DWORD* vect_cntl = (DWORD*)(VIC_BASE_ADDR + VECT_CNTL_INDEX);
	  
	VICIntEnClr = 1 << IntNumber;	/* Disable Interrupt */
	
	for ( i = 0; i < VIC_SIZE; ++i )
	{
		/* find first un-assigned VIC address for the handler */
		if( vect_addr[i] == (DWORD)NULL )
		{
		    vect_addr[i] = (DWORD)HandlerAddr;	/* set interrupt vector */
		    vect_cntl[i] = (DWORD)(IRQ_SLOT_EN | IntNumber);
	    	VICIntEnable = 1 << IntNumber;	/* Enable Interrupt */

				return TRUE;
		}
	}

	return FALSE;		/* fatal error, can't find empty vector slot */
}

/******************************************************************************
** Function name:		uninstall_irq
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
BOOL uninstall_irq( DWORD IntNumber )
{
	DWORD i;
	DWORD* vect_addr = (DWORD*)(VIC_BASE_ADDR + VECT_ADDR_INDEX);
	DWORD* vect_cntl = (DWORD*)(VIC_BASE_ADDR + VECT_CNTL_INDEX);
	  
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
