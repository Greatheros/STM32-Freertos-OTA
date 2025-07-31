#include "IDWG.h"

void IWDG_Init(void) 
{
    // 使能写访问（解锁IWDG）
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    
    // 设置分频系数和重装载值，20秒内喂狗
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    IWDG_SetReload(6250-1);
    
    // 重载计数器（首次喂狗）
    IWDG_ReloadCounter();
    
    // 使能看门狗
    IWDG_Enable();
}

