#include "DHT11.h"
#include "delay.h" 


uint8_t RH,RL,TH,TL,revise=0; 

/****************
函数名：DHT11_Init
函数作用：DHT11初始化
输入参数：无
输出参数：无
***************/
void DHT11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);

    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_SDA); // 拉高总线
    delay_ms(1000);
}

/****************
函数名：DHT11_Rst
函数作用：复位DHT11
输入参数：无
输出参数：无
***************/
void DHT11_Rst(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);

    GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_GPIO_SDA); // 拉低总线
    delay_ms(20);                                    // 保持低电平20ms
    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_SDA);   // 松开总线控制
    delay_us(30);                                    // 等待DHT11响应
   
    GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_SDA;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

/****************
函数名：DHT11_Check
函数作用：等待DHT11响应，成功返回1，失败返回0
输入参数：无
输出参数：无
***************/
int DHT11_Check(void)
{
    uint8_t retry = 0;
    // 等待DHT11拉低总线
    while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_SDA)==Bit_SET && retry < 100)
    {
        retry++;
        delay_us(1);
    }
    if(retry >= 100)
    { 
        return 0;    // DHT11没有响应
    }
    else{
        while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_SDA)==Bit_RESET); // 等待DHT11拉高总线
        delay_us(87);
    }

    return 1;      //DHT11响应成功
}


/****************
函数名：Read_bit
函数作用：读取一位数据
输入参数：无
输出参数：0x01 OR 0x00
***************/
uint8_t Read_bit(void)
{
    while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_SDA)==Bit_RESET); // 等待54us低电平结束

    delay_us(40);                         //等待40us
    if(GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_SDA)==Bit_SET) // 如果高电平持续时间大于26us，则为1
    {
        while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_SDA)==Bit_SET); // 等待高电平结束
        return 1; // 返回1
    }
    else
    {
        return 0; // 返回0
    }
    
}


/****************
函数名：Read_Byte
函数作用：读取一字节的数据
输入参数：无
输出参数：Byte（读取到的8位数据，40位为完整的DHT11数据）
***************/
uint8_t Read_Byte(void)
{
    uint8_t Byte=0;
    for ( int8_t i=7;i>=0;i--)
    {
        Byte |= Read_bit()<<i;
    }
    return Byte;
}

/****************
函数名：Read_DHT11_data
函数作用：读取的完整的DHT11数据
输入参数：无
输出参数：1（数据接收成功） OR 0（数据接收失败）
***************/
uint8_t Read_DHT11_data(void)
{
    DHT11_Rst();
    if(DHT11_Check()==1) 
    {
        RH=Read_Byte();    //接收湿度高八位，整数部分  
        RL=Read_Byte();    //接收湿度低八位，小数部分  
        TH=Read_Byte();    //接收温度高八位，整数部分 
        TL=Read_Byte();    //接收温度低八位，小数部分
        revise=Read_Byte(); //接收校正位
        if((RH+RL+TH+TL)==revise)
        {
            return 1; //数据接收成功
        }
        
    }
    return 0;     //数据接收失败
}

/****************
函数名：DHT11_Tset
函数作用：在OLED上显示读取到的DHT11数据
输入参数：无
输出参数：无
***************/
/*void DHT11_Tset(void)
{
    if(Read_DHT11_data()==1)
    {
        OLED_ShowString(0,0,"Temp:",OLED_8X16);
        OLED_ShowNum(45,0,TH,2,OLED_8X16);
        OLED_ShowChar(72,0,'.',OLED_8X16);
        OLED_ShowNum(81,0,TL,2,OLED_8X16);

        OLED_ShowString(0,16,"Humi:",OLED_8X16);
        OLED_ShowNum(45,16,RH,2,OLED_8X16);
        OLED_ShowChar(72,16,'.',OLED_8X16);
        OLED_ShowNum(81,16,RL,2,OLED_8X16);
    }
}*/

