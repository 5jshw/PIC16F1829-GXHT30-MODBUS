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
char EEPROM_OADDR_EDATA[2];
// ��������EEPROM�е�ֵ
char EEPROM_BUAD_EDATA[2];
// �¶�����ֵ��EEPROM�е�ֵ
char EEPROM_TEMPAMEND_EDATA[2];
// ʪ������ֵ��EEPROM�е�ֵ
char EEPROM_HUMIAMEND_EDATA[2];

const unsigned short sBUAD[3] = {415, 207, 103};	// �����ʵ�����	4800 9600 19200	

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
	Rs485Initialise(EEPROM_OADDR_EDATA[1]);	// ����modbusͨ��
	OSCCON = 0b01110010;	// ��������Ϊ8MHz, ��ֹPLL(��Ƶ), �ڲ�����
	ANSELA = 0;				// �ر�AD�ӿ� A
	ANSELB = 0;				// �ر�AD�ӿ� B

	TRISCbits.TRISC7 = 0;
	APFCON0bits.RXDTSEL = 1;	// ���趨Ϊ����RC5
	APFCON0bits.TXCKSEL = 1;	// ���趨Ϊ����RC4
	TXSTAbits.BRGH = 1;		// ���ٲ�����
	BAUDCONbits.BRG16 = 1;	// ʹ�� 16 λ�����ʷ�����
	SPBRG = sBUAD[(EEPROM_BUAD_EDATA[1] - 1)];	// 8MHz�� Ĭ��9600

	TXSTAbits.SYNC = 0;		// �첽ģʽ
	RCSTAbits.SPEN = 1;		// ʹ���첽����
	RCSTAbits.RX9 = 0;		// �رյ�9λ����
	TXSTAbits.TX9 = 0;		// �رյ�9λ����
	TXSTAbits.TXEN = 1;		// ����ʹ��
	RCSTAbits.CREN = 1;		// ��������ʹ��
	// �ж�����
	INTCONbits.GIE = 1;		// ȫ���ж�
	INTCONbits.PEIE = 1;	// �����ж�
	PIE1bits.RCIE = 1;		// �����ж�
	
	PIE1bits.TMR1IE = 1;	// ʱ���ж�
	
	T1CONbits.T1CKPS = 2;	// 1:4Ԥ��Ƶ
	T1CONbits.T1OSCEN = 1;	// ר��������·
	TMR1H = 0;				// ���ü���
	TMR1L = 0;
	PIR1bits.TMR1IF = 0;	// ���ʱ���жϱ�־
	T1CONbits.TMR1ON = 1;	// ʹ��ʱ��

}

