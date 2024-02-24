#include <xc.h>
#include <pic16f1829.h>
#include "GXHT30.h"
#include "MB1.h"

double Temperature; //温度数据，变量
double Humidity; //湿度数据，变量

//初始化I2C
void IIC_init(void)
{
	SCL_O;
	SDA_O; 
	SCL_L;
	SDA_L;
}

//I2C起始信号
void IIC_Start(void)
{
	SDA_O;			// 初始化
	SCL_O;
	SCL_L;
	SDA_L;
	__delay_us(20);
	SDA_H;			// 上拉数据
	__delay_us(10);
	SCL_H;			// 上拉时钟
	__delay_us(10);	//				SCL __/---\__
	SDA_L;			// 下拉数据		SDA ----\____
	__delay_us(10);	//
	SCL_L;			// 下拉时钟
	__delay_us(20);
}

//I2C终止信号
void IIC_Stop(void)
{
	SDA_O;			//初始化
	SCL_O;
	SCL_L;
	SDA_L;			//下拉数据
	__delay_us(20);
	SCL_H;			//上拉时钟
	__delay_us(10);	//				SCL __/---\__
	SDA_H;			//上拉数据		SDA ____/----
	__delay_us(10);	//
	SCL_L;			//下拉时钟
	__delay_us(10);
	SDA_L;
	__delay_us(20); 
}

//I2C应答信号
void IIC_Ack(void)
{
	SDA_O;			//初始化
	SCL_O;
	__delay_us(20);
	SCL_L;
	SDA_L;			//下拉数据
	__delay_us(20);	//				SCL __/---\__
	SCL_H;			//上拉时钟		SDA _________  
	__delay_us(20);	//
	SCL_L;			//下拉时钟
	__delay_us(20);
}

//I2C不应答信号
void IIC_NAck(void)
{
	SDA_O;			//初始化
	SCL_O;
	__delay_us(20);
	SCL_L;
	__delay_us(20);
	SDA_H;			//上拉数据		SCL __/---\__
	__delay_us(20);	//				SDA ---------
	SCL_H;			//上拉时钟
	__delay_us(20);	//
	SCL_L;			//下拉时钟
	__delay_us(20);
	SDA_L;
	__delay_us(20);
}

//I2C等待应答
unsigned char IIC_WAck(void)
{
	unsigned int i = 0;		//长整型
	SDA_H;					//上拉数据
	__delay_us(20);
	SDA_I;					//数据接口输入
	__delay_us(20);
	SCL_H;					//上拉时钟
	
	while(SDA)				//判断传感器应答信号
	{	
		i++;
		if(i > 100)			//简易时钟 判断是否接收到应答信号，即在SCL高电平时SDA下拉
		{
			SCL_L;
			IIC_Stop();	//如果超过规定时间没有动作，则主机发送结束信号
			return 1;		//返回数值1，收到不应答信号
		}
	}
	__delay_us(20);
	SCL_L;					//下拉时钟信号
	return 0;				//收到应答信号
}

//发送字节
void IIC_SendByte(unsigned char txd)
{
	unsigned char i = 0;
	SDA_O; //数据接口输出
	SCL_O;
	SCL_L; //下拉时钟
	__delay_us(20);
	for(i = 0; i < 8; i++) //发送一字节8位数据
	{
		if((txd & 0x80) > 0) //判断该变量最高位的值
		{
			SDA_H; //为1时，上拉数据
		}	
		else
		{
			SDA_L; //为0时，下拉数据
 		}
		txd <<= 1; //发送完成后将待发送数据左移一位，准备发送下一位数据
		__delay_us(20);
		SCL_H; //上拉时钟，发送数据
		__delay_us(20);
		SCL_L; //下拉时钟，发送完成
		__delay_us(20);//延迟为了数据稳定
		SDA_L;
		__delay_us(20);
	}
}

//接收字节
unsigned char IIC_ReadByte(unsigned char ack) //ack 判断字节数据是否接收完成
{
   unsigned char i = 0, receive = 0; //i是为了接收统计，receive是为了接收发来的数据
   SDA_I; //数据接口输入
   //SCL_I;
   __delay_us(20);
   for(i = 0; i < 8; i++)
   {
   		SCL_L; //下拉时钟，复位
		__delay_us(20);
		SCL_H; //上拉时钟
		__delay_us(20);
		receive <<= 1;
		if(SDA)	receive++;
	}
   
	SCL_L; //下拉时钟，复位
	__delay_us(20);
   	if(ack) //这是判断在调用该函数时的ack的值，也即是否接受完数据，是否发送结束信号。
	{
	   	IIC_Ack();	//一位字节数据接收完毕，回复应答
	}
	else
	{
		IIC_NAck();	//最后一字节数据接收完毕，回复不应答
 	}
	__delay_us(20);
	return receive; //返回接收到的一字节数据
}

//读取温度
void GXHT30_read_result(unsigned char addr)
{
	unsigned int tem,hum;	//合并数据用，变量
	unsigned int buff[6];	//接收数据用，数组
	Temperature = 0;		//温度数据，变量
	Humidity = 0;			//湿度数据，变量
	IIC_Start();			//起始信号
	IIC_SendByte(addr | 0x01); // 1 ，地址最后一位表示读写操作，1为读操作
	
	if(IIC_WAck() == 0)
	{
		SDA_I; //数据接口输入
		SCL_L;
		__delay_us(20);
		buff[0] = IIC_ReadByte(1); //温度1

		buff[1] = IIC_ReadByte(1); //温度2

		buff[2] = IIC_ReadByte(1); //CRC校验码

		buff[3] = IIC_ReadByte(1); //湿度1

		buff[4] = IIC_ReadByte(1); //湿度2

		buff[5] = IIC_ReadByte(0); //CRC校验码，最后一个字节

		IIC_Stop(); //发送结束信号
	}
	tem = ((buff[0] << 8) | buff[1]);	// 合并温度数据
	hum = ((buff[3] << 8) | buff[4]);	// 合并湿度数据
	
	Temperature = (175.0 * (float)tem / 65535.0 - 45.0);	// T = -45 + 175 * tem / (2^16-1)
	Humidity = (100.0 * (float)hum / 65535.0);				// RH = hum*100 / (2^16-1)
	
	hum = 0;	//变量重置
	tem = 0;	//变量重置
}

//发送控制命令
void GXHT30_write_cmd(unsigned char addr, unsigned char MSB, unsigned char LSB)
{
	IIC_Start();			//起始信号
	IIC_SendByte(addr);		// 0 ，地址最后一位表示读写操作，0为写操作
	IIC_WAck();				//等待应答
	IIC_SendByte(MSB);		//高位指令
	IIC_WAck();
	IIC_SendByte(LSB);		//低位指令
	IIC_WAck();
	IIC_Stop();				//发送结束信号
}

void GXHT30_single_call(void)
{
	PORTCbits.RC7 = 1;
	__delay_us(20);
	GXHT30_write_cmd(adr, 0x2C, 0x10);	// 开启时钟延伸，低重复率
	__delay_ms(5);
	GXHT30_read_result(adr);
	PORTCbits.RC7 = 0;
}