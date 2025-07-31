#include "BOOT.H"

//OTA
OTA_InfoCB OTA_Info;

//定义用户程序复位函数类型
typedef void (*UserResethandler)(void);
//定义用户程序复位函数
UserResethandler UserReset;

/********************************
 *函数名：Bootloader_Clear()
 *函数功能：Bootloader运行后的善后，关闭所使用的外设
 *输入：无
 *输出：无
 ***********************************/
void Bootloader_Clear(void)
{
    GPIO_DeInit(GPIOA);
    SPI_I2S_DeInit(SPI1);	
    USART_DeInit(USART1);
    DMA_DeInit(DMA1_Channel5);
}


/********************************
 *函数名： Load_App(uint32_t address)
 *函数功能：加载用户APP程序，跳转到A去用户程序
 *输入：address ->A区起始地址
 *输出：无
 ***********************************/
void Load_App(uint32_t address)
{
    //如果值存在SRAM里（20kb）
    if(*(uint32_t *)address>=0x20000000 && *(uint32_t *)address <0x20005000)  
    {
        Bootloader_Clear();
        __disable_irq();
        __set_MSP(*(uint32_t *)START_ADDRESS);  //设置主堆栈指针。重新设定栈顶代地址，把栈顶地址设置为用户代码指向的栈顶地址。
        UserReset = (UserResethandler)*(uint32_t *)(START_ADDRESS+4);  //获取用户程序的复位处理函数地址。
        UserReset();                            //跳转到用户程序
    }
}

/********************************
 *函数名： GetTid（）
 *函数功能：从外部存储设备获取升级Tid值与文件大小
 *输入：无
 *输出：无
 ***********************************/
void GetTid(void)
{
    W25Q64_ReadData(0x000100,(uint8_t *)&OTA_Info.OTA_Tid,4);   //获取TID
    W25Q64_ReadData(0x000200,(uint8_t *)&OTA_Info.OTA_Size,4);  //获取bin文件大小
}

/********************************
 *函数名：SectorErase（）
 *函数功能：擦除外部寄存器所需的扇区，用于等待存放即将到来的升级包数据
 *输入：无
 *输出：无
 ***********************************/
void SectorErase(void)
{
    //擦除扇区，准备从0x001000开始写入，所以从这里擦除
    OTA_Info.sector = (OTA_Info.OTA_Size + 4095) / 4096;  
    for(uint16_t i = 1; i <= OTA_Info.sector; i++)
    {
        W25Q64_SectorErase(i * Download_Address);     
    }

}

/**
 * 从HTTP响应中提取实际数据（跳过响应头）
 * @param buffer 包含完整HTTP响应的缓冲区
 * @param len 缓冲区有效数据长度
 * @return 指向实际数据起始位置的指针，若未找到分隔符则返回NULL
 */
uint8_t* ExtractHttpData(uint8_t* buffer, uint16_t len)
{
    // 至少需要4字节才能包含\r\n\r\n
    if (len < 4) return NULL;
    
    // 查找\r\n\r\n（0x0D 0x0A 0x0D 0x0A）
    for (uint16_t i = 0; i <= len - 4; i++) {
        if (buffer[i] == 0x0D && buffer[i+1] == 0x0A && 
            buffer[i+2] == 0x0D && buffer[i+3] == 0x0A) {
            // 找到分隔符，返回其后的位置（即数据起始位置）
            return &buffer[i + 4];
        }
    }
    
    // 未找到分隔符（可能数据不完整）
    return NULL;
}


/********************************
 *函数名：Download（）
 *函数功能：下载升级包
 *输入：无
 *输出：无
 ***********************************/
void Download(void)
{
    //分片下载，每片256字节
    uint16_t total_chunks = (OTA_Info.OTA_Size + 256 - 1) / 256;     //计算片数
    for (uint16_t i = 0; i < total_chunks; i++)
    {
        uint32_t start = i * 256;
        uint32_t end = start + 256 - 1;

        if (end >= OTA_Info.OTA_Size) {
            end = OTA_Info.OTA_Size - 1;         // 最后一个分片可能不足256字节
        }

        download = 0;                   //重置下载标志
        printf(
            "GET /fuse-ota/RUUPvSGLQd/One/%u/download HTTP/1.1\r\n"
            "Authorization: version=2022-05-01&res=userid%%2F447128&et=1969281206&method=md5&sign=E9gCDQbH30%%2BFhkm0xqkbxg%%3D%%3D\r\n"
            "Host: iot-api.heclouds.com\r\n"
            "Range:bytes=%u-%u\r\n"
            "\r\n",
            OTA_Info.OTA_Tid          // 使用之前提取的tid值
            ,start,end
        );
        while(download==0);         //等待分片下载完成

        uint16_t recv_len = ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
        uint8_t* data_start = ExtractHttpData((uint8_t *)Read_Buffer, recv_len);
        if (data_start != NULL) 
        {
            uint16_t data_len = (uint16_t)((uint8_t*)Read_Buffer + recv_len - data_start);
            W25Q64_PageProgram(Download_Address+i*256,data_start,data_len);
        }
        ESP8266_Clear();

    }

}


/********************************
 *函数名：OTA_Init（）
 *函数功能：OTA下载工作的全流程，最终将下载的数据存到外部存储器中
 *输入：无
 *输出：无
 ***********************************/
uint8_t OTA_Init(void)
{
	/* 1.复位指令 */
	ESP8266_SendCmd("AT+RST", "OK");

	/* 2.设置为station模式 */
	ESP8266_SendCmd("AT+CWMODE=1", "OK");

	/* 3.连接热点 */
	ESP8266_SendCmd("AT+CWJAP=\"" WIFI_NAME "\",\"" WIFI_WORD "\"", "OK");

	/* 4. 建立TCP连接 */
	ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"" Host "\",80", "CONNECT");

	/* 5. 设置为透传模式 */
	 ESP8266_SendCmd("AT+CIPMODE=1", "OK");

	/* 6. 进入透传发送模式 */
	ESP8266_SendCmd("AT+CIPSEND", ">");

    /*7.擦除扇区*/
    SectorErase();

    /*8.下载升级包*/
    Download();

    /*9.退出透传*/
	Delay_ms(1000);
	printf("+++");
	Delay_ms(1000);

    /*10.关闭TCP连接*/
	ESP8266_SendCmd("AT+CIPCLOSE", "CLOSED");

    printf("下载完成\r\n");
    return 1;

}

/********************************
 *函数名：FLASH_Transport()
 *函数功能：将W25Q64中的升级包资源搬运到单片机FLASH中，实现升级
 *输入：无
 *输出：无
 ***********************************/
void FLASH_Transport(void)
{   
    uint32_t data;
    uint16_t total_chunks = (OTA_Info.OTA_Size + 1024 - 1) / 1024;     //计算页数
    uint32_t total_word  = (OTA_Info.OTA_Size + 4 - 1) / 4;           //计算字数
    for (uint16_t i = 0; i < total_chunks; i++)
    {
        MyFLASH_ErasePage(START_ADDRESS+i*1024);      //擦除页
    }
    for(uint32_t j = 0;j<total_word;j++)
    {
        W25Q64_ReadData(Download_Address+j*4,( uint8_t *)&data,4);
        MyFLASH_ProgramWord(START_ADDRESS+j*4,data);
    }
    W25Q64_PageProgram(0x000300,"V2.0",4);                  //更新版本号

}

/********************************
 *函数名：OTA_Upgrade()
 *函数功能：进行OTA更新升级
 *输入：无
 *输出：无
 ***********************************/
void OTA_Upgrade(void)
{
    GetTid();
    OTA_Init();
    FLASH_Transport();
	OTA_Info.OTA_flag = 0x87654321;
	W25Q64_PageProgram(0x000000,(uint8_t*)&OTA_Info.OTA_flag,4);
}

/********************************
 *函数名：Boot_getflag()
 *函数功能：从W25Q64中获取OTA_flag值
 *输入：无
 *输出：无
 ***********************************/
void Boot_getflag(void)
{
    W25Q64_ReadData(0x000000,(uint8_t*)&OTA_Info.OTA_flag,4);
}

