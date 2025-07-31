#ifndef __FREERTOSDEMO_H
#define __FREERTOSDEMO_H

#include "stm32f10x.h"                  // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#include "esp8266.h"
#include "OLED.h"
#include "DHT11.h"
#include "IDWG.h"

//任务栈大小管理区
#define Control_depth 72
#define led_depth     162
#define servo_depth   162
#define res_depth     162
#define dis_depth     64

//任务优先级管理区
#define Control_Priority 4
#define led_Priority     2
#define servo_Priority   2
#define res_Priority     1
#define dis_Priority     3

extern QueueHandle_t  Semaphore_Post;
extern SemaphoreHandle_t rev_state;                                   //接收状态信号量

void Freertos_start(void);

#endif
