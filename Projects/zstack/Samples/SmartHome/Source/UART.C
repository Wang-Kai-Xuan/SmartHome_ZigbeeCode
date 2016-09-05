#include <ioCC2530.h>

#define uchar unsigned char
#define uint unsigned int
void InitUart();              //��ʼ������
void Uart_Send_String(char *Data,int len);

/**************************************************************** 
   ���ڳ�ʼ������     
***********************************************************/
void InitUart()
{
	//CLKCONCMD &= ~0x40; // ����ϵͳʱ��ԴΪ 32MHZ����
	//while(CLKCONSTA & 0x40);                     // �ȴ������ȶ� 
//	CLKCONCMD &= ~0x47;                          // ����ϵͳ��ʱ��Ƶ��Ϊ 32MHZ
	PERCFG&=~0x01;   //��2������λ�ã�0ʹ�ñ���λ��1��1ʹ�ñ���λ��2
	P0SEL |= 0x0C;   //P0_2 RXD P0_3 TXD ���蹦�� 0000 1100
	
	U0CSR |= 0xC0;  //���ڽ���ʹ��  1100 0000 ����UARTģʽ+��������
	U0UCR |= 0x00;  //����żУ�飬1λֹͣλ
	
	U0GCR |= 11;           //U0GCR��U0BAUD���     
	U0BAUD |= 216;       // ��������Ϊ115200 
	
	IEN0 |= 0X04;     //�����ڽ����ж� 'URX0IE = 1',Ҳ����д�� URX0IE=1;
	//URX0IE = 1;
	EA=1;    
}

/**************************************************************** 
���ڷ����ַ�������    
****************************************************************/ 
void Uart_Send_String(char *Data,int len){
    for(int j=0;j<len;j++){
        U0DBUF = *Data++;
        while(UTX0IF == 0);
        UTX0IF = 0;
    }
}

#pragma vector=URX0_VECTOR
__interrupt void uart0()
{
    URX0IF=0;//�жϽ��ձ�־�����������������ô�����־λ����1���������Ǳ���Ҫ���жϺ����а�����0
    char ch=U0DBUF;
    U0DBUF=ch;
    while(UTX0IF==0);
    UTX0IF=0;
}

