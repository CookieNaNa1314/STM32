/*
 *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
 *File name:	io_extension.c
 *Auther:	H_YAN      ID:      Version:  v1.0       Date:2025.08.18
 *Description:	IO扩展，用于驱动固态继电器状态切换
 *Others:
 *History:
		1、Date:
	     Author:
	     Modification:
		2、...

 */

#include "io_extension.h"


/*
*********************************************************************************************************
* 函 数 名: GPIO_Level_Switch
* 功能说明: 切换指定端口的电平
* 形  参: uint8_t level, uint8_t gpio
*		@ref1: level：高低电平				0：表示低电平			1：表示高电平
*		@ref2: gpio：指定端口
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
void GPIO_Level_Switch(uint8_t level, uint8_t gpio)
{
	switch(gpio)
	{
		case 0:
			{
				if(level)
				{
					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_3, GPIO_PIN_RESET);
				}
			}
			break;
		case 1:
			{
				if(level)
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
				}
			}
			break;
		case 2:
			{
				if(level)
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
				}
			}
			break;
		case 3:
			{
				if(level)
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
				}
				else
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
				}
			}
			break;
		
		default:
			break;
	}
}


/*
*********************************************************************************************************
* 函 数 名: GPIO_Extension_Init
* 功能说明: 初始化扩展GPIO端口
* 形  参: 无
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void	GPIO_Extension_Init(void)
{
  
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_3; 						//PD3
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;		//推挽输出
	GPIO_InitStruct.Pull = GPIO_NOPULL;						
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//高速
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_13; 				//PC6、PC7、PC13	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;		//推挽输出
	GPIO_InitStruct.Pull = GPIO_NOPULL;						
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//高速
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
}


/*******END OF FILE******/
