#ifndef __SERVO_H
#define __SERVO_H

#include "stm32f10x.h"                  // Device header

/*占用的定时器3的4通道，请一并修改*/
#define SERVO_RCC         RCC_APB2Periph_GPIOB
#define SERVO_PORT        GPIOB
#define SERVO_PIN         GPIO_Pin_1


void Servo_Init(void);
void Servo_angle(float angle);

#endif
