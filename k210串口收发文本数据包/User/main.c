#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Serial.h"
#include "LED.h"
#include "string.h"

int main(void)
{
	OLED_Init();
	LED_Init();
	Serial_Init();
	
	OLED_ShowString(1, 1, "TxPacket");
	OLED_ShowString(3, 1, "RxPacket");
	
	while (1)
	{
		if (Serial_RxFlag == 1)
		{
			OLED_ShowString(4, 1, Serial_RxPacket);	
			Serial_RxFlag = 0;
		}
	}
}
