#include <xc.h>
#include <stdlib.h>
#include "rs485.h"
#include "GXHT30.h"
#include "MB1.h"

extern int Temperature;		// �¶�
extern int Humidity;		// ʪ��
extern char cOurAddr;		// �ӻ���ַ
extern char cRs485RxChar;	// modbus��ȡ����
extern const char EEPROM_OADDR_EADDR[2];		// �ӻ���ַ��EEPROM�е�λ��
extern const char EEPROM_BUAD_EADDR[2];			// ��������EEPROM�е�λ��
extern const char EEPROM_TEMPAMEND_EADDR[2];	// �¶�����ֵ��EEPROM�е�λ��
extern const char EEPROM_HUMIAMEND_EADDR[2];	// ʪ������ֵ��EEPROM�е�λ��
extern char EEPROM_OADDR_EDATA[2];		// �ӻ���ַ��EEPROM�е�ֵ
extern char EEPROM_BUAD_EDATA[2];		// ��������EEPROM�е�ֵ
extern char EEPROM_TEMPAMEND_EDATA[2];	// �¶�����ֵ��EEPROM�е�ֵ
extern char EEPROM_HUMIAMEND_EDATA[2];	// ʪ������ֵ��EEPROM�е�ֵ
const unsigned short sBUAD[3] = {415, 207, 103};	// �����ʵ�����	4800	9600	19200
static char Temp[2];
static char Humi[2];
static char wCmd;
static char sValue[8];		// ��������


void main(void)
{
	char cPacketReady;	// ���ݰ�׼����־
	char wReAddr[2], wReValue[2], wReData[8];	// ��ַ ���� ����
	main_init();		// �����ʼ��
	IIC_init();			// IIC��ʼ��
	__delay_ms(50);
	while(1)
	{
		GXHT30_single_call();			// ���ζ�ȡ��ʪ��
		bit_Value();					// ��ֵת��
		PORTCbits.RC7 = 0;
		__delay_ms(100);
		
		cPacketReady = Rs485Process();	// ���ݰ�Ч���ж�
		if(!cPacketReady)
		{
			Rs485GetPacket(wCmd, &wReAddr[0], &wReValue[0], &wReData[0]);	// �������ݰ�
						// ������� �Ĵ�����ַ�� �Ĵ��������� �Ĵ�������
			switch(wCmd)
			{
				case	3:	// ��ȡ���ּĴ���
					if(wReAddr[0] == 0x01)	// ���ּĴ���
					{
						if(wReAddr[1] == 0x01)	// �豸��ַ
						{
							switch(wReValue[1])	// �Ĵ�������
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
						else if(wReAddr[1] == 0x02)	// ������
						{
							switch(wReValue[1])	// �Ĵ�������
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
						else if(wReAddr[1] == 0x03)	// �¶�����ֵ
						{
							switch(wReValue[1])	// �Ĵ�������
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
						else if(wReAddr[1] == 0x04)	// ʪ������ֵ
						{
							if(wReValue[1] == 0x01)
							{
								EEPROM_Read(&EEPROM_HUMIAMEND_EADDR[0], &EEPROM_HUMIAMEND_EDATA[0]);
								Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_HUMIAMEND_EDATA[0]);
							}
						}
					}
				break;
				case	4:	// ��ȡ����Ĵ���
					if(wReAddr[0] == 0x00)	// ����Ĵ���
					{
						if(wReAddr[1] == 0x01 && wReValue[1] == 0x01)	// �¶�ֵ
						{
							Rs485SendPacket(0x04, 2, 0, 0, &Temp[0]);
						}
						else if(wReAddr[1] == 0x02 && wReValue[1] == 0x01)	// ʪ��ֵ
						{
							Rs485SendPacket(0x04, 2, 0, 0, &Humi[0]);
						}
						else if(wReAddr[1] == 0x01 && wReValue[1] == 0x02)	// ��ʪ��ֵ
						{
							sValue[0] = Temp[0];
							sValue[1] = Temp[1];
							sValue[2] = Humi[0];
							sValue[3] = Humi[1];
							Rs485SendPacket(0x04, 4, 0, 0, &sValue[0]);
						}
					}
				break;
				case	6:	// д�뵥�����ּĴ���
					if(wReAddr[0] == 0x01)	// ���ּĴ���
					{
						// �豸��ַ ������ �¶�����ֵ ʪ������ֵ
						if(wReAddr[1] == 0x01 || wReAddr[1] == 0x02 || wReAddr[1] == 0x03 || wReAddr[1] == 0x04)
						{
							EEPROM_Write(&wReAddr[0], &wReData[0]);
							Rs485SendPacket(0x06, 0, &wReAddr[0], 0, &wReData[0]);
						}
					}
				break;
				case	10:	// д�������ּĴ���
					if(wReAddr[0] == 0x01)	// ���ּĴ���
					{
						if(wReAddr[1] == 0x01)	// �豸��ַ
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
						else if(wReAddr[1] == 0x02)	// ������
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
						else if(wReAddr[1] == 0x03)	// �¶�����ֵ
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
						else if(wReAddr[1] == 0x04)	// ʪ������ֵ
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
				default:	// ����
					sValue[0] = 0xFF;
					sValue[1] = 0xFF;
					Rs485SendPacket(0x04, 2, 0, 0, &sValue[0]);
				break;
			}
		}
	}
}
void bit_Value(void)	// ��ֵ��ֺ���
{
	Temp[0] = (char)((Temperature & 0xFF00) >> 8);
	Temp[1] = (char)(Temperature & 0x00FF);
	Humi[0] = (char)((Humidity & 0xFF00) >> 8);
	Humi[1] = (char)(Humidity & 0x00FF);
}

void __interrupt() UsartInterruptISR(void)	// �жϴ�����
{	
	while(PIR1bits.RCIF == 1)
	{
		if((RCSTAbits.FERR == 0) && (RCSTAbits.OERR == 0))
		{
			cRs485RxChar = RCREG;
			Rs485Decode();	// ����
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

void EEPROM_Write(char const *eAddr, char *eData)	// ������д��EEPROM
{
	INTCONbits.GIE = 0;
	EEADRH = eAddr[0];			// д��ַ
	EEADRL = eAddr[1];
	EEDATH = eData[0];			// д����
	EEDATL = eData[1];
	EECON1bits.EEPGD = 0;		// ��������EEPROM�洢��
	EECON1bits.WREN = 1;		// ������/��������
	EECON2 = 0x55;		
	EECON2 = 0xAA;
	EECON1bits.WR = 1;			// ����д����
	while(EECON1bits.WR == 1);	// �ȴ�д��������
	EECON1bits.WREN = 0;		// д��ֹ
	INTCONbits.GIE = 1;
	
}

void EEPROM_Read(char const *eAddr, char *eData)	// �����ݶ���EEPROM
{
	INTCONbits.GIE = 0;
	EEADRH = eAddr[0];			// д��ַ
	EEADRL = eAddr[1];
	EECON1bits.EEPGD = 0;		// ��������EEPROM�洢��
	EECON1bits.RD = 1;			// ������λ
	while(EECON1bits.RD == 0);	// �ȴ���ȡ���
	eData[0] = EEDATH;			// ������
	eData[1] = EEDATL;
	INTCONbits.GIE = 1;
	
}

