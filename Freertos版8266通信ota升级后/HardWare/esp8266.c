#include "esp8266.h"

// ONENET平台产品的物模型属性
GongNeng_Type LightSwitch = bool; // 如灯光LightSwitch对应平台boll类型
GongNeng_Type Servoangle = string;
GongNeng_Type Temp = doub;
GongNeng_Type Humi = doub;

PACK Up_pack = {"V1.0",0,0,0x1234567F};
char json_id[12] = {0}; // 平台在下发属性设置请求时，会附带本次命令的id，此处用于存储该id值

/**************************
 *函数名：ESP8266_Clear()
 *函数功能：清空接收缓冲区
 *输入：无
 *输出：无
 **************************/
void ESP8266_Clear(void)
{
	memset(Read_Buffer, 0, sizeof(Read_Buffer)); // 清空读取缓冲区
}

// 该函数已废弃
/**************************
 *函数名：ESP8266_WaitRecive（）
 *函数功能：等待数据接收完成
 *输入：无
 *输出：如果接收完成则返回REV_OK，否则返回REV_WAIT。
 **************************/
/*_Bool ESP8266_WaitRecive(void)
{

	if(rev_state==REV_OK)
	{
		return REV_OK;
	}
	if(xSemaphoreTake(rev_state,0)==pdTRUE)
	{
		return REV_OK;
	}
	return REV_WAIT;

}*/

/**************************
 *函数名：ESP8266_ReadData（）
 *函数功能：读取串口的数据
 *输入：buffer->将数据要写入到的缓冲区地址
 *      max_len->缓冲区大小
 *输出：返回实际读取的字节数
 **************************/
uint16_t ESP8266_ReadData(char *buffer, uint16_t max_len)
{
	uint16_t bytes_to_read = 0;
	uint16_t write_snapshot = 0;

	taskENTER_CRITICAL();
	write_snapshot = Write_index; // 获取当前写入位置快照
	taskEXIT_CRITICAL();

	// 计算有多少数据可以读
	if (write_snapshot >= PreWrite_index)
	{
		bytes_to_read = write_snapshot - PreWrite_index;
	}
	else
	{
		// 环形缓冲区绕回的情况
		bytes_to_read = Buffer_Size_Max - PreWrite_index + write_snapshot;
	}

	// 限制读取长度，避免溢出
	if (bytes_to_read > max_len)
	{
		bytes_to_read = max_len;
	}

	// 如果有数据可读
	if (bytes_to_read > 0)
	{
		// 处理环形缓冲区绕回的情况
		if (PreWrite_index + bytes_to_read > Buffer_Size_Max)
		{
			// 数据分成两段
			uint16_t len1 = Buffer_Size_Max - PreWrite_index;
			memcpy(buffer, (const void *)&USART1_RX_BUF[PreWrite_index], len1);
			memcpy(buffer + len1, (const void *)USART1_RX_BUF, bytes_to_read - len1);
			PreWrite_index = bytes_to_read - len1;
		}
		else
		{
			// 数据在一段连续内存中
			memcpy(buffer, (const void *)&USART1_RX_BUF[PreWrite_index], bytes_to_read);
			PreWrite_index += bytes_to_read;
		}
	}
	/*if (bytes_to_read > 0 && bytes_to_read <= max_len)
	{
		buffer[bytes_to_read] = '\0';  // 仅在有数据且不溢出时加
	}*/
	return bytes_to_read; // 返回实际读取的字节数
}

/**************************
 *函数名：ESP8266_SendCmd
 *函数功能：发送AT指令到ESP8266模块，并等待响应。
 *      cmd 要发送的AT指令
 *      res 期望的响应值
 *	  Repert 是否重复发送命令，1表示重复发送，0表示不重复发送(重复发送不推荐在Freertos中使用)
 *输出：如果检测到期望值则返回1，否则返回0。
 **************************/
int ESP8266_SendCmd(char *cmd, char *res, int Repert)
{

	uint16_t count;
	if (Repert == 1)
	{
	repeat:
		count = 200;
		printf("%s\r\n", cmd); // 发送命令
		while (count--)
		{
			ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
			if (strstr((const char *)Read_Buffer, res) != NULL) // 如果检索到关键词
			{
				ESP8266_Clear(); // 清空缓存
				return 1;
			}
			ESP8266_Clear();
			delay_ms(10);
		}
		// ESP8266_Clear();
		goto repeat; // 如果超时未收到响应，重新发送命令
	}
	else
	{
		count = 200;
		taskENTER_CRITICAL();  // 进入临界区
		printf("%s\r\n", cmd); // 发送命令
		taskEXIT_CRITICAL();
		while (count--)
		{
			ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
			if (strstr((const char *)Read_Buffer, res) != NULL) // 如果检索到关键词
			{
				ESP8266_Clear(); // 清空缓存
				return 1;
			}
			ESP8266_Clear();
			vTaskDelay(10);
		}
		// ESP8266_Clear();
		return 0; // 如果超时未收到响应，返回0
	}
}

/**************************
 *函数名：MQTT_Get()
 *函数功能：检查接收到的数据中是否包含特定的设备和属性。
 *输入：res1 设备名或关键字，用于识别设备
 *      res2 属性值，用于识别设备的属性状态。
 *输出：如果接收到的数据中包含指定的设备和属性，则返回1，否则返回0。
 **************************/
int MQTT_Get(const char *res1, const char *res2)
{
	// 示例接收数据格式："+MQTTSUBRECV:0,\"$sys/0S43la9qNI/LED/thing/property/set\",58,{\"id\":\"58\",\"version\":\"1.0\",\"params\":{\"LightSwitch\":false}}"
	char *json = NULL;
	ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);

	json = strstr((const char *)Read_Buffer, ONENET_Set);

	if (json != NULL)
	{
		if (strchr(json, '{') != NULL)
		{
			strncpy(json_id, strchr(json, '{'), 11);
		}

		json += strlen(ONENET_Set);

		if (strstr(json, res1) != NULL && strstr(json, res2) != NULL)
		{
			return 1;
		}
	}
	return 0; // 未找到匹配的设备和属性*/
}

/**************************
 *函数名：esp_Init()
 *函数功能：初始化ESP8266模块
 *输入：无
 *输出：0
 **************************/
int Esp8266_Init(void)
{

	/* 1.复位指令 */
	ESP8266_SendCmd("AT+RST", "OK", 1);

	/* 2.设置为station模式 */
	ESP8266_SendCmd("AT+CWMODE=1", "OK", 1);

	/* 3.启动DHCP */
	ESP8266_SendCmd("AT+CWDHCP=1,1", "OK", 1);

	/* 4.连接热点 */
	ESP8266_SendCmd("AT+CWJAP=\"" WIFI_NAME "\",\"" WIFI_WORD "\"", "OK", 1);

	/* 5.配置MQTT用户信息 */
	ESP8266_SendCmd("AT+MQTTUSERCFG=0,1,\"" Equipment_Name "\",\"" ID "\",\"" mqtt_token "\",0,0,\"\"", "OK", 1);

	/* 6.建立MQTT连接 */
	ESP8266_SendCmd("AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1", "OK", 1);

	/* 7.订阅主题 */
	// 主题用于接收服务器对客户端发布消息的回复
	ESP8266_SendCmd("AT+MQTTSUB=0,\"" ONENET_Reply "\",1", "OK", 1);

	// 主题用于接收服务器下发的属性设置命令
	ESP8266_SendCmd("AT+MQTTSUB=0,\"" ONENET_Set "\",1", "OK", 1);

	/* 8.发布消息 */

	ESP8266_SendCmd("AT+MQTTPUB=0,\"" STM32_post "\",\"{\\\"id\\\":\\\"125\\\"\\,\\\"params\\\":{\\\"LightSwitch\\\":{\\\"value\\\":false\\}}}\",0,0", "success", 1);
	ESP8266_SendCmd("AT+MQTTPUB=0,\"" STM32_post "\",\"{\\\"id\\\":\\\"127\\\"\\,\\\"params\\\":{\\\"Servoangle\\\":{\\\"value\\\":\\\"zero\\\"\\}}}\",0,0", "success", 1);

	return 0;
}

/********************************
 *函数名：ShangChuan()
 *函数功能：向服务器发送属性
 *输入：Type 待发送功能属性的数据类型，填写时与标识符同名
 *	  Logo 待发送功能属性的标识符
 *	  ptr  待发送的数据
 *输出：BaseType_t pdTRUE or pdFALSE
 ***********************************/

BaseType_t ShangChuan(int Type, const char *Logo, const char *ptr)
{
	BaseType_t Output = pdFALSE;

	char buffer[156];
	static uint8_t count = 0;
	count++;
	switch (Type)
	{
	case bool:
		snprintf(buffer, sizeof(buffer),
				 "AT+MQTTPUB=0,\"" STM32_post "\",\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%s\\}}}\",0,0\r\n", count, Logo, ptr);
		break;

	case string:
		snprintf(buffer, sizeof(buffer),
				 "AT+MQTTPUB=0,\"" STM32_post "\",\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":\\\"%s\\\"\\}}}\",0,0\r\n", count, Logo, ptr);
		break;

	case doub:
		snprintf(buffer, sizeof(buffer),
				 "AT+MQTTPUB=0,\"" STM32_post "\",\"{\\\"id\\\":\\\"%d\\\"\\,\\\"params\\\":{\\\"%s\\\":{\\\"value\\\":%s\\}}}\",0,0\r\n", count, Logo, ptr);
		break;
	default:
		break;
	}
	if (xSemaphoreTake(Semaphore_Post, 2500) == pdTRUE)
	{
		if (ESP8266_SendCmd(buffer, "success", 0) == 1)
		{
			Output = pdTRUE; // 发送属性到服务器
		}
		// memset(buffer,0,sizeof(buffer));
		xSemaphoreGive(Semaphore_Post); // 释放信号量
	}
	return Output;
}

/********************************
 *函数名：OTA_Init()
 *函数功能：检测是否有新版本需要升级
 *输入：无
 *输出：1 ->有新版本   2 ->无新版本
 ***********************************/
uint8_t OTA_Init(void)
{
	/* 1.复位指令 */
	ESP8266_SendCmd("AT+RST", "OK", 1);

	/* 2.设置为station模式 */
	ESP8266_SendCmd("AT+CWMODE=1", "OK", 1);

	/* 3.连接热点 */
	ESP8266_SendCmd("AT+CWJAP=\"" WIFI_NAME "\",\"" WIFI_WORD "\"", "OK", 1);

	/* 4. 建立TCP连接 */
	ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"" Host "\",80", "CONNECT", 1);

	/* 5. 设置为透传模式 */
	// ESP8266_SendCmd("AT+CIPMODE=1", "OK", 1);

	/* 6. 进入透传发送模式 */
	ESP8266_SendCmd("AT+CIPSEND=287", ">", 1);

	/* 7. 发送版本信息（POST请求） */
	W25Q64_ReadData(0x000300,(uint8_t*)&Up_pack.version,4);
	printf(
		"POST /fuse-ota/RUUPvSGLQd/One/version HTTP/1.1\r\n"
		"Content-Type: application/json\r\n"
		"Authorization: version=2022-05-01&res=userid%%2F447128&et=1969281206&method=md5&sign=E9gCDQbH30%%2BFhkm0xqkbxg%%3D%%3D\r\n"
		"Host: iot-api.heclouds.com\r\n"
		"Content-Length: 41\r\n"
		"\r\n"
		"{\"s_version\":\"%s\", \"f_version\": \"V1.0\"}\r\n"
		"\r\n",
		Up_pack.version);

	delay_ms(3500);
	ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
	if (strstr((const char *)Read_Buffer, "succ") != NULL) // 如果检索到关键词
	{
		printf("上报版本号成功\r\n");
	}
	ESP8266_Clear();

	/* 8. 检测升级任务 */
	ESP8266_SendCmd("AT+CIPSEND=242", ">", 1);
	printf(
		"GET /fuse-ota/RUUPvSGLQd/One/check?type=2&version=1.0 HTTP/1.1\r\n"
		"Content-Type: application/json\r\n"
		"Authorization: version=2022-05-01&res=userid%%2F447128&et=1969281206&method=md5&sign=E9gCDQbH30%%2BFhkm0xqkbxg%%3D%%3D\r\n"
		"Host: iot-api.heclouds.com\r\n"
		"\r\n");
	delay_ms(3500);
	ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
	if (strstr((const char *)Read_Buffer, "not exist") != NULL) // 如果检索到关键词
	{
		printf("已是最新版本\r\n");
		ESP8266_Clear();
		ESP8266_SendCmd("AT+CIPCLOSE", "CLOSED", 1);
		return 0;
	}
	else
	{
		char *ptr = (char *)Read_Buffer;

		// 1. 提取 target (版本号)
		char *target_start = strstr(ptr, "\"target\":\"");
		if (target_start)
		{
			target_start += 10; // 跳过 "\"target\":\""
			char *target_end = strchr(target_start, '\"');
			if (target_end && (target_end - target_start < sizeof(Up_pack.version)))
			{
				strncpy(Up_pack.version, target_start, target_end - target_start);
				//Up_pack.version[target_end - target_start] = '\0';
				ptr = target_end; // 移动指针位置
			}
		}

		// 2. 提取 tid (任务ID)
		char *tid_start = strstr(ptr, "\"tid\":");
		if (tid_start)
		{
			tid_start += 6; // 跳过 "\"tid\":"
			Up_pack.tid = (uint32_t)strtoul(tid_start, NULL, 10);
			ptr = tid_start; // 移动指针位置
			
		}

		// 3. 提取 size (固件大小)
		char *size_start = strstr(ptr, "\"size\":");
		if (size_start)
		{
			size_start += 7; // 跳过 "\"size\":"
			Up_pack.size = (uint32_t)strtoul(size_start, NULL, 10);
			
		}
		ESP8266_Clear();
	}

	/*9.关闭TCP连接*/
	ESP8266_SendCmd("AT+CIPCLOSE", "CLOSED", 1);
	return 1;
}


