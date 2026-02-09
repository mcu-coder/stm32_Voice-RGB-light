#include "led.h"

uint8_t ledDutyRatio = 0;				//存储当前的LED亮度数值
LED_LIGHT_COLOUR ledLightColour = COOL_LIGHT;

void LED_Init(void)
{
	//开启GPIOB时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	//配置LED引脚为模拟输出模式
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = LED1 | LED2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PROT, &GPIO_InitStructure);
	GPIO_SetBits(LED_PROT, LED1 | LED2);
}

void LED_PWM_KEY(void)
{
	if (KeyNum == 3)
	{
		static uint8_t ledKeyNum = 0;  // 定义初始值
		KeyNum = 0;                    // 复位按键计数
		
		// 增加 ledKeyNum 并更新 ledDutyRatio
		ledKeyNum++;
		ledDutyRatio = (ledDutyRatio + 35 > 100) ? 100 : ledDutyRatio + 35;
		
		// 检查 ledKeyNum 是否超过颜色数量阈值
		if (ledKeyNum > COLOUR_AMOUNT) {
			ledKeyNum = 0;
			ledDutyRatio = 0;  // 重置亮度
		}
		
		// 更新亮度数据
		currentDataPoint.valueLamp_brightness = ledDutyRatio;
	}
	if (KeyNum == 4)
	{
		KeyNum = 0;
		ledLightColour ++;
		if (ledLightColour >= COLOUR_AMOUNT)
		{
			ledLightColour = COOL_LIGHT;
		}
		currentDataPoint.valueLEDMode = ledLightColour;
	}
	switch (ledLightColour)
	{
		case COOL_LIGHT:
			TIM_SetCompare1(TIM1, 0);
			TIM_SetCompare2(TIM1, ledDutyRatio);
			break;
		
		case WARM_LIGHT:
			TIM_SetCompare1(TIM1, ledDutyRatio);
			TIM_SetCompare2(TIM1, 0);
			break;
		
		case NEUTRAL_LIGHT:
			TIM_SetCompare1(TIM1, ledDutyRatio);
			TIM_SetCompare2(TIM1, ledDutyRatio);			
			break;
		
		default: break;
	}
}


void LED_APP_Command(void)
{	
	if (!systemModel && LEDFlag)
	{
		LEDFlag = 0;
		ledDutyRatio = currentDataPoint.valueLamp_brightness;
	}
	else if (systemModel && sensorData.people)
	{
		uint16_t tempData;
		if (currentDataPoint.valueIllumination_threshold > sensorData.lux)
		{
			tempData = currentDataPoint.valueIllumination_threshold - sensorData.lux;
			if (tempData > ledDutyRatio)
			{
				ledDutyRatio++;	
				if (ledDutyRatio >= LED_PWM_UPPER_LIMIT)
				{
					ledDutyRatio = 100;
				}	
			}
			else if (tempData < ledDutyRatio)
			{
				ledDutyRatio--;	
				if (ledDutyRatio >= LED_PWM_UPPER_LIMIT)
				{
					ledDutyRatio = 0;
				}				
			}
		}
		else if (currentDataPoint.valueIllumination_threshold < sensorData.lux) 	
		{
			ledDutyRatio--;	
			if (ledDutyRatio >= LED_PWM_UPPER_LIMIT)
			{
				ledDutyRatio = 0;
			}		
		}
		currentDataPoint.valueLamp_brightness = ledDutyRatio;
	}
	else if (systemModel && sensorData.people == 0)
	{
		ledDutyRatio = 0;
		currentDataPoint.valueLamp_brightness = ledDutyRatio;
	}
	else if (systemModel)
	{
		if (currentDataPoint.valueLamp_brightness != ledDutyRatio)
		{
			currentDataPoint.valueLamp_brightness = ledDutyRatio;
		}
	}
	switch (ledLightColour)
	{
		case COOL_LIGHT:
			TIM_SetCompare1(TIM1, 0);
			TIM_SetCompare2(TIM1, ledDutyRatio);
			break;
		
		case WARM_LIGHT:
			TIM_SetCompare1(TIM1, ledDutyRatio);
			TIM_SetCompare2(TIM1, 0);
			break;
		
		case NEUTRAL_LIGHT:
			TIM_SetCompare1(TIM1, ledDutyRatio);
			TIM_SetCompare2(TIM1, ledDutyRatio);			
			break;
		
		default: break;
	}
}

