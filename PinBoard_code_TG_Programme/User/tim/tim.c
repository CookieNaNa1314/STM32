/*
 *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
 *File name:
 *Auther:	H_YAN      ID:      Version:  v1.0       Date:2025.07.21
 *Description:	初始化定时器，用于数据接收超时使用
 *Others:
 *History:
		1、Date:
	     Author:
	     Modification:
		2、...

 */


#include "tim.h"   
#include "fdcan.h"   

#include "command_analyse.h"

TIM_HandleTypeDef		TIM6_Handler;			//tim6基本定时器句柄


/*
*********************************************************************************************************
* 函 数 名: TIM6_Init
* 功能说明: 基本定时器6初始化
* 形  参: uint16_t arr, uint16_t psc
*		@ref1: arr：自动装载值
*		@ref2: psc：预分频系数
*	@note：				Tout = ((arr + 1) * (psc + 1)) / Fclk (s);				Fclk：定时器时钟频率，本工程配置的系统频率为400MHz，APB1频率配置为AHB1频率的2分频（100MHz）
*																																					，TIM6挂载在APB1上，APB1挂载在AHB上，所以APB1外设频率为100MHz，定时器频率需*2，所有Fclk=200Mhz
* 返 回 值: 
*********************************************************************************************************
*/
void TIM6_Init(uint16_t arr, uint16_t psc)
{  
  TIM6_Handler.Instance=TIM6;                          		//基本定时器6
  TIM6_Handler.Init.Prescaler=psc;                     		//分频
  TIM6_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    		//向上计数器
  TIM6_Handler.Init.Period=arr;                        		//自动装载值
  TIM6_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1; //时钟分频因子
	if (HAL_TIM_Base_Init(&TIM6_Handler) != HAL_OK)
  {
    Error_Handler();
  }
  
//  HAL_TIM_Base_Start_IT(&TIM6_Handler); //使能定时器6和定时器6更新中断：TIM_IT_UPDATE（不在这里开启定时器）
}


/*
*********************************************************************************************************
* 函 数 名: TIM6_Start
* 功能说明: 开启定时器6中断
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
void TIM6_Start(void)
{
	__HAL_TIM_SET_COUNTER(&TIM6_Handler, 0);		//计数清零
	HAL_TIM_Base_Start_IT(&TIM6_Handler);				//开启定时器中断
}


/*
*********************************************************************************************************
* 函 数 名: TIM6_Stop
* 功能说明: 关闭定时器6中断
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
void TIM6_Stop(void)
{
	__HAL_TIM_SET_COUNTER(&TIM6_Handler, 0);		//计数清零
	HAL_TIM_Base_Stop_IT(&TIM6_Handler);			//会调用HAL_TIM_Base_MspDeInit()
}


/*
*********************************************************************************************************
* 函 数 名: HAL_TIM_Base_MspInit
* 功能说明: 定时器底层驱动使能
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM6)
	{
		__HAL_RCC_TIM6_CLK_ENABLE();            //使能TIM6时钟
		HAL_NVIC_SetPriority(TIM6_DAC_IRQn,1,2);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);          //开启ITM6中断   
	}  
}


/*
*********************************************************************************************************
* 函 数 名: HAL_TIM_Base_MspDeInit
* 功能说明: 定时器底层驱动失能
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM6)
    {
      __HAL_RCC_TIM6_CLK_DISABLE();
      HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
    }
}



/*
*********************************************************************************************************
* 函 数 名: HAL_TIM_PeriodElapsedCallback
* 功能说明: 定时器中断回调函数
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：		0.5s定时
* 返 回 值: 
*********************************************************************************************************
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == (&TIM6_Handler))
    {
      TIM6_Stop();
			
			//处理其他事情
			
			g_fdcan1_rx_specialdata[3 + 2 * g_fdcan1_rx_special_cnt] = 'Y';		//添加'确认'字符
			g_fdcan1_rx_specialdata[0] = 3 + 2 * g_fdcan1_rx_special_cnt;			//重新计算这一帧有效数据长度；			3 + 2 * fdcan1_rx_special_cnt = ('确认'字符+功能命令码)(1 + 2 = 3)  +  子模块ID(2) * 子模块个数(fdcan1_rx_special_cnt)			'长度'不算是有效字节
			
			pindoard_reply_to_pc(0x0000, g_fdcan1_rx_specialdata);						//封装应答帧，回复给PC
			
			memset(g_fdcan1_rx_specialdata, 0, sizeof(g_fdcan1_rx_specialdata));	//数组置0，以便下次使用
			g_fdcan1_rx_special_cnt = 0;																					//计数置0，以便下次使用
    }
}



/*********************************************END OF FILE**********************/
