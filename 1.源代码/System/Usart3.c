#include "usart3.h"	

uint8_t Usart3_RxPacket[10];				//定义接收数据包数组
uint8_t Usart3_RxFlag;					//定义接收数据包标志位
uint8_t Usart3_seshig;					
void Uart3_Init(void)
{
    //GPIO端口设置
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能Usart3，GPIOA时钟
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOA时钟
	
    //Usart3_RX	  GPIOB.11初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//PA3
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.11  

  //Usart3 NVIC 配置
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级4
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = 9600;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx;	//收发模式

    USART_Init(USART3, &USART_InitStructure); //初始化串口3
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
    USART_Cmd(USART3, ENABLE);                    //使能串口3 

}





/**
***********************************************************
* @brief 对数据进行异或运算
* @param data, 存储数组的首地址
* @param len, 要计算的元素的个数
* @return 异或运算结果
***********************************************************
*/
uint8_t CalXorSum(const uint8_t *data, uint32_t len)
{
	uint8_t xorSum = 0;
	for (uint32_t i = 0; i < len; i++)
	{
		xorSum ^= data[i];
	}
	return xorSum;
}

/**
***********************************************************************
包格式：帧头0  帧头1  数据长度  功能字    指令  异或校验数据
        0x55   0xAA    0x02      0x01     0x01      0xFD
***********************************************************************
*/


void USART3_IRQHandler(void)                	//串口2中断服务程序
{
		uint8_t Res;
		static uint8_t RxState = 0;		//当前状态机状态
		static uint8_t pRxPacket = 0;	//当前接收数据位置

		if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
		{
			Res =USART_ReceiveData(USART3);	//读取接收到的数据	
			switch (RxState)
			{
				case 0: 
					if (Res == 0x55)
					{
						Usart3_RxPacket[pRxPacket] = Res;	//将数据存入数组
						pRxPacket++;
						RxState = 1;
					}
					else
					{
						pRxPacket = 0;
						RxState = 0;
					}
					break;

				case 1: 
					if (Res == 0xAA)
					{
						Usart3_RxPacket[pRxPacket] = Res;	//将数据存入数组
						pRxPacket++;
						RxState = 2;
					}
					else
					{
						pRxPacket = 0;
						RxState = 0;
					}
					break;
					
				case 2: 
					Usart3_RxPacket[pRxPacket] = Res;	//将数据存入数组
					pRxPacket++;
					if(pRxPacket >= 6)
					{
						if ((uint8_t)CalXorSum(Usart3_RxPacket, 5) != (uint8_t)Usart3_RxPacket[5])
						{
							pRxPacket = 0;
							RxState = 0;
						}
						else
						{
							RxState = 0;
							pRxPacket = 0;
							Usart3_seshig = 66;
							Usart3_RxFlag = 1;		//接收数据包标志位置1，成功接收一个数据包
						}
					}
					break;
					
				default: 
					break;
			}
			
			USART_ClearITPendingBit(USART3, USART_IT_RXNE);		//清除标志位
		} 
} 


