#ifndef __FDCAN_H
#define	__FDCAN_H

#include "stm32h7xx_hal.h"

extern FDCAN_HandleTypeDef 	hFDCan1;


extern uint8_t	g_fdcan1_rx_specialdata[64];
extern uint8_t	g_fdcan1_rx_special_cnt;


typedef	struct{
	uint8_t	frame_seq;					//帧序号，1~256，标记拆分帧的顺序
	
	uint8_t	frame_total_num;		//总帧数，1~256，总共拆分多少帧
	
	uint8_t	data[62];						//有效数据，62字节，每帧中的有效数据
	
}FDCAN_Multi_Pocket_Frame;		//FDCAN分包发送结构体(共64字节)，此结构体仅是体现自定义协议帧的格式，实际未使用




void FDCAN1_Init(void);

uint8_t FDCAN1_Send_Msg(uint32_t id, uint8_t* msg, uint32_t len);

#endif /* __FDCAN_H */
