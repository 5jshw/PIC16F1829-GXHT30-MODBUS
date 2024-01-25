
char Rs485Process(void);			// ���ݰ�Ч���ж�
void Rs485Initialise(char cAddr);	// ��ʼ��
char Rs485Decode(void);				// ���
void Rs485UpdataCrc(char cVal);		// ����У����
void CRC16_Init(void);				// У�����ʼ��
void CRC16_Updata4Bits(char val);	// ����У����
void Rs485SendPacket(char sCmd, char sLen, char *sReAddr, char *sReValue, char *sReData);	// �������ݰ�
void Rs485GetPacket(char gCmd, char *gReAddr, char *gReValue, char *gReData);	// ���ݽ��յ������ݸ������� ������� �Ĵ�����ַ�� �Ĵ��������� �Ĵ�������
void Rs485SendChar(char c);			// �����ַ�
char PostValidataPacket(void);		// ������֤���ݰ�
char PacketHasPayload(char ccCommand);	// ��������������
unsigned int ModBusCRC16(unsigned char *data, unsigned int len);

#define FALSE 1
#define TRUE 0

#define TRMT_MASK 2			// ���ڼ��TRMTλ״̬������

// ���ݰ�������״̬����״̬
#define PKT_WAIT_ADDR		0	// ��ַ
#define PKT_CMD				1	// ��������
#define PKT_RE_ADDR			2	// �Ĵ�����ַ
#define PKT_RE_VALUE		3	// �Ĵ�������
#define PKT_RE_DATA2		4	// �Ĵ���˫�ֽ�����
#define PKT_RE_DATAS		5	// �Ĵ������ֽ�����
#define PKT_WAIT_CRC_HIGH	6	// У������ֽ�
#define PKT_WAIT_CRC_LOW	7	// У������ֽ�
#define PKT_COMPLETE		8	// ����
#define PKT_VALID			9	// ��Ч
#define PKT_INVALID			255	// ��Ч
#define PKT_DATA_LEN		12	// ��Ĵ����ֽڳ���

// Error codes �������
#define BAD_LENGTH	1	// ���ȴ���
#define BAD_CRC		2	// У�������
#define BAD_NADDR	3	// ��ַ����
#define BAD_BUAD	4	// �����ʴ���

// Command Types ������������
#define READ_HOLDRE		0x03	// ��ȡ���ּĴ���
#define READ_INPUTRE	0x04	// ��ȡ����Ĵ���
#define WRITE_SHOLDRE	0x06	// д�뵥�����ּĴ���
#define WRITE_MHOLDRE	0x10	// д�������ּĴ���
