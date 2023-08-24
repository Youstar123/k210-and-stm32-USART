#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>

char Serial_RxPacket[100];	
uint8_t Serial_RxFlag;

void Serial_Init(void)
{	//开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	//初始化GPIO，PA9复用推挽输出，PA10上拉输入
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//初始化USART
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;//波特率
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不使用硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Tx |USART_Mode_Rx;//开启USART发送和接收模式
	USART_InitStructure.USART_Parity = USART_Parity_No;//无校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//八位字长
	USART_Init(USART1, &USART_InitStructure);
	//开启USART接收中断,RXNE中断标志位到NVIC
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);
	//配置NVIC
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//优先级分组，一般为2
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	//使能USART
	USART_Cmd(USART1, ENABLE);
}
//发送一个字节
void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);//写入TDR，写入操作会将TXE清零，不需要手动清零
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);//等待TDR的数据转移到移位寄存器
}
//发送一个数组
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Array[i]);
	}
}
//发送字符串
void Serial_SendString(char *String)//字符串自带结束标志位，不需要传递长度
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}
//把数字以十进制从各位上拆开
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}
//发送数字
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');//+0是以字符的形式发送
	}
}
//printf函数重定向，从输出到屏幕改为输出到串口
int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}
//把sprintf打印到字符串里并发送的部分封装
void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}
void USART1_IRQHandler(void)
{
	static uint8_t RxState = 0;
	static uint8_t pRxPacket = 0;
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		uint8_t RxData = USART_ReceiveData(USART1);
		
		if (RxState == 0)
		{
			if (RxData == 'A' && Serial_RxFlag == 0)
			{
				RxState = 1;
				pRxPacket = 0;
			}
		}
		else if (RxState == 1)
		{
			if (RxData == 'B')
			{
				RxState = 2;
			}
			else
			{
				Serial_RxPacket[pRxPacket] = RxData;
				pRxPacket ++;
			}
		}
		else if (RxState == 2)
		{
			if (RxData == 'C')
			{
				RxState = 0;
				Serial_RxPacket[pRxPacket] = '\0';
				Serial_RxFlag = 1;
			}
		}
		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
