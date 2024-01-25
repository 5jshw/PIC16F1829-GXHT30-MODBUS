#include <xc.h>
#include "MB1.h"
#include "GXHT30.h"
#include "rs485.h"

	// �ӻ���ַ��EEPROM�е�λ��
	const char EEPROM_OADDR_EADDR[2] = {0x01, 0x01};
	// ��������EEPROM�е�λ��
	const char EEPROM_BUAD_EADDR[2] = {0x01, 0x02};
	// �¶�����ֵ��EEPROM�е�λ��
	const char EEPROM_TEMPAMEND_EADDR[2] = {0x01, 0x03};
	// ʪ������ֵ��EEPROM�е�λ��
	const char EEPROM_HUMIAMEND_EADDR[2] = {0x01, 0x04};
	
	// �ӻ���ַ��EEPROM�е�ֵ
	char EEPROM_OADDR_EDATA[2] = {0, 0};
	// ��������EEPROM�е�ֵ
	char EEPROM_BUAD_EDATA[2] = {0, 0};
	// �¶�����ֵ��EEPROM�е�ֵ
	char EEPROM_TEMPAMEND_EDATA[2] = {0, 0};
	// ʪ������ֵ��EEPROM�е�ֵ
	char EEPROM_HUMIAMEND_EDATA[2] = {0, 0};
	
	extern const unsigned short sBUAD[3];	// �����ʵ�����	4800	9600	19200
	
void main_init(void)
{
	// ������ַĬ��ֵ
	EEPROM_Read(&EEPROM_OADDR_EADDR[0], &EEPROM_OADDR_EDATA[0]);	// ��ȡ������ַ
	if(EEPROM_OADDR_EDATA[1] == 0)	EEPROM_OADDR_EDATA[1] = 1;	// ����ʹ��ʱ��Ϊ��������Ĭ�ϵ�ַ
	Rs485Initialise(0x01);	// ����modbusͨ��
	// ������Ĭ��ֵ
	EEPROM_Read(&EEPROM_BUAD_EADDR[0], &EEPROM_BUAD_EDATA[0]);		// ��ȡ������
	if(EEPROM_BUAD_EDATA[1] == 0)	EEPROM_BUAD_EDATA[1] = 1;
	// �¶�����ֵ��Ĭ��ֵ
	EEPROM_Read(&EEPROM_TEMPAMEND_EADDR[0], &EEPROM_TEMPAMEND_EDATA[0]);
	// ʪ������ֵ��Ĭ��ֵ
	EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);

	T_DI = 0;
	T_RO = 1;
	DI = 0;	
	RO = 0;
	OSCCON = 0b01110000;	// ��������Ϊ8MHz, ��ֹPLL(��Ƶ)
	ADCON0 = 0x10;	// �ر�ADת��ģ��
	ANSELA = 0;		// �ر�AD�ӿ�
	OPTION_REG = 0b11010011;	// ��ֹ������ 1:16Ԥ��Ƶ
	APFCON0bits.RXDTSEL = 1;	// ���趨Ϊ����RC5
	APFCON0bits.TXCKSEL = 1;	// ���趨Ϊ����RC4
	TXSTAbits.BRGH = 1;		// ���ٲ�����
	BAUDCONbits.BRG16 = 1;	// ʹ�� 16 λ�����ʷ�����
	//SPBRG = sBUAD[EEPROM_BUAD_EDATA[1]];	// 8MHz�� Ĭ��9600
	SPBRG = 207;
	TXSTAbits.SYNC = 0;		// �첽ģʽ
	RCSTAbits.SPEN = 1;		// ʹ���첽����
	RCSTAbits.RX9 = 0;		// �رյ�9λ����
	TXSTAbits.TX9 = 0;		// �رյ�9λ����
	TXSTAbits.TXEN = 1;		// ����ʹ��
	RCSTAbits.CREN = 1;		// ��������ʹ��
	
	// �ж�����
	INTCONbits.GIE = 1;
	INTCONbits.PEIE = 1;
	//INTCONbits.TMR0IE = 1;
	PIE1bits.RCIE = 1;
	
	TRISCbits.TRISC7 = 0;
	PORTCbits.RC7 = 0;
}

