/*
  *Copyright: Copyright (c) 2024 REINSTEK Co. Ltd. All rights reserved.
  *File name:	crc.c
  *Auther:	H_YAN      ID:      Version: v1.0        Date:2025.06.24
  *Description:	用于使用CRC16-CCITT-FLASE方法进行CRC校验
  *Others:
  *History:
 		1、Date:
 	     Author:
 	     Modification:
 		2、...
 
  */
  
#include "crc.h"   

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CRC16_POLY 0x1021

// CRC16 查找表
static uint16_t crc16_table[256] = {0};


/*
*********************************************************************************************************
* 函 数 名: crc16_init
* 功能说明: CRC16-CCITT-FALSE校验初始化，计算校验码并填充到CRC16查找表中
* 形  参: 无
*		@ref1: 
*		@ref2: 
*	@note：					此校验采用查找表的方式（更快）
* 返 回 值: 无
*********************************************************************************************************
*/
void crc16_init(void) 
{
  for(uint16_t i = 0; i < 256; i++)
	{
    uint16_t crc = i << 8; // 初始值
    for(uint8_t j = 0; j < 8; j++) 
		{
      if(crc & 0x8000) 
			{
         crc = (crc << 1) ^ CRC16_POLY;
      }else
			{
         crc <<= 1;
      }
    }
    crc16_table[i] = crc;
  }
}


/*
*********************************************************************************************************
* 函 数 名: crc16_ccitt_false
* 功能说明: 计算 CRC16-CCITT-FALSE 校验值
* 形  参: const uint8_t* data, uint8_t length
*		@ref1: data：需要计算校验码的数组
*		@ref2: length：计算的长度
*	@note：
* 返 回 值: CRC，校验码
*********************************************************************************************************
*/
uint16_t crc16_ccitt_false(const uint8_t* data, uint8_t length)
{
	assert_param(data != NULL);
	assert_param(length >= 0);
	
  uint16_t crc = 0xFFFF; // 初始化 CRC 值

  for (uint16_t i = 0; i < length; i++) 
	{
    // 通过查表更新 CRC 值
    uint8_t table_index = (crc >> 8) ^ data[i];
    crc = (crc << 8) ^ crc16_table[table_index];
  }

  return crc; // 返回计算出的 CRC 值
}


/*********************************************END OF FILE**********************/
