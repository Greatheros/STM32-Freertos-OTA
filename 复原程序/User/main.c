#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "W25Q64.h"

uint8_t MID;							//定义用于存放MID号的变量
uint16_t DID;							//定义用于存放DID号的变量


uint32_t arr=0x12345679;
uint32_t brr=0;


int main(void)
{
	/*模块初始化*/
	OLED_Init();						//OLED初始化
	W25Q64_Init();						//W25Q64初始化
	
	/*显示静态字符串*/
	OLED_ShowString(1, 1, "MID:   DID:");
	OLED_ShowString(2, 1, "W:");
	OLED_ShowString(3, 1, "R:");
	
	/*显示ID号*/
	W25Q64_ReadID(&MID, &DID);			//获取W25Q64的ID号
	OLED_ShowHexNum(1, 5, MID, 2);		//显示MID
	OLED_ShowHexNum(1, 12, DID, 4);		//显示DID
	
	/*W25Q64功能函数测试*/
	W25Q64_SectorErase(0x000000);					//扇区擦除
	W25Q64_SectorErase(0x000300);
	W25Q64_PageProgram(0x000000, (uint8_t*)&arr, 4);	//将写入数据的测试数组写入到W25Q64中
	
	W25Q64_PageProgram(0x000300, "V1.0", 4);
	
	W25Q64_ReadData(0x000000, (uint8_t*)&brr, 4);		//读取刚写入的测试数据到读取数据的测试数组中
	
	
	OLED_ShowHexNum(3, 3, (uint8_t)(brr>>24), 2);		
	OLED_ShowHexNum(3, 6, (uint8_t)(brr>>16), 2);		
	OLED_ShowHexNum(3, 9, (uint8_t)(brr>>8), 2);		
	OLED_ShowHexNum(3, 12, (uint8_t)brr, 2);		

	while (1)
	{
		
	}
}
