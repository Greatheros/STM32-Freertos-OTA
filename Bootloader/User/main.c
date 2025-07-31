#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "W25Q64.h"
#include "Boot.h"


int main(void)
{
	Usart1_init();
	W25Q64_Init();
	Boot_getflag();
	if(OTA_Info.OTA_flag == OTA_FLAG_KEY)
    {
        printf("更新A区\r\n");
        OTA_Upgrade();
		Load_App(START_ADDRESS);
    }
    else{
        printf("跳转至A区\r\n");
        Load_App(START_ADDRESS);
    }

}

