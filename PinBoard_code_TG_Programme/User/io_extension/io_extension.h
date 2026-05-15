#ifndef __IO_EXTENSION_H
#define __IO_EXTENSION_H

#include "sys.h"
#include "stm32h7xx_hal.h"


void GPIO_Level_Switch(uint8_t level, uint8_t gpio);
void GPIO_Extension_Init(void);


#endif /* __IO_EXTENSION_H */

