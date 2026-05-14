#ifndef __CRC_H
#define	__CRC_H

#include "stm32h7xx.h"


void crc16_init(void);
uint16_t crc16_ccitt_false(const uint8_t* data, uint8_t length);



#endif /* __CRC_H */
