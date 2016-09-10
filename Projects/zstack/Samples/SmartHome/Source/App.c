#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"
#include "App.h"
#include "AppHw.h"
#include "OnBoard.h"
#include"74LS164_8LED.h"
/* HAL */
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_key.h"
#include "MT_UART.h"
#include "MT_APP.h"
#include "MT.h"
#include "string.h"

enum Command{
    COMMAND_NULL = '0',LIGHT_OPEN,LIGHT_CLOSE,CURTAIN_OPEN,CURTAIN_CLOSE
};
unsigned char press_key_5[]="key_5 was pressed";
unsigned char press_key_4[]="key_4 was pressed";
unsigned char press_key_3[]="key_3 was pressed";
unsigned char light_state_open[]="light state: open";
unsigned char light_state_close[]="light state: close";
unsigned char curtain_state_open[]="curtain state: open";
unsigned char curtain_state_close[]="curtain state: close";

uint8 AF_openCurtain[1]={'a'};
uint8 AF_closeCurtain[1]={'b'};

uint8 didCloseCurtain[1]={'c'};
uint8 didOpenCurtain[1]={'d'};

uint8 AF_openLight[1]={'e'};
uint8 AF_closeLight[1]={'f'};

uint8 didCloseLight[1]={'g'};
uint8 didOpenLight[1]={'h'};

int stepLastTime = 100;
int stepDelayTime = 1000;
void customInit(void);
void initStepperMotor(void);
void initSegmentDisplay(void);
const cId_t App_ClusterList[SD_APP_MAX_CLUSTERS] =
{
    SD_APP_BROADCAST_CLUSTERID,
    SD_APP_POINT_TO_POINT_CLUSTERID
};//这就是每一个端点里面的簇有2个并且簇的ID分别是1，2

const SimpleDescriptionFormat_t App_SimpleDesc =
{
    SD_APP_ENDPOINT,              //  int Endpoint;
    SD_APP_PROFID,                //  uint16 AppProfId[2];
    SD_APP_DEVICEID,              //  uint16 AppDeviceId[2];
    SD_APP_DEVICE_VERSION,        //  int   AppDevVer:4;
    SD_APP_FLAGS,                 //  int   AppFlags:4;
    SD_APP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
    (cId_t *)App_ClusterList,  //  uint8 *pAppInClusterList;
    SD_APP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
    (cId_t *)App_ClusterList   //  uint8 *pAppInClusterList;
};

endPointDesc_t App_epDesc;//定义的端点描述符
uint8 App_TaskID;
devStates_t App_NwkState;
uint8 App_TransID;  // This is the unique message ID (counter)
afAddrType_t App_Broadcast_DstAddr;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void processNetworkMsg( afIncomingMSGPacket_t *pckt );
void App_SendBroadcastMessage(uint8 data[]);
void Delay(long time);
#define uchar unsigned char
uchar corotation[4] ={0x80,0x40,0x20,0x10}; // 正转
uchar reverse[4]={0x10,0x20,0x40,0x80}; // 反转
/**
 * @brief openLight 执行打开电灯这个动作
 *
 */
void openLight();
/**
 * @brief closeLight 执行关闭电灯这个动作
 */
void closeLight();
/**
 * @brief openCurtain 执行打开电灯这个动作
 */
void openCurtain();

/**
 * @brief closeCurtain 执行关闭电灯这个动作
 */
void closeCurtain();

/**
 * @brief recvSerialData 这个函数中接收串口的数据，包括手机、电脑端通过AP发送的指令和数据
 * @param cmdMsg
 */
void processSerialMsg(mtOSALSerialData_t *cmdMsg);

void App_Init( uint8 task_id )
{
    App_TaskID = task_id;
    App_NwkState = DEV_INIT;
    App_TransID = 0;

    MT_UartInit();//串口初始化
    MT_UartRegisterTaskID(task_id);//登记任务号
    #if defined ( BUILD_ALL_DEVICES)
    if (readCoordinatorJumper())
        zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
    else
        zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
    #endif // BUILD_ALL_DEVICES

    #if defined ( HOLD_AUTO_START )
        ZDOInitDevice(0);
    #endif

    App_Broadcast_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
    App_Broadcast_DstAddr.endPoint = SD_APP_ENDPOINT;
    App_Broadcast_DstAddr.addr.shortAddr = 0xFFFF;

    // Fill out the endpoint description.
    App_epDesc.endPoint = SD_APP_ENDPOINT;
    App_epDesc.task_id = &App_TaskID;
    App_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&App_SimpleDesc;
    App_epDesc.latencyReq = noLatencyReqs;

    // Register the endpoint description with the AF
    afRegister( &App_epDesc );

    // Register for all key events - This app will handle all key events
    RegisterForKeys( App_TaskID );
}

void processKeyChange(afIncomingMSGPacket_t *MSGpkt)
{
    if(((keyChange_t *)MSGpkt)->keys & 0x01){/*按钮5*/
        HalUARTWrite(0,press_key_5,strlen(press_key_5));
    }
    if(((keyChange_t *)MSGpkt)->keys & 0x02){/*按钮4*/
        HalUARTWrite(0,press_key_4,strlen(press_key_4));
    }
}

void processZDOStateChange(afIncomingMSGPacket_t *MSGpkt)
{
    App_NwkState = (devStates_t)(MSGpkt->hdr.status);
    if ((App_NwkState == DEV_ZB_COORD)|| (App_NwkState == DEV_ROUTER) || (App_NwkState == DEV_END_DEVICE)){
        if(App_NwkState == DEV_ZB_COORD){
               LS164_BYTE(11);//'C'协调器
        }
        if(App_NwkState == DEV_ROUTER){
              LS164_BYTE(12);//‘R’路由器
        }
        if(App_NwkState == DEV_END_DEVICE){
             LS164_BYTE(13);//‘E’终端
        }
    }else{
        // Device is no longer in the network
    }
}

uint16 App_ProcessEvent( uint8 task_id, uint16 events ){
    afIncomingMSGPacket_t *MSGpkt;
    (void)task_id;  // Intentionally unreferenced parameter

    if (events & SYS_EVENT_MSG){
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(App_TaskID);
        while (MSGpkt){
            switch (MSGpkt->hdr.event){
            case CMD_SERIAL_MSG:
                processSerialMsg((mtOSALSerialData_t *)MSGpkt);
                break;
            case KEY_CHANGE:
                processKeyChange(MSGpkt);
                break;
            case AF_INCOMING_MSG_CMD:
                processNetworkMsg(MSGpkt);
                break;
            case ZDO_STATE_CHANGE:
                processZDOStateChange(MSGpkt);
                break;
            default:
                break;
            }
        osal_msg_deallocate( (uint8 *)MSGpkt ); // Release the memory
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( App_TaskID );// Next - if one is available
        }
        return (events ^ SYS_EVENT_MSG); // return unprocessed events
    }
    return 0;
}

void processZigbeeBroadcast(afIncomingMSGPacket_t *pkt)
{
    /*process curtain*/
    if(pkt->cmd.Data[0]==AF_openCurtain[0]){
       openCurtain();
       App_SendBroadcastMessage(didOpenCurtain);
    }
    if(pkt->cmd.Data[0]==AF_closeCurtain[0]){
       closeCurtain();
       App_SendBroadcastMessage(didCloseCurtain);
    }
    if(pkt->cmd.Data[0]==didCloseCurtain[0]){
       HalUARTWrite(0,curtain_state_close,strlen(curtain_state_close));
    }
    if(pkt->cmd.Data[0]==didOpenCurtain[0]){
       HalUARTWrite(0,curtain_state_open,strlen(curtain_state_open));
    }

    /*process light*/
    if(pkt->cmd.Data[0]==AF_openLight[0]){
       openLight();
       App_SendBroadcastMessage(didOpenLight);
    }
    if(pkt->cmd.Data[0]==AF_closeLight[0]){
       closeLight();
       App_SendBroadcastMessage(didCloseLight);
    }
    if(pkt->cmd.Data[0]==didCloseLight[0]){
       HalUARTWrite(0,light_state_close,strlen(light_state_close));
    }
    if(pkt->cmd.Data[0]==didOpenLight[0]){
       HalUARTWrite(0,light_state_open,strlen(light_state_open));
    }    
}

void processNetworkMsg( afIncomingMSGPacket_t *pkt )
{
    switch ( pkt->clusterId ){
    case SD_APP_POINT_TO_POINT_CLUSTERID:
        if(pkt->cmd.Data[0]=='P'){
           P0_1 ^=1;
        }
        break;

    case SD_APP_BROADCAST_CLUSTERID:
        processZigbeeBroadcast(pkt);
        break;
    }
}

void App_SendBroadcastMessage(uint8 data[1])
{
    if (AF_DataRequest(&App_Broadcast_DstAddr,
              &App_epDesc,
              SD_APP_BROADCAST_CLUSTERID,
              1,
              data,
              &App_TransID,
              AF_DISCV_ROUTE,
              AF_DEFAULT_RADIUS ) == afStatus_SUCCESS ){

    }else{

    }
}

void openLight()
{
    P0_1 = 1;
}

void closeLight()
{
    P0_1 = 0;
}

void openCurtain()
{
    for(int j=0;j<stepLastTime;j++){
        for(int i=0;i<4;i++){
          P0=corotation[i];
          Delay(stepDelayTime);
        }
    }
}

void closeCurtain()
{
    for(int j=0;j<stepLastTime;j++){
        for(int i=0;i<4;i++){
          P0=corotation[i];
          Delay(stepDelayTime);
        }
    }
}

void sendMessageOnZigbee(uint8 *str){
    uint8 data[1]={str[1]};
    if (AF_DataRequest(&App_Broadcast_DstAddr,
                  &App_epDesc,
                  SD_APP_BROADCAST_CLUSTERID,
                  1,
                  data,
                  &App_TransID,
                  AF_DISCV_ROUTE,
                  AF_DEFAULT_RADIUS ) == afStatus_SUCCESS ){
    }else{

    }
}

void processSerialMsg(mtOSALSerialData_t *cmdMsg)
{
    uint8 len,*str=NULL;     //len有用数据长度
    str = cmdMsg->msg;          //指向数据开头
    len=*str;                 //msg里的第1个字节代表后面的数据长度
    if(str[1] == LIGHT_OPEN){
        App_SendBroadcastMessage(AF_openLight);
    }
    if(str[1] == LIGHT_CLOSE){
        App_SendBroadcastMessage(AF_closeLight);
    }
    if(str[1] == CURTAIN_OPEN){
        App_SendBroadcastMessage(AF_openCurtain);
    }
    if(str[1] == CURTAIN_CLOSE){
        App_SendBroadcastMessage(AF_closeCurtain);
    }
}

void initSegmentDisplay(void)
{
    LS164_Cfg();    /* 数码管配置 */
    LS164_BYTE(0);
}

void initLight(void)
{
    P0DIR |=0X02; 	/* 用于控制电灯的IO */
    P0_2 = 0;
}

void initStepperMotor(void)
{
    P0SEL &= 0x0f; // 1111 0000
    P0DIR |= ~0x0f;
}

void customInit(void){
  initLight(); 		/* 控制打开/关闭电灯的IO */
  initStepperMotor();		/* 控制打开/关闭窗帘的IO */
  initSegmentDisplay();	/* 数码管显示"0" */
}
void Delay(long time){
  while(-- time);
}
