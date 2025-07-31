#include "usart.h"	  
 
volatile char USART1_RX_BUF[Buffer_Size_Max];  //接收缓冲区
char Read_Buffer[Buffer_Size_Max+10];
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

void USART1_IRQHandler(void)                	//串口2中断服务程序
{
	if(USART_GetITStatus(USART1, USART_IT_IDLE) != RESET)  
	{
		uint16_t temp;  // 临时变量，用于读取DR
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        temp = USART1->SR;  // 读SR寄存器
        temp = USART1->DR;  // 读DR寄存器（两步结合才能清除IDLE标志）
		(void)temp;
		if (((Write_index + 1) % Buffer_Size_Max) != PreWrite_index) 
		{
			Write_index = (Buffer_Size_Max-DMA1_Channel5->CNDTR)%Buffer_Size_Max;
			xSemaphoreGiveFromISR(rev_state,&xHigherPriorityTaskWoken);
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
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
