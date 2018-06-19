/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    adcuser.h
 * Purpose: Audio Device Class Custom User Definitions
 * Version: V1.20
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef __ADCUSER_H__
#define __ADCUSER_H__


/* Audio Device In/Out Endpoint Address */
#define ADC_EP_OUT       0x03

/* Audio Device Class Requests Callback Functions */
extern esBL ADC_IF_GetRequest (void);
extern esBL ADC_IF_SetRequest (void);
extern esBL ADC_EP_GetRequest (void);
extern esBL ADC_EP_SetRequest (void);


#endif  /* __ADCUSER_H__ */
