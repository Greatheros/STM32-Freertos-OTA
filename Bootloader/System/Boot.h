#ifndef __BOOT_H
#define __BOOT_H

#include "Delay.h" 
#include "w25q64.h"
#include "myflash.h"
#include "Usart.h"

#define START_ADDRESS     0x08005000       //用户程序的起始地址
#define Download_Address  0x001000        //升级包的bin存放在W25Q64的起始位置        
#define OTA_FLAG_KEY      0x1234567F      //OTA锁，若符合则进行OTA升级，否则跳转至用户程序
#define WIFI_NAME   "axx"               //wifi名称
#define WIFI_WORD   "hjh1008611"        //wifi密码
#define Host       "iot-api.heclouds.com"   //域名

typedef struct
{
    uint32_t OTA_flag;         //OTA升级标志位
    char OTA_version[6];       //OTA版本号
    uint32_t OTA_Tid;          //升级包TID值
    uint32_t OTA_Size;         //升级包大小
    uint8_t sector;           //扇区大小
}OTA_InfoCB;
extern OTA_InfoCB OTA_Info;

void Boot_getflag(void);
void Load_App(uint32_t address);
void OTA_Upgrade(void);

#endif
