
#define SCL_O	T_SCL = 0;	// ʱ�Ӷ˿����
#define SCL_I	T_SCL = 1;	// ʱ�Ӷ˿�����

#define SCL_H	SCL = 1;	// ����ʱ��
#define SCL_L	SCL = 0;	// ����ʱ��

#define SDA_O	T_SDA = 0;	// ���ݶ˿����
#define SDA_I	T_SDA = 1;	// ���ݶ˿�����

#define SDA_H	SDA = 1;	// ��������
#define SDA_L	SDA = 0;	// ��������

#define adr	0x88	// ������Ĭ�ϵ�ַ

void IIC_init(void);	// ��ʼ��
void IIC_Start(void);	// ��ʼ�ź�
void IIC_Stop(void);	// �����ź�
void IIC_Ack(void);		// Ӧ���ź�
void IIC_NAck(void);	// ��Ӧ���ź�
unsigned char IIC_WAck(void);			// �ȴ�Ӧ��
void IIC_SendByte(unsigned int txd);	// �����ֽ�
unsigned char IIC_ReadByte(unsigned int ack);	// �����ֽ�
void GXHT30_read_result(unsigned char addr);	// ��ȡ�¶�
void GXHT30_write_cmd(unsigned char addr, unsigned char MSB, unsigned char LSB);	// ���Ϳ�������
void GXHT30_single_call(void);	// ���ε���

