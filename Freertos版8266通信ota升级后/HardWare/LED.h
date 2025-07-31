#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"                  // Device header


#define LED_RCC RCC_APB2Periph_GPIOA
#define LED_PORT GPIOA
#define LED_PIN GPIO_Pin_0

void LED_Init(void);
void LED_on(void);
void LED_off(void);
void LED_Tun(void);

#endif
