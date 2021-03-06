/**
  ******************************************************************************
  * @file    BMP180.c 
  * @author  DPL
  * @version V1.0.0
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
	*BMP180驱动程序
	*2014-12-19
	*Leven
	*主要函数为BMP180_getdat();
	*两种读取方法：
	*可以函数直接输出32位的数据
	*也可以直接调用全局变量，bmp180_temp,bmp180_press分别是温度和压强
	*说明：
	*压强数据最后两位是小数，单位是帕斯卡，数据大约是10170000
	*温度数据单位没留意，不过也用不上。。。。
	*温度和压强分别引入一阶和二阶低通滤波，压强的低通是分段的低通，当接近目标值时将上限截止频率降低
	*不想用滤波也可以将滤波的部分注释掉
  * 										COPYRIGHT 2011 STMicroelectronics
  *******************************************************************************/  

/* Includes ------------------------------------------------------------------*/
#include "bmp180.h"
/* Varibales -----------------------------------------------------------------*/
short ac1;
short ac2; 
short ac3; 
unsigned short ac4;
unsigned short ac5;
unsigned short ac6;
short b1; 
short b2;
short mb;
short mc;
short md;
s32   bmp180_temp,bmp180_press;
int flag = 0;
/********************************************************************
**Function: BMP180_delay
**Note 		: BMP180延时函数
**Input 	: None
**Output 	: None
**Use 		: BMP180_delay();
*********************************************************************/
void BMP180_delay(void)
{
	u32 count=200000;
	while(count--);
}

/********************************************************************
**Function: BMP180_Multiple_Read
**Note 		: BMP180 读取函数
**Input 	: u8 add	
						u8*BUF	
						u16 num	
**Output 	: None
**Use 		: BMP180_Multiple_Read(u8 add,u8*BUF,u16 num)
*********************************************************************/
void BMP180_Multiple_Read(u8 add,u8*buffer,u16 num)
{
		u8 i;
    IIC_HTS_Start();                          //起始信号
    IIC_HTS_Send_Byte(BMP180_Address);        //发送设备地址+写信号
		IIC_HTS_Wait_Ack();
    IIC_HTS_Send_Byte(add);                   //发送存储单元地址
		IIC_HTS_Wait_Ack();
    IIC_HTS_Start();                          //起始信号
    IIC_HTS_Send_Byte(BMP180_Address+1);      //发送设备地址+读信号
		flag = IIC_HTS_Wait_Ack();
		for (i=0; i<num; i++)                  		//连续读取num个地址数据，存储中BUF
    {
      if (i == num-1)
      {
        buffer[i] = IIC_HTS_Read_Byte(0);     //最后一个数据需要回NOACK
      }
      else
      {
        buffer[i] = IIC_HTS_Read_Byte(1);      //返回ACK
      }
		}
		IIC_HTS_Stop();                           //停止信号
}

/********************************************************************
**Function: BMP180_Write
**Note 		: BMP180 读取函数
**Input 	: u8 add	
						u8 data	
**Output 	: None
**Use 		: BMP180_Write(u8 add, u8 data)
*********************************************************************/
void BMP180_Write(u8 add, u8 data)
{
    IIC_HTS_Start();                  		//起始信号
    IIC_HTS_Send_Byte(BMP180_Address);   //发送设备地址+写信号
		IIC_HTS_Wait_Ack();

    IIC_HTS_Send_Byte(add);    					//内部寄存器地址，请参考中文pdf 
		IIC_HTS_Wait_Ack();

    IIC_HTS_Send_Byte(data);       				//内部寄存器数据，请参考中文pdf
		IIC_HTS_Wait_Ack();

    IIC_HTS_Stop();                   		//发送停止信号
}

/********************************************************************
**Function: Read_calibration
**Note 		: BMP180 读取补偿数据
**Input 	: None
**Output 	: None
**Use 		: Read_calibration()
*********************************************************************/
void Read_calibration(void)
{
  u8 buf[2];
  BMP180_Multiple_Read(0xaa,buf, 2);
  ac1 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xac,buf, 2);
  ac2 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xae,buf, 2);
  ac3 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xb0,buf, 2);
  ac4 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xb2,buf, 2);
  ac5 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xb4,buf, 2);
  ac6 = buf[0] << 8 |buf[1]; 
  BMP180_Multiple_Read(0xb6,buf, 2);
  b1 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xb8,buf, 2);
  b2 = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xba,buf, 2);
  mb = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xbc,buf, 2);
  mc = buf[0] << 8 |buf[1];
  BMP180_Multiple_Read(0xbe,buf, 2);
  md = buf[0] << 8 |buf[1];
}

/********************************************************************
**Function: BMP180_Update
**Note 		: BMP180 读取压强  二阶低通滤波
**Input 	: None
**Output 	: None
**Use 		: BMP180_Update()
*********************************************************************/
s32 BMP180_Update(void)
{
  long x1,x2,x3,b3,b5,b6,b7,UP,press,UT,temp;
  unsigned long b4;
  u8 ReadBuffer[3];
  char oss = 0;  
  static float press_fliter=101700,press_q=0.1,temp_fliter=20000,temp_q=0.1;

  Read_calibration();
  
  BMP180_Write(0xf4,0x2e);
  BMP180_delay();						//delay 4.5ms
  BMP180_Multiple_Read(0xf6,ReadBuffer,2);
  UT = ReadBuffer[0] << 8 | ReadBuffer[1];
  
  BMP180_Write(0xf4,(0x34 +(oss<<6)));
  BMP180_delay();						//delay 4.5ms
  BMP180_Multiple_Read(0xf6,ReadBuffer,3);
  UP = ((ReadBuffer[0] << 16)+(ReadBuffer[1] << 8)+ReadBuffer[2]) >> (8 - oss);

  x1 = ((UT - ac6) * ac5) >> 15; 
  x2 = ((long) mc << 11) / (x1 + md);
  b5 = x1 + x2;
  temp = ((b5 + 8)*100) >> 4;
  
  temp_fliter = temp_fliter*(1-temp_q)+(float)temp*temp_q;
  bmp180_temp=temp_fliter;
  
  b6 = b5 - 4000;
  x1 = (b2 * ((b6 * b6) >> 12)) >> 11;
  x2 = (ac2 * b6) >> 11;
  x3 = x1 + x2;
  b3 = ((((long)ac1 * 4 + x3) << oss) + 2) / 4;
  x1 = (ac3 * b6) >> 13;
  x2 = (b1 * ((b6 * b6)>> 12)) >> 16;
  x3 = ((x1 + x2 )+ 2) >> 2;
  b4 = (ac4 * (unsigned long)(x3 + 32768)) >> 15;
  b7 = ((unsigned long)UP - b3) * (50000 >> oss);
  if(b7 < 0x80000000)
    press = (b7 * 2) / b4;
  else
    press = (b7 / b4 ) * 2;
  x1 = ((press) >> 8) * ((press) >> 8);
  x1 = (x1 * 3038) >> 16;
  x2 = (-7357 * press) >> 16;
  press = press + (((x1 + x2 + 3791)) >> 4);
  
  if(fabs(press_fliter-press)<100)press_q=fabs(press_fliter-press)/1000;
  else press_q=0.1;
  press_fliter = press_fliter*(1-press_q)+(float)press*press_q;
  bmp180_press  = press_fliter*100;
	if(flag==1) bmp180_press = 0;
  return bmp180_press;
}



