#include "tim2.h"                  // Device header
uint32_t delay20ms;
static uint16_t delay1min;			//定时器1分钟计时
uint8_t timingBuzzerFlag;	//定时器蜂鸣器启动标志位
uint8_t timingold; 			// 定时提醒旧的时间
uint8_t timingvariation; 	// 定时提醒倒计时
uint8_t valueDistanceBuzzerFlag;
void Timer2_Init(u16 Prescaler, u16 Period)	//0-65535
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = Period;
	TIM_TimeBaseInitStructure.TIM_Prescaler = Prescaler;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);
	
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2, ENABLE);
}

void SetGizwitsMode(void)
{
	static uint32_t time;
	if (gizwitsModeFlag)
	{
		time++;
		if (time == 600)
		{
			Buzzer_OFF();
		}
		else if (time >= 2000)
		{
			printf("发送系统模式切换指令\n");	
			gizwitsSetMode(gizwitsModeFlag);
			printf("切换系统模式\n");
			gizwitsModeFlag = 0;
		}

	}
}

void TIM2_IRQHandler(void)	//2ms定时中断
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET ) 
	{
		Key_scan();	//按键扫描函数
		
		//定时提醒计时
		if (Sensorthreshold.timing && menu == 1)
		{
			delay1min++;
			if (delay1min > 30000)	//1分钟		30000
			{
				delay1min = 0;
				Sensorthreshold.timing --;
				currentDataPoint.valuetiming = Sensorthreshold.timing;
				if (Sensorthreshold.timing == 0)
				{
					timingBuzzerFlag = 1;
				}
			}
		}
		
		//HC_SR04的20ms延迟
		if (HC_SR04_DelayStructure.delay20msStartFlag)
		{
			HC_SR04_DelayStructure.delay20ms++;
			if(HC_SR04_DelayStructure.delay20ms > 10)
			{
				HC_SR04_DelayStructure.delay20msStartFlag = 0;
				HC_SR04_DelayStructure.delay20msFinishFlag = 1;
			}	
		}
		//HC_SR04的100ms延迟
		if (HC_SR04_DelayStructure.delay100msStartFlag)
		{
			HC_SR04_DelayStructure.delay100ms++;
			
			if(HC_SR04_DelayStructure.delay100ms > 50)
			{
				HC_SR04_DelayStructure.delay100msStartFlag = 0;
				HC_SR04_DelayStructure.delay100msFinishFlag = 1;
			}	
		}		
		
		//控制延迟
		delay20ms++;
		if(delay20ms < 10)
		{
			delay20ms = 0;
			LED_APP_Command();	
			
			if (systemModel)
			{
				//距离阈值报警
				if (Sensorthreshold.Distance_threshold > sensorData.distance)
				{
					valueDistanceBuzzerFlag = 1;
					Buzzer_ON();
				}
				else if(Sensorthreshold.Distance_threshold < sensorData.distance && valueDistanceBuzzerFlag)
				{
					Buzzer_OFF();
				}			
			}			
		}
		
		//机智云配网按键，可以重新进行配网。	
		SetGizwitsMode();		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);				
	}	
}

