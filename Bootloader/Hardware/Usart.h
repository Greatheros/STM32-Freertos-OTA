#ifndef __USART_H
#define __USART_H

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include <stdio.h>
#include <string.h>
#include "stdarg.h"


#define Buffer_Size_Max 1024
#define Download_address  0x001000      //下载到w25q64上的起始地址

extern volatile char USART1_RX_BUF[Buffer_Size_Max];  //串口接收缓冲区
extern char Read_Buffer[Buffer_Size_Max];             //读取缓冲区
extern volatile uint16_t Write_index;                 //接收缓冲区写入位置
extern volatile uint16_t PreWrite_index;              //上次接收缓冲区写入位置
extern uint8_t download;

void Usart1_init(void);
int ESP8266_SendCmd(char *cmd, char *res);
uint16_t ESP8266_ReadData(char *buffer, uint16_t max_len);
void ESP8266_Clear(void);

#endif
