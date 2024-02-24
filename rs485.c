#include <xc.h>
#include <stdlib.h>
#include "rs485.h"
#include "MB1.h"

char cOurAddr;			// 暂存将发送数据的指定从机地址
char cRs485RxChar;		// 暂存从RS485接收到的单个字符
static char cRS485State;// 暂存RS485数据处理的当前状态
static char cNetAddr;	// 暂存接收到的指定从机地址
static char cCommand;	// 暂存数据包中的命令字节
static char cDataLen;	// 暂存数据长度
static char cRxCrcHigh, cRxCrcLow;		// 暂存接收到的数据包的CRC校验和的高位和低位
static char cCalcCrcHigh, cCalcCrcLow;	// 暂存计算出的CRC校验和的高位和低位
static char cBufPtr;	// 一个指针 用于跟踪在数组中当前的写入位置
static char cReAddr[2];	// 暂存寄存器地址
static char cReValue[2];// 暂存寄存器数量
static char cReData[8];	// 暂存寄存器数据
static char CRC_Value[16], CRC_Len;


void Rs485Initialise(char cAddr)	// 初始化 RS485 网络驱动程序 同时指定将发送数据的从机地址
{
  cOurAddr = cAddr;				// 存储将发送数据的指定从机地址低字节
  cRS485State = PKT_WAIT_ADDR;	// 初始化状态机指针
  T_RO = 1;
  PIE1bits.RCIE = 1;			// 开启接收中断
}

char Rs485Process(void)		// 数据包效用判断
{
	char cPktReady;			// 数据包状态
	cPktReady = FALSE;		// 若地址错误 则为假
	if(cRS485State == PKT_COMPLETE) // 检查数据包是否轮询完毕
	{
		if(cNetAddr == cOurAddr)	// 检查接收的地址位是否对应指定从机的地址位
		{
			cRS485State = PostValidataPacket();	// 验证CRC数据包 确保接收的地址位（数据）是正确的
			if(cRS485State == PKT_INVALID)		// 若CRC校验后无效
			{
				cPktReady = FALSE;				// 数据包未准备好
				cRS485State = PKT_WAIT_ADDR;	// 将状态机指针复位 等待下次接收
			}
			else if(cRS485State == PKT_VALID)	// 若CRC校验后有效
			{
				cPktReady = TRUE;				// 地址正确 CRC校验正确 表示已做好解包准备
				cRS485State = PKT_WAIT_ADDR;	// 将状态机指针复位 等待下次接收
			}
		}
		else
		{
			cPktReady = FALSE;
			cRS485State = PKT_WAIT_ADDR;	// 将状态机指针复位 等待下次接收
		}
	}
	INTCONbits.GIE = 1;		// 开启全局中断
	return cPktReady;		// 返回一个值 表示是否准备好此次通讯
}

char Rs485Decode(void)		// 解码从485接收到的数据包
{
	switch(cRS485State)	// 根据状态机的当前状态进行解码
	{
		// 等待接收从机地址
		case PKT_WAIT_ADDR:		// 0
								cNetAddr = cRs485RxChar; // 存储接收到的从机地址
								cRS485State++; // 移动到下一个状态
								break;
		// 等待接收控制命令字节
		case PKT_CMD:			// 1
								cCommand = cRs485RxChar;	// 存储控制命令字节
								cRS485State++;
								break;
		// 等待接收寄存器地址
		case PKT_RE_ADDR:		// 2
								cReAddr[cBufPtr] = cRs485RxChar;
								if(cBufPtr == 1)
								{
									cBufPtr = 0;
									if(cCommand == WRITE_SHOLDRE)	// 写入单个保持寄存器 0x06
									{
										cRS485State = PKT_RE_DATA2;	// 跳转到双字节数据接收模块
									}
									else if(cCommand == WRITE_MHOLDRE)	// 写入多个保持寄存器 0x10
									{
										cRS485State = PKT_RE_VALUE;	// 跳转到寄存器数量接收模块
									}
									else	// 读取输入或保持寄存器 0x03	0x04
									{
										cRS485State = PKT_RE_VALUE;	// 跳转到寄存器数量接收模块
									}
								}
								else
								{
									cBufPtr++;
								}
								break;
		// 等待接收寄存器数量
		case PKT_RE_VALUE:		// 3
								cReValue[cBufPtr] = cRs485RxChar;
								if(cBufPtr == 1)	// 根据控制命令来判断是读取还是写入，双字节还是多字节
								{
									cBufPtr = 0;
									if(cCommand == WRITE_MHOLDRE)	// 写入多个保持寄存器
									{
										cRS485State = PKT_DATA_LEN;	// 跳转到字节长度接收模块
									}
									else	// 0x03 0x04
									{
										cRS485State = PKT_WAIT_CRC_HIGH;	// 其余控制命令跳转到CRC接收模块
									}
								}
								else
								{
									cBufPtr++;
								}
								break;
		// 接收单个保持寄存器数据
		case PKT_RE_DATA2:		// 4
								cReData[cBufPtr] = cRs485RxChar;
								if(cBufPtr == 1)
								{
									cBufPtr = 0;
									cRS485State = PKT_WAIT_CRC_HIGH;	// 跳转到CRC接收模块
								}
								else
								{
									cBufPtr++;
								}
								break;
		// 接收多寄存器字节长度		
		case PKT_DATA_LEN:		// 12
								cDataLen = cRs485RxChar;
								cRS485State = PKT_RE_DATAS;	// 跳转到寄存器多字节数据接收模块
								break;
								
		// 接收多个保持寄存器数据
		case PKT_RE_DATAS:		// 5
								cReData[cBufPtr] = cRs485RxChar;	// 将接收到的数据存储到缓冲区
								cBufPtr++; //移动缓冲区指针
								if(cBufPtr == (cDataLen - 1))				// 若接收的数据长度达到预期数据长度
								{
									cBufPtr = 0;
									cRS485State = PKT_WAIT_CRC_HIGH;// 跳转到CRC接收模块
								}
								break;
		// 等待接收CRC高字节
		case PKT_WAIT_CRC_HIGH:	// 6
								cRxCrcHigh = cRs485RxChar;	// 存储CRC高字节
								cRS485State++;	// 移动到下一个字节
								break;
		// 等待接收CRC低字节
		case PKT_WAIT_CRC_LOW:	// 7
								cRxCrcLow = cRs485RxChar;	// 存储CRC低字节
								cRS485State = PKT_COMPLETE;	// 将状态机设置为已完成接收状态
								
								break;
		// 数据包已接收完毕 处于空闲状态
		case PKT_COMPLETE:		// 8
								break;	// 不执行任何操作
		case PKT_VALID:			// 9
								break;	// 不执行任何操作
		case PKT_INVALID:		// 255
								break;	// 不执行任何操作
		// 任何其他情况下 重置起始状态
		default:
								break;
	}
	return cRS485State; // 返回当前的状态机状态
}

void Rs485SendPacket(char sCmd, char sLen, char *sReAddr, char *sReValue, char *sReData) // 通过RS485链路发送数据包
{
	unsigned int CRC16;
	char i;
	PIE1bits.RCIE = 0;	// 关闭接收中断
	T_RO = 0;
	__delay_ms(2);		// 缓冲时间
	// CRC校验前准备
	CRC_Value[0] = cOurAddr;
	CRC_Value[1] = sCmd;
	switch(CRC_Value[1])
	{
		case	0x04:	// 读输入寄存器
			CRC_Value[2] = sLen;
			switch(CRC_Value[2])
			{
				case	2:
					CRC_Value[3] = sReData[0];
					CRC_Value[4] = sReData[1];
					CRC_Len = 5;
				break;
				case	4:
					CRC_Value[3] = sReData[0];
					CRC_Value[4] = sReData[1];
					CRC_Value[5] = sReData[2];
					CRC_Value[6] = sReData[3];
					CRC_Len = 7;
				break;
			}
		break;
		case	0x03:	// 读保持寄存器
			CRC_Value[2] = sLen;
			switch(CRC_Value[2])
			{
				case	2:
					CRC_Value[3] = sReData[0];
					CRC_Value[4] = sReData[1];
					CRC_Len = 5;
				break;
				case	4:
					CRC_Value[3] = sReData[0];
					CRC_Value[4] = sReData[1];
					CRC_Value[5] = sReData[2];
					CRC_Value[6] = sReData[3];
					CRC_Len = 7;
				break;
				case	6:
					CRC_Value[3] = sReData[0];
					CRC_Value[4] = sReData[1];
					CRC_Value[5] = sReData[2];
					CRC_Value[6] = sReData[3];
					CRC_Value[7] = sReData[4];
					CRC_Value[8] = sReData[5];
					CRC_Len = 9;
				break;
				case	8:
					CRC_Value[3] = sReData[0];
					CRC_Value[4] = sReData[1];
					CRC_Value[5] = sReData[2];
					CRC_Value[6] = sReData[3];
					CRC_Value[7] = sReData[4];
					CRC_Value[8] = sReData[5];
					CRC_Value[9] = sReData[6];
					CRC_Value[10] = sReData[7];
					CRC_Len = 11;
				break;
			}
		break;
		case	0x06:	// 写单个保持寄存器
			CRC_Value[2] = sReAddr[0];
			CRC_Value[3] = sReAddr[1];
			CRC_Value[4] = sReData[0];
			CRC_Value[5] = sReData[1];
			CRC_Len = 6;
		break;
		case	0x10:	// 写多个保持寄存器
			CRC_Value[2] = sReAddr[0];
			CRC_Value[3] = sReAddr[1];
			CRC_Value[4] = sReValue[0];
			CRC_Value[5] = sReValue[1];
			CRC_Len = 6;
		break;
	}
	CRC16 = ModBusCRC16(&CRC_Value[0], CRC_Len);
	for(i = 0; i < CRC_Len; i++)
	{
		Rs485SendChar(CRC_Value[i]);
	}
	Rs485SendChar((char)((CRC16 & 0xFF00) >> 8));
	Rs485SendChar((char)(CRC16 & 0x00FF));

	__delay_ms(1);
	T_RO = 1;
	PIE1bits.RCIE = 1;		// 打开接收中断
}

void Rs485GetPacket(char *gCmd, char *gReAddr, char *gReValue, char *gReData) // 将数据传递给主程序
{
	char c;				// 循环计数
	*gCmd = cCommand;	// 传递控制命令
	gReAddr[0] = cReAddr[0];	// 传递寄存器地址
	gReAddr[1] = cReAddr[1];
	gReValue[0] = cReValue[0];	// 传递寄存器数量
	gReValue[1] = cReValue[1];
	for(c = 0; c < 8; c++)		// 传递寄存器数据
	{
		gReData[c] = cReData[c];
	}
}

unsigned int ModBusCRC16(char *data, char len)	// CRC16-MODBUS校验
{
    unsigned int i, j, tmp, CRC16;

    CRC16 = 0xFFFF;             //CRC寄存器初始值
    for (i = 0; i < len; i++)
    {
        CRC16 ^= data[i];
        for (j = 0; j < 8; j++)
        {
            tmp = (unsigned int)(CRC16 & 0x0001);
            CRC16 >>= 1;
            if (tmp == 1)
            {
                CRC16 ^= 0xA001;    //异或多项式
            }
        }
    }
    return CRC16;
}

void Rs485SendChar(char c) // 发送一字节
{
	TXREG = c;						// 加载数据以发送
	while (!(TXSTA & TRMT_MASK));	// 等待发送完成
}

char PostValidataPacket(void) // 验证接收的数据CRC校验码
{
	unsigned int CRC16;
	ReadCRCValue();
	CRC16 = ModBusCRC16(&CRC_Value[0], CRC_Len);

	if((cRxCrcHigh == ((CRC16 & 0x00FF))) && (cRxCrcLow == ((CRC16 & 0xFF00) >> 8)))	// 验证校验CRC与接收CRC是否相等
	{
		cRS485State = PKT_VALID;	// 返回有效值	
	}
	else	
	{
		cRS485State = PKT_INVALID;	// 返回无效值
	}
	return cRS485State; // 返回计算后状态机指示器
}

void ReadCRCValue(void)	// 接收数据进行CRC校验前准备工作
{
	CRC_Value[0] = cNetAddr;
	CRC_Value[1] = cCommand;
	CRC_Value[2] = cReAddr[0];
	CRC_Value[3] = cReAddr[1];
	switch(cCommand)
	{
		case	0x04:	// 读输入寄存器
		{
			CRC_Value[4] = cReValue[0];
			CRC_Value[5] = cReValue[1];
			CRC_Len = 6;
		}
		break;
		case	0x03:	// 读保持寄存器
		{
			CRC_Value[4] = cReValue[0];
			CRC_Value[5] = cReValue[1];
			CRC_Len = 6;
		}
		break;
		case	0x06:	// 写单个保持寄存器
		{
			CRC_Value[4] = cReData[0];
			CRC_Value[5] = cReData[1];
			CRC_Len = 6;
		}
		break;
		case	0x10:	// 写多个保持寄存器
		{
			CRC_Value[4] = cReValue[0];
			CRC_Value[5] = cReValue[1];
			CRC_Value[6] = cDataLen;
			switch(CRC_Value[6])
			{
				case	2:
					CRC_Value[7] = cReData[0];
					CRC_Value[8] = cReData[1];
					CRC_Len = 9;
					break;
				case	4:
					CRC_Value[7] = cReData[0];
					CRC_Value[8] = cReData[1];
					CRC_Value[9] = cReData[2];
					CRC_Value[10] = cReData[3];
					CRC_Len = 11;
					break;
				case	6:
					CRC_Value[7] = cReData[0];
					CRC_Value[8] = cReData[1];
					CRC_Value[9] = cReData[2];
					CRC_Value[10] = cReData[3];
					CRC_Value[11] = cReData[4];
					CRC_Value[12] = cReData[5];
					CRC_Len = 13;
					break;
				case	8:
					CRC_Value[7] = cReData[0];
					CRC_Value[8] = cReData[1];
					CRC_Value[9] = cReData[2];
					CRC_Value[10] = cReData[3];
					CRC_Value[11] = cReData[4];
					CRC_Value[12] = cReData[5];
					CRC_Value[13] = cReData[6];
					CRC_Value[14] = cReData[7];
					CRC_Len = 15;
					break;
			}
		}
		break;
	}
}


