/*
 *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
 *File name:
 *Auther:	H_YAN      ID:      Version:  v1.0       Date:2025.07.01	
 *Description:	FDCAN通信配置
 *Others:
 *History:
		1、Date:
	     Author:
	     Modification:
		2、...

 */

#include "fdcan.h"   
#include "command_analyse.h"

#include "tim.h"

#include "delay.h"
#include "debug.h"


static const uint8_t	VALID_DATA_SIZE = 62;				//有效数据大小

static	uint8_t	fdcan1_receive_count = 0;					//中断接收帧数计数

static  uint8_t	fdcan1_rxdata[256] = {0};					//存储FDCAN接收到的子模块应答数据，大小需为64的倍数，且要为全局变量（因要接收分包）

uint8_t	g_fdcan1_rx_specialdata[64] = {0};				//存储FDCAN接收到的子模块的特殊应答帧（全局，一般是存储FDCAN广播给所有子模块后，子模块返回的数据）

uint8_t	g_fdcan1_rx_special_cnt = 0;							//用于接收特殊应答帧计数（全局）

FDCAN_HandleTypeDef 		hFDCan1 = {0};						//FDCAN1句柄
FDCAN_RxHeaderTypeDef 	FDCAN1_Rx = {0};					//FDCAN1 发送句柄
FDCAN_TxHeaderTypeDef 	FDCAN1_Tx = {0};					//FDCAN1 接收句柄

/*
*********************************************************************************************************
* 函 数 名: HAL_FDCAN_MspInit
* 功能说明: FDCAN端口初始化
* 形  参: 	FDCAN_HandleTypeDef* hfdcan
*		@ref1:	hfdcan：FDCAN句柄
*		@ref2:
* 返 回 值:	无
*********************************************************************************************************
*/
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef* hfdcan)
{
  GPIO_InitTypeDef 	GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef FDCAN_PeriphClk;
  
  //FDCAN时钟源配置为PLL1Q
  FDCAN_PeriphClk.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
  FDCAN_PeriphClk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
  HAL_RCCEx_PeriphCLKConfig(&FDCAN_PeriphClk);
    
	if(hfdcan->Instance == FDCAN1)
	{
		__HAL_RCC_FDCAN_CLK_ENABLE();             //使能FDCAN时钟
		__HAL_RCC_GPIOH_CLK_ENABLE();			        //开启GPIOH时钟
		
		/**FDCAN1 GPIO Configuration    
		PH14     ------> FDCAN1_RX
		PH13     ------> FDCAN1_TX 
		*/
		GPIO_InitStruct.Pin = GPIO_PIN_13;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
	
		GPIO_InitStruct.Pin = GPIO_PIN_14;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
		HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
	
		HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 1);
		HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);   
	}
}


/*
*********************************************************************************************************
* 函 数 名: HAL_FDCAN_MspDeInit
* 功能说明: 失能FDCAN
* 形  参: FDCAN_HandleTypeDef* hfdcan
*		@ref1:	hfdcan：FDCAN句柄
*		@ref2:
* 返 回 值:	无
*********************************************************************************************************
*/
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef* hfdcan)
{
	if(hfdcan->Instance == FDCAN1)
	{
		__HAL_RCC_FDCAN_FORCE_RESET();       //复位FDCAN1
    __HAL_RCC_FDCAN_RELEASE_RESET();     //停止复位
		
		HAL_GPIO_DeInit(GPIOH, GPIO_PIN_13 | GPIO_PIN_14);
	}
}


/*
*********************************************************************************************************
* 函 数 名: FDCAN1_Init
* 功能说明: FDCAN1外设初始化
* 形  参: 	配置仲裁段和数据段波特率一致，均为1MHz，计算公式：波特率=FD_CLK/(seg1+seg2+1)	，FD_CLK已配置为20M
*		@ref1:
*		@ref2:
* 返 回 值:	无
*********************************************************************************************************
*/
void FDCAN1_Init(void)
{ 
  FDCAN_FilterTypeDef 	FDCAN_RXFilter = {0};        
  
  /*CAN单元初始化*/ 
  hFDCan1.Instance = FDCAN1;
  hFDCan1.Init.FrameFormat = FDCAN_FRAME_FD_BRS;           //FD模式  
  hFDCan1.Init.Mode = FDCAN_MODE_NORMAL;                   //正常模式
  hFDCan1.Init.AutoRetransmission = ENABLE;                //打开自动重传，传统模式下关闭
  hFDCan1.Init.TransmitPause = DISABLE;                    //关闭传输暂停    
  hFDCan1.Init.ProtocolException = DISABLE;                  //关闭协议异常处理
	
  hFDCan1.Init.NominalPrescaler = 1;                       //分频系数
  hFDCan1.Init.NominalSyncJumpWidth = 5;                 	 //重新同步跳跃宽度
  hFDCan1.Init.NominalTimeSeg1 = 14;                       //采样点之前标志时间段:2~256
  hFDCan1.Init.NominalTimeSeg2 = 5;                        //采样点之后标志时间段:2~128
  hFDCan1.Init.DataPrescaler = 1;                          //数据段预分频
  hFDCan1.Init.DataSyncJumpWidth = 5;                    	 //数据位同步跳跃宽度
  hFDCan1.Init.DataTimeSeg1 = 14;                          //采样点之前数据时间段
  hFDCan1.Init.DataTimeSeg2 = 5;                           //采样点之后数据时间段
	
  hFDCan1.Init.MessageRAMOffset = 0;                       //指定消息RAM起始地址
  hFDCan1.Init.StdFiltersNbr = 0;                          //标准信息ID滤波器编号，此参数配置需 >0 否则无法进入中断，它的作用是你需要过滤掉标准CANID的数量
  hFDCan1.Init.ExtFiltersNbr = 1;                          //扩展信息ID滤波器编号
	
  hFDCan1.Init.RxFifo0ElmtsNbr = 4;                        //接收FIFO0元素编号，也就是同时能够接收多少个报文，需配合 FDCAN_IT_RX_FIFO0_FULL 和 FDCAN_IT_RX_FIFO0_WATERMARK 中断使用，
																													 //此处使用的是 FDCAN_IT_RX_FIFO0_NEW_MESSAGE 中断，所以该设置无明显作用
  hFDCan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_64;       //接收FIFO0元素大小：64字节
  hFDCan1.Init.RxBuffersNbr = 0;                           //接收缓冲编号
  hFDCan1.Init.TxEventsNbr = 0;                            //发送事件编号
  hFDCan1.Init.TxBuffersNbr = 0;                           //发送缓冲编号
  hFDCan1.Init.TxFifoQueueElmtsNbr = 10;                    //发送FIFO序列元素编号，也就是同时能够发送多少个报文（重！！！）
  hFDCan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;  //发送FIFO序列模式
  hFDCan1.Init.TxElmtSize = FDCAN_DATA_BYTES_64;            //发送大小:64字节
  
  if(HAL_FDCAN_Init(&hFDCan1) != HAL_OK)
	{
		Error_Handler();
	}

  /* 配置RX滤波器 */
  FDCAN_RXFilter.IdType = FDCAN_EXTENDED_ID;                //扩展ID
  FDCAN_RXFilter.FilterIndex = 0;                           //滤波器编号，使用几个滤波器就配置几个(同上面的 StdFiltersNbr)，依次类推0，1，2，...
  FDCAN_RXFilter.FilterType = FDCAN_FILTER_MASK;            //滤波器类型，掩码模式
  FDCAN_RXFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;    //过滤器0关联到FIFO1  
  FDCAN_RXFilter.FilterID1 = 0x00000000;                    //32位ID1，0x00000000 此配置为接收所有ID
  FDCAN_RXFilter.FilterID2 = 0x00000000;                    //32位ID2，0x00000000 此配置为接收所有ID
  if(HAL_FDCAN_ConfigFilter(&hFDCan1, &FDCAN_RXFilter) != HAL_OK)
  {
     Debug_Printf("滤波器初始失败\n");
  }

	/* 拒绝接收匹配不成功的标准ID和扩展ID,不接受标准/扩展远程帧 */
	if(HAL_FDCAN_ConfigGlobalFilter(&hFDCan1,FDCAN_REJECT,FDCAN_REJECT,ENABLE,ENABLE) != HAL_OK)
	{
		Error_Handler();
	}
	
  /* 配置接收 FIFO 0 watermark 为 2 */
//  HAL_FDCAN_ConfigFifoWatermark(&hFDCan1, FDCAN_CFG_RX_FIFO0, 2);	//该配置类似于阈值作用，若RxFifo0ElmtsNbr配置为5，watermark配置为3，
																																		//那么在连续接收3个报文后，会触发watermark中断，防止丢包（需配置FDCAN_IT_RX_FIFO0_WATERMARK中断才有用）
  
	/* 使能FDCAN接收中断 */
  HAL_FDCAN_ActivateNotification(&hFDCan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);		//新消息中断，即一有消息就触发
	
	/* 开启FDCAN */
  HAL_FDCAN_Start(&hFDCan1);                               
}


/*
*********************************************************************************************************
* 函 数 名: FDCAN1_Send_Msg
* 功能说明: FDCAN发送数据
* 形  参: uint32_t id, uint8_t* msg, uint32_t len
*		@ref1:	id:	需要发送的帧ID
*		@ref2:	msg：需要发送的数据
*		@ref3:	len：需要发送数据的长度
*	@note：
* 返 回 值: 0
*********************************************************************************************************
*/
uint8_t FDCAN1_Send_Msg(uint32_t id, uint8_t* msg, uint32_t len)
{
	assert_param(id >= 0);
	assert_param(msg != NULL);
	assert_param((len >= 0) && (len <= 0x1FFFFFFF));
		
	uint8_t	total_frame = (len / VALID_DATA_SIZE) + 1;				//计算需要发送的总帧数
//	Debug_Printf("\r\n总帧数为：%d\r\n", total_frame);
	
	for(uint8_t i = 0; i < total_frame; i++)		//循环发送
	{
		uint8_t	tx_data[64] = {0};
		tx_data[0] = i + 1;												//帧序号，从1开始
		tx_data[1] = total_frame;
		
		if(tx_data[0] == tx_data[1])							//此时为最后一帧
		{
			uint8_t	remainder = len % VALID_DATA_SIZE;									//计算最后一帧的有效数据数量(0~62)
			memcpy(&tx_data[2], &msg[i * VALID_DATA_SIZE], remainder);	//不足64字节的，用0补齐，防止地址下标溢出
		}
		else
		{
			memcpy(&tx_data[2], &msg[i * VALID_DATA_SIZE], VALID_DATA_SIZE);	//满足64字节的
		}
		
		FDCAN1_Tx.Identifier = id;                             //32位ID
		FDCAN1_Tx.IdType = FDCAN_EXTENDED_ID;                  //扩展ID
		FDCAN1_Tx.TxFrameType = FDCAN_DATA_FRAME;              //数据帧
		FDCAN1_Tx.DataLength = FDCAN_DLC_BYTES_64;             //数据长度
		FDCAN1_Tx.ErrorStateIndicator = FDCAN_ESI_ACTIVE;             
		FDCAN1_Tx.BitRateSwitch = FDCAN_BRS_ON;                //开启速率切换
		FDCAN1_Tx.FDFormat = FDCAN_FD_CAN;                  	 //FDCAN模式                   
		FDCAN1_Tx.TxEventFifoControl = FDCAN_NO_TX_EVENTS;     //无发送事件
		FDCAN1_Tx.MessageMarker = 0;   												 //消息标签，可以在发送消息的时候给每个报文打上这个标签，
																													 //用来在获取事件的时候方便用户知道是发送的哪个标签的报文(可用HAL_FDCAN_GetTxEvent()函数来获取标签)  
		/* 将需要发送的数据压入到TX FIFO */
		if(HAL_FDCAN_AddMessageToTxFifoQ(&hFDCan1, &FDCAN1_Tx, tx_data) != HAL_OK)
		{
			Debug_Printf("\r\n装载失败\r\n");
//			while(1);
		}
//		delay_us(500);
	}
  return 0;
}


/*
*********************************************************************************************************
* 函 数 名: HAL_FDCAN_RxFifo0Callback
* 功能说明: FDCAN中断回调函数，接收子模块返回数据，并重新封装
* 形  参: FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs
*		@ref1:	hfdcan：FDCAN句柄
*		@ref2:	RxFifo0ITs：接收中断
*	@note：
* 返 回 值:无
*********************************************************************************************************
*/
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if(hfdcan->Instance == FDCAN1)
	{
		if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)   //FIFO0新数据中断
		{
			uint8_t rxdata[64] = {0};
			
			/* 提取FIFO0中接收到的数据 */
			HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &FDCAN1_Rx, rxdata);
			
			uint8_t	frame_seq = rxdata[0];					//帧序号
			uint8_t	frame_total_num = rxdata[1];		//总帧数
			
//			Debug_Printf("CAN1接收数据\n");
//			Debug_Printf("id:%#x\r\n", FDCAN1_Rx.Identifier);
//			Debug_Printf("len:%d\r\n", FDCAN1_Rx.DataLength >> 16);			//FDCAN中数据段中的4位数据长度-对应64字节数据二进制表示为1111，十进制为15
//			Debug_Printf("seq:%d，frame_total_num：%d\r\n", frame_seq, frame_total_num);
			
			memcpy(&fdcan1_rxdata[(frame_seq - 1) * VALID_DATA_SIZE], &rxdata[2], VALID_DATA_SIZE);		//存储数据
			
			fdcan1_receive_count++;
			
			if(fdcan1_receive_count == frame_total_num)		//分包接收完毕
			{
				
				if(fdcan1_rxdata[1] == 0x00 && fdcan1_rxdata[2] == 0x00)		//接收到0x0000的功能码命令（特殊命令，即通讯转接板广播命令）
				{
					HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
					
					TIM6_Start();				//开启/重置定时器6（若定时器发生中断，则认为子模块全部应答成功，进入中断回调函数操作）
					
					//fdcan1_rx_specialdata[0]是存储这一帧数据长度的，暂时不进行赋值，在接收完毕之后再赋值
					g_fdcan1_rx_specialdata[1] = fdcan1_rxdata[1];         //存储功能码
					g_fdcan1_rx_specialdata[2] = fdcan1_rxdata[2];         //存储功能码
					
					memcpy(&g_fdcan1_rx_specialdata[3 + 2 * g_fdcan1_rx_special_cnt], &fdcan1_rxdata[3], 2);		//依次存储各个子模块返回的子模块ID，默认2字节
					
					g_fdcan1_rx_special_cnt++;
					
					memset(&fdcan1_rxdata, 0, frame_total_num * 64);
					fdcan1_receive_count = 0;			//计数复位
				}
				else
				{		
					HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
					
					pindoard_reply_to_pc(FDCAN1_Rx.Identifier, fdcan1_rxdata);			//封装应答帧，回复给PC
					
					memset(&fdcan1_rxdata, 0, frame_total_num * 64);
					fdcan1_receive_count = 0;			//计数复位
				}
			}
			HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
		}
	}
}



/*********************************************END OF FILE**********************/
