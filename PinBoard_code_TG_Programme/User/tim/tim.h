#ifndef __TIM_H
#define	__TIM_H

#include "stm32h7xx.h"

extern TIM_HandleTypeDef		TIM6_Handler;


void TIM6_Init(uint16_t arr, uint16_t psc);

void TIM6_Start(void);
void TIM6_Stop(void);

#endif /* __TIM_H */
