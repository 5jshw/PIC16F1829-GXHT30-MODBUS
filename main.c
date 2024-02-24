#include <xc.h>
#include <stdlib.h>
#include <pic16f1829.h>
#include "rs485.h"
#include "GXHT30.h"
#include "MB1.h"


extern double Temperature;	// 温度
extern double Humidity;		// 湿度
extern char cOurAddr;		// 从机地址
extern char cRs485RxChar;	// modbus读取数据
extern const char EEPROM_OADDR_EADDR[2] = {0x01, 0x01};		// 从机地址在EEPROM中的位置
extern const char EEPROM_BUAD_EADDR[2] = {0x01, 0x02};			// 波特率在EEPROM中的位置
extern const char EEPROM_TEMPAMEND_EADDR[2] = {0x01, 0x03};	// 温度修正值在EEPROM中的位置
extern const char EEPROM_HUMIAMEND_EADDR[2] = {0x01, 0x04};	// 湿度修正值在EEPROM中的位置
extern char EEPROM_OADDR_EDATA[2];		// 从机地址在EEPROM中的值
extern char EEPROM_BUAD_EDATA[2];		// 波特率在EEPROM中的值
extern char EEPROM_TEMPAMEND_EDATA[2];	// 温度修正值在EEPROM中的值
extern char EEPROM_HUMIAMEND_EDATA[2];	// 湿度修正值在EEPROM中的值
extern const unsigned short sBUAD[3] = {415, 207, 103};	// 波特率调节器	4800	9600	19200
static char Temp[2];	// 测量温度
static char Humi[2];	// 测量湿度
static char wCmd;		// 控制命令
static char sValue[8];	// 发送数据
static char wReAddr[2], wReValue[2], wReData[8];	// 地址 数量 数据
static char Time1;

void main(void)
{
	char cPacketReady;	// 数据包准备标志
	main_init();		// 程序初始化
	IIC_init();			// IIC初始化
	__delay_ms(100);
	while(1)
	{
		/*
		GXHT30_single_call();			// 单次读取温湿度
		bit_Value();					// 数值转换
		__delay_ms(800);
		*/
		cPacketReady = Rs485Process();	// 数据包效用判断
		if(!cPacketReady)
		{
			Rs485GetPacket(&wCmd, &wReAddr[0], &wReValue[0], &wReData[0]);	// 传递数据包
						// 控制命令， 寄存器地址， 寄存器数量， 寄存器数据
			switch(wCmd)
			{
				case	0x03:	// 读取保持寄存器
					if(wReAddr[0] == 0x01 && wReValue[0] == 0x00)	// 保持寄存器 寄存器地址高位为1，寄存器数量高位为0
					{
						if(wReAddr[1] == 0x01)	// 设备地址 寄存器低位判断
						{
							switch(wReValue[1])	// 寄存器数量
							{
								case	0x01:
									EEPROM_OADDR_EDATA[1] = EEPROM_READ(EEPROM_OADDR_EADDR[1]);
									Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_OADDR_EDATA[0]);
								break;
								case	0x02:
									EEPROM_OADDR_EDATA[1] = EEPROM_READ(EEPROM_OADDR_EADDR[1]);
									EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
									sValue[0] = EEPROM_OADDR_EDATA[0];
									sValue[1] = EEPROM_OADDR_EDATA[1];
									sValue[2] = EEPROM_BUAD_EDATA[0];
									sValue[3] = EEPROM_BUAD_EDATA[1];
									Rs485SendPacket(0x03, 4, 0, 0, &sValue[0]);
								break;
								case	0x03:
									EEPROM_OADDR_EDATA[1] = EEPROM_READ(EEPROM_OADDR_EADDR[1]);
									EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
									EEPROM_TEMPAMEND_EDATA[1] = EEPROM_READ(EEPROM_TEMPAMEND_EADDR[1]);
									sValue[0] = EEPROM_OADDR_EDATA[0];
									sValue[1] = EEPROM_OADDR_EDATA[1];
									sValue[2] = EEPROM_BUAD_EDATA[0];
									sValue[3] = EEPROM_BUAD_EDATA[1];
									sValue[4] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[5] = EEPROM_TEMPAMEND_EDATA[1];
									Rs485SendPacket(0x03, 6, 0, 0, &sValue[0]);
								break;
								case	0x04:
									EEPROM_OADDR_EDATA[1] = EEPROM_READ(EEPROM_OADDR_EADDR[1]);
									EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
									EEPROM_TEMPAMEND_EDATA[1] = EEPROM_READ(EEPROM_TEMPAMEND_EADDR[1]);
									EEPROM_HUMIAMEND_EDATA[1] = EEPROM_READ(EEPROM_HUMIAMEND_EADDR[1]);
									sValue[0] = EEPROM_OADDR_EDATA[0];
									sValue[1] = EEPROM_OADDR_EDATA[1];
									sValue[2] = EEPROM_BUAD_EDATA[0];
									sValue[3] = EEPROM_BUAD_EDATA[1];
									sValue[4] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[5] = EEPROM_TEMPAMEND_EDATA[1];
									sValue[6] = EEPROM_HUMIAMEND_EDATA[0];
									sValue[7] = EEPROM_HUMIAMEND_EDATA[1];
									Rs485SendPacket(0x03, 8, 0, 0, &sValue[0]);
								break;
							}
						}
						else if(wReAddr[1] == 0x02)	// 波特率
						{
							switch(wReValue[1])	// 寄存器数量
							{
								case	0x01:
									EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
									Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_BUAD_EDATA[0]);
								break;
								case	0x02:
									EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
									EEPROM_TEMPAMEND_EDATA[1] = EEPROM_READ(EEPROM_TEMPAMEND_EADDR[1]);
									sValue[0] = EEPROM_BUAD_EDATA[0];
									sValue[1] = EEPROM_BUAD_EDATA[1];
									sValue[2] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[3] = EEPROM_TEMPAMEND_EDATA[1];
									Rs485SendPacket(0x03, 4, 0, 0, &sValue[0]);
								break;
								case	0x03:
									EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
									EEPROM_TEMPAMEND_EDATA[1] = EEPROM_READ(EEPROM_TEMPAMEND_EADDR[1]);
									EEPROM_HUMIAMEND_EDATA[1] = EEPROM_READ(EEPROM_HUMIAMEND_EADDR[1]);
									sValue[0] = EEPROM_BUAD_EDATA[0];
									sValue[1] = EEPROM_BUAD_EDATA[1];
									sValue[2] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[3] = EEPROM_TEMPAMEND_EDATA[1];
									sValue[4] = EEPROM_HUMIAMEND_EDATA[0];
									sValue[5] = EEPROM_HUMIAMEND_EDATA[1];
									Rs485SendPacket(0x03, 6, 0, 0, &sValue[0]);
								break;
							}
						}
						else if(wReAddr[1] == 0x03)	// 温度修正值
						{
							switch(wReValue[1])	// 寄存器数量
							{
								case	0x01:
									EEPROM_TEMPAMEND_EDATA[1] = EEPROM_READ(EEPROM_TEMPAMEND_EADDR[1]);
									Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_TEMPAMEND_EDATA[0]);
								break;
								case	0x02:
									EEPROM_TEMPAMEND_EDATA[1] = EEPROM_READ(EEPROM_TEMPAMEND_EADDR[1]);
									EEPROM_HUMIAMEND_EDATA[1] = EEPROM_READ(EEPROM_HUMIAMEND_EADDR[1]);
									sValue[4] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[5] = EEPROM_TEMPAMEND_EDATA[1];
									sValue[6] = EEPROM_HUMIAMEND_EDATA[0];
									sValue[7] = EEPROM_HUMIAMEND_EDATA[1];
									Rs485SendPacket(0x03, 4, 0, 0, &sValue[0]);
								break;
							}
						}
						else if(wReAddr[1] == 0x04)	// 湿度修正值
						{
							if(wReValue[1] == 0x01)
							{
								EEPROM_HUMIAMEND_EDATA[1] = EEPROM_READ(EEPROM_HUMIAMEND_EADDR[1]);
								Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_HUMIAMEND_EDATA[0]);
							}
						}
					}
				break;
				case	0x04:	// 读取输入寄存器
					if(wReAddr[0] == 0x00)	// 输入寄存器
					{
						if(wReAddr[1] == 0x01 && wReValue[1] == 0x01)	// 温度值
						{
							Rs485SendPacket(0x04, 2, 0, 0, &Temp[0]);
						}
						else if(wReAddr[1] == 0x02 && wReValue[1] == 0x01)	// 湿度值
						{
							Rs485SendPacket(0x04, 2, 0, 0, &Humi[0]);
						}
						else if(wReAddr[1] == 0x01 && wReValue[1] == 0x02)	// 温湿度值
						{
							sValue[0] = Temp[0];
							sValue[1] = Temp[1];
							sValue[2] = Humi[0];
							sValue[3] = Humi[1];
							Rs485SendPacket(0x04, 4, 0, 0, &sValue[0]);
						}
					}
				break;
				case	0x06:	// 写入单个保持寄存器
					if(wReAddr[0] == 0x01)	// 保持寄存器
					{
						// 设备地址 波特率 温度修正值 湿度修正值
						if(wReAddr[1] == 0x01 || wReAddr[1] == 0x02 || wReAddr[1] == 0x03 || wReAddr[1] == 0x04)
						{
							if(wReAddr[1] == 0x01)	// 设备地址范围：1 ~ 247
							{
								if(wReData[1] == 0 || wReData[1] > 247)
								{
									break;
								}
								else
								{
									EEPROM_WRITE(wReAddr[1], wReData[1]);
									Rs485SendPacket(0x06, 0, &wReAddr[0], 0, &wReData[0]);
									Rs485Initialise(wReData[1]);
								}
							}
							else
							{
								EEPROM_WRITE(wReAddr[1], wReData[1]);
								Rs485SendPacket(0x06, 0, &wReAddr[0], 0, &wReData[0]);
							}
						}
					}
				break;
				case	0x10:	// 写入多个保持寄存器
					if(wReAddr[0] == 0x01)	// 保持寄存器
					{
						if(wReAddr[1] == 0x01)	// 设备地址
						{
							if(wReValue[0] == 0x00)
							{
								switch(wReValue[1])
								{
									case	1:
										EEPROM_WRITE(EEPROM_OADDR_EADDR[1], wReData[1]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	2:
										EEPROM_WRITE(EEPROM_OADDR_EADDR[1], wReData[1]);
										EEPROM_WRITE(EEPROM_BUAD_EADDR[1], wReData[3]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	3:
										EEPROM_WRITE(EEPROM_OADDR_EADDR[1], wReData[1]);
										EEPROM_WRITE(EEPROM_BUAD_EADDR[1], wReData[3]);
										EEPROM_WRITE(EEPROM_TEMPAMEND_EADDR[1], wReData[5]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	4:
										EEPROM_WRITE(EEPROM_OADDR_EADDR[1], wReData[1]);
										EEPROM_WRITE(EEPROM_BUAD_EADDR[1], wReData[3]);
										EEPROM_WRITE(EEPROM_TEMPAMEND_EADDR[1], wReData[5]);
										EEPROM_WRITE(EEPROM_HUMIAMEND_EADDR[1], wReData[7]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
								}
							}
						}
						else if(wReAddr[1] == 0x02)	// 波特率
						{
							if(wReValue[0] == 0x00)
							{
								switch(wReValue[1])
								{
									case	1:
										EEPROM_WRITE(EEPROM_BUAD_EADDR[1], wReData[1]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	2:
										EEPROM_WRITE(EEPROM_BUAD_EADDR[1], wReData[1]);
										EEPROM_WRITE(EEPROM_TEMPAMEND_EADDR[1], wReData[3]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	3:
										EEPROM_WRITE(EEPROM_BUAD_EADDR[1], wReData[1]);
										EEPROM_WRITE(EEPROM_TEMPAMEND_EADDR[1], wReData[3]);
										EEPROM_WRITE(EEPROM_HUMIAMEND_EADDR[1], wReData[5]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
								}
							}
						}
						else if(wReAddr[1] == 0x03)	// 温度修正值
						{
							if(wReValue[0] == 0x00)
							{
								switch(wReValue[1])
								{
									case	1:
										EEPROM_WRITE(EEPROM_TEMPAMEND_EADDR[1], wReData[1]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	2:
										EEPROM_WRITE(EEPROM_TEMPAMEND_EADDR[1], wReData[1]);
										EEPROM_WRITE(EEPROM_HUMIAMEND_EADDR[1], wReData[3]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
								}
							}
						}
						else if(wReAddr[1] == 0x04)	// 湿度修正值
						{
							if(wReValue[0] == 0x00)
							{
								if(wReValue[1] == 0x01)
								{
									EEPROM_WRITE(EEPROM_HUMIAMEND_EADDR[1], wReData[1]);
									Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
								}
							}
						}
					}
				break;
				default:	// 测试
					sValue[0] = 0xFF;
					sValue[1] = 0xFF;
					Rs485SendPacket(0x10, 8, &wReAddr[0], &wReValue[0], &sValue[0]);
				break;
			}
		}
	}
}

void bit_Value(void)	// 数值拆分函数
{
	int T1 = 0, H1 = 0;
	T1 = (int)Temperature * 10;
	H1 = (int)Humidity * 10;
	Temp[0] = (char)(T1 >> 8);
	Temp[1] = (char)(T1 & 0x00FF);
	Humi[0] = (char)(H1 >> 8);
	Humi[1] = (char)(H1 & 0x00FF);
}

void __interrupt() UsartInterruptISR(void)	// 中断处理函数
{	
	if(PIR1bits.TMR1IF == 1)
	{
		PIE1bits.TMR1IE = 0;
		PIR1bits.TMR1IF = 0;
		Time1++;
		if(Time1 == 10)
		{
			GXHT30_single_call();			// 单次读取温湿度
			bit_Value();					// 数值转换
			Time1 = 0;
		}
		PIE1bits.TMR1IE = 1;
	}
	while(PIR1bits.RCIF == 1)
	{
		INTCONbits.GIE = 0;
		if((RCSTAbits.FERR == 0) && (RCSTAbits.OERR == 0))
		{
			cRs485RxChar = RCREG;
			Rs485Decode();	// 解码
			//RCSTAbits.CREN = 1;
		}
		else
		{
			RCSTAbits.CREN = 0;
			cRs485RxChar = RCREG;
			cRs485RxChar = RCREG;
			RCSTAbits.CREN = 1;
		}
		INTCONbits.GIE = 1;
	}
	
}
