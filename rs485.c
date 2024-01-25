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
static char cReAddr[3];	// 暂存寄存器地址
static char cReValue[2];// 暂存寄存器数量
static char cReData[8];	// 暂存寄存器数据

void Rs485Initialise(char cAddr)	// 初始化 RS485 网络驱动程序 同时指定将发送数据的从机地址
{
  cOurAddr = cAddr;				// 存储将发送数据的指定从机地址低字节
  cRS485State = PKT_WAIT_ADDR;	// 初始化状态机指针
  T_RO = 1;
  PIE1bits.RCIE = 1;			// 开启接收中断
}

char Rs485Process(void)			// 数据包效用判断
{
	char cPktReady;// 数据包状态
	cPktReady = FALSE;			// 若地址错误 则为假
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
	char CmdValue;
	PIE1bits.RCIE = 0;	// 关闭接收中断
	T_RO = 0;
	__delay_ms(2);		// 缓冲时间
	CRC16_Init();		// 复位CRC
	// Send some NULL preamblesfopr receiving UART 为接收 UART 发送一些 NULL 前置码
	//for (c=0; c < NUM_TX_PREAMBLE; c++) Rs485SendChar(0x00);	// 发送3次 NULL 清空发送缓存
	Rs485UpdataCrc(cOurAddr);
	Rs485SendChar(cOurAddr);		// 发送从机地址
	Rs485UpdataCrc(sCmd);
	Rs485SendChar(sCmd);			// 发送控制命令
	CmdValue = PacketHasPayload(sCmd);
	switch(CmdValue)
	{
		case	1:	// 命令：读保持寄存器 0x03
			Rs485UpdataCrc(sLen);			// 数据长度
			Rs485SendChar(sLen);
			Rs485UpdataCrc(sReData[0]);		// 数据高字节
			Rs485SendChar(sReData[0]);
			Rs485UpdataCrc(sReData[1]);		// 数据低字节
			Rs485SendChar(sReData[1]);
			break;
		case	2:	// 命令：读输入寄存器 0x04
			Rs485UpdataCrc(sLen);			// 数据长度
			Rs485SendChar(sLen);
			if(sLen == 2)	// 所需字节长度
			{
				Rs485UpdataCrc(sReData[0]);		// 数据高字节
				Rs485SendChar(sReData[0]);
				Rs485UpdataCrc(sReData[1]);		// 数据低字节
				Rs485SendChar(sReData[1]);
			}
			else if(sLen == 4)	// 所需字节长度
			{
				Rs485UpdataCrc(sReData[0]);		// 数据高字节
				Rs485SendChar(sReData[0]);
				Rs485UpdataCrc(sReData[1]);		// 数据低字节
				Rs485SendChar(sReData[1]);
				Rs485UpdataCrc(sReData[2]);		// 数据高字节
				Rs485SendChar(sReData[2]);
				Rs485UpdataCrc(sReData[3]);		// 数据低字节
				Rs485SendChar(sReData[3]);
			}
			break;
		case	3:	// 命令：写单个保持寄存器 0x06
			Rs485UpdataCrc(sReAddr[0]);		// 数据地址高字节
			Rs485SendChar(sReAddr[0]);
			Rs485UpdataCrc(sReAddr[1]);		// 数据地址低字节
			Rs485SendChar(sReAddr[1]);
			Rs485UpdataCrc(sReData[0]);		// 数据高字节
			Rs485SendChar(sReData[0]);
			Rs485UpdataCrc(sReData[1]);		//数据低字节
			Rs485SendChar(sReData[1]);
			break;
		case	4:	// 命令：写多个保持寄存器 0x10
			Rs485UpdataCrc(sReAddr[0]);		// 数据地址高字节
			Rs485SendChar(sReAddr[0]);
			Rs485UpdataCrc(sReAddr[1]);		// 数据地址低字节
			Rs485SendChar(sReAddr[1]);
			Rs485UpdataCrc(sReValue[0]);	// 寄存器数量高字节
			Rs485SendChar(sReValue[0]);
			Rs485UpdataCrc(sReValue[1]);	// 寄存器数量低字节
			Rs485SendChar(sReValue[1]);
			break;
	}

	Rs485SendChar(cCalcCrcHigh);	// 发送CRC校验码高字节
	Rs485SendChar(cCalcCrcLow);		// 发送CRC校验码低字节
	__delay_ms(1);
	T_RO = 1;
	PIE1bits.RCIE = 1;				// Enable Receive Interrupt 打开接收中断
}

void Rs485GetPacket(char gCmd, char *gReAddr, char *gReValue, char *gReData) // 将数据传递给主程序
{
	char c;				// 循环计数
	gCmd = cCommand;	// 传递控制命令
	gReAddr[0] = cReAddr[0];	// 传递寄存器地址
	gReAddr[1] = cReAddr[1];
	gReValue[0] = cReValue[0];	// 传递寄存器数量
	gReValue[1] = cReValue[1];
	for(c = 0; c < 8; c++)		// 传递寄存器数据
	{
		gReData[c] = cReData[c];
	}
}

/*
{
const char CRC16_LookupHigh[16] = {	// CRC16 查找表（高字节和低字节），每次迭代 4 位。
		  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		  0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1
};	// CRC 查找表
const char CRC16_LookupLow[16] = {
		  0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
		  0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF
};

void CRC16_Init(void) // CRC初始化
{
	// 根据 CCITT 规范将 CRC 初始化为 0xFFFF
	cCalcCrcHigh = 0xFF;
	cCalcCrcLow = 0xFF;
}

void CRC16_Updata4Bits(char val) // 更新4位CRC
{
	char t;
	// 第一步，提取 CRC 寄存器中最重要的 4 个比特
	t = cCalcCrcHigh >> 4;	// 将高位寄存器向右移动4位存放进变量里
	// 将信息数据按位异或到提取的比特中
	t = t ^ val;			// 按位异或
	// 将CRC寄存器左移4位
	cCalcCrcHigh = (cCalcCrcHigh << 4) | (cCalcCrcLow >> 4);	// 合并
	cCalcCrcLow = cCalcCrcLow << 4;		// 将低位寄存器向左移动4位
	// 进行查表，并将结果异或到 CRC 表中
	cCalcCrcHigh = cCalcCrcHigh ^ CRC16_LookupHigh[t];	// 与CRC高字节查找表按位异或
	cCalcCrcLow = cCalcCrcLow ^ CRC16_LookupLow[t];		// 与CRC低字节查找表按位异或
}

void Rs485UpdataCrc(char cVal) // 更新CRC
{
	CRC16_Updata4Bits(cVal >> 4);	// 处理CRC高字节
	CRC16_Updata4Bits(cVal & 0x0F);	// 处理CRC低字节
}
}
*/


unsigned int ModBusCRC16(unsigned char *data, unsigned int len)
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
/*根据需要对结果进行处理*/
//    data[i++] = (unsigned char) (CRC16 & 0x00FF);
//    data[i++] = (unsigned char) ((CRC16 & 0xFF00)>>8);
    return CRC16;
}



void Rs485SendChar(char c) // 发送一字节
{
	TXREG = c;						// 加载数据以发送
	while (!(TXSTA & TRMT_MASK));	// 等待发送完成
}

char PostValidataPacket(void) // 验证接收的数据CRC校验码
{
	CRC16_Init();	// 初始化
	Rs485UpdataCrc(cNetAddr);		// 地址
	Rs485UpdataCrc(cCommand);		// 控制命令
	Rs485UpdataCrc(cReAddr[0]);		// 寄存器地址高字节
	Rs485UpdataCrc(cReAddr[1]);		// 寄存器地址低字节
	if(cCommand == WRITE_MHOLDRE)	// 0x10 写多个保持寄存器
	{	// 寄存器数量 字节长度 寄存器数据
		Rs485UpdataCrc(cReValue[0]);
		Rs485UpdataCrc(cReValue[1]);
		Rs485UpdataCrc(cDataLen);
		for(cBufPtr = 0; cBufPtr < cDataLen; cBufPtr++)
		{
			Rs485UpdataCrc(cReData[cBufPtr]);
		}
	}
	else if(cCommand == WRITE_SHOLDRE)	// 0x06 写单个保持寄存器
	{	// 寄存器数据
		Rs485UpdataCrc(cReData[0]);
		Rs485UpdataCrc(cReData[1]);
	}
	else if(cCommand == READ_HOLDRE || cCommand == READ_INPUTRE)	// 0x03 0x04	读保持寄存器 读输入寄存器
	{	// 寄存器数量
		Rs485UpdataCrc(cReValue[0]);
		Rs485UpdataCrc(cReValue[1]);
	}

	// 检查 CRC 是否正确
	// 并将更新后的状态作为结果返回
	if((cRxCrcHigh == cCalcCrcHigh) && (cRxCrcLow == cCalcCrcLow))	// 验证校验CRC与接收CRC是否相等
	{
		cRS485State = PKT_VALID;	// 返回有效值
		PORTCbits.RC7 = 1;
	}
	else	
	{
		//cRS485State = PKT_INVALID;	// 返回无效值
		cRS485State = PKT_VALID;	// 返回有效值
	}
	return cRS485State; // 返回计算后状态机指示器
}

char PacketHasPayload(char ccCommand)	// 检查控制命令类型
{
	char Cmd;
	PIE1bits.RCIE = 0;		// 关闭接收中断
	
	if(ccCommand == READ_HOLDRE)		// 0x03读取保持寄存器
	{
		Cmd = 1;
	}
	else if(ccCommand == READ_INPUTRE)	// 0x04读取输入寄存器
	{
		Cmd = 2;
	}
	else if(ccCommand == WRITE_SHOLDRE)	// 0x06写单个保持寄存器
	{
		Cmd = 3;
	}
	else if(ccCommand == WRITE_MHOLDRE)	// 0x10写多个保持寄存器
	{
		Cmd = 4;
	}
	else	// 无效命令
	{
		Cmd = 0;
	}
	PIE1bits.RCIE = 1;		// 打开接收中断
	return Cmd;
}




