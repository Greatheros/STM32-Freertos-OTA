#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <string.h>
#include "stdarg.h"
#include "freertos_demo.h"

#define Buffer_Size_Max 512
#define REV_WAIT 0
#define REV_OK 1


extern volatile char USART1_RX_BUF[Buffer_Size_Max];  //串口接收缓冲区
extern char Read_Buffer[Buffer_Size_Max+10];             //读取缓冲区
extern volatile uint16_t Write_index;                 //接收缓冲区写入位置
extern volatile uint16_t PreWrite_index;              //上次接收缓冲区写入位置
//extern _Bool rev_state;

void Usart1_init(void);
#endif
