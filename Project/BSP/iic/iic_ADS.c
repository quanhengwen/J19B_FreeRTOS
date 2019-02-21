/**
  ******************************************************************************
  * @file    myiic_ADS.c 
  * @author  DPL
  * @version 
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  ******************************************************************************
  */  
/* Includes ------------------------------------------------------------------*/
#include "iic_ADS.h"
/********************************************************************
**Function: IIC_ADS_Init
**Note 		: IIC_ADS ��ʼ��
**Input 	: None
**Output 	: None
**Use 		:	IIC_ADS_Init();
*********************************************************************/
void IIC_ADS_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��GPIOBʱ��
	   
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB,GPIO_Pin_6|GPIO_Pin_7); 	//PB6,PB7 �����
}
//����IIC��ʼ�ź�
void IIC_ADS_Start(void)
{
	SDA_ADS_OUT();     //sda�����
	IIC_ADS_SDA=1;	  	  
	IIC_ADS_SCL=1;
	delay_us(4);
 	IIC_ADS_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_ADS_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_ADS_Stop(void)
{
	SDA_ADS_OUT();//sda�����
	IIC_ADS_SCL=0;
	IIC_ADS_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_ADS_SCL=1; 
	IIC_ADS_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_ADS_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_ADS_IN();      //SDA����Ϊ����  
	IIC_ADS_SDA=1;delay_us(1);	   
	IIC_ADS_SCL=1;delay_us(1);	 
	while(READ_SDA_ADS)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_ADS_Stop();
			return 1;
		}
	}
	IIC_ADS_SCL=0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_ADS_Ack(void)
{
	IIC_ADS_SCL=0;
	SDA_ADS_OUT();
	IIC_ADS_SDA=0;
	delay_us(2);
	IIC_ADS_SCL=1;
	delay_us(2);
	IIC_ADS_SCL=0;
}
//������ACKӦ��		    
void IIC_ADS_NAck(void)
{
	IIC_ADS_SCL=0;
	SDA_ADS_OUT();
	IIC_ADS_SDA=1;
	delay_us(2);
	IIC_ADS_SCL=1;
	delay_us(2);
	IIC_ADS_SCL=0;
}					 				     
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_ADS_Send_Byte(u8 txd)
{                        
    u8 t;   
		SDA_ADS_OUT(); 	    
    IIC_ADS_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
		if((txd&0x80)>>7)
			IIC_ADS_SDA=1;
		else
			IIC_ADS_SDA=0;
		txd<<=1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_ADS_SCL=1;
		delay_us(2); 
		IIC_ADS_SCL=0;	
		delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_ADS_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_ADS_IN();//SDA����Ϊ����
  for(i=0;i<8;i++ )
	{
    IIC_ADS_SCL=0; 
    delay_us(2);
		IIC_ADS_SCL=1;
    receive<<=1;
    if(READ_SDA_ADS) receive++;   
		delay_us(1); 
  }					 
  if (!ack)
    IIC_ADS_NAck();//����nACK
  else
    IIC_ADS_Ack(); //����ACK   
	return receive;
}