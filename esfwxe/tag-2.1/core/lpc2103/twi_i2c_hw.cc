// lpc i2c driver implementation for lpc2101-03 cores
// this file is explicitly included from ../twi_i2c.c
// do not include it in the project directly
//
#ifndef _inc_twi_i2c_c_
#define _inc_twi_i2c_c_

#define I2C_I2EN				0x00000040  /* I2C Control Set Register */
#define I2C_AA					0x00000004
#define I2C_SI					0x00000008
#define I2C_STO					0x00000010
#define I2C_STA					0x00000020

//////////////////////////////////////////////////////////////////// i2c helper defines
//

#define I2C_ENABLE_PWR(portNum)		PCONP |= PCI2C ## portNum
#define I2C_DISABLE_PWR(portNum)	PCONP	&= ~PCI2C ## portNum

// i2c0 p0.2-p0.3		

#define I2C_PINS_SELECT0  \
  	PINSEL0 &= ~0x000000F0;	\
  	PINSEL0 |=  0x00000050	

// i2c1 p0.17-p0.18		
#define I2C_PINS_SELECT1  \
  	PINSEL1 &= ~0x0000003C;	\
  	PINSEL1 |=  0x00000014	

#define I2C_CONFIG(portNum, dcb) \
		I2C_ENABLE_PWR(portNum); \
		I2C_PINS_SELECT ## portNum; \
  	I2C_RESET(portNum);    			\
  	I2C ## portNum ## SCLL = MAX( Fpclk/(2 * dcb->baud), 4);		\
  	I2C ## portNum ## SCLH = I2C ## portNum ## SCLL

#ifdef USE_FREE_RTOS

#	pragma push
#	pragma ARM
	// dummy asm to include context switch macros in our driver
	__asm void twi_i2cPortmacro(void) 
	{
		INCLUDE C:/FreeRTOS/Source/portable/RVDS/ARM7_LPC21xx/portmacro.inc
	}
#	pragma pop 

#	define I2C_INIT_SLAVE(portNum, dcb) \
		I2C## portNum ## ADR = dcb->slaveAddr; \
  	if( irqInstall( I2C ## portNum ## _INT, (void*)&i2cSlaveHandler ## portNum, HIGHEST_PRIORITY ) == TRUE ) \
		{	\
			ports[i2c## portNum].dcb = *dcb; \
			ports[i2c## portNum].mutex = rtosMutexCreate(); \
 			return ((i2cHANDLE)&ports[i2c## portNum]); \
		}

#	define I2C_INIT_MASTER(portNum, dcb) \
  	if( irqInstall( I2C ## portNum ## _INT, (void*)&i2cMasterHandler ## portNum, HIGHEST_PRIORITY ) == TRUE ) \
		{	\
			ports[i2c## portNum].dcb = *dcb; \
			ports[i2c## portNum].mutex = rtosMutexCreate(); \
 			return ((i2cHANDLE)&ports[i2c## portNum]); \
		}

#else

#	define I2C_INIT_SLAVE(portNum, dcb) \
		I2C## portNum ## ADR = dcb->slaveAddr; \
  	if( irqInstall( I2C ## portNum ## _INT, (void*)&i2cSlaveWorker ## portNum, HIGHEST_PRIORITY ) == TRUE ) \
		{	\
			ports[i2c## portNum].dcb = *dcb; \
 			return ((i2cHANDLE)&ports[i2c## portNum]); \
		}

#	define I2C_INIT_MASTER(portNum, dcb) \
  	if( irqInstall( I2C ## portNum ## _INT, (void*)&i2cMasterWorker ## portNum, HIGHEST_PRIORITY ) == TRUE ) \
		{	\
			ports[i2c## portNum].dcb = *dcb; \
 			return ((i2cHANDLE)&ports[i2c## portNum]); \
		}

#endif

#define I2C_SET_CTL( portNum, bits )  I2C ## portNum ## CONSET = (bits) 
#define I2C_CLR_CTL( portNum, bits )	I2C ## portNum ## CONCLR = (bits)

#define I2C_ENABLE(portNum)						I2C_SET_CTL(portNum, I2C_I2EN)
#define I2C_RESET(portNum) 						I2C_CLR_CTL(portNum, I2C_SI | I2C_I2EN | I2C_AA | I2C_STO)

#define I2C_SEND_START( portNum )			I2C_SET_CTL(portNum, I2C_STA)

#define I2C_SEND_RESTART( portNum )	\
	I2C_SET_CTL(portNum, I2C_STA | I2C_AA); \
	I2C_CLR_CTL(portNum, I2C_SI)

#define I2C_SEND_START_ACK( portNum )	\
	I2C_SET_CTL(portNum, I2C_AA); \
	I2C_CLR_CTL(portNum, I2C_SI | I2C_STA)

#define I2C_SEND_ACK( portNum )	\
	I2C_SET_CTL(portNum, I2C_AA); \
	I2C_CLR_CTL(portNum, I2C_SI)

#define I2C_SEND_NACK( portNum )	\
	I2C_CLR_CTL(portNum, I2C_SI | I2C_AA)

#define I2C_SEND_STOP( portNum )	\
  I2C_SET_CTL( portNum, I2C_STO | I2C_AA ); \
	I2C_CLR_CTL( portNum, I2C_SI )

/////////////////////////////////////////////////////// worker procs /////////////////////////////////////////////////////
#ifdef USE_I2C_PORT0

#ifdef USE_I2C_MASTER_MODE
// i2c master mode interrupt handler. when bug-free stage is reached - make port-dependent macro from it,
// and put in twi_i2c c file
//
#	pragma push
#	pragma ARM

#	if defined( USE_FREE_RTOS )
void i2cMasterWorker0(void) 
#	else
static void i2cMasterWorker0(void) __irq
#	endif
{
	DEF_PORT_STRUCT_VAR(i2cPortStruct, &ports[i2c0]);

#	ifdef USE_FREE_RTOS
	esBL higherPriorityTaskWoken = FALSE;
#	endif	

	ps->lineStatus = I2C0STAT;

  switch( ps->lineStatus )
  {
	// Master General
	case TW_START:													// A Start condition is issued.
	case TW_REP_START:											// A repeated started is issued
		ps->dcr &= ~i2cReady;									// set process state
		I2C0DAT = ps->addrIO;									// send client address
		I2C_SEND_START_ACK(0);								// Start ACK
		break;
	// Master Transmitter & Receiver Status codes
	case TW_MT_SLA_ACK:											// 0x18: Slave address acknowledged
	case TW_MT_DATA_ACK:										// 0x28: Data acknowledged
		if(	ps->pos < ps->end )
		{
			I2C0DAT = *(ps->pos);
			++ps->pos;
			I2C_SEND_ACK(0);
		}
		else
		{
			// send stop condition
			I2C_SEND_STOP(0);
			// set port ready flag
			ps->dcr	|= i2cReady; 
		}
		break;
	case TW_MR_DATA_NACK:				// 0x58: Data received, NACK reply issued
		// store final received data byte
		if(	ps->pos < ps->end )
		{
			*(ps->pos) = I2C0DAT;
			++ps->pos;
		}
		else
			ps->err = i2cBufferOverflow;		// buffer overflow
		// send stop condition
		I2C_SEND_STOP(0);
		// set port ready flag
		ps->dcr |= i2cReady;
		break;
	// continue to transmit STOP condition
	case TW_MR_SLA_NACK:				// 0x48: Slave address not acknowledged
	case TW_MT_SLA_NACK:				// 0x20: Slave address not acknowledged
		ps->err = i2cNoDev;
	case TW_MT_DATA_NACK:				// 0x30: Data not acknowledged
		// transmit stop condition
		I2C_SEND_STOP(0);
		// set port ready flag
		ps->dcr |= i2cReady;
		break;
	case TW_MT_ARB_LOST:				// 0x38: Bus arbitration lost
	//case TW_MR_ARB_LOST:			// 0x38: Bus arbitration lost	
		ps->err = i2cArbtLost;
		// release bus, new start condition will be re-transfered
		// when bus become free again
		I2C_SEND_RESTART(0);
		break;
	case TW_MR_DATA_ACK:				// 0x50: Data acknowledged
		// store received data byte
		*(ps->pos) = I2C0DAT;
		++ps->pos;
	// fall-through to see if more bytes will be received
	case TW_MR_SLA_ACK:					// 0x40: Slave address acknowledged
		if(	ps->pos < ps->end-1 )
		{
			I2C_SEND_ACK(0); 	// we can receive more bytes
		}
		else
		{
			I2C_SEND_NACK(0); // NACK data (we can receive one more byte, clear SI)
		}
		break;

	// Misc
	case TW_NO_INFO:						// 0xF8: No relevant state information
		// do nothing
		I2C_CLR_CTL(0, I2C_SI);
		break;
	case TW_BUS_ERROR:					// 0x00: Bus error due to illegal start or stop condition
		ps->err = i2cBusError;
		// release bus on error
		I2C_SEND_STOP(0);
		// set port ready flag
		ps->dcr	|= i2cReady;
		break;
  }

  VICVectAddr = 0;		// aknowledge interrupt
#	ifdef USE_FREE_RTOS
  // we can switch context if necessary
	rtosExitSwitchingIsr( higherPriorityTaskWoken );
#	endif
}

#	pragma pop

#endif // USE_I2C_MASTER_MODE

#ifdef USE_I2C_SLAVE_MODE	 // todo: implement

#	pragma push
#	pragma ARM

// i2c slave mode interrupt worker
#	if defined( USE_FREE_RTOS )
static void i2cSlaveWorker0(void) 
#	else
static void i2cSlaveWorker0(void) __irq
#	endif
{
	#error "Not implemented!"
}

#	pragma pop

#endif //	USE_I2C_SLAVE_MODE

#ifdef USE_FREE_RTOS

#	pragma push
#	pragma ARM

// Assembler ISR handler. Calls respective worker inside
__asm void i2cMasterHandler0(void)
{
	PRESERVE8
	ARM

	portSAVE_CONTEXT

; ---------- call ISR worker
	IMPORT i2cMasterWorker0
	BL 		 i2cMasterWorker0
	
	portRESTORE_CONTEXT
}

#	pragma pop

#endif // defined( USE_FREE_RTOS )

#endif // USE_I2C_PORT0

#ifdef USE_I2C_PORT1

#ifdef USE_I2C_MASTER_MODE

#	pragma push
#	pragma ARM

// i2c master mode interrupt handler. when bug-free stage is reached - make port-dependent macro from it,
// and put in twi_i2c c file
//
#	if defined( USE_FREE_RTOS )
static void i2cMasterWorker1(void) 
#	else
static void i2cMasterWorker1(void) __irq
#	endif
{
	DEF_PORT_STRUCT_VAR(i2cPortStruct, &ports[i2c1]);

#	ifdef USE_FREE_RTOS
	esBL higherPriorityTaskWoken = FALSE;
#	endif	

	ps->lineStatus = I2C1STAT;

  switch( ps->lineStatus )
  {
	// Master General
	case TW_START:													// A Start condition is issued.
	case TW_REP_START:											// A repeated started is issued
		ps->dcr &= ~i2cReady;									// set process state
		I2C1DAT = ps->addrIO;									// send client address
		I2C_SEND_START_ACK(1);								// Start ACK
		break;
	// Master Transmitter & Receiver Status codes
	case TW_MT_SLA_ACK:											// 0x18: Slave address acknowledged
	case TW_MT_DATA_ACK:										// 0x28: Data acknowledged
		if(	ps->pos < ps->end )
		{
			I2C1DAT = *(ps->pos);
			++ps->pos;
			I2C_SEND_ACK(1);
		}
		else
		{
			// send stop condition
			I2C_SEND_STOP(1);
			// set port ready flag
			ps->dcr	|= i2cReady; 
		}
		break;
	case TW_MR_DATA_NACK:				// 0x58: Data received, NACK reply issued
		// store final received data byte
		if(	ps->pos < ps->end )
		{
			*(ps->pos) = I2C1DAT;
			++ps->pos;
		}
		else
			ps->err = i2cBufferOverflow;		// buffer overflow
		// send stop condition
		I2C_SEND_STOP(1);
		// set port ready flag
		ps->dcr |= i2cReady;
		break;
	// continue to transmit STOP condition
	case TW_MR_SLA_NACK:				// 0x48: Slave address not acknowledged
	case TW_MT_SLA_NACK:				// 0x20: Slave address not acknowledged
		ps->err = i2cNoDev;
	case TW_MT_DATA_NACK:				// 0x30: Data not acknowledged
		// transmit stop condition
		I2C_SEND_STOP(1);
		// set port ready flag
		ps->dcr |= i2cReady;
		break;
	case TW_MT_ARB_LOST:				// 0x38: Bus arbitration lost
	//case TW_MR_ARB_LOST:			// 0x38: Bus arbitration lost	
		ps->err = i2cArbtLost;
		// release bus, new start condition will be re-transfered
		// when bus become free again
		I2C_SEND_RESTART(1);
		break;
	case TW_MR_DATA_ACK:				// 0x50: Data acknowledged
		// store received data byte
		*(ps->pos) = I2C1DAT;
		++ps->pos;
	// fall-through to see if more bytes will be received
	case TW_MR_SLA_ACK:					// 0x40: Slave address acknowledged
		if(	ps->pos < ps->end-1 )
		{
			I2C_SEND_ACK(1); 	// we can receive more bytes
		}
		else
		{
			I2C_SEND_NACK(1); // NACK data (we can receive one more byte, clear SI)
		}
		break;

	// Misc
	case TW_NO_INFO:						// 0xF8: No relevant state information
		// do nothing
		I2C_CLR_CTL(1, I2C_SI);
		break;
	case TW_BUS_ERROR:					// 0x00: Bus error due to illegal start or stop condition
		ps->err = i2cBusError;
		// release bus on error
		I2C_SEND_STOP(1);
		// set port ready flag
		ps->dcr	|= i2cReady;
		break;
  }

//  IDISABLE;
  VICVectAddr = 0;		// aknowledge interrupt
#	ifdef USE_FREE_RTOS
  // we can switch context if necessary
	rtosExitSwitchingIsr( higherPriorityTaskWoken );
#	endif
}

#	pragma pop

#endif // USE_I2C_MASTER_MODE

#ifdef USE_I2C_SLAVE_MODE	 // todo: implement

#	pragma push
#	pragma ARM

// i2c slave mode interrupt worker
#	if defined( USE_FREE_RTOS )
static void i2cSlaveWorker1(void) 
#	else
static void i2cSlaveWorker1(void) __irq
#	endif
{
	#error "Not implemented!"
}

# pragma pop

#endif //	USE_I2C_SLAVE_MODE

#if defined( USE_FREE_RTOS )
#	pragma push
#	pragma ARM
// Assembler ISR handler. Calls respective worker inside
__asm void i2cMasterHandler1(void)
{
	PRESERVE8
	ARM

	portSAVE_CONTEXT

; ---------- call ISR worker
	IMPORT i2cMasterWorker1
	BL 		 i2cMasterWorker1
	
	portRESTORE_CONTEXT
}
# pragma pop
#endif // defined( USE_FREE_RTOS )

#endif // USE_I2C_PORT1


#endif // _inc_twi_i2c_c_
