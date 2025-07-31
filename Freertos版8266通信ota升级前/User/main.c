#include "stm32f10x.h"                  // Device header
#include "Usart.h"
#include "esp8266.h"
#include "freertos_demo.h"
 

int main(void)
{
    Delay_Init();
	Usart1_init();
    OLED_Init();
    LED_Init();
    Servo_Init();
    W25Q64_Init();
    DHT11_Init();
    Freertos_start();  //启动freertos任务调度器
    while (1)
    {
        printf("Freertos意外退出,请重启!");
        OLED_Clear();
        OLED_ShowString(0,32,"Unexpected exit",OLED_6X8);
        OLED_Update();
    }
    
}



