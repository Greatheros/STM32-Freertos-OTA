#include "usart.h"	  
 
char USART2_RX_BUF[Buffer_Size_Max];  //���ջ�����
uint16_t Write_index=0;        //���ջ�����д��λ��
uint16_t PreWrite_index=0;           //�ϴν��ջ�����д��λ��

/*******�ض���Printf*********/
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
/*******�ض���Printf*********/
   	
void Usart2_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	/*ʹ��USART2����ʱ��*/  
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
 

	/*USART2_GPIO��ʼ������*/ 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;			//USART2_TXD(PA.2)     
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_TXD(PA.9)  


	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				//USART2_RXD(PA.3)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);					//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_RXD(PA.10)


	/*USART2 ��ʼ������*/
	USART_InitStructure.USART_BaudRate = 115200;										//���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//����ģʽ����Ϊ�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure);										//��ʼ������2

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	/*Usart1 NVIC����*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;	//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��VIC�Ĵ��� 

	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//ʹ�ܴ���2�����ж�

	USART_Cmd(USART2, ENABLE);                    			//ʹ�ܴ��� 
	USART_ClearFlag(USART2, USART_FLAG_TC);					//���������ɱ�־

}

void USART2_IRQHandler(void)                	//����2�жϷ������
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  
	{
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		USART2_RX_BUF[Write_index]=USART2->DR;;  //�����յ������ݴ��뻺����
		Write_index=(Write_index+1)%Buffer_Size_Max;    //���������������С�����ͷ��ʼд��
 
	}
	
} 


