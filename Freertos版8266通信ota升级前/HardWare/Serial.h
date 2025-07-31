#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"                  // Device header
#include "stdio.h"	

#define Buffer_Size_Max 1024

extern char USART2_RX_BUF[Buffer_Size_Max];  //接收缓冲区
extern uint16_t Write_index;  //接收缓冲区写入位置
extern uint16_t PreWrite_index;  //上次接收缓冲区写入位置

void Usart2_init(void);
#endif
