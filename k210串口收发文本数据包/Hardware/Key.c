#include "stm32f10x.h"  // Device header
#include "Delay.h"
void Key_Init()
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//配置成上拉输入，初始高电平，按下低电平。
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1| GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//输入模式下，输出速度其实是没用的
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
}
uint8_t Key_GetNum(void)//读取按键状态返回键码，uint8_t是定义返回键码的数据类型
{
uint8_t KeyNum=0;//定义局部变量，如果没有按键按下就返回零
if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0)
{
    Delay_ms(20);//按下时按键消抖
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)==0);//按键按下时一直循环等待，直到松手跳出循环
	Delay_ms(20);//松手时按键消抖
    KeyNum=1;
}
if(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0)
{
    Delay_ms(20);
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11)==0);
	Delay_ms(20);
    KeyNum=2;
}
	return KeyNum;
}
