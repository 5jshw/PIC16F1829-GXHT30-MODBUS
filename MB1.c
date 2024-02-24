#include <xc.h>
#include "MB1.h"
#include "GXHT30.h"
#include "rs485.h"

// 从机地址在EEPROM中的位置
const char EEPROM_OADDR_EADDR[2] = {0x01, 0x01};
// 波特率在EEPROM中的位置
const char EEPROM_BUAD_EADDR[2] = {0x01, 0x02};
// 温度修正值在EEPROM中的位置
const char EEPROM_TEMPAMEND_EADDR[2] = {0x01, 0x03};
// 湿度修正值在EEPROM中的位置
const char EEPROM_HUMIAMEND_EADDR[2] = {0x01, 0x04};

// 从机地址在EEPROM中的值
char EEPROM_OADDR_EDATA[2];
// 波特率在EEPROM中的值
char EEPROM_BUAD_EDATA[2];
// 温度修正值在EEPROM中的值
char EEPROM_TEMPAMEND_EDATA[2];
// 湿度修正值在EEPROM中的值
char EEPROM_HUMIAMEND_EDATA[2];

const unsigned short sBUAD[3] = {415, 207, 103};	// 波特率调节器	4800 9600 19200	

void main_init(void)
{
	__EEPROM_DATA(0, 0, 0, 0, 0, 0, 0, 0);
	//EEPROM_READ();
	EEPROM_BUAD_EDATA[1] = EEPROM_READ(EEPROM_BUAD_EADDR[1]);
	if(EEPROM_BUAD_EDATA[1] == 0)
	{
		EEPROM_BUAD_EDATA[1] = 2;
		EEPROM_WRITE(EEPROM_BUAD_EADDR[1], EEPROM_BUAD_EDATA[1]);
	}
	
	EEPROM_OADDR_EDATA[1] = EEPROM_READ(EEPROM_OADDR_EADDR[1]);
	if(EEPROM_OADDR_EDATA[1] == 0 || EEPROM_OADDR_EDATA[1] > 247)
	{
		EEPROM_OADDR_EDATA[1] = 1;
		EEPROM_WRITE(EEPROM_OADDR_EADDR[1], EEPROM_OADDR_EDATA[1]);
	}
	//EEPROM_OADDR_EDATA[1]
	T_DI = 0;
	T_RO = 1;
	DI = 0;	
	RO = 0;
	Rs485Initialise(EEPROM_OADDR_EDATA[1]);	// 重置modbus通信
	OSCCON = 0b01110010;	// 设置振荡器为8MHz, 禁止PLL(倍频), 内部振荡器
	ANSELA = 0;				// 关闭AD接口 A
	ANSELB = 0;				// 关闭AD接口 B

	TRISCbits.TRISC7 = 0;
	APFCON0bits.RXDTSEL = 1;	// 将设定为接收RC5
	APFCON0bits.TXCKSEL = 1;	// 将设定为发送RC4
	TXSTAbits.BRGH = 1;		// 高速波特率
	BAUDCONbits.BRG16 = 1;	// 使用 16 位波特率发生器
	SPBRG = sBUAD[(EEPROM_BUAD_EDATA[1] - 1)];	// 8MHz下 默认9600

	TXSTAbits.SYNC = 0;		// 异步模式
	RCSTAbits.SPEN = 1;		// 使能异步串口
	RCSTAbits.RX9 = 0;		// 关闭第9位接收
	TXSTAbits.TX9 = 0;		// 关闭第9位发送
	TXSTAbits.TXEN = 1;		// 发送使能
	RCSTAbits.CREN = 1;		// 连续接收使能
	// 中断设置
	INTCONbits.GIE = 1;		// 全局中断
	INTCONbits.PEIE = 1;	// 外设中断
	PIE1bits.RCIE = 1;		// 接收中断
	
	PIE1bits.TMR1IE = 1;	// 时钟中断
	
	T1CONbits.T1CKPS = 2;	// 1:4预分频
	T1CONbits.T1OSCEN = 1;	// 专用振荡器电路
	TMR1H = 0;				// 重置计数
	TMR1L = 0;
	PIR1bits.TMR1IF = 0;	// 清空时钟中断标志
	T1CONbits.TMR1ON = 1;	// 使能时钟

}

