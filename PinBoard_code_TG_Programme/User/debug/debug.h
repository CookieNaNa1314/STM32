#ifndef __DEBUG_H
#define __DEBUG_H

/**********************此.h文件用于debug调试使用，实际release后，失能debug功能*************************/
#include "main.h"

#include "sys.h"
#include "delay.h"
#include "stm32h7xx.h"

#include "stdio.h"


#define		DEBUG_PRINTF					1					//打印调试功能

																					//assert调试功能hal库自带，可在stm32h7xx_hal_conf.h中 使能或失能
																			
/*--------------------------------------------打印调试-----------------------------------------------*/
#if		DEBUG_PRINTF
#define		Debug_Printf(fmt, ...) \
					printf("DEBUG: %s:%d: " fmt, __FILE__, __LINE__, ##__VA_ARGS__)			//输出附加信息：文件名+行号+输出内容
#else
#define		Debug_Printf(fmt, ...)	//空宏，不输出
#endif



#endif	/*__DEBUG_H */
