#ifndef __USART2_H
#define __USART2_H

#include "stm32f10x.h"                  // Device header
#include "oled.h"

extern uint8_t Usart3_RxPacket[10];				//定义接收数据包数组
extern uint8_t Usart3_RxFlag;
extern uint8_t Usart3_seshig;

void Uart3_Init(void);
uint8_t CalXorSum(const uint8_t *data, uint32_t len);

#endif


