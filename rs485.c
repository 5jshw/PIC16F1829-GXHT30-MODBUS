#include <xc.h>
#include <stdlib.h>
#include "rs485.h"
#include "MB1.h"

char cOurAddr;			// �ݴ潫�������ݵ�ָ���ӻ���ַ
char cRs485RxChar;		// �ݴ��RS485���յ��ĵ����ַ�
static char cRS485State;// �ݴ�RS485���ݴ���ĵ�ǰ״̬
static char cNetAddr;	// �ݴ���յ���ָ���ӻ���ַ
static char cCommand;	// �ݴ����ݰ��е������ֽ�
static char cDataLen;	// �ݴ����ݳ���
static char cRxCrcHigh, cRxCrcLow;		// �ݴ���յ������ݰ���CRCУ��͵ĸ�λ�͵�λ
static char cCalcCrcHigh, cCalcCrcLow;	// �ݴ�������CRCУ��͵ĸ�λ�͵�λ
static char cBufPtr;	// һ��ָ�� ���ڸ����������е�ǰ��д��λ��
static char cReAddr[2];	// �ݴ�Ĵ�����ַ
static char cReValue[2];// �ݴ�Ĵ�������
static char cReData[8];	// �ݴ�Ĵ�������
static char CRC_Value[16], CRC_Len;


void Rs485Initialise(char cAddr)	// ��ʼ�� RS485 ������������ ͬʱָ�����������ݵĴӻ���ַ
{
  cOurAddr = cAddr;				// �洢���������ݵ�ָ���ӻ���ַ���ֽ�
  cRS485State = PKT_WAIT_ADDR;	// ��ʼ��״̬��ָ��
  T_RO = 1;
  PIE1bits.RCIE = 1;			// ���������ж�
}

char Rs485Process(void)		// ���ݰ�Ч���ж�
{
	char cPktReady;			// ���ݰ�״̬
	cPktReady = FALSE;		// ����ַ���� ��Ϊ��
	if(cRS485State == PKT_COMPLETE) // ������ݰ��Ƿ���ѯ���
	{
		if(cNetAddr == cOurAddr)	// �����յĵ�ַλ�Ƿ��Ӧָ���ӻ��ĵ�ַλ
		{
			cRS485State = PostValidataPacket();	// ��֤CRC���ݰ� ȷ�����յĵ�ַλ�����ݣ�����ȷ��
			if(cRS485State == PKT_INVALID)		// ��CRCУ�����Ч
			{
				cPktReady = FALSE;				// ���ݰ�δ׼����
				cRS485State = PKT_WAIT_ADDR;	// ��״̬��ָ�븴λ �ȴ��´ν���
			}
			else if(cRS485State == PKT_VALID)	// ��CRCУ�����Ч
			{
				cPktReady = TRUE;				// ��ַ��ȷ CRCУ����ȷ ��ʾ�����ý��׼��
				cRS485State = PKT_WAIT_ADDR;	// ��״̬��ָ�븴λ �ȴ��´ν���
			}
		}
		else
		{
			cPktReady = FALSE;
			cRS485State = PKT_WAIT_ADDR;	// ��״̬��ָ�븴λ �ȴ��´ν���
		}
	}
	INTCONbits.GIE = 1;		// ����ȫ���ж�
	return cPktReady;		// ����һ��ֵ ��ʾ�Ƿ�׼���ô˴�ͨѶ
}

char Rs485Decode(void)		// �����485���յ������ݰ�
{
	switch(cRS485State)	// ����״̬���ĵ�ǰ״̬���н���
	{
		// �ȴ����մӻ���ַ
		case PKT_WAIT_ADDR:		// 0
								cNetAddr = cRs485RxChar; // �洢���յ��Ĵӻ���ַ
								cRS485State++; // �ƶ�����һ��״̬
								break;
		// �ȴ����տ��������ֽ�
		case PKT_CMD:			// 1
								cCommand = cRs485RxChar;	// �洢���������ֽ�
								cRS485State++;
								break;
		// �ȴ����ռĴ�����ַ
		case PKT_RE_ADDR:		// 2
								cReAddr[cBufPtr] = cRs485RxChar;
								if(cBufPtr == 1)
								{
									cBufPtr = 0;
									if(cCommand == WRITE_SHOLDRE)	// д�뵥�����ּĴ��� 0x06
									{
										cRS485State = PKT_RE_DATA2;	// ��ת��˫�ֽ����ݽ���ģ��
									}
									else if(cCommand == WRITE_MHOLDRE)	// д�������ּĴ��� 0x10
									{
										cRS485State = PKT_RE_VALUE;	// ��ת���Ĵ�����������ģ��
									}
									else	// ��ȡ����򱣳ּĴ��� 0x03	0x04
									{
										cRS485State = PKT_RE_VALUE;	// ��ת���Ĵ�����������ģ��
									}
								}
								else
								{
									cBufPtr++;
								}
								break;
		// �ȴ����ռĴ�������
		case PKT_RE_VALUE:		// 3
								cReValue[cBufPtr] = cRs485RxChar;
								if(cBufPtr == 1)	// ���ݿ����������ж��Ƕ�ȡ����д�룬˫�ֽڻ��Ƕ��ֽ�
								{
									cBufPtr = 0;
									if(cCommand == WRITE_MHOLDRE)	// д�������ּĴ���
									{
										cRS485State = PKT_DATA_LEN;	// ��ת���ֽڳ��Ƚ���ģ��
									}
									else	// 0x03 0x04
									{
										cRS485State = PKT_WAIT_CRC_HIGH;	// �������������ת��CRC����ģ��
									}
								}
								else
								{
									cBufPtr++;
								}
								break;
		// ���յ������ּĴ�������
		case PKT_RE_DATA2:		// 4
								cReData[cBufPtr] = cRs485RxChar;
								if(cBufPtr == 1)
								{
									cBufPtr = 0;
									cRS485State = PKT_WAIT_CRC_HIGH;	// ��ת��CRC����ģ��
								}
								else
								{
									cBufPtr++;
								}
								break;
		// ���ն�Ĵ����ֽڳ���		
		case PKT_DATA_LEN:		// 12
								cDataLen = cRs485RxChar;
								cRS485State = PKT_RE_DATAS;	// ��ת���Ĵ������ֽ����ݽ���ģ��
								break;
								
		// ���ն�����ּĴ�������
		case PKT_RE_DATAS:		// 5
								cReData[cBufPtr] = cRs485RxChar;	// �����յ������ݴ洢��������
								cBufPtr++; //�ƶ�������ָ��
								if(cBufPtr == (cDataLen - 1))				// �����յ����ݳ��ȴﵽԤ�����ݳ���
								{
									cBufPtr = 0;
									cRS485State = PKT_WAIT_CRC_HIGH;// ��ת��CRC����ģ��
								}
								break;
		// �ȴ�����CRC���ֽ�
		case PKT_WAIT_CRC_HIGH:	// 6
								cRxCrcHigh = cRs485RxChar;	// �洢CRC���ֽ�
								cRS485State++;	// �ƶ�����һ���ֽ�
								break;
		// �ȴ�����CRC���ֽ�
		case PKT_WAIT_CRC_LOW:	// 7
								cRxCrcLow = cRs485RxChar;	// �洢CRC���ֽ�
								cRS485State = PKT_COMPLETE;	// ��״̬������Ϊ����ɽ���״̬
								
								break;
		// ���ݰ��ѽ������ ���ڿ���״̬
		case PKT_COMPLETE:		// 8
								break;	// ��ִ���κβ���
		case PKT_VALID:			// 9
								break;	// ��ִ���κβ���
		case PKT_INVALID:		// 255
								break;	// ��ִ���κβ���
		// �κ���������� ������ʼ״̬
		default:
								break;
	}
	return cRS485State; // ���ص�ǰ��״̬��״̬
}

void Rs485SendPacket(char sCmd, char sLen, char *sReAddr, char *sReValue, char *sReData) // ͨ��RS485��·�������ݰ�
{
	unsigned int CRC16;
	char i;
	PIE1bits.RCIE = 0;	// �رս����ж�
	T_RO = 0;
	__delay_ms(2);		// ����ʱ��
	// CRCУ��ǰ׼��
	CRC_Value[0] = cOurAddr;
	CRC_Value[1] = sCmd;
	switch(CRC_Value[1])
	{
		case	0x04:	// ������Ĵ���
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
		case	0x03:	// �����ּĴ���
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
		case	0x06:	// д�������ּĴ���
			CRC_Value[2] = sReAddr[0];
			CRC_Value[3] = sReAddr[1];
			CRC_Value[4] = sReData[0];
			CRC_Value[5] = sReData[1];
			CRC_Len = 6;
		break;
		case	0x10:	// д������ּĴ���
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
	PIE1bits.RCIE = 1;		// �򿪽����ж�
}

void Rs485GetPacket(char *gCmd, char *gReAddr, char *gReValue, char *gReData) // �����ݴ��ݸ�������
{
	char c;				// ѭ������
	*gCmd = cCommand;	// ���ݿ�������
	gReAddr[0] = cReAddr[0];	// ���ݼĴ�����ַ
	gReAddr[1] = cReAddr[1];
	gReValue[0] = cReValue[0];	// ���ݼĴ�������
	gReValue[1] = cReValue[1];
	for(c = 0; c < 8; c++)		// ���ݼĴ�������
	{
		gReData[c] = cReData[c];
	}
}

unsigned int ModBusCRC16(char *data, char len)	// CRC16-MODBUSУ��
{
    unsigned int i, j, tmp, CRC16;

    CRC16 = 0xFFFF;             //CRC�Ĵ�����ʼֵ
    for (i = 0; i < len; i++)
    {
        CRC16 ^= data[i];
        for (j = 0; j < 8; j++)
        {
            tmp = (unsigned int)(CRC16 & 0x0001);
            CRC16 >>= 1;
            if (tmp == 1)
            {
                CRC16 ^= 0xA001;    //������ʽ
            }
        }
    }
    return CRC16;
}

void Rs485SendChar(char c) // ����һ�ֽ�
{
	TXREG = c;						// ���������Է���
	while (!(TXSTA & TRMT_MASK));	// �ȴ��������
}

char PostValidataPacket(void) // ��֤���յ�����CRCУ����
{
	unsigned int CRC16;
	ReadCRCValue();
	CRC16 = ModBusCRC16(&CRC_Value[0], CRC_Len);

	if((cRxCrcHigh == ((CRC16 & 0x00FF))) && (cRxCrcLow == ((CRC16 & 0xFF00) >> 8)))	// ��֤У��CRC�����CRC�Ƿ����
	{
		cRS485State = PKT_VALID;	// ������Чֵ	
	}
	else	
	{
		cRS485State = PKT_INVALID;	// ������Чֵ
	}
	return cRS485State; // ���ؼ����״̬��ָʾ��
}

void ReadCRCValue(void)	// �������ݽ���CRCУ��ǰ׼������
{
	CRC_Value[0] = cNetAddr;
	CRC_Value[1] = cCommand;
	CRC_Value[2] = cReAddr[0];
	CRC_Value[3] = cReAddr[1];
	switch(cCommand)
	{
		case	0x04:	// ������Ĵ���
		{
			CRC_Value[4] = cReValue[0];
			CRC_Value[5] = cReValue[1];
			CRC_Len = 6;
		}
		break;
		case	0x03:	// �����ּĴ���
		{
			CRC_Value[4] = cReValue[0];
			CRC_Value[5] = cReValue[1];
			CRC_Len = 6;
		}
		break;
		case	0x06:	// д�������ּĴ���
		{
			CRC_Value[4] = cReData[0];
			CRC_Value[5] = cReData[1];
			CRC_Len = 6;
		}
		break;
		case	0x10:	// д������ּĴ���
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


