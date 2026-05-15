 /*
  *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
  *File name:	main.c
  *Auther:	H_YAN      ID:      Version:  v1.0       Date:2025.06.24	
  *Description:	主函数
  *Others:
  *History:
 		1、Date:
 	     Author:
 	     Modification:
 		2、...
 
  */
 

#include "main.h"
#include "DeviceConfigs.h"						//该头文件包含了所有的头文件，仅需在此处调用一次即可 

/*
*********************************************************************************************************
* 函 数 名: main
* 功能说明: 主函数
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
int main(void)
{
	Cache_Enable(); 										//打开L1-Cache
	
	HAL_Init();													//初始化HAL库
	
	Stm32_Clock_Init(160,5,2,80);				//系统时钟初始化成400MHz，同时配置FDCAN系统时钟为20M
	
	delay_init(400);										//初始化延时函数
	
	IWDG_Init(IWDG_PRESCALER_128, 1250);//看门狗初始化，5s超时，128分频最短超时时间为4ms，最长超时时间为16384ms
	
	TIM6_Init(9999, 9999);							//配置定时0.5s
	
	crc16_init();												//CRC校验初始化
	
	Ring_Buffer_Chapter_Init();					//分段环形缓冲区初始化
	
//	GPIO_Extension_Init();							//初始化扩展的GPIO端口

	USART1_Config(115200);							//配置串口1为：115200 8-N-1，调试输出
	UART4_Config(115200);								//配置串口4为：115200 8-N-1，PC通信(RS485)
	
	FDCAN1_Init();											//初始化配置FDCAN1
	
	Debug_Printf("通讯转接板初始化成功-TG1000\r\n");

  while(1)
	{	
		pc_to_pinboard_command_analyse();	//处理PC上位机指令	
		IWDG_Feed();											//喂狗
	}  
}


/*
*********************************************************************************************************
* 函 数 名: Error_Handler
* 功能说明: 错误处理
* 形  参: 无
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}


/*
*********************************************************************************************************
* 函 数 名: assert_failed
* 功能说明: assert（断言）调试失败函数，负责输出调试错误信息
* 形  参: uint8_t* file, uint32_t line
*		@ref1: file：出错文件位置
*		@ref2: line：出错行数
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line)
{ 
	Debug_Printf("Wrong parameters value: file %s on line %d\r\n", file, line);
	while (1)
	{}
}
#endif

/****************************END OF FILE***************************/
