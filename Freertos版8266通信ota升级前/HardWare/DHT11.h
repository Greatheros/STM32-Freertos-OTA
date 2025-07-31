#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header


// 定义DHT11引脚
#define DHT11_GPIO_PORT     GPIOB
#define DHT11_GPIO_SDA      GPIO_Pin_11
#define DHT11_GPIO_CLK      RCC_APB2Periph_GPIOB



// DHT11各种数据， RH,RL为湿度高低八位，TH,TL为温度高低八位，revise为校正位
extern uint8_t RH,RL,TH,TL,revise; 

// 函数声明
void DHT11_Init(void);               //DHT11初始化
uint8_t Read_DHT11_data(void);       //读取DHT11数据
//void DHT11_Tset(void);               //在OLED上显示DHT11数据

#endif



