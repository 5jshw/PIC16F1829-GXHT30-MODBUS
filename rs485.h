
char Rs485Process(void);			// 数据包效用判断
void Rs485Initialise(char cAddr);	// 初始化
char Rs485Decode(void);				// 解包
void Rs485UpdataCrc(char cVal);		// 更新校验码
void CRC16_Init(void);				// 校验码初始化
void CRC16_Updata4Bits(char val);	// 更新校验码
void Rs485SendPacket(char sCmd, char sLen, char *sReAddr, char *sReValue, char *sReData);	// 发送数据包
void Rs485GetPacket(char gCmd, char *gReAddr, char *gReValue, char *gReData);	// 传递接收到的数据给主程序 控制命令， 寄存器地址， 寄存器数量， 寄存器数据
void Rs485SendChar(char c);			// 发送字符
char PostValidataPacket(void);		// 发送验证数据包
char PacketHasPayload(char ccCommand);	// 检查控制命令类型
unsigned int ModBusCRC16(unsigned char *data, unsigned int len);

#define FALSE 1
#define TRUE 0

#define TRMT_MASK 2			// 用于检查TRMT位状态的掩码

// 数据包解码器状态机的状态
#define PKT_WAIT_ADDR		0	// 地址
#define PKT_CMD				1	// 控制命令
#define PKT_RE_ADDR			2	// 寄存器地址
#define PKT_RE_VALUE		3	// 寄存器数量
#define PKT_RE_DATA2		4	// 寄存器双字节数据
#define PKT_RE_DATAS		5	// 寄存器多字节数据
#define PKT_WAIT_CRC_HIGH	6	// 校验码高字节
#define PKT_WAIT_CRC_LOW	7	// 校验码低字节
#define PKT_COMPLETE		8	// 完整
#define PKT_VALID			9	// 有效
#define PKT_INVALID			255	// 无效
#define PKT_DATA_LEN		12	// 多寄存器字节长度

// Error codes 错误代码
#define BAD_LENGTH	1	// 长度错误
#define BAD_CRC		2	// 校验码错误
#define BAD_NADDR	3	// 地址错误
#define BAD_BUAD	4	// 波特率错误

// Command Types 控制命令类型
#define READ_HOLDRE		0x03	// 读取保持寄存器
#define READ_INPUTRE	0x04	// 读取输入寄存器
#define WRITE_SHOLDRE	0x06	// 写入单个保持寄存器
#define WRITE_MHOLDRE	0x10	// 写入多个保持寄存器
