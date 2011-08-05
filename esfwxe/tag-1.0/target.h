/*****************************************************************************
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2006.09.20  ver 1.00    Prelimnary version, first Release
 *
******************************************************************************/
#ifndef __TARGET_H 
#define __TARGET_H

#include <targetConfig.h>

// common type definitions
#include <common/type.h>

// include MCU stuff
#if LPC23XX == 1
	#include <common/lpc23xx/lpc23xx.h> 							
#endif

#if LPC214X == 1
	#include <common/lpc214x/lpc214x.h>
#endif

#if LPC2103 == 1
	#include <common/lpc2103/lpc2103.h>
#endif 

#ifdef __cplusplus
   extern "C" {
#endif

void ConfigurePLL(void);
void PowerResetInit(void);
void GPIOResetInit( void );
// burnTmo is timeout needed to erase-burn firmware, before device 
// is programmatically reset with watchdog
void RunBootLoader( DWORD burnTmo );

// this should be called as soon as possible after the main 
// program entry point is reached
void targetResetInit(void);

#ifdef __cplusplus
   }
#endif
 
#endif /* end __TARGET_H */
/******************************************************************************
**                            End Of File
******************************************************************************/
