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
static char cReAddr[3];	// �ݴ�Ĵ�����ַ
static char cReValue[2];// �ݴ�Ĵ�������
static char cReData[8];	// �ݴ�Ĵ�������

void Rs485Initialise(char cAddr)	// ��ʼ�� RS485 ������������ ͬʱָ�����������ݵĴӻ���ַ
{
  cOurAddr = cAddr;				// �洢���������ݵ�ָ���ӻ���ַ���ֽ�
  cRS485State = PKT_WAIT_ADDR;	// ��ʼ��״̬��ָ��
  T_RO = 1;
  PIE1bits.RCIE = 1;			// ���������ж�
}

char Rs485Process(void)			// ���ݰ�Ч���ж�
{
	char cPktReady;// ���ݰ�״̬
	cPktReady = FALSE;			// ����ַ���� ��Ϊ��
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
	char CmdValue;
	PIE1bits.RCIE = 0;	// �رս����ж�
	T_RO = 0;
	__delay_ms(2);		// ����ʱ��
	CRC16_Init();		// ��λCRC
	// Send some NULL preamblesfopr receiving UART Ϊ���� UART ����һЩ NULL ǰ����
	//for (c=0; c < NUM_TX_PREAMBLE; c++) Rs485SendChar(0x00);	// ����3�� NULL ��շ��ͻ���
	Rs485UpdataCrc(cOurAddr);
	Rs485SendChar(cOurAddr);		// ���ʹӻ���ַ
	Rs485UpdataCrc(sCmd);
	Rs485SendChar(sCmd);			// ���Ϳ�������
	CmdValue = PacketHasPayload(sCmd);
	switch(CmdValue)
	{
		case	1:	// ��������ּĴ��� 0x03
			Rs485UpdataCrc(sLen);			// ���ݳ���
			Rs485SendChar(sLen);
			Rs485UpdataCrc(sReData[0]);		// ���ݸ��ֽ�
			Rs485SendChar(sReData[0]);
			Rs485UpdataCrc(sReData[1]);		// ���ݵ��ֽ�
			Rs485SendChar(sReData[1]);
			break;
		case	2:	// ���������Ĵ��� 0x04
			Rs485UpdataCrc(sLen);			// ���ݳ���
			Rs485SendChar(sLen);
			if(sLen == 2)	// �����ֽڳ���
			{
				Rs485UpdataCrc(sReData[0]);		// ���ݸ��ֽ�
				Rs485SendChar(sReData[0]);
				Rs485UpdataCrc(sReData[1]);		// ���ݵ��ֽ�
				Rs485SendChar(sReData[1]);
			}
			else if(sLen == 4)	// �����ֽڳ���
			{
				Rs485UpdataCrc(sReData[0]);		// ���ݸ��ֽ�
				Rs485SendChar(sReData[0]);
				Rs485UpdataCrc(sReData[1]);		// ���ݵ��ֽ�
				Rs485SendChar(sReData[1]);
				Rs485UpdataCrc(sReData[2]);		// ���ݸ��ֽ�
				Rs485SendChar(sReData[2]);
				Rs485UpdataCrc(sReData[3]);		// ���ݵ��ֽ�
				Rs485SendChar(sReData[3]);
			}
			break;
		case	3:	// ���д�������ּĴ��� 0x06
			Rs485UpdataCrc(sReAddr[0]);		// ���ݵ�ַ���ֽ�
			Rs485SendChar(sReAddr[0]);
			Rs485UpdataCrc(sReAddr[1]);		// ���ݵ�ַ���ֽ�
			Rs485SendChar(sReAddr[1]);
			Rs485UpdataCrc(sReData[0]);		// ���ݸ��ֽ�
			Rs485SendChar(sReData[0]);
			Rs485UpdataCrc(sReData[1]);		//���ݵ��ֽ�
			Rs485SendChar(sReData[1]);
			break;
		case	4:	// ���д������ּĴ��� 0x10
			Rs485UpdataCrc(sReAddr[0]);		// ���ݵ�ַ���ֽ�
			Rs485SendChar(sReAddr[0]);
			Rs485UpdataCrc(sReAddr[1]);		// ���ݵ�ַ���ֽ�
			Rs485SendChar(sReAddr[1]);
			Rs485UpdataCrc(sReValue[0]);	// �Ĵ����������ֽ�
			Rs485SendChar(sReValue[0]);
			Rs485UpdataCrc(sReValue[1]);	// �Ĵ����������ֽ�
			Rs485SendChar(sReValue[1]);
			break;
	}

	Rs485SendChar(cCalcCrcHigh);	// ����CRCУ������ֽ�
	Rs485SendChar(cCalcCrcLow);		// ����CRCУ������ֽ�
	__delay_ms(1);
	T_RO = 1;
	PIE1bits.RCIE = 1;				// Enable Receive Interrupt �򿪽����ж�
}

void Rs485GetPacket(char gCmd, char *gReAddr, char *gReValue, char *gReData) // �����ݴ��ݸ�������
{
	char c;				// ѭ������
	gCmd = cCommand;	// ���ݿ�������
	gReAddr[0] = cReAddr[0];	// ���ݼĴ�����ַ
	gReAddr[1] = cReAddr[1];
	gReValue[0] = cReValue[0];	// ���ݼĴ�������
	gReValue[1] = cReValue[1];
	for(c = 0; c < 8; c++)		// ���ݼĴ�������
	{
		gReData[c] = cReData[c];
	}
}

/*
{
const char CRC16_LookupHigh[16] = {	// CRC16 ���ұ����ֽں͵��ֽڣ���ÿ�ε��� 4 λ��
		  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		  0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1
};	// CRC ���ұ�
const char CRC16_LookupLow[16] = {
		  0x00, 0x21, 0x42, 0x63, 0x84, 0xA5, 0xC6, 0xE7,
		  0x08, 0x29, 0x4A, 0x6B, 0x8C, 0xAD, 0xCE, 0xEF
};

void CRC16_Init(void) // CRC��ʼ��
{
	// ���� CCITT �淶�� CRC ��ʼ��Ϊ 0xFFFF
	cCalcCrcHigh = 0xFF;
	cCalcCrcLow = 0xFF;
}

void CRC16_Updata4Bits(char val) // ����4λCRC
{
	char t;
	// ��һ������ȡ CRC �Ĵ���������Ҫ�� 4 ������
	t = cCalcCrcHigh >> 4;	// ����λ�Ĵ��������ƶ�4λ��Ž�������
	// ����Ϣ���ݰ�λ�����ȡ�ı�����
	t = t ^ val;			// ��λ���
	// ��CRC�Ĵ�������4λ
	cCalcCrcHigh = (cCalcCrcHigh << 4) | (cCalcCrcLow >> 4);	// �ϲ�
	cCalcCrcLow = cCalcCrcLow << 4;		// ����λ�Ĵ��������ƶ�4λ
	// ���в������������ CRC ����
	cCalcCrcHigh = cCalcCrcHigh ^ CRC16_LookupHigh[t];	// ��CRC���ֽڲ��ұ�λ���
	cCalcCrcLow = cCalcCrcLow ^ CRC16_LookupLow[t];		// ��CRC���ֽڲ��ұ�λ���
}

void Rs485UpdataCrc(char cVal) // ����CRC
{
	CRC16_Updata4Bits(cVal >> 4);	// ����CRC���ֽ�
	CRC16_Updata4Bits(cVal & 0x0F);	// ����CRC���ֽ�
}
}
*/


unsigned int ModBusCRC16(unsigned char *data, unsigned int len)
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
/*������Ҫ�Խ�����д���*/
//    data[i++] = (unsigned char) (CRC16 & 0x00FF);
//    data[i++] = (unsigned char) ((CRC16 & 0xFF00)>>8);
    return CRC16;
}



void Rs485SendChar(char c) // ����һ�ֽ�
{
	TXREG = c;						// ���������Է���
	while (!(TXSTA & TRMT_MASK));	// �ȴ��������
}

char PostValidataPacket(void) // ��֤���յ�����CRCУ����
{
	CRC16_Init();	// ��ʼ��
	Rs485UpdataCrc(cNetAddr);		// ��ַ
	Rs485UpdataCrc(cCommand);		// ��������
	Rs485UpdataCrc(cReAddr[0]);		// �Ĵ�����ַ���ֽ�
	Rs485UpdataCrc(cReAddr[1]);		// �Ĵ�����ַ���ֽ�
	if(cCommand == WRITE_MHOLDRE)	// 0x10 д������ּĴ���
	{	// �Ĵ������� �ֽڳ��� �Ĵ�������
		Rs485UpdataCrc(cReValue[0]);
		Rs485UpdataCrc(cReValue[1]);
		Rs485UpdataCrc(cDataLen);
		for(cBufPtr = 0; cBufPtr < cDataLen; cBufPtr++)
		{
			Rs485UpdataCrc(cReData[cBufPtr]);
		}
	}
	else if(cCommand == WRITE_SHOLDRE)	// 0x06 д�������ּĴ���
	{	// �Ĵ�������
		Rs485UpdataCrc(cReData[0]);
		Rs485UpdataCrc(cReData[1]);
	}
	else if(cCommand == READ_HOLDRE || cCommand == READ_INPUTRE)	// 0x03 0x04	�����ּĴ��� ������Ĵ���
	{	// �Ĵ�������
		Rs485UpdataCrc(cReValue[0]);
		Rs485UpdataCrc(cReValue[1]);
	}

	// ��� CRC �Ƿ���ȷ
	// �������º��״̬��Ϊ�������
	if((cRxCrcHigh == cCalcCrcHigh) && (cRxCrcLow == cCalcCrcLow))	// ��֤У��CRC�����CRC�Ƿ����
	{
		cRS485State = PKT_VALID;	// ������Чֵ
		PORTCbits.RC7 = 1;
	}
	else	
	{
		//cRS485State = PKT_INVALID;	// ������Чֵ
		cRS485State = PKT_VALID;	// ������Чֵ
	}
	return cRS485State; // ���ؼ����״̬��ָʾ��
}

char PacketHasPayload(char ccCommand)	// ��������������
{
	char Cmd;
	PIE1bits.RCIE = 0;		// �رս����ж�
	
	if(ccCommand == READ_HOLDRE)		// 0x03��ȡ���ּĴ���
	{
		Cmd = 1;
	}
	else if(ccCommand == READ_INPUTRE)	// 0x04��ȡ����Ĵ���
	{
		Cmd = 2;
	}
	else if(ccCommand == WRITE_SHOLDRE)	// 0x06д�������ּĴ���
	{
		Cmd = 3;
	}
	else if(ccCommand == WRITE_MHOLDRE)	// 0x10д������ּĴ���
	{
		Cmd = 4;
	}
	else	// ��Ч����
	{
		Cmd = 0;
	}
	PIE1bits.RCIE = 1;		// �򿪽����ж�
	return Cmd;
}




