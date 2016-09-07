#ifndef SD_APP_H
#define SD_APP_H

void customInit();
void handleZigbeeMessage(  byte *command);
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
    extern void App_Init( uint8 task_id );

    /*
     * Task Event Processor for the Generic Application
     */
    extern UINT16 App_ProcessEvent( uint8 task_id, uint16 events );

    #ifdef __cplusplus
}
#endif

#endif
