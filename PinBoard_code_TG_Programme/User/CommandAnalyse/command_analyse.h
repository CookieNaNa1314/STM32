#ifndef __COMMAND_ANALYSE_H
#define	__COMMAND_ANALYSE_H

#include "stm32h7xx.h"

#define		GET_PC_BUFFER_SIZE						128				//最大存储PC上位机发送的数据大小
#define		SEND_SUB_BUFFER_SIZE					128				//最大存储发送给子模块的数据大小
#define		REPLY_PC_BUFFER_SIZE					128				//最大存储应答给PC上位机的数据大小


void	pc_to_pinboard_command_analyse(void);

void  pindoard_reply_to_pc(uint32_t	submodule_id, uint8_t *recvdata);


#endif /* __COMMAND_ANALYSE_H */
