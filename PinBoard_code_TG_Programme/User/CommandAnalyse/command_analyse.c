/*
  *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
  *File name:	command_analyse.c
  *Auther:	H_YAN      ID:      Version: v1.0        Date:2025.06.30
  *Description:	用于使用串口命令接收解析
  *Others:
  *History:
 		1、Date:
 	     Author:
 	     Modification:
 		2、...
 
  */
  
#include "command_analyse.h"
#include "fdcan.h"  

#include "usart.h"
#include "crc.h"
#include "debug.h"
#include "ring_buffer_chapter.h"


/*
*********************************************************************************************************
* 函 数 名: pinboard_sendto_submodule
* 功能说明: 重新分解封装PC上位机发来的数据帧，并发送给对应的子模块
* 形  参: uint8_t	*senddata, uint8_t	*get_pc_data
*		@ref1: 	senddata：通讯转接板重新封装需发送给子模块的数据
*		@ref2: 	get_pc_data：PC发给通讯转接板的数据
*	@note：
* 返 回 值: 
*********************************************************************************************************
*/
static void pinboard_sendto_submodule(uint8_t	*senddata, uint8_t	*get_pc_data)
{
	assert_param(senddata != NULL);
	assert_param(get_pc_data != NULL);
	
	uint16_t	frame_len = get_pc_data[2];
	frame_len = frame_len << 8 | get_pc_data[1];	//获取当前帧长度
	
	uint16_t	submodule_id = get_pc_data[3];
	submodule_id = submodule_id << 8 | get_pc_data[4];	//获取当前帧需发送给子模块的ID
	
	uint16_t	new_frame_len = frame_len - 4;			//重新计算需下发数据的有效长度，frame_len - 4 = frame_len - 子模块地址(2字节) - CRC校验(2字节)
	
	senddata[0] = new_frame_len;									//赋值
	
	memcpy(&senddata[1], &get_pc_data[5], new_frame_len);		//赋值
	
	FDCAN1_Send_Msg(submodule_id, senddata, new_frame_len + 1);	//发送，这里+1 表示还需要加上“长度位”
	
//	Debug_Printf("\r\n解析发送完毕\r\n");
}


/*
*********************************************************************************************************
* 函 数 名: pc_to_pinboard_command_analyse
* 功能说明: 解析上位机给通讯转接板发送的指令
* 形  参: 无
*		@ref1:	
*		@ref2:	
*	@note： 帧格式：帧头(1)+长度(2，小端序)+子模块地址(2)+子模块功能指令(2)+N字节参数+校验码(2，小端序)
*	@note： 数组下标：	0				1  2							3 4							5 6 					...			len-1  len-2 
* 返 回 值:无
*********************************************************************************************************
*/
void	pc_to_pinboard_command_analyse(void)
{
	uint32_t RBC_NUM = RBC_Get_Chapter_Number(&rbc);		//获取当前分段缓冲区中还有多少个分段，也就是还有多少个数据帧未使用
	
	if(RBC_NUM != 0)		//还有数据帧未读取
	{
		uint8_t		get_pc_buffer[GET_PC_BUFFER_SIZE] = {0};													//用于存储从分段环形缓冲区中取出的一帧数据
		uint8_t		canfd_send_submodule_buffer[SEND_SUB_BUFFER_SIZE] = {0};					//用于存储需下发给子模块的指令
		
		RBC_Read_Chapter(&rbc, get_pc_buffer, NULL);			//读取一帧数据
		
		uint16_t	frame_len = get_pc_buffer[2];
		frame_len = frame_len << 8 | get_pc_buffer[1];	//获取当前帧长度
		
		uint16_t	frame_command = get_pc_buffer[5];
		frame_command = frame_command << 8 | get_pc_buffer[6];//获取当前子模块功能指令
		
		//解析
		switch (get_pc_buffer[3])			//get_pc_buffer[3]、get_pc_buffer[4]子模块地址
    {
    	case 0x00:		//通讯转接板指令
				switch (frame_command)
        {
        	case 0x0000://扫描指令，扫描有哪些子模块已连接
							
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并以广播的形式发送给所有子模块
						
        		break;
					
        	case 0x0001://其他操作
						
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);
					
        		break;
					
        	default:
        		break;
        }
				if(get_pc_buffer[4] == 0x00)
				{
					Debug_Printf("给第%d个通讯转接板发送信息", get_pc_buffer[4] + 1);
				}
				else if(get_pc_buffer[4] == 0x01)
				{
					Debug_Printf("给第%d个通讯转接板发送信息", get_pc_buffer[4] + 1);
				}
    		break;//跳出switch(get_pc_buffer[3])
			
    	case 0x01:		//温度采集板子模块指令
				switch (frame_command)
        {
        	case 0x0000://读取温度子模块连接状态
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;

        	default:		//剩余所有命令
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//剩余所有命令无需判断，直接重新封装并发送
        		break;
        }
    		break;			//跳出switch(get_pc_buffer[3])
			
			case 0x02:		//PID控制板子模块指令
				switch (frame_command)
        {
        	case 0x0000://读取PID控制子模块连接状态
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;
					
        	case 0x0401://设置PID控温控制各个参数（目标温度、目标速率、控温模式）
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;
					
        	case 0x0402://读取线性扫温-PID参数
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;
					
        	case 0x0403://设置线性扫温-PID参数
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;
					
        	case 0x0404://读取恒温-PID参数
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;
					
        	case 0x0405://设置恒温-PID参数
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;
					
        	default:		//剩余所有命令
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//剩余所有命令无需判断，直接重新封装并发送
        		break;
        }
				break;
						
			case 0x03:		//热流控制板子模块指令
				switch (frame_command)
        {
        	case 0x0000://读取热流控制子模块连接状态
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//重新封装并发送
        		break;

        	default:		//剩余所有命令
						pinboard_sendto_submodule(canfd_send_submodule_buffer, get_pc_buffer);			//剩余所有命令无需判断，直接重新封装并发送
        		break;
        }
				break;
			
			case 0x04:		//焦耳校准板子模块指令
				break;
			
			case 0x05:		//电机控制板子模块指令
				break;
			
			case 0x06:		//AD/DA板子模块指令
				break;
			
			case 0x07:		//I/O板子模块指令
				break;
			
			case 0x08:		//？？？子模块指令（待定）
				break;
			
    	default:
    		break;
    }
	}
	else
	{
		//Debug_Printf("\r\n无待处理数据帧.\r\n");
		//其他动作
	}
}


/*
*********************************************************************************************************
* 函 数 名: pindoard_reply_to_pc
* 功能说明: FDCAN1接收子模块返回的数据帧，同时封装后，发送给PC上位机
* 形  参: uint8_t	*recvdata, uint8_t	*replydata
*		@ref1: 	recvdata：接收子模块返回的命令数组
*		@ref2: 	replydata：应答给PC上位机的数组
*	@note： recvdata：03 00 00 59
* @note： replydata：01 07 00 00 00 00 00 59 4E CE
* 返 回 值: 无
*********************************************************************************************************
*/
void  pindoard_reply_to_pc(uint32_t	submodule_id, uint8_t *recvdata)
{
	uint8_t	replydata[REPLY_PC_BUFFER_SIZE] = {0};		//存储应答给PC上位机的数据
	
	uint8_t	recv_len = recvdata[0];	//子模块应答的数据长度
	uint8_t new_len = recv_len + 4; //重新计算后的有效长度，加上CRC（2字节）和子模块地址（2字节）
	
	//重新封装
	replydata[0] = 0x01;					//帧头
	replydata[1] = new_len;			//长度(小端序，低8位)
	replydata[2] = 0x00;					//长度(小端序，高8位)，暂定数据长度不会超过256
	
	replydata[3] = (uint8_t)(submodule_id >> 8);	//子模块地址，高字节
	replydata[4] = (uint8_t)(submodule_id);	      //子模块地址，低字节
	
	memcpy(&replydata[5], &recvdata[1], recv_len);		//存储应答数据
	
	uint16_t new_crc = crc16_ccitt_false(replydata, new_len + 1);		//重新计算CRC校验位，+1指的是加上帧头
	
	replydata[new_len + 1] = (uint8_t)(new_crc);         //CRC(小端序，低8位)
	replydata[new_len + 2] = (uint8_t)(new_crc >> 8);    //CRC(小端序，高8位)
	
	Uart4_SendString(replydata, new_len + 3);		//加上帧头(1字节)和长度(2字节)，应答给PC端
}


/*********************************************END OF FILE**********************/
