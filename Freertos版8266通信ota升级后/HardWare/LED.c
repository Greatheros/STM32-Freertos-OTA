#include "led.h"

void LED_Init(void)
{ 
   GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(LED_RCC, ENABLE);	 
    GPIO_InitStructure.GPIO_Pin = LED_PIN;				 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
    GPIO_Init(LED_PORT, &GPIO_InitStructure);					
    GPIO_SetBits(LED_PORT,LED_PIN);						 
}



void LED_on(void)
{
  GPIO_ResetBits(LED_PORT,LED_PIN);
}

void LED_off(void)
{
	GPIO_SetBits(LED_PORT,LED_PIN);	
}

void LED_Tun(void)
{
  if(GPIO_ReadOutputDataBit(LED_PORT,LED_PIN)==1)
  {
    GPIO_ResetBits(LED_PORT,LED_PIN);
  }
  else{GPIO_SetBits(LED_PORT,LED_PIN);}
}
