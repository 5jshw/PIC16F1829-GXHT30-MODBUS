#include <xc.h>
#include <stdlib.h>
#include <pic16f1829.h>
#include "rs485.h"
#include "GXHT30.h"
#include "MB1.h"


extern double Temperature;	// �¶�
extern double Humidity;		// ʪ��
extern char cOurAddr;		// �ӻ���ַ
extern char cRs485RxChar;	// modbus��ȡ����
extern const char EEPROM_OADDR_EADDR[2] = {0x01, 0x01};		// �ӻ���ַ��EEPROM�е�λ��
extern const char EEPROM_BUAD_EADDR[2] = {0x01, 0x02};			// ��������EEPROM�е�λ��
extern const char EEPROM_TEMPAMEND_EADDR[2] = {0x01, 0x03};	// �¶�����ֵ��EEPROM�е�λ��
extern const char EEPROM_HUMIAMEND_EADDR[2] = {0x01, 0x04};	// ʪ������ֵ��EEPROM�е�λ��
extern char EEPROM_OADDR_EDATA[2];		// �ӻ���ַ��EEPROM�е�ֵ
extern char EEPROM_BUAD_EDATA[2];		// ��������EEPROM�е�ֵ
extern char EEPROM_TEMPAMEND_EDATA[2];	// �¶�����ֵ��EEPROM�е�ֵ
extern char EEPROM_HUMIAMEND_EDATA[2];	// ʪ������ֵ��EEPROM�е�ֵ
extern const unsigned short sBUAD[3] = {415, 207, 103};	// �����ʵ�����	4800	9600	19200
static char Temp[2];	// �����¶�
static char Humi[2];	// ����ʪ��
static char wCmd;		// ��������
static char sValue[8];	// ��������
static char wReAddr[2], wReValue[2], wReData[8];	// ��ַ ���� ����
static char Time1;

void main(void)
{
	char cPacketReady;	// ���ݰ�׼����־
	main_init();		// �����ʼ��
	IIC_init();			// IIC��ʼ��
	__delay_ms(100);
	while(1)
	{
		/*
		GXHT30_single_call();			// ���ζ�ȡ��ʪ��
		bit_Value();					// ��ֵת��
		__delay_ms(800);
		*/
		cPacketReady = Rs485Process();	// ���ݰ�Ч���ж�
		if(!cPacketReady)
		{
			Rs485GetPacket(&wCmd, &wReAddr[0], &wReValue[0], &wReData[0]);	// �������ݰ�
						// ������� �Ĵ�����ַ�� �Ĵ��������� �Ĵ�������
			switch(wCmd)
			{
				case	0x03:	// ��ȡ���ּĴ���
					if(wReAddr[0] == 0x01 && wReValue[0] == 0x00)	// ���ּĴ��� �Ĵ�����ַ��λΪ1���Ĵ���������λΪ0
					{
						if(wReAddr[1] == 0x01)	// �豸��ַ �Ĵ�����λ�ж�
						{
							switch(wReValue[1])	// �Ĵ�������
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
						else if(wReAddr[1] == 0x02)	// ������
						{
							switch(wReValue[1])	// �Ĵ�������
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
						else if(wReAddr[1] == 0x03)	// �¶�����ֵ
						{
							switch(wReValue[1])	// �Ĵ�������
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
						else if(wReAddr[1] == 0x04)	// ʪ������ֵ
						{
							if(wReValue[1] == 0x01)
							{
								EEPROM_HUMIAMEND_EDATA[1] = EEPROM_READ(EEPROM_HUMIAMEND_EADDR[1]);
								Rs485SendPacket(0x03, 2, 0, 0, &EEPROM_HUMIAMEND_EDATA[0]);
							}
						}
					}
				break;
				case	0x04:	// ��ȡ����Ĵ���
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
				case	0x06:	// д�뵥�����ּĴ���
					if(wReAddr[0] == 0x01)	// ���ּĴ���
					{
						// �豸��ַ ������ �¶�����ֵ ʪ������ֵ
						if(wReAddr[1] == 0x01 || wReAddr[1] == 0x02 || wReAddr[1] == 0x03 || wReAddr[1] == 0x04)
						{
							if(wReAddr[1] == 0x01)	// �豸��ַ��Χ��1 ~ 247
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
				case	0x10:	// д�������ּĴ���
					if(wReAddr[0] == 0x01)	// ���ּĴ���
					{
						if(wReAddr[1] == 0x01)	// �豸��ַ
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
						else if(wReAddr[1] == 0x02)	// ������
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
						else if(wReAddr[1] == 0x03)	// �¶�����ֵ
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
						else if(wReAddr[1] == 0x04)	// ʪ������ֵ
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
				default:	// ����
					sValue[0] = 0xFF;
					sValue[1] = 0xFF;
					Rs485SendPacket(0x10, 8, &wReAddr[0], &wReValue[0], &sValue[0]);
				break;
			}
		}
	}
}

void bit_Value(void)	// ��ֵ��ֺ���
{
	int T1 = 0, H1 = 0;
	T1 = (int)Temperature * 10;
	H1 = (int)Humidity * 10;
	Temp[0] = (char)(T1 >> 8);
	Temp[1] = (char)(T1 & 0x00FF);
	Humi[0] = (char)(H1 >> 8);
	Humi[1] = (char)(H1 & 0x00FF);
}

void __interrupt() UsartInterruptISR(void)	// �жϴ�����
{	
	if(PIR1bits.TMR1IF == 1)
	{
		PIE1bits.TMR1IE = 0;
		PIR1bits.TMR1IF = 0;
		Time1++;
		if(Time1 == 10)
		{
			GXHT30_single_call();			// ���ζ�ȡ��ʪ��
			bit_Value();					// ��ֵת��
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
		INTCONbits.GIE = 1;
	}
	
}
