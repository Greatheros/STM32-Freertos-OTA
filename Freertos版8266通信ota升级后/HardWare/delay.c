#include "stm32f10x.h"
#include "delay.h"


void Delay_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);  // 使能TIM1时钟
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;              // 自动重载值（最大65535us）
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;           // 预分频器，72MHz/72=1MHz（1计数=1us）
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    TIM_Cmd(TIM1, ENABLE);  // 使能TIM1
}


// 微秒级延时函数
void delay_us(uint32_t us)
{
    uint32_t start, current, elapsed = 0;
    
    // 获取当前计数器值
    start = TIM1->CNT;
    
    // 循环等待直到达到指定微秒数
    while (elapsed < us) 
    {
        current = TIM1->CNT;
        if (current >= start) 
        {
            elapsed = current - start;
        }
        else 
        {
            // 处理计数器溢出（当计数器从65535回到0）
            elapsed = (0xFFFF - start) + current;
        }
    }
}

//毫秒级延时
void delay_ms(uint32_t ms)
{
	while(ms)
	{
		delay_us(1000);
		ms--;
	}
}

