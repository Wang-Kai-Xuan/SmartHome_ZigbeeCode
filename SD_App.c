/*--------------------������¼----------------------------------

ͨ��Э�飨��Ա���Ŀ�������ԣ������֣�
1.��������
	��������	|	��������
2.��������
	ָ��		|	ѡ��

��ƺʹ����Ŀ������̣�
1.PC���ֻ�����ָ�ZigBeeЭ��ջ�����¼���Ȼ�����¼������������ȡָ�
2.��ȡָ�������ж�ָ�����ͣ�Ȼ������Ӧ�Ĵ���

˼��-�費��Ҫ�Լ�����PCB?
	��Ҫ�����ɣ�
		1.����ʦһ��ӡ�����ǲ��������ֳɵĶ�����
		2.�ܹ�ѧ��PCB�����֪ʶ�ͼ����������ڲ�����Ҫ��
		
	����Ҫ�����ɣ�
		1.��ЩPCBû�мӹ������޷����
		2.��ѭ�����Ƶ�˼�룺��Զ��Ҫ�ظ������ӡ����������壩

------------------------------------------------------*/
#include "OSAL.h"
#include "ZGlobals.h"
#include "AF.h"
#include "aps_groups.h"
#include "ZDApp.h"
#include "SD_App.h"
#include "SD_AppHw.h"
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

enum CommonicationProtocol{
    OPEN_LIGHT = '1',OPEN_CURTAIN,CLOSE_LIGHT,CLOSE_CURTAIN
};


//#define BUF_LEN 2
//char recv_data[BUF_LEN];
//char send_data[BUF_LEN];

// This list should be filled with Application specific Cluster IDs.
const cId_t SD_App_ClusterList[SD_APP_MAX_CLUSTERS] =
{
    SD_APP_BROADCAST_CLUSTERID,
    SD_APP_POINT_TO_POINT_CLUSTERID
};//�����ÿһ���˵�����Ĵ���2�����Ҵص�ID�ֱ���1��2

const SimpleDescriptionFormat_t SD_App_SimpleDesc =
{
    SD_APP_ENDPOINT,              //  int Endpoint;
    SD_APP_PROFID,                //  uint16 AppProfId[2];
    SD_APP_DEVICEID,              //  uint16 AppDeviceId[2];
    SD_APP_DEVICE_VERSION,        //  int   AppDevVer:4;
    SD_APP_FLAGS,                 //  int   AppFlags:4;
    SD_APP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
    (cId_t *)SD_App_ClusterList,  //  uint8 *pAppInClusterList;
    SD_APP_MAX_CLUSTERS,          //  uint8  AppNumInClusters;
    (cId_t *)SD_App_ClusterList   //  uint8 *pAppInClusterList;
};

// This is the Endpoint/Interface description.  It is defined here, but
// filled-in in SD_App_Init().  Another way to go would be to fill
// in the structure here and make it a "const" (in code space).  The
// way it's defined in this sample app it is define in RAM.

endPointDesc_t SD_App_epDesc;//����Ķ˵�������
/* Task ID for internal task/event processing This variable will be received when SD_App_Init() is called.*/
uint8 SD_App_TaskID;
devStates_t SD_App_NwkState;
uint8 SD_App_TransID;  // This is the unique message ID (counter)
afAddrType_t SD_App_Broadcast_DstAddr;


/*********************************************************************
 * LOCAL FUNCTIONS
 */
void SD_App_MessageMSGCB( afIncomingMSGPacket_t *pckt );
void SD_App_SendBroadcastMessage( void );
void recvSerialData(mtOSALSerialData_t *cmdMsg);


/*********************************************************************
 * @fn      SD_App_Init
 *
 * @brief   Initialization function for the Generic App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SD_App_Init( uint8 task_id )
{
	SD_App_TaskID = task_id;
	SD_App_NwkState = DEV_INIT;
	SD_App_TransID = 0;
	
	MT_UartInit();//���ڳ�ʼ��
	MT_UartRegisterTaskID(task_id);//�Ǽ������
	HalUARTWrite(0,"Hello World\n",12); //������0��'�ַ�'���ַ���������	

	#if defined ( BUILD_ALL_DEVICES )
	if ( readCoordinatorJumper() )
        zgDeviceLogicalType = ZG_DEVICETYPE_COORDINATOR;
	else
        zgDeviceLogicalType = ZG_DEVICETYPE_ROUTER;
	#endif // BUILD_ALL_DEVICES
	
	#if defined ( HOLD_AUTO_START )
        ZDOInitDevice(0);
	#endif

    SD_App_Broadcast_DstAddr.addrMode = (afAddrMode_t)AddrBroadcast;
	SD_App_Broadcast_DstAddr.endPoint = SD_APP_ENDPOINT;
	SD_App_Broadcast_DstAddr.addr.shortAddr = 0xFFFF;

    // Fill out the endpoint description.
	SD_App_epDesc.endPoint = SD_APP_ENDPOINT;
	SD_App_epDesc.task_id = &SD_App_TaskID;
	SD_App_epDesc.simpleDesc = (SimpleDescriptionFormat_t *)&SD_App_SimpleDesc;
	SD_App_epDesc.latencyReq = noLatencyReqs;

    // Register the endpoint description with the AF
	afRegister( &SD_App_epDesc );

    // Register for all key events - This app will handle all key events
	RegisterForKeys( SD_App_TaskID );
}

/*********************************************************************
 * @fn      SD_App_ProcessEvent
 *
 * @brief   Generic Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 SD_App_ProcessEvent( uint8 task_id, uint16 events )
{
	afIncomingMSGPacket_t *MSGpkt;
	
	(void)task_id;  // Intentionally unreferenced parameter
	
    if (events & SYS_EVENT_MSG){

        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive(SD_App_TaskID);
        while (MSGpkt){
            switch (MSGpkt->hdr.event){

            case CMD_SERIAL_MSG:
                recvSerialData((mtOSALSerialData_t *)MSGpkt);
                break;

            case KEY_CHANGE:
                if(((keyChange_t *)MSGpkt)->keys & 0x01){//��ť5
				  
                   SD_App_SendBroadcastMessage();
				   
                }
                if(((keyChange_t *)MSGpkt)->keys & 0x02){//��ť4
				  
                }
                break;
                // Received when a messages is received (OTA) for this endpoint
            case AF_INCOMING_MSG_CMD:
                SD_App_MessageMSGCB( MSGpkt );
                break;
            case ZDO_STATE_CHANGE:
                SD_App_NwkState = (devStates_t)(MSGpkt->hdr.status);
                if ((SD_App_NwkState == DEV_ZB_COORD)|| (SD_App_NwkState == DEV_ROUTER) || (SD_App_NwkState == DEV_END_DEVICE)){
					if(SD_App_NwkState == DEV_ZB_COORD){
						   LS164_BYTE(11);//'C'Э����
					}
					if(SD_App_NwkState == DEV_ROUTER){
						  LS164_BYTE(12);//��R��·����
					}
					if(SD_App_NwkState == DEV_END_DEVICE){
						 LS164_BYTE(13);//��E���ն�
					}
				}else{
					// Device is no longer in the network
                }
                break;

            default:
                break;
            }

        osal_msg_deallocate( (uint8 *)MSGpkt ); // Release the memory
        MSGpkt = (afIncomingMSGPacket_t *)osal_msg_receive( SD_App_TaskID );// Next - if one is available
        }
        return (events ^ SYS_EVENT_MSG); // return unprocessed events
    }
    return 0;
}

void SD_App_MessageMSGCB( afIncomingMSGPacket_t *pkt )
{
	switch ( pkt->clusterId ){
	case SD_APP_POINT_TO_POINT_CLUSTERID:
		if(pkt->cmd.Data[0]=='P'){
		   P0_1 ^=1;      
		}
		break;
	
	case SD_APP_BROADCAST_CLUSTERID:
		if(pkt->cmd.Data[0]=='B'){      
		   P0_4 = ~P0_4;
		}
		break;
	}
}

void SD_App_SendBroadcastMessage( void )
{
	uint8 data[1]={'B'};
	if (AF_DataRequest(&SD_App_Broadcast_DstAddr,
		      &SD_App_epDesc,
		      SD_APP_BROADCAST_CLUSTERID,
		      1,
		      data,
		      &SD_App_TransID,
		      AF_DISCV_ROUTE,
		      AF_DEFAULT_RADIUS ) == afStatus_SUCCESS ){
	
    }else{

	}
}
/**
 * @brief openLight ִ�д򿪵���������
 *
 */
void openLight()
{
    P0_7 = 1;
}
/**
 * @brief closeLight ִ�йرյ���������
 */
void closeLight()
{
    P0_7 = 0;
}
/**
 * @brief openCurtain ִ�д򿪴����������
 */
void openCurtain()
{

}
/**
 * @brief closeCurtain ִ�йرյ���������
 */
void closeCurtain()
{

}

void recvSerialData(mtOSALSerialData_t *cmdMsg)
{
    uint8 len,*str=NULL;     //len�������ݳ���
    str = cmdMsg->msg;          //ָ�����ݿ�ͷ
	len=*str;                 //msg��ĵ�1���ֽڴ����������ݳ���

    HalUARTWrite(0,"get data",8);
    for(uint8 i=1;i<=len;i++)/*��ӡ�����ڽ��յ������ݣ�������ʾ*/
	HalUARTWrite(0,str+i,1 ); 
    HalUARTWrite(0,"\n",1 );//����

    if(str[1] == OPEN_LIGHT){
        openLight();
    }
    if(str[1] == CLOSE_LIGHT){
        closeLight();
    }
    if(str[1] == OPEN_CURTAIN){
        openCurtain();
    }
    if(str[1] == CLOSE_CURTAIN){
        closeCurtain();
    }
}
void ownInit()
{
	P1DIR |=0x01;
	
	P0DIR |=0X10;	/* ����P0_3����������TX����ӳ�� */
	P0DIR |=0X02;	/* ����P0_3����������RX����ӳ�� */

	P0DIR |=0X80; 	/* ���ڿ��Ƶ�Ƶ�IO */
    P0_7 = 0; 		/* ���ƴ�/�رյ�Ƶ�IO */

	P2DIR |= 0x01;	/* ���ڿ��ƴ�����IO */
	P2_0 = 0;		/* ���ƴ�/�رմ�����IO */
	
	LS164_Cfg();    /* ��������� */
	LS164_BYTE(0);	/* �������ʾ"0" */
}
