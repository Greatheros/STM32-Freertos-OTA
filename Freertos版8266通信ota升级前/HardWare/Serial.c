#include "usart.h"	  
 
char USART2_RX_BUF[Buffer_Size_Max];  //接收缓冲区
uint16_t Write_index=0;        //接收缓冲区写入位置
uint16_t PreWrite_index=0;           //上次接收缓冲区写入位置

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
	while((USART2->SR&0X40)==0);
    USART2->DR = (uint8_t) ch;      
	return ch;
}
/*******重定向Printf*********/
   	
void Usart2_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*使能USART2外设时钟*/  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
 

	/*USART2_GPIO初始化设置*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;			//USART2_TXD(PA.2)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//调用库函数中的GPIO初始化函数，初始化USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//USART2_RXD(PA.3)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//调用库函数中的GPIO初始化函数，初始化USART1_RXD(PA.10)


	/*USART2 初始化设置*/
	USART_InitStructure.USART_BaudRate = 115200;										//设置波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;								//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//工作模式设置为收发模式
	USART_Init(USART2, &USART_InitStructure);										//初始化串口2

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/*Usart1 NVIC配置*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//抢占优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//从优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化VIC寄存器 

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//使能串口2接收中断

	USART_Cmd(USART2, ENABLE);                    			//使能串口 
	USART_ClearFlag(USART2, USART_FLAG_TC);					//清除发送完成标志

}

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		USART2_RX_BUF[Write_index]=USART2->DR;;  //将接收到的数据存入缓冲区
		Write_index=(Write_index+1)%Buffer_Size_Max;    //如果超出缓冲区大小，则从头开始写入
 
	}
	
} 


