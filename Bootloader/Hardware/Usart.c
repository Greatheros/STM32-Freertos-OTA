#include "usart.h"	  
 
uint8_t download = 0;
volatile char USART1_RX_BUF[Buffer_Size_Max];  //接收缓冲区
char Read_Buffer[Buffer_Size_Max];
volatile uint16_t Write_index=0;              //接收缓冲区写入位置
volatile uint16_t PreWrite_index=0;           //上次接收缓冲区写入位置

//_Bool rev_state1=REV_WAIT;         //接收状态

/*******重定向Printf*********/
struct __FILE 
{ 
	int handle; 
 
}; 
 
FILE __stdout;       
void _sys_exit(int x) 
{ 
	x = x; 
} 

int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);
    USART1->DR = (uint8_t) ch;      
	return ch;
}
/*******重定向Printf*********/
   	
void Usart1_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*使能USART2外设时钟*/  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);

	/*USART2_GPIO初始化设置*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			//USART1_TXD
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//调用库函数中的GPIO初始化函数，初始化USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//初始化USART1_RXD(PA.10)


	/*USART2 初始化设置*/
	USART_InitStructure.USART_BaudRate = 115200;										//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//工作模式设置为收发模式
	USART_Init(USART1, &USART_InitStructure);										//初始化串口1

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/*Usart1 NVIC配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//从优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 

	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);			//使能串口1接收中断*/

	USART_Cmd(USART1, ENABLE);                    			//使能串口 
	//USART_ClearFlag(USART1, USART_FLAG_TC);					//清除发送完成标志

	//*******DMA初始化
	DMA_InitTypeDef DMA_Initstructrue;
	DMA_Initstructrue.DMA_BufferSize=Buffer_Size_Max;
	DMA_Initstructrue.DMA_DIR=DMA_DIR_PeripheralSRC;    //外设到内存
	DMA_Initstructrue.DMA_M2M=DMA_M2M_Disable;
	DMA_Initstructrue.DMA_Priority=DMA_Priority_High;
	DMA_Initstructrue.DMA_Mode=DMA_Mode_Circular;         //循环
	
	DMA_Initstructrue.DMA_MemoryBaseAddr=(uint32_t)USART1_RX_BUF;
	DMA_Initstructrue.DMA_MemoryDataSize=DMA_MemoryDataSize_Byte;
	DMA_Initstructrue.DMA_MemoryInc=DMA_MemoryInc_Enable;        
	
	DMA_Initstructrue.DMA_PeripheralBaseAddr=(uint32_t)&USART1->DR;  //**注意
	DMA_Initstructrue.DMA_PeripheralDataSize=DMA_PeripheralDataSize_Byte;
	DMA_Initstructrue.DMA_PeripheralInc=DMA_PeripheralInc_Disable;   //不自增
	
	DMA_Init(DMA1_Channel5,&DMA_Initstructrue);
	DMA_Cmd(DMA1_Channel5,ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//从优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 

	DMA_ITConfig(DMA1_Channel5,DMA_IT_TC,ENABLE);	
	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);

}


void ESP8266_Clear(void)
{
	memset(Read_Buffer, 0, sizeof(Read_Buffer)); // 清空读取缓冲区
}

/**************************
 *函数名：ESP8266_ReadData（）
 *函数功能：读取串口的数据
 *输入：buffer->将数据要写入到的缓冲区地址
 *      max_len->缓冲区大小
 *输出：返回实际读取的字节数
 **************************/
uint16_t ESP8266_ReadData(char *buffer, uint16_t max_len)
{
	uint16_t bytes_to_read = 0;
	uint16_t write_snapshot = 0;

	write_snapshot = Write_index; // 获取当前写入位置快照
	// 计算有多少数据可以读
	if (write_snapshot >= PreWrite_index)
	{
		bytes_to_read = write_snapshot - PreWrite_index;
	}
	else
	{
		// 环形缓冲区绕回的情况
		bytes_to_read = Buffer_Size_Max - PreWrite_index + write_snapshot;
	}

	// 限制读取长度，避免溢出
	if (bytes_to_read > max_len)
	{
		bytes_to_read = max_len;
	}

	// 如果有数据可读
	if (bytes_to_read > 0)
	{
		// 处理环形缓冲区绕回的情况
		if (PreWrite_index + bytes_to_read > Buffer_Size_Max)
		{
			// 数据分成两段
			uint16_t len1 = Buffer_Size_Max - PreWrite_index;
			memcpy(buffer, (const void *)&USART1_RX_BUF[PreWrite_index], len1);
			memcpy(buffer + len1, (const void *)USART1_RX_BUF, bytes_to_read - len1);
			PreWrite_index = bytes_to_read - len1;
		}
		else
		{
			// 数据在一段连续内存中
			memcpy(buffer, (const void *)&USART1_RX_BUF[PreWrite_index], bytes_to_read);
			PreWrite_index += bytes_to_read;
		}
	}
	return bytes_to_read; // 返回实际读取的字节数
}

/**************************
 *函数名：ESP8266_SendCmd
 *函数功能：发送AT指令到ESP8266模块，并等待响应。
 *      cmd 要发送的AT指令
 *      res 期望的响应值
 *	 
 *输出：如果检测到期望值则返回1，否则返回0。
 **************************/
int ESP8266_SendCmd(char *cmd, char *res)
{

	uint16_t count;
	repeat:
		count = 200;
		printf("%s\r\n", cmd); // 发送命令
		while (count--)
		{
			ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
			if (strstr((const char *)Read_Buffer, res) != NULL) // 如果检索到关键词
			{
				ESP8266_Clear(); // 清空缓存
				return 1;
			}
			ESP8266_Clear();
			Delay_ms(10);
		}
		goto repeat; // 如果超时未收到响应，重新发送命令
	
}

void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	/*static uint8_t rx_buf[4]={0};
	static uint8_t download = 0;
	static uint32_t count=0;*/

	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  
	{
		uint16_t temp;  // 临时变量，用于读取DR
        temp = USART1->SR;  // 读SR寄存器
        temp = USART1->DR;  // 读DR寄存器（两步结合才能清除IDLE标志）
		(void)temp;
		if (((Write_index + 1) % Buffer_Size_Max) != PreWrite_index) 
		{
			Write_index = (Buffer_Size_Max-DMA1_Channel5->CNDTR)%Buffer_Size_Max;
			download = 1;
		} 
	}
	
} 



void DMA1_Channel5_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
    {
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
}



