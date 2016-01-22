#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

#include <string.h>


// Cathodes
#define DIGIT_SEGMENTS_COUNT	8
#define DIGIT_SEGMANTS_PORT		GPIOA
int digit_segments[8] =			{GPIO_Pin_0 , GPIO_Pin_1 , GPIO_Pin_2 , GPIO_Pin_3 , GPIO_Pin_4 , GPIO_Pin_5 , GPIO_Pin_6 , GPIO_Pin_7};
int all_digit_segments =		(GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);

// Anodes
#define DIGITS_COUNT			2
#define DIGITS_PORT				GPIOC
int digits[2] =					{GPIO_Pin_13 , GPIO_Pin_14};
int all_digits =				(GPIO_Pin_13 | GPIO_Pin_14);

// Display
int display_data[2][8];
int current_digit = 0;

// Controller
int old_clk_state = 0;
int old_sw_state = 0;
int press_time = 0;

// Game
int current_number = 0;
int guessed_number = 17;

// Other
int tick_time = 10000;
int total_time = 0;

void Sleep(int i)
{
	while (i > 0)
	{
		i--;
	}
}

void DisplayInit()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef port;

    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_Out_PP;
    port.GPIO_Pin = all_digit_segments;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(DIGIT_SEGMANTS_PORT, &port);

    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_Out_PP;
    port.GPIO_Pin = all_digits;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(DIGITS_PORT, &port);

    int i;
    for (i = 0; i < DIGIT_SEGMENTS_COUNT; i++)
    {
    	GPIO_ResetBits(DIGIT_SEGMANTS_PORT, digit_segments[i]);
    }

    for (i = 0; i < DIGITS_COUNT; i++)
    {
    	GPIO_ResetBits(DIGITS_PORT, digits[i]);
    }
}

void DisplayTick()
{
	int i;
    for (i = 0; i < DIGITS_COUNT; i++)
    {
    	GPIO_SetBits(DIGITS_PORT, digits[i]);
    }

    for (i = 0; i < DIGIT_SEGMENTS_COUNT; i++)
    {
    	if (display_data[current_digit][i])
    	{
			GPIO_SetBits(DIGIT_SEGMANTS_PORT, digit_segments[i]);
    	}
    	else
    	{
			GPIO_ResetBits(DIGIT_SEGMANTS_PORT, digit_segments[i]);
    	}
    }

    GPIO_ResetBits(DIGITS_PORT, digits[current_digit]);

	// Shift digit
	current_digit++;
	current_digit = current_digit < DIGITS_COUNT ? current_digit : 0;
}


void SetDisplayData(int d0, int d1)
{
	int map[16][8] = {
		{1, 0, 1, 1, 0, 1, 1, 1},	// 0
		{0, 0, 0, 0, 0, 1, 1, 0},	// 1
		{0, 1, 1, 1, 0, 0, 1, 1},	// 2
		{0, 1, 0, 1, 0, 1, 1, 1},	// 3
		{1, 1, 0, 0, 0, 1, 1, 0},	// 4
		{1, 1, 0, 1, 0, 1, 0, 1},	// 5
		{1, 1, 1, 1, 0, 1, 0, 1},	// 6
		{0, 0, 0, 0, 0, 1, 1, 1},	// 7
		{1, 1, 1, 1, 0, 1, 1, 1},	// 8
		{1, 1, 0, 1, 0, 1, 1, 1},	// 9

		{0, 0, 0, 0, 0, 0, 0, 0},	// empty

		{0, 1, 0, 0, 0, 0, 0, 0},	// less 1
		{1, 0, 1, 1, 0, 0, 0, 1},	// less 2

		{0, 0, 0, 1, 0, 1, 1, 1},	// more 1
		{0, 1, 0, 0, 0, 0, 0, 0},	// more 2

		{0, 0, 0, 1, 0, 0, 0, 1}	// equal
	};

	memcpy(display_data[0], map[d0], 8 * 4);
	memcpy(display_data[1], map[d1], 8 * 4);
}

void SetDisplayNumber(int n)
{
	if (n > 99 || n < 0)
	{
		return;
	}

	if (n < 10)
	{
		SetDisplayData(10, n);
	}
	else
	{
		SetDisplayData(n / 10, n % 10);
	}
}

void ControllerInit()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef port;

    GPIO_StructInit(&port);
    port.GPIO_Mode = GPIO_Mode_IPU;
    port.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    port.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &port);
}

void ControllerTick()
{
	int new_clk_state = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_7);
	int new_sw_state = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9);
	int direction = !GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8);

	direction = direction ? 1 : -1;

	// Rotation
	if (old_clk_state == 0 && new_clk_state == 1)
	{
		current_number += direction;

		current_number += 100;
		current_number %= 100;
	}

	// Press
	if (new_sw_state)
	{
		if (current_number < guessed_number)
		{
			SetDisplayData(13, 14);
		}
		if (current_number > guessed_number)
		{
			SetDisplayData(11, 12);
		}
		if (current_number == guessed_number)
		{
			SetDisplayData(15, 15);
		}
	}
	else
	{
		SetDisplayNumber(current_number);
	}

	// New game
	if (old_sw_state == 0 && new_sw_state == 1)
	{
		press_time = 0;
	}

	if (old_sw_state == 1 && new_sw_state == 1)
	{
		press_time += tick_time;

		if (press_time > 10000000)
		{
			guessed_number = total_time / tick_time % 100;
			SetDisplayData(12, 13);
		}
	}

	old_sw_state = new_sw_state;
	old_clk_state = new_clk_state;
}

int main(void)
{
	DisplayInit();
	ControllerInit();
	SetDisplayNumber(current_number);

    while(1)
    {
		DisplayTick();
		ControllerTick();

		Sleep(tick_time);
		total_time += tick_time;
    }
}
