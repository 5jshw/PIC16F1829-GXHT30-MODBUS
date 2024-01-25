#include <xc.h>
#include "GXHT30.h"
#include "MB1.h"

int Temperature; //�¶����ݣ�����
int Humidity; //ʪ�����ݣ�����

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
	SDA_O;			//���ݶ˿����
	SCL_L;
		
	SDA_H;			//��������
	__delay_ms(1);
	SCL_H;			//����ʱ��
	__delay_ms(1);	//				SCL __/---\__
	SDA_L;			//��������		SDA ----\____ 
	__delay_ms(1);	//
	SCL_L; 		   	//����ʱ��
	
}

//I2C��ֹ�ź�
void IIC_Stop(void)
{
	SDA_O;
	SCL_L;			//��ʼ��
	
	SDA_L;			//��������
	__delay_ms(1);	//
	SCL_H;			//����ʱ��
	__delay_ms(1);	//				SCL __/---\__
	SDA_H;			//��������		SDA ____/----
	__delay_ms(1);	//
	SCL_L;			//����ʱ��
	
}

//I2CӦ���ź�
void IIC_Ack(void)
{
	SDA_O;
	SCL_L;			//��ʼ��
	
	SDA_L;			//��������
	__delay_ms(1);		//			SCL __/---\__
	SCL_H;			//����ʱ��		SDA _________  
	__delay_ms(1);		//
	SCL_L;			//����ʱ��
	
}

//I2C��Ӧ���ź�
void IIC_NAck(void)
{
	SDA_O;
	SCL_L;			//��ʼ��
	
	SDA_H;			//��������
	__delay_ms(1);		//			SCL __/---\__
	SCL_H;			//����ʱ��		SDA ---------
	__delay_ms(1);		//
	SCL_L;			//����ʱ��
	
}

//I2C�ȴ�Ӧ��
unsigned char IIC_WAck(void)
{
	unsigned char i = 0; //������
	SDA_I; //���ݽӿ�����
	SDA_H; //��������
	SCL_H; //����ʱ��
	
	while(SDA) //�жϴ�����Ӧ���ź�
	{
		if(i >= 255) //����ʱ�� �ж��Ƿ���յ�Ӧ���źţ�����SCL�ߵ�ƽʱSDA����
		{
			IIC_Stop(); //��������涨ʱ��û�ж��������������ͽ����ź�
			__delay_ms(1);
			return 1; //������ֵ1���յ���Ӧ���ź�
		}
		i++;
	}
	
	SCL_L; //����ʱ��
	return 0; //�յ�Ӧ���ź�
	
}

//�����ֽ�
void IIC_SendByte(unsigned int txd)
{
	unsigned int i = 0;
 
	SDA_O; //���ݽӿ����
	SCL_L; //����ʱ��
 
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
		SCL_H; //����ʱ�ӣ���������
		__delay_ms(1);
		SCL_L; //����ʱ�ӣ��������
		__delay_ms(1); //�ӳ�Ϊ�������ȶ�
	}
	
}

//�����ֽ�
unsigned char IIC_ReadByte(unsigned int ack) //ack �ж��ֽ������Ƿ�������
{
   unsigned char i = 0, receive = 0; //i��Ϊ�˽���ͳ�ƣ�receive��Ϊ�˽��շ���������
   SDA_I; //���ݽӿ�����
   for(i = 0; i < 8; i++)
   {
   		SCL_L; //����ʱ�ӣ���λ
		__delay_ms(1);
		SCL_H; //����ʱ��
		
		while(!SCL);
		receive <<= 1;
		if(SDA == 1)
		{
			receive++;
		}		
		/*
		while(SCL);
		
		receive <<= 1;
		
		if(SDA == 1)
		{
			receive++;
		}
		*/
		/*
        if(SDA == 0) //���մ���������������
        {
	        receive << 1; //����һλ
	    } 
	    else if(SDA == 1)
	    {
		    receive++; //���λ��һ
	    }
	    */
	}
	__delay_ms(1);
   	if(ack == 0) //�����ж��ڵ��øú���ʱ��ack��ֵ��Ҳ���Ƿ���������ݣ��Ƿ��ͽ����źš�
	{
	   	IIC_NAck(); //���һ�ֽ����ݽ�����ϣ��ظ���Ӧ��
	}
	else
	{
		IIC_Ack(); //һλ�ֽ����ݽ�����ϣ��ظ�Ӧ��
 	}	
	return receive; //���ؽ��յ���һ�ֽ�����
	
}

//��ȡ�¶�
void GXHT30_read_result(unsigned char addr)
{
	unsigned int tem,hum; //�ϲ������ã�����
	unsigned int buff[6]; //���������ã�����
	Temperature = 0;	//�¶����ݣ�����
	Humidity = 0;	//ʪ�����ݣ�����
	
	IIC_Start(); //��ʼ�ź�
	__delay_ms(1);
	IIC_SendByte(addr | 0x01); // 1 ����ַ���һλ��ʾ��д������1Ϊ������
	__delay_ms(1);
	if(IIC_WAck() == 0)
	{
		SDA_I; //���ݽӿ�����
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
	
	Temperature = (int)((175.0 * (float)tem / 65535.0 - 45.0) * 10); // T = -45 + 175 * tem / (2^16-1)
	Humidity = (int)((100.0 * (float)hum /65535.0) * 10); // RH = hum*100 / (2^16-1)
	
	hum = 0;	//��������
	tem = 0;	//��������
	
	__delay_ms(1);
}

//���Ϳ�������
void GXHT30_write_cmd(unsigned char addr, unsigned char MSB, unsigned char LSB)
{
	IIC_Start();			//��ʼ�ź�
	__delay_ms(1);
	IIC_SendByte(addr);		// 0 ����ַ���һλ��ʾ��д������0Ϊд����
	__delay_ms(1);
	IIC_WAck();				//�ȴ�Ӧ��
	__delay_ms(1);
	IIC_SendByte(MSB);		//��λָ��
	__delay_ms(1);
	IIC_WAck();
	__delay_ms(1);
	IIC_SendByte(LSB);		//��λָ��
	__delay_ms(1);
	IIC_WAck();
	__delay_ms(1);
	IIC_Stop();				//���ͽ����ź�
	__delay_ms(1);
	
}

void GXHT30_single_call(void)
{
	GXHT30_write_cmd(adr, 0x2C, 0x0D);
	__delay_ms(20);
	GXHT30_read_result(adr);
}