#ifndef __ESP8266_H
#define __ESP8266_H

#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "delay.h"
#include "usart.h"	
#include "LED.H"
#include "Servo.h"
#include "W25Q64.h"
#include "freertos_demo.h"

//这里添加或修改物模型属性
typedef enum 
{
	bool=0,
	string,
	int32,
	flo,
	doub
} GongNeng_Type;
extern GongNeng_Type LightSwitch;      //如灯光LightSwitch对应平台boll类型
extern GongNeng_Type Servoangle; 
extern GongNeng_Type Temp;
extern GongNeng_Type Humi;

extern char json_id[12]; //用于存储JSON数据中的id

typedef struct 
{
	char version[6];
	uint32_t tid;
	uint32_t size;
	uint32_t ota_key;
}PACK;
extern PACK Up_pack;

#define WIFI_NAME   "axx"               //wifi名称
#define WIFI_WORD   "hjh1008611"        //wifi密码
#define Equipment_Name   "One"          //设备名称
#define ID  "RUUPvSGLQd"                //产品ID
/*生成的token*/
#define mqtt_token  "version=2018-10-31&res=products%2FRUUPvSGLQd&et=1969281206&method=md5&sign=%2FRjTZa2JM5UBgONK1le3Zg%3D%3D"   

#define ONENET_Set    "$sys/"ID"/"Equipment_Name"/thing/property/set"                   //设备属性设置请求,平台下发属性设置
#define ONENET_Reply  "$sys/"ID"/"Equipment_Name"/thing/property/post/reply"            //设备属性上报响应,STM32给平台发送当前属性值后平台给予回应
#define STM32_post    "$sys/"ID"/"Equipment_Name"/thing/property/post"                  //设备属性上报请求,STM32给平台发送当前属性值
#define STM32_Set     "$sys/"ID"/"Equipment_Name"/thing/property/set_reply"             //设备属性设置响应,平台下发属性设置后STM32给予回应

#define Host       "iot-api.heclouds.com"   //域名
#define Path       "/fuse-ota/"ID"/"Equipment_Name"/version"


uint8_t OTA_Init(void);			 //OTA升级检测
int Esp8266_Init(void); 		//初始化ESP8266模块
void ESP8266_Clear(void);		//清理缓冲区
int ESP8266_SendCmd(char *cmd, char *res,int Repert);  //利用8266发送信息
int MQTT_Get(const char *res1, const char *res2);     
BaseType_t ShangChuan(int Type, const char *Logo,const char *ptr);
uint16_t ESP8266_ReadData(char *buffer, uint16_t max_len);

#endif
