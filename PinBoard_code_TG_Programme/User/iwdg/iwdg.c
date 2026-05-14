/*
 *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
 *File name:	iwdg.c
 *Auther:	H_YAN      ID:      Version:  v1.0       Date:2025.07.29
 *Description:看门狗初始化，防止系统卡死
 *Others:
 *History:
		1、Date:
	     Author:
	     Modification:
		2、...

 */


#include "iwdg.h"

IWDG_HandleTypeDef IWDG_Handler; //独立看门狗句柄


/*
*********************************************************************************************************
* 函 数 名: IWDG_Init
* 功能说明: 独立看门狗初始化
* 形  参: uint8_t prer, uint16_t rlr
*		@ref1: prer：看门狗分频系数，0 ~ 7分别代表4  8  16  32  64  128  256分频
*		@ref2: rlr：重装载值，0~0XFFF.
*	@note：看门狗使用系统LSI时钟，为32KHz，时间计算公式为：T(ms) = (( 4 * 2^prer) * rlr) / 32
* 返 回 值: 无
*********************************************************************************************************
*/
void IWDG_Init(uint8_t prer, uint16_t rlr)
{
  IWDG_Handler.Instance=IWDG1;
  IWDG_Handler.Init.Prescaler=prer;    //设置IWDG分频系数
  IWDG_Handler.Init.Reload=rlr;        //重装载
  IWDG_Handler.Init.Window=IWDG_WINDOW_DISABLE;//关闭窗口功能
  if (HAL_IWDG_Init(&IWDG_Handler) != HAL_OK)
  {
    Error_Handler();
  }
}
    

/*
*********************************************************************************************************
* 函 数 名: IWDG_Feed
* 功能说明: 喂独立看门狗
* 形  参: 无
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void IWDG_Feed(void)
{   
    HAL_IWDG_Refresh(&IWDG_Handler); //重装载
}

