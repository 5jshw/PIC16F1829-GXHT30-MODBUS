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
	char EEPROM_OADDR_EDATA[2] = {0, 0};
	// 波特率在EEPROM中的值
	char EEPROM_BUAD_EDATA[2] = {0, 0};
	// 温度修正值在EEPROM中的值
	char EEPROM_TEMPAMEND_EDATA[2] = {0, 0};
	// 湿度修正值在EEPROM中的值
	char EEPROM_HUMIAMEND_EDATA[2] = {0, 0};
	
	extern const unsigned short sBUAD[3];	// 波特率调节器	4800	9600	19200
	
void main_init(void)
{
	// 本机地址默认值
	EEPROM_Read(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);	// 读取本机地址
	if(EEPROM_OADDR_EDATA[1] == 0)	EEPROM_OADDR_EDATA[1] = 1;	// 初次使用时，为本机设置默认地址
	Rs485Initialise(0x01);	// 重置modbus通信
	// 波特率默认值
	EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);		// 读取波特率
	if(EEPROM_BUAD_EDATA[1] == 0)	EEPROM_BUAD_EDATA[1] = 1;
	// 温度修正值的默认值
	EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
	// 湿度修正值的默认值
	EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);

	T_DI = 0;
	T_RO = 1;
	DI = 0;	
	RO = 0;
	OSCCON = 0b01110000;	// 设置振荡器为8MHz, 禁止PLL(倍频)
	ADCON0 = 0x10;	// 关闭AD转换模块
	ANSELA = 0;		// 关闭AD接口
	OPTION_REG = 0b11010011;	// 禁止弱上拉 1:16预分频
	APFCON0bits.RXDTSEL = 1;	// 将设定为接收RC5
	APFCON0bits.TXCKSEL = 1;	// 将设定为发送RC4
	TXSTAbits.BRGH = 1;		// 高速波特率
	BAUDCONbits.BRG16 = 1;	// 使用 16 位波特率发生器
	//SPBRG = sBUAD[EEPROM_BUAD_EDATA[1]];	// 8MHz下 默认9600
	SPBRG = 207;
	TXSTAbits.SYNC = 0;		// 异步模式
	RCSTAbits.SPEN = 1;		// 使能异步串口
	RCSTAbits.RX9 = 0;		// 关闭第9位接收
	TXSTAbits.TX9 = 0;		// 关闭第9位发送
	TXSTAbits.TXEN = 1;		// 发送使能
	RCSTAbits.CREN = 1;		// 连续接收使能
	
	// 中断设置
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	//INTCONbits.TMR0IE = 1;
	PIE1bits.RCIE = 1;
	
	TRISCbits.TRISC7 = 0;
	PORTCbits.RC7 = 0;
}

