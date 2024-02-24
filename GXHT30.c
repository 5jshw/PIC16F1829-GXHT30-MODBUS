#include <xc.h>
#include <pic16f1829.h>
#include "GXHT30.h"
#include "MB1.h"

double Temperature; //�¶����ݣ�����
double Humidity; //ʪ�����ݣ�����

//��ʼ��I2C
void IIC_init(void)
{
	SCL_O;
	SDA_O; 
	SCL_L;
	SDA_L;
}

//I2C��ʼ�ź�
void IIC_Start(void)
{
	SDA_O;			// ��ʼ��
	SCL_O;
	SCL_L;
	SDA_L;
	__delay_us(20);
	SDA_H;			// ��������
	__delay_us(10);
	SCL_H;			// ����ʱ��
	__delay_us(10);	//				SCL __/---\__
	SDA_L;			// ��������		SDA ----\____
	__delay_us(10);	//
	SCL_L;			// ����ʱ��
	__delay_us(20);
}

//I2C��ֹ�ź�
void IIC_Stop(void)
{
	SDA_O;			//��ʼ��
	SCL_O;
	SCL_L;
	SDA_L;			//��������
	__delay_us(20);
	SCL_H;			//����ʱ��
	__delay_us(10);	//				SCL __/---\__
	SDA_H;			//��������		SDA ____/----
	__delay_us(10);	//
	SCL_L;			//����ʱ��
	__delay_us(10);
	SDA_L;
	__delay_us(20); 
}

//I2CӦ���ź�
void IIC_Ack(void)
{
	SDA_O;			//��ʼ��
	SCL_O;
	__delay_us(20);
	SCL_L;
	SDA_L;			//��������
	__delay_us(20);	//				SCL __/---\__
	SCL_H;			//����ʱ��		SDA _________  
	__delay_us(20);	//
	SCL_L;			//����ʱ��
	__delay_us(20);
}

//I2C��Ӧ���ź�
void IIC_NAck(void)
{
	SDA_O;			//��ʼ��
	SCL_O;
	__delay_us(20);
	SCL_L;
	__delay_us(20);
	SDA_H;			//��������		SCL __/---\__
	__delay_us(20);	//				SDA ---------
	SCL_H;			//����ʱ��
	__delay_us(20);	//
	SCL_L;			//����ʱ��
	__delay_us(20);
	SDA_L;
	__delay_us(20);
}

//I2C�ȴ�Ӧ��
unsigned char IIC_WAck(void)
{
	unsigned int i = 0;		//������
	SDA_H;					//��������
	__delay_us(20);
	SDA_I;					//���ݽӿ�����
	__delay_us(20);
	SCL_H;					//����ʱ��
	
	while(SDA)				//�жϴ�����Ӧ���ź�
	{	
		i++;
		if(i > 100)			//����ʱ�� �ж��Ƿ���յ�Ӧ���źţ�����SCL�ߵ�ƽʱSDA����
		{
			SCL_L;
			IIC_Stop();	//��������涨ʱ��û�ж��������������ͽ����ź�
			return 1;		//������ֵ1���յ���Ӧ���ź�
		}
	}
	__delay_us(20);
	SCL_L;					//����ʱ���ź�
	return 0;				//�յ�Ӧ���ź�
}

//�����ֽ�
void IIC_SendByte(unsigned char txd)
{
	unsigned char i = 0;
	SDA_O; //���ݽӿ����
	SCL_O;
	SCL_L; //����ʱ��
	__delay_us(20);
	for(i = 0; i < 8; i++) //����һ�ֽ�8λ����
	{
		if((txd & 0x80) > 0) //�жϸñ������λ��ֵ
		{
			SDA_H; //Ϊ1ʱ����������
		}	
		else
		{
			SDA_L; //Ϊ0ʱ����������
 		}
		txd <<= 1; //������ɺ󽫴�������������һλ��׼��������һλ����
		__delay_us(20);
		SCL_H; //����ʱ�ӣ���������
		__delay_us(20);
		SCL_L; //����ʱ�ӣ��������
		__delay_us(20);//�ӳ�Ϊ�������ȶ�
		SDA_L;
		__delay_us(20);
	}
}

//�����ֽ�
unsigned char IIC_ReadByte(unsigned char ack) //ack �ж��ֽ������Ƿ�������
{
   unsigned char i = 0, receive = 0; //i��Ϊ�˽���ͳ�ƣ�receive��Ϊ�˽��շ���������
   SDA_I; //���ݽӿ�����
   //SCL_I;
   __delay_us(20);
   for(i = 0; i < 8; i++)
   {
   		SCL_L; //����ʱ�ӣ���λ
		__delay_us(20);
		SCL_H; //����ʱ��
		__delay_us(20);
		receive <<= 1;
		if(SDA)	receive++;
	}
   
	SCL_L; //����ʱ�ӣ���λ
	__delay_us(20);
   	if(ack) //�����ж��ڵ��øú���ʱ��ack��ֵ��Ҳ���Ƿ���������ݣ��Ƿ��ͽ����źš�
	{
	   	IIC_Ack();	//һλ�ֽ����ݽ�����ϣ��ظ�Ӧ��
	}
	else
	{
		IIC_NAck();	//���һ�ֽ����ݽ�����ϣ��ظ���Ӧ��
 	}
	__delay_us(20);
	return receive; //���ؽ��յ���һ�ֽ�����
}

//��ȡ�¶�
void GXHT30_read_result(unsigned char addr)
{
	unsigned int tem,hum;	//�ϲ������ã�����
	unsigned int buff[6];	//���������ã�����
	Temperature = 0;		//�¶����ݣ�����
	Humidity = 0;			//ʪ�����ݣ�����
	IIC_Start();			//��ʼ�ź�
	IIC_SendByte(addr | 0x01); // 1 ����ַ���һλ��ʾ��д������1Ϊ������
	
	if(IIC_WAck() == 0)
	{
		SDA_I; //���ݽӿ�����
		SCL_L;
		__delay_us(20);
		buff[0] = IIC_ReadByte(1); //�¶�1

		buff[1] = IIC_ReadByte(1); //�¶�2

		buff[2] = IIC_ReadByte(1); //CRCУ����

		buff[3] = IIC_ReadByte(1); //ʪ��1

		buff[4] = IIC_ReadByte(1); //ʪ��2

		buff[5] = IIC_ReadByte(0); //CRCУ���룬���һ���ֽ�

		IIC_Stop(); //���ͽ����ź�
	}
	tem = ((buff[0] << 8) | buff[1]);	// �ϲ��¶�����
	hum = ((buff[3] << 8) | buff[4]);	// �ϲ�ʪ������
	
	Temperature = (175.0 * (float)tem / 65535.0 - 45.0);	// T = -45 + 175 * tem / (2^16-1)
	Humidity = (100.0 * (float)hum / 65535.0);				// RH = hum*100 / (2^16-1)
	
	hum = 0;	//��������
	tem = 0;	//��������
}

//���Ϳ�������
void GXHT30_write_cmd(unsigned char addr, unsigned char MSB, unsigned char LSB)
{
	IIC_Start();			//��ʼ�ź�
	IIC_SendByte(addr);		// 0 ����ַ���һλ��ʾ��д������0Ϊд����
	IIC_WAck();				//�ȴ�Ӧ��
	IIC_SendByte(MSB);		//��λָ��
	IIC_WAck();
	IIC_SendByte(LSB);		//��λָ��
	IIC_WAck();
	IIC_Stop();				//���ͽ����ź�
}

void GXHT30_single_call(void)
{
	PORTCbits.RC7 = 1;
	__delay_us(20);
	GXHT30_write_cmd(adr, 0x2C, 0x10);	// ����ʱ�����죬���ظ���
	__delay_ms(5);
	GXHT30_read_result(adr);
	PORTCbits.RC7 = 0;
}