/*
 *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
 *File name:	usart.c
 *Auther:		H_YAN	      ID:      Version: V1.0        Date:2025.08.18
 *Description:	用于定义串口功能
 *Others:无
 *History:
		1、Date:
	     Author:
	     Modification:
		2、...

 */
#include "usart.h"

#include "crc.h"

#include "debug.h"

#include "string.h"
#include "ring_buffer_chapter.h"


static uint8_t	usart4_rxbuffer[USART4_RX_BUF_SIZ] = {0};		//串口4存储接收数据数组，用于存储  上位机（PC）  发来的数据
static uint16_t usart4_rx_len = 0;													//串口4接收有效数据长度

static uint8_t huart4_error_cnt = 0;												//用于存储当前接收错误计数，若超过指定次数，则认为串口通信异常，重启本串口

UART_HandleTypeDef debug_huart1 = {0};                //串口1句柄，用于debug输出
UART_HandleTypeDef rs485_huart4 = {0};                //串口4-485通信句柄


/*
*********************************************************************************************************
* 函 数 名: rs485_uart4_re
* 功能说明: RS485_uart4使能引脚电平配置
* 形  参: uint8_t state
*		@ref1: state：状态				0：表示接收状态			1：表示发送状态					一般默认情况下应该要处于接收状态
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
static void	rs485_uart4_re(uint8_t state)
{
//	delay_ms(15);		//延时15ms，等待RS485芯片反应————（2025.11.28修改，485发送添加TC检测后，可以不用延迟）
	if(state)
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	}
//	delay_ms(15);		//延时15ms，等待RS485芯片反应————（2025.11.28修改，485发送添加TC检测后，可以不用延迟）
}


/*
 *********************************************************************************************************
 * 函 数 名: USART1_Config
 * 功能说明: 串口1初始化，用于调试输出
 * 形  参: uint32_t bound
 *		@ref1:	bound：波特率
 *		@ref2:
 *	@note：
 * 返 回 值:无
 *********************************************************************************************************
 */ 
void USART1_Config(uint32_t bound)
{
  debug_huart1.Instance = USART1;
	debug_huart1.Init.BaudRate = bound;				    //波特率
	debug_huart1.Init.WordLength = UART_WORDLENGTH_8B;   //字长为8位数据格式
	debug_huart1.Init.StopBits = UART_STOPBITS_1;	    //一个停止位
	debug_huart1.Init.Parity = UART_PARITY_NONE;		    //无奇偶校验位
	debug_huart1.Init.Mode = UART_MODE_TX_RX;		    //收发模式
	debug_huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;   //无硬件流控
	debug_huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	
	if (HAL_UART_Init(&debug_huart1) != HAL_OK)
  {
    Error_Handler();
  }
}


/*
 *********************************************************************************************************
 * 函 数 名: UART4_Config
 * 功能说明: 串口4初始化
 * 形  参: uint32_t bound
 *		@ref1:	bound：波特率
 *		@ref2:
 *	@note：
 * 返 回 值:无
 *********************************************************************************************************
 */ 
void UART4_Config(uint32_t bound)
{
  rs485_huart4.Instance = UART4;
	rs485_huart4.Init.BaudRate = bound;				    //波特率
	rs485_huart4.Init.WordLength = UART_WORDLENGTH_8B;   //字长为8位数据格式
	rs485_huart4.Init.StopBits = UART_STOPBITS_1;	    //一个停止位
	rs485_huart4.Init.Parity = UART_PARITY_NONE;		    //无奇偶校验位
	rs485_huart4.Init.Mode = UART_MODE_TX_RX;		    //收发模式
	rs485_huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;   //无硬件流控
	rs485_huart4.Init.OverSampling = UART_OVERSAMPLING_16;
	
	if (HAL_UART_Init(&rs485_huart4) != HAL_OK)
  {
    Error_Handler();
  }
	__HAL_UART_ENABLE_IT(&rs485_huart4, UART_IT_RXNE);		//使能串口接收中断
	__HAL_UART_ENABLE_IT(&rs485_huart4, UART_IT_IDLE);		//使能串口空闲中断
	
	rs485_uart4_re(0);				//初始化为接收状态
}


/*
*********************************************************************************************************
* 函 数 名: HAL_UART_MspInit
* 功能说明: 串口端口初始化
* 形  参: UART_HandleTypeDef *huart
*		@ref1: huart：UART句柄
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure = {0};
	
	if(huart->Instance == USART1)//如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
	
		GPIO_Initure.Pin = GPIO_PIN_9;			//PA9——>USART1_TX
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull = GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;//高速
		GPIO_Initure.Alternate = GPIO_AF7_USART1;	//复用为USART1
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//初始化PA9

		GPIO_Initure.Pin = GPIO_PIN_10;			//PA10——>USART1_RX
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);	   	//初始化PA10
	}
	
	if(huart->Instance == UART4)//如果是串口4，进行串口4 MSP初始化
	{
		__HAL_RCC_GPIOB_CLK_ENABLE();			//使能GPIOB时钟
		__HAL_RCC_UART4_CLK_ENABLE();			//使能UART4时钟
	
		GPIO_Initure.Pin = GPIO_PIN_9;			//PB9——>RS_485_UART4_TX
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull = GPIO_PULLDOWN;			//上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;//高速
		GPIO_Initure.Alternate = GPIO_AF8_UART4;	//复用为UART4
		HAL_GPIO_Init(GPIOB, &GPIO_Initure);	   	//初始化PB9

		GPIO_Initure.Pin = GPIO_PIN_8;			//PB8——>RS_485_UART4_RX
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull = GPIO_PULLUP;			//上拉，必须上拉，否则每次切换485时，会额外发送一个0x00字节无用数据
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;//高速
		GPIO_Initure.Alternate = GPIO_AF8_UART4;	//复用为UART4
		HAL_GPIO_Init(GPIOB, &GPIO_Initure);	   	//初始化PB8
		
		GPIO_Initure.Pin = GPIO_PIN_7; 			//PB7——>RS_485_UART4_EN
		GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;		//推挽输出
		GPIO_Initure.Pull = GPIO_PULLUP;						//上拉
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;//高速
		HAL_GPIO_Init(GPIOB,&GPIO_Initure);	   				//初始化PB7
		
		HAL_NVIC_SetPriority(UART4_IRQn, 0, 1);			//抢占优先级0，子优先级1
		HAL_NVIC_EnableIRQ(UART4_IRQn);				//使能UART4中断通道	
	}

}


/*
*********************************************************************************************************
* 函 数 名: HAL_UART_MspDeInit
* 功能说明: 串口失能
* 形  参: UART_HandleTypeDef* huart
*		@ref1: huart：UART句柄
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9      ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
	if(huart->Instance==UART4)
	{
		__HAL_RCC_UART4_CLK_DISABLE();
		
		/**UART4 GPIO Configuration
    PB9      ------> RS_485_UART4_TX
    PB8      ------> RS_485_UART4_RX
		PB7      ------> RS_485_UART4_EN
    */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9);
	}
}


/*
*********************************************************************************************************
* 函 数 名: UART4_IRQHandler
* 功能说明: 串口4中断函数
* 形  参: 
*		@ref1: 
*		@ref2: 
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void UART4_IRQHandler(void)
{
	uint8_t rx_data = 0;
	
	if(__HAL_UART_GET_FLAG(&rs485_huart4, UART_FLAG_RXNE) != RESET)  //如果接收到了一个字节的数据
	{
		if(usart4_rx_len >= sizeof(usart4_rxbuffer))	//若接收的一帧字节数大于数组上限
		{
			usart4_rx_len = 0;
		}
		HAL_UART_Receive(&rs485_huart4, &rx_data, 1, 10);	//此处已清除接收中断标志位
		
		usart4_rxbuffer[usart4_rx_len++] = rx_data;		//存储数据
	}
	
	if(__HAL_UART_GET_FLAG(&rs485_huart4, UART_FLAG_IDLE) != RESET)//如果接受到了一帧数据
	{ 
		if(usart4_rxbuffer[0] == 0x01)
		{
			uint16_t	calculate_frame_crc = crc16_ccitt_false(usart4_rxbuffer, usart4_rx_len - 2);;						//重新计算当前帧的CRC校验码
			
			uint16_t  current_frame_crc = usart4_rxbuffer[usart4_rx_len - 1];
			current_frame_crc = current_frame_crc << 8 | usart4_rxbuffer[usart4_rx_len - 2];									//获得当前传过来数据帧的CRC(小端序)
			
			if(calculate_frame_crc == current_frame_crc)		//若CRC校验一致(即数据帧接收正确)
			{
				RBC_Write_String(&rbc, usart4_rxbuffer, usart4_rx_len);		//写入分段环形缓冲区
				RBC_Ending_Chapter(&rbc);																	//记录分段结尾
				
				memset(usart4_rxbuffer, 0, usart4_rx_len);								//接收BUFF清零
				usart4_rx_len = 0;																				//接收字节计数清零
			}
			else
			{
				memset(usart4_rxbuffer, 0, usart4_rx_len);
				usart4_rx_len = 0;
				Debug_Printf("\r\n此PC帧CRC校验异常，原CRC为：%X，重新计算的CRC为：%X，丢弃。\r\n",current_frame_crc, calculate_frame_crc);
			}
			
			huart4_error_cnt = 0;  //异常计数清零
		}
		else
		{
			for(uint8_t i = 0; i < usart4_rx_len; i++)
			{
				printf("0x%02X ", usart4_rxbuffer[i]);
			}
			printf("长度为：%d \r\n", usart4_rx_len);
			
			if(usart4_rx_len == 0)		//接收异常(2025.11.03添加)
			{
				if(huart4_error_cnt >= 2)		//连续两次接收到异常帧，则重启本串口
				{
					UART4_Config(115200);		//重新初始化
				}
				huart4_error_cnt++;
			}
			
			memset(usart4_rxbuffer, 0, usart4_rx_len);
			usart4_rx_len = 0;
			Debug_Printf("\r\n此PC帧不符合通信要求，丢弃。\r\n");
		}
		__HAL_UART_CLEAR_IDLEFLAG(&rs485_huart4);//清除标志位
	 }
  HAL_UART_IRQHandler(&rs485_huart4);
}


/*
*********************************************************************************************************
* 函 数 名: Uart4_SendString
* 功能说明: 串口4发送数据
* 形  参: uint8_t *str, uint8_t length
*		@ref1: str：需发送的字符串
*		@ref2: length：发送字符串的长度
*	@note：
* 返 回 值: 无
*********************************************************************************************************
*/
void Uart4_SendString(uint8_t *str, uint8_t length)
{
	unsigned int k = 0;
	
	rs485_uart4_re(1);		//切换发送态
  do 
  {
     HAL_UART_Transmit(&rs485_huart4, (uint8_t *)(str + k), 1, 10);
     k++;
  } while(k < length);
	
	while(__HAL_UART_GET_FLAG(&rs485_huart4, UART_FLAG_TC) == RESET);			//等待TC置位，2025.11.28添加，添加该语句后，485切换可以不用延时
	rs485_uart4_re(0);		//切换接收态
}



///重定向c库函数printf到串口DEBUG_USART，重定向后可使用printf函数
int fputc(int ch, FILE *f)
{
	/* 发送一个字节数据到串口DEBUG_USART */
	HAL_UART_Transmit(&debug_huart1, (uint8_t *)&ch, 1, 10);	
	
	return (ch);
}


///重定向c库函数scanf到串口DEBUG_USART，重写向后可使用scanf、getchar等函数
int fgetc(FILE *f)
{
		
	int ch;
	HAL_UART_Receive(&debug_huart1, (uint8_t *)&ch, 1, 10);	
	return (ch);
}
/*********************************************END OF FILE**********************/
