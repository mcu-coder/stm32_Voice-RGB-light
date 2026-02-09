#include "stm32f10x.h"                  // Device header
#include "adcx.h"
#include "ldr.h"
#include "oled.h"
#include "dht11.h"
#include "pwm.h"
#include "led.h"
#include "key.h"
#include "tim2.h"   
#include "tim3.h"   
#include "hc_sr501.h"
#include "hc_sr04.h"
#include "sensormodules.h"
#include "usart.h"
#include "gizwits_product.h"
#include "myrtc.h"
#include "flash.h"
#include "iwdg.h"
#include "usart3.h"

#define KEY_Long1	11

#define KEY_1	1
#define KEY_2	2
#define KEY_3	3
#define KEY_4	4

#define FLASH_START_ADDR	0x0801f000	//写入的起始地址

uint8_t hc501;						//存储人体信号
uint8_t systemModel = 0;				//存储系统当前模式

uint8_t hour,minute,second;			//时 分 秒
uint8_t menu = 1;					//显示菜单变量
uint8_t gizwitsModeFlag;			// 配网标志位

SensorModules sensorData;	//声明传感器数据结构体变量
SensorThresholdValue Sensorthreshold;	//声明传感器阈值结构体变量


enum 
{
	display_page = 1,
	settingsPage,
	timeSettingsPage,
	gizwitsMode	
}MenuPages;

/**
  * @brief  显示主页面固定内容
  * @param  无
  * @retval 无
  */
void OLED_Menu(void)
{
	//显示“Time：”
	OLED_ShowString(1, 1, "Time");
	OLED_ShowChar(1, 5, ':');
	
	//显示“温度：”
	OLED_ShowChinese(2,1, 0);
	OLED_ShowChinese(2,2, 1);
	OLED_ShowChar(2, 5, ':');
	 
	//显示“距离：”
	OLED_ShowChinese(3,5, 17);
	OLED_ShowChinese(3,6, 18);
	OLED_ShowChar(3, 13, ':');
		
	//显示“亮度：”
	OLED_ShowChinese(4,1, 6);
	OLED_ShowChinese(4,2, 1);
	OLED_ShowChar(4, 5, ':');
}

/**
  * @brief  显示主页面的传感器数据等信息
  * @param  无
  * @retval 无
  */
void OLED_Menu_SensorData(void)
{
		 

		//显示光强数据
		OLED_ShowNum(3, 6, sensorData.lux, 3);	

		//显示距离数据
		OLED_ShowNum(3, 14, sensorData.distance, 2);

		//显示亮度等级
		OLED_ShowNum(4, 6, ledDutyRatio, 3);
		OLED_ShowChar(4, 9, '%');	
	 
		//显示系统当前模式 手动模式 or 自动模式
		if (systemModel)
		{
			//显示“自动”
			OLED_ShowChinese(4, 7, 9);
			OLED_ShowChinese(4, 8, 10);					
		}
		else
		{
			//显示“手动”
			OLED_ShowChinese(4, 7, 11);
			OLED_ShowChinese(4, 8, 12);			
		}
}

/**
  * @brief  显示系统设置界面
  * @param  无
  * @retval 无
  */
void OLED_SetInterfacevoid(void)
{
	//显示“定时提醒”
	OLED_ShowChinese(1, 2, 30);
	OLED_ShowChinese(1, 3, 31);	
	OLED_ShowChinese(1, 4, 32);
	OLED_ShowChinese(1, 5, 33);	
	OLED_ShowChar(1, 11, ':');	
	 
	
	//显示“光照阈值”
	OLED_ShowChinese(3, 2, 4);
	OLED_ShowChinese(3, 3, 5);	
	OLED_ShowChinese(3, 4, 26);
	OLED_ShowChinese(3, 5, 27);	
	OLED_ShowChar(3, 11, ':');

	//显示”距离阈值“
	OLED_ShowChinese(4, 2, 17);
	OLED_ShowChinese(4, 3, 18);	
	OLED_ShowChinese(4, 4, 26);
	OLED_ShowChinese(4, 5, 27);	
	OLED_ShowChar(4, 11, ':');

	//显示定时提醒数值
	OLED_ShowNum(1, 13, Sensorthreshold.timing, 2);
 
}

/**
  * @brief  记录阈值界面下按KEY1的次数
  * @param  无
  * @retval 返回次数
  */
uint8_t SetSelection(void)
{
	static uint8_t count = 1;
	if(KeyNum == KEY_1)
	{
		KeyNum = 0;
		count++;
		if (count > 4)
		{
			count = 1;			
		}
	}
	return count;
}

/**
  * @brief  显示阈值界面的选择符号
  * @param  num 为显示的位置
  * @retval 无
  */
void OLED_Option(uint8_t num)
{
	switch(num)
	{
		case 1:	
			OLED_ShowChar(1,1,'>');
			OLED_ShowChar(2,1,' ');
			OLED_ShowChar(3,1,' ');
			OLED_ShowChar(4,1,' ');
			break;
		 
		case 2:	
			OLED_ShowChar(1,1,' ');
			OLED_ShowChar(2,1,' ');
			OLED_ShowChar(3,1,' ');
			OLED_ShowChar(4,1,'>');
			break;

		default: break;
	}
}

/**
  * @brief  记录时间调节界面下按KEY1的次数
  * @param  无
  * @retval 返回次数
  */
uint8_t SetTimeSelection(void)
{
	static uint8_t count = 1;
	if(KeyNum == KEY_1)
	{
		KeyNum = 0;
		count++;
		if (count > 3)
		{
			count = 1;			
		}
	}
	return count;
}

/**
  * @brief  显示时间调节界面的选择符号
  * @param  num 为显示的位置
  * @retval 无
  */
void OLED_Time_Option(u8 num)
{
	switch(num)
	{
		case 1:	
			OLED_ShowChar(2,6,'v');
			OLED_ShowChar(2,9,' ');
			OLED_ShowChar(2,12,' ');
			break;
		case 2:	
			OLED_ShowChar(2,6,' ');
			OLED_ShowChar(2,9,'v');
			OLED_ShowChar(2,12,' ');
			break;
		case 3:	
			OLED_ShowChar(2,6,' ');
			OLED_ShowChar(2,9,' ');
			OLED_ShowChar(2,12,'v');
			break;
		default: break;
	}
}

/**
  * @brief  显示时间调节界面的内容
  * @param  无
  * @retval 无
  */
void OLED_ThresholdTime(void)
{
	//系统时间：
	OLED_ShowChinese(1, 3, 20); 
	OLED_ShowChinese(1, 4, 21); 
	OLED_ShowChinese(1, 5, 28); 
	OLED_ShowChinese(1, 6, 29); 
	OLED_ShowChar(1, 13, ':');

	OLED_ShowNum(3,5,hour,2);
	OLED_ShowChar(3,7,':');
	OLED_ShowNum(3,8,minute,2);
	OLED_ShowChar(3,10,':');
	OLED_ShowNum(3,11,second,2);
}

/**
  * @brief  对阈值界面的传感器阈值进行修改
  * @param  num 为当前用户需要更改的传感器阈值位置
  * @retval 无
  */
void ThresholdModification(uint8_t num)
{
	switch (num)
	{
		case 1:
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;
				Sensorthreshold.timing ++;
				if (Sensorthreshold.timing > 60)
				{
					Sensorthreshold.timing = 0;
				}				
			}
			else if (KeyNum == KEY_4)
			{
				KeyNum = 0;
				Sensorthreshold.timing --;
				if (Sensorthreshold.timing > 60)
				{
					Sensorthreshold.timing = 60;
				}				
			}			
			break;		
	
		case 2:
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;
				OLED_Clear();
				menu = timeSettingsPage;
				
				hour = MyRTC_Time[3];
				minute = MyRTC_Time[4];
				second = MyRTC_Time[5];	
			}
			else if (KeyNum == KEY_4)
			{
				KeyNum = 0;
				OLED_Clear();
				menu = timeSettingsPage;
				
				hour = MyRTC_Time[3];
				minute = MyRTC_Time[4];
				second = MyRTC_Time[5];
			}			
			break;		
 
				
		default: break;		
	}
}

/**
  * @brief  对系统时间进行修改
  * @param  num 为当前用户需要更改的时分秒位置
  * @retval 无
  */
void TimeModification(uint8_t num)
{
	switch (num)
	{
		case 1:
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;
				hour++;
				if (hour > 24)
				{
					hour = 0;
				}
			}
			else if (KeyNum == KEY_4)
			{
				KeyNum = 0;
				hour --;
				if (hour > 24)
				{
					hour = 24;
				}				
			}		
			break;		

		case 2:
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;
				minute++;
				if (minute > 60)
				{
					minute = 0;
				}
			}
			else if (KeyNum == KEY_4)
			{
				KeyNum = 0;
				minute --;
				if (minute > 60)
				{
					minute = 60;
				}				
			}					
			break;
		case 3:
			if (KeyNum == KEY_3)
			{
				KeyNum = 0;
				second++;
				if (second > 60)
				{
					second = 0;
				}
			}
			else if (KeyNum == KEY_4)
			{
				KeyNum = 0;
				second --;
				if (second > 60)
				{
					second = 60;
				}				
			}		
			break;
		default: break;		
	}
}

/**
  * @brief  获取语音模块数据
  * @param  无
  * @retval 无
  */
void SNR8016(void)
{
	if (Usart3_RxFlag == 1)
	{
		Usart3_RxFlag = 0;
		switch (Usart3_RxPacket[3])
		{
			/*控制开关灯*/
			case 1:
				if (!systemModel)
				{
					if (Usart3_RxPacket[4])
					{
						ledDutyRatio = 100;
						TIM_SetCompare1(TIM1, ledDutyRatio);
					}
					else
					{
						ledDutyRatio = 0;
						TIM_SetCompare1(TIM1, ledDutyRatio);						
					}
				}
				currentDataPoint.valueLamp_brightness = ledDutyRatio;
				break;
				
			/*控制灯暗一点或亮一点*/
			case 2:
				if (!systemModel)
				{
					if (Usart3_RxPacket[4])
					{
						ledDutyRatio += 25;
						if (ledDutyRatio > 100)
						{
							ledDutyRatio = 100;
						}
						TIM_SetCompare1(TIM1, ledDutyRatio);
					}
					else
					{
						ledDutyRatio -= 25;
						if (ledDutyRatio > 100)
						{
							ledDutyRatio = 0;
						}
						TIM_SetCompare1(TIM1, ledDutyRatio);						
					}
				}
				currentDataPoint.valueLamp_brightness = ledDutyRatio;
				break;
				 
			default:
				break;
		}
	}
}

/**
  * @brief  定时结束后蜂鸣器提醒
  * @param  无
  * @retval 无
  */
void TimingAlarm(void)
{
	if (timingBuzzerFlag)
	{
		timingBuzzerFlag = 0;
		Buzzer_ON(); 
		Delay_ms(100);
		Buzzer_OFF();
		Delay_ms(100);
	}
}

void scanGizwitsModeKey(void)
{
	 
		if (GPIO_ReadInputDataBit(KEY_PROT, KEY3_GPIO_PIN) == 0)
		{	
			menu = gizwitsMode;
			gizwitsModeFlag = 1;
			Buzzer_ON();
		}
}

/**
  * @brief  获取传感器的数据
  * @param  无
  * @retval 无
  */
void sensorScan(void)
{
		DHT11_Read_Data(&sensorData.humi, &sensorData.temp);
		HC_SR04_Deboanle(&sensorData.distance);
		LDR_LuxData(&sensorData.lux);
		HC_SR501_Input(&sensorData.people);		
}


int main(void)
{
	Buzzer_Init();
	LDR_Init();
	OLED_Init();
	DHT11_Init(); 
	
	ADCX_Init(); 
	Uart1_Init(115200); 
	Uart3_Init();
	
	MyRTC_Init();	
	LED_Init();
	
	Sensorthreshold.Illumination_threshold = FLASH_R(FLASH_START_ADDR);	//从指定页的地址读FLASH
	Sensorthreshold.Distance_threshold = FLASH_R(FLASH_START_ADDR+2);	//从指定页的地址读FLASH
	Sensorthreshold.timing = FLASH_R(FLASH_START_ADDR+4);	//从指定页的地址读FLASH
	
	GENERAL_TIM_Init();
	userInit();		//完成机智云初始赋值
	gizwitsInit();	//开辟一个环形缓冲区
	 
	Delay_ms(1000);
//	scanGizwitsModeKey();	
	gizwitsSetMode(WIFI_AIRLINK_MODE);
	Delay_ms(1000);
	while (1)
	{
 		IWDG_ReloadCounter(); //重新加载计数值 喂狗
		sensorScan();	//获取传感器数据
		SNR8016();	//执行语音指令
		
		switch (menu)
		{
			case display_page:

				MyRTC_ReadTime();	//调用此函数后，RTC硬件电路里时间值将刷新到全局数组
				OLED_Menu_SensorData();	//显示主页面传感器数据、系统模式等内容
				OLED_Menu();	//显示主页面的固定内容
				TimingAlarm();
				if (!systemModel)
				{
					LED_PWM_KEY();	//按键控制LED的PWM			
				}
				
				//切换系统模式
				if (KeyNum == KEY_1)
				{
					KeyNum = 0;
					systemModel = ~systemModel;
					if (systemModel)
					{
						currentDataPoint.valueModel = 1;
						ledDutyRatio = 0;
						TIM_SetCompare1(TIM1, ledDutyRatio);
						
					}
					else
					{
						currentDataPoint.valueModel = 0;
						ledDutyRatio = 0;
						TIM_SetCompare1(TIM1, ledDutyRatio);
						Buzzer_OFF();
					}
				}				
				
				//判断是否进入阈值设置界面
				if (KeyNum == KEY_Long1)
				{
					KeyNum = 0;
					OLED_Clear();	//清屏
					menu = settingsPage;	//跳转到阈值设置界面
				}
				break;
				
			case settingsPage:
				OLED_SetInterfacevoid();	//显示阈值设置界面的固定内容
				OLED_Option(SetSelection());	//实现阈值设置页面的选择功能
				ThresholdModification(SetSelection());	//实现阈值调节功能	
			
				//判断是否退出阈值设置界面
				if (KeyNum == KEY_2)
				{
					KeyNum = 0;
					OLED_Clear();	//清屏
					menu = display_page;	//跳转到主界面
					
					//存储修改的传感器阈值至flash内				
					FLASH_W(FLASH_START_ADDR, Sensorthreshold.Illumination_threshold, Sensorthreshold.Distance_threshold,
							Sensorthreshold.timing);
					
					currentDataPoint.valueIllumination_threshold = Sensorthreshold.Illumination_threshold;
					currentDataPoint.valueDistance_threshold = Sensorthreshold.Distance_threshold;
					currentDataPoint.valuetiming = Sensorthreshold.timing;
				}
				break;
				
			case timeSettingsPage:
				OLED_ThresholdTime();	//显示时间设置界面的内容
				OLED_Time_Option(SetTimeSelection());	//实现间设置界面的选择功能
				TimeModification(SetTimeSelection());	//实现时间调节功能	
				
				//判断是否退出时间设置界面
				if (KeyNum == KEY_2)
				{
					KeyNum = 0;
					//将更改的数据赋值回RTC数组中
					MyRTC_Time[3] = hour;	
					MyRTC_Time[4] = minute;
					MyRTC_Time[5] = second;		
					MyRTC_SetTime();	//调用此函数后，全局数组里时间值将刷新到RTC硬件电路	
		
					OLED_Clear();	//清屏
					menu = settingsPage;	//回到阈值设置界面
				}
				break;

			case gizwitsMode:
				if (gizwitsModeFlag == 1)
				{
					//显示“热点配网”
					OLED_ShowChinese(2,3,38);
					OLED_ShowChinese(2,4,39);
					OLED_ShowChinese(2,5,40);
					OLED_ShowChinese(2,6,41);
				}
				else if (gizwitsModeFlag == 2)
				{
					//显示“一键配网”
					OLED_ShowChinese(2,3,34);
					OLED_ShowChinese(2,4,35);
					OLED_ShowChinese(2,5,36);
					OLED_ShowChinese(2,6,37);
				}
				if (!gizwitsModeFlag)
				{
					KeyNum = 0;
					OLED_Clear();
					menu = display_page;
				}				
				break;
			
			default:
				break;				
		}	
		
		//判断上位机是否更改阈值，如更改则保存至flash中
		if (valueFlashflag)
		{
			valueFlashflag = 0;
			//存储修改的传感器阈值至flash内				
			FLASH_W(FLASH_START_ADDR, Sensorthreshold.Illumination_threshold, Sensorthreshold.Distance_threshold,
					Sensorthreshold.timing);
		}

		userHandle();	//更新机智云数据点变量存储的值
		gizwitsHandle((dataPoint_t *)&currentDataPoint);	//数据上传至机智云					
	}
}
