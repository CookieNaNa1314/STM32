#ifndef __IWDG_H
#define __IWDG_H
#include "sys.h"

//看门狗配置参数
void IWDG_Init(uint8_t prer, uint16_t rlr);//IWDG初始化
void IWDG_Feed(void);											 //IWDG喂狗

#endif	/*__IWDG_H */
