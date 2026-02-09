#ifndef	_LED_H_
#define	_LED_H_

#include "stm32f10x.h"                  // Device header
#include "key.h" 
#include "gizwits_product.h"
#include "sensormodules.h"
#include "tim2.h"

#define LED1			GPIO_Pin_14
#define LED2			GPIO_Pin_13
#define LED_PROT		GPIOB

/* 声明灯光类型 */
typedef enum
{
	COOL_LIGHT = 0,		// 冷光
	WARM_LIGHT,	  		// 暖光
	NEUTRAL_LIGHT,		// 中性光
	COLOUR_AMOUNT
} LED_LIGHT_COLOUR;

extern LED_LIGHT_COLOUR ledLightColour;

#define LED_PWM_UPPER_LIMIT			100		//LED亮度数值的上限为100%

extern uint8_t ledDutyRatio;	

void LED_Init(void);
void LED_PWM_KEY(void);
void LED_APP_Command(void);
#endif
