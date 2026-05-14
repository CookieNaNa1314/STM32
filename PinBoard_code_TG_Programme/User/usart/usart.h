#ifndef __USART_H
#define	__USART_H

#include "stm32h7xx.h"
#include "main.h"
#include <stdio.h>


#define USART4_RX_BUF_SIZ							128						//串口4接收BUFF数组大小

void USART1_Config(uint32_t bound);

void UART4_Config(uint32_t bound);


void Uart4_SendString(uint8_t *str, uint8_t length);

#endif /* __USART_H */
