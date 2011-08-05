#include <common/target.h>
#pragma hdrstop

#include "userSWI.h"

#pragma ARM

// user swi handlers implementation
// 

// swi implementations
//
#define IMPL_SWI
#define DEF_SWI_HANDLER(swiNumber, swiHandlerName)
#define IMPL_SWI_HANDLER( swiHandlerName ) \
	void swiHandlerName ## _Impl(void)

#include <swiImpl.cc>

// C - swi invocator
void onSwi(DWORD opcode)
{
	switch(opcode) 
	{

#define DEF_SWI_HANDLER(swiNumber, swiHandlerName) \
	case swiNumber: \
		swiHandlerName ## _Impl(); \
		break;
#define IMPL_SWI_HANDLER

#include <swiImpl.cc>

		default:
			// undefined swi handler
			break;
	}
}

// top-level SWI handler routine
__asm void SWI_Handler(void)
{
  PRESERVE8                   		; 8-Byte aligned Stack

  PUSH   	{R0, LR}         				; Store R0, LR
  MRS     R0, SPSR								; Get SPSR
  TST     R0, #0x20								; Check Thumb Bit
  LDRNEH  R0, [LR,#-2]						; Thumb: Load Halfword
  BICNE   R0, R0, #0xFF00					;        Extract SWI Number
  LDREQ   R0, [LR,#-4]						; ARM:   Load Word
  BICEQ   R0, R0, #0xFF000000			;        Extract SWI Number

	IMPORT	onSwi	
	MOV     LR, PC									; save swi invocator return address
	BL			onSwi										; call C SWI invocator, SWI opcode will be passed to it in R0 register
	
	POP			{R0, PC}^        				; pop R0 & LR into PC and return
}
