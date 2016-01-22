#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void sleep(int i)
{
	while (i > 0)
	{
		i--;
	}
}

int main(void)
{
#define PINS_COUNT 8
	int pins[8] = {GPIO_Pin_0, GPIO_Pin_1, GPIO_Pin_2, GPIO_Pin_3, GPIO_Pin_4, GPIO_Pin_5, GPIO_Pin_6, GPIO_Pin_7};
	int allPins = 0;

	int i;
	for (i = 0; i < PINS_COUNT; i++)
	{
		allPins |= pins[i];
	}

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef port;

    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_Out_PP;
    port.GPIO_Pin = allPins;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &port);

    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_Out_PP;
    port.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOC, &port);


	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	GPIO_SetBits(GPIOC, GPIO_Pin_14);


    while(1)
    {
    	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    	GPIO_SetBits(GPIOC, GPIO_Pin_14);

    	for (i = 0; i < PINS_COUNT; i++)
    	{
    		int j;
        	for (j = 0; j < PINS_COUNT; j++)
        	{
    			GPIO_ResetBits(GPIOA, pins[j]);
        	}

        	GPIO_SetBits(GPIOA, pins[i]);
    		sleep(1000000);
    	}

    	GPIO_SetBits(GPIOC, GPIO_Pin_13);
    	GPIO_ResetBits(GPIOC, GPIO_Pin_14);

    	for (i = 0; i < PINS_COUNT; i++)
    	{
    		int j;
        	for (j = 0; j < PINS_COUNT; j++)
        	{
    			GPIO_ResetBits(GPIOA, pins[j]);
        	}

        	GPIO_SetBits(GPIOA, pins[i]);
    		sleep(1000000);
    	}
    }
}
