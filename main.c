#include <xc.h>
#include <stdlib.h>
#include "rs485.h"
#include "GXHT30.h"
#include "MB1.h"

extern int Temperature;		// 温度
extern int Humidity;		// 湿度
extern char cOurAddr;		// 从机地址
extern char cRs485RxChar;	// modbus读取数据
extern const char EEPROM_OADDR_EADDR[2];		// 从机地址在EEPROM中的位置
extern const char EEPROM_BUAD_EADDR[2];			// 波特率在EEPROM中的位置
extern const char EEPROM_TEMPAMEND_EADDR[2];	// 温度修正值在EEPROM中的位置
extern const char EEPROM_HUMIAMEND_EADDR[2];	// 湿度修正值在EEPROM中的位置
extern char EEPROM_OADDR_EDATA[2];		// 从机地址在EEPROM中的值
extern char EEPROM_BUAD_EDATA[2];		// 波特率在EEPROM中的值
extern char EEPROM_TEMPAMEND_EDATA[2];	// 温度修正值在EEPROM中的值
extern char EEPROM_HUMIAMEND_EDATA[2];	// 湿度修正值在EEPROM中的值
const unsigned short sBUAD[3] = {415, 207, 103};	// 波特率调节器	4800	9600	19200
static char Temp[2];
static char Humi[2];
static char wCmd;
static char sValue[8];		// 发送数据


void main(void)
{
	char cPacketReady;	// 数据包准备标志
	char wReAddr[2], wReValue[2], wReData[8];	// 地址 数量 数据
	main_init();		// 程序初始化
	IIC_init();			// IIC初始化
	__delay_ms(50);
	while(1)
	{
		GXHT30_single_call();			// 单次读取温湿度
		bit_Value();					// 数值转换
		PORTCbits.RC7 = 0;
		__delay_ms(100);
		
		cPacketReady = Rs485Process();	// 数据包效用判断
		if(!cPacketReady)
		{
			Rs485GetPacket(wCmd, &wReAddr[0], &wReValue[0], &wReData[0]);	// 传递数据包
						// 控制命令， 寄存器地址， 寄存器数量， 寄存器数据
			switch(wCmd)
			{
				case	3:	// 读取保持寄存器
					if(wReAddr[0] == 0x01)	// 保持寄存器
					{
						if(wReAddr[1] == 0x01)	// 设备地址
						{
							switch(wReValue[1])	// 寄存器数量
							{
								case	1:
									EEPROM_Read(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
									Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_OADDR_EDATA[0]);
								break;
								case	2:
									EEPROM_Read(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
									EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
									sValue[0] = EEPROM_OADDR_EDATA[0];
									sValue[1] = EEPROM_OADDR_EDATA[1];
									sValue[2] = EEPROM_BUAD_EDATA[0];
									sValue[3] = EEPROM_BUAD_EDATA[1];
									Rs485SendPacket(0x03, 4, 0, 0, &sValue[0]);
								break;
								case	3:
									EEPROM_Read(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
									EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
									EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
									sValue[0] = EEPROM_OADDR_EDATA[0];
									sValue[1] = EEPROM_OADDR_EDATA[1];
									sValue[2] = EEPROM_BUAD_EDATA[0];
									sValue[3] = EEPROM_BUAD_EDATA[1];
									sValue[4] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[5] = EEPROM_TEMPAMEND_EDATA[1];
									Rs485SendPacket(0x03, 6, 0, 0, &sValue[0]);
								break;
								case	4:
									EEPROM_Read(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
									EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
									EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
									EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
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
								case	1:
									EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
									Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_BUAD_EDATA[0]);
								break;
								case	2:
									EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
									EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
									sValue[0] = EEPROM_BUAD_EDATA[0];
									sValue[1] = EEPROM_BUAD_EDATA[1];
									sValue[2] = EEPROM_TEMPAMEND_EDATA[0];
									sValue[3] = EEPROM_TEMPAMEND_EDATA[1];
									Rs485SendPacket(0x03, 4, 0, 0, &sValue[0]);
								break;
								case	3:
									EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
									EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
									EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
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
								case	1:
									EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
									Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_TEMPAMEND_EDATA[0]);
								break;
								case	2:
									EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
									EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
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
								EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
								Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_HUMIAMEND_EDATA[0]);
							}
						}
					}
				break;
				case	4:	// 读取输入寄存器
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
				case	6:	// 写入单个保持寄存器
					if(wReAddr[0] == 0x01)	// 保持寄存器
					{
						// 设备地址 波特率 温度修正值 湿度修正值
						if(wReAddr[1] == 0x01 || wReAddr[1] == 0x02 || wReAddr[1] == 0x03 || wReAddr[1] == 0x04)
						{
							EEPROM_Write(&wReAddr[0], &wReData[0]);
							Rs485SendPacket(0x06, 0, &wReAddr[0], 0, &wReData[0]);
						}
					}
				break;
				case	10:	// 写入多个保持寄存器
					if(wReAddr[0] == 0x01)	// 保持寄存器
					{
						if(wReAddr[1] == 0x01)	// 设备地址
						{
							if(wReValue[0] == 0x00)
							{
								switch(wReValue[1])
								{
									case	1:
										EEPROM_Write(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	2:
										EEPROM_Write(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
										EEPROM_Write(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	3:
										EEPROM_Write(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
										EEPROM_Write(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
										EEPROM_Write(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	4:
										EEPROM_Write(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);
										EEPROM_Write(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
										EEPROM_Write(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
										EEPROM_Write(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
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
										EEPROM_Write(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	2:
										EEPROM_Write(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
										EEPROM_Write(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	3:
										EEPROM_Write(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);
										EEPROM_Write(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
										EEPROM_Write(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
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
										EEPROM_Write(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
										Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
									break;
									case	2:
										EEPROM_Write(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
										EEPROM_Write(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
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
									EEPROM_Write(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
									Rs485SendPacket(0x10, 0, &wReAddr[0], &wReValue[0], 0);
								}
							}
						}
					}
				break;
				default:	// 测试
					sValue[0] = 0xFF;
					sValue[1] = 0xFF;
					Rs485SendPacket(0x04, 2, 0, 0, &sValue[0]);
				break;
			}
		}
	}
}
void bit_Value(void)	// 数值拆分函数
{
	Temp[0] = (char)((Temperature & 0xFF00) >> 8);
	Temp[1] = (char)(Temperature & 0x00FF);
	Humi[0] = (char)((Humidity & 0xFF00) >> 8);
	Humi[1] = (char)(Humidity & 0x00FF);
}

void __interrupt() UsartInterruptISR(void)	// 中断处理函数
{	
	while(PIR1bits.RCIF == 1)
	{
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
	}
}

void EEPROM_Write(char const *eAddr, char *eData)	// 将数据写入EEPROM
{
	INTCONbits.GIE = 0;
	EEADRH = eAddr[0];			// 写地址
	EEADRL = eAddr[1];
	EEDATH = eData[0];			// 写数据
	EEDATL = eData[1];
	EECON1bits.EEPGD = 0;		// 访问数据EEPROM存储器
	EECON1bits.WREN = 1;		// 允许编程/擦除周期
	EECON2 = 0x55;		
	EECON2 = 0xAA;
	EECON1bits.WR = 1;			// 启动写操作
	while(EECON1bits.WR == 1);	// 等待写操作结束
	EECON1bits.WREN = 0;		// 写禁止
	INTCONbits.GIE = 1;
	
}

void EEPROM_Read(char const *eAddr, char *eData)	// 将数据读出EEPROM
{
	INTCONbits.GIE = 0;
	EEADRH = eAddr[0];			// 写地址
	EEADRL = eAddr[1];
	EECON1bits.EEPGD = 0;		// 访问数据EEPROM存储器
	EECON1bits.RD = 1;			// 读控制位
	while(EECON1bits.RD == 0);	// 等待读取完成
	eData[0] = EEDATH;			// 读数据
	eData[1] = EEDATL;
	INTCONbits.GIE = 1;
	
}

