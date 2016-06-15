/**************************************************************************************************
  Filename:       SD_App.h
  Revised:        $Date: 2007-10-27 17:22:23 -0700 (Sat, 27 Oct 2007) $
  Revision:       $Revision: 15795 $

  Description:    This file contains the Sample Application definitions.


  Copyright 2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED 揂S IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, 
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE, 
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com. 
**************************************************************************************************/
#ifndef SD_APP_H
#define SD_APP_H

extern void ownInit();

#ifdef __cplusplus
extern "C"
{
    #endif
    #include "ZComDef.h"
    // These constants are only for example and should be changed to the device's needs
    #define SD_APP_ENDPOINT           15  //只要是1-240都可以
    #define SD_APP_PROFID             0x0F08
    #define SD_APP_DEVICEID           0x0001
    #define SD_APP_DEVICE_VERSION     0
    #define SD_APP_FLAGS              0
    #define SD_APP_MAX_CLUSTERS       2
    #define SD_APP_BROADCAST_CLUSTERID 1
    #define SD_APP_POINT_TO_POINT_CLUSTERID  2
    // Send Message Timeout
    #define SD_APP_SEND_PERIODIC_MSG_TIMEOUT   1000     // Every 5 seconds
    // Application Events (OSAL) - These are bit weighted definitions.
    #define SD_APP_SEND_PERIODIC_MSG_EVT       0x0001

    // Group ID for Flash Command
    #define SD_APP_FLASH_GROUP                  0x0001

    // Flash Command Duration - in milliseconds
    #define SD_APP_FLASH_DURATION               1000
    /*
     * Task Initialization for the Generic Application
     */
    extern void SD_App_Init( uint8 task_id );

    /*
     * Task Event Processor for the Generic Application
     */
    extern UINT16 SD_App_ProcessEvent( uint8 task_id, uint16 events );

    #ifdef __cplusplus
}
#endif

#endif
