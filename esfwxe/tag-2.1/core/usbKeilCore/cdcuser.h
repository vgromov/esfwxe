/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    cdcuser.h
 * Purpose: USB Communication Device Class User module Definitions
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

#ifndef __CDCUSER_H__
#define __CDCUSER_H__

/* CDC Requests Callback Functions */
extern esBL CDC_SendEncapsulatedCommand  (void);
extern esBL CDC_GetEncapsulatedResponse  (void);
extern esBL CDC_SetCommFeature           (unsigned short wFeatureSelector);
extern esBL CDC_GetCommFeature           (unsigned short wFeatureSelector);
extern esBL CDC_ClearCommFeature         (unsigned short wFeatureSelector);
extern esBL CDC_GetLineCoding            (void);
extern esBL CDC_SetLineCoding            (void);
extern esBL CDC_SetControlLineState      (unsigned short wControlSignalBitmap);
extern esBL CDC_SendBreak                (unsigned short wDurationOfBreak);

/* CDC Bulk Callback Functions */
extern void CDC_StartOfFrame             (void);
extern void CDC_BulkIn                   (void);
extern void CDC_BulkOut                  (void);

/* CDC Notification Callback Function */
extern void CDC_NotificationIn           (void);

/* CDC Initializtion Function */
extern void CDC_Init (void);

/* CDC prepare the SERAIAL_STATE */
extern unsigned short CDC_GetSerialState (void);

#endif  /* __CDCUSER_H__ */

