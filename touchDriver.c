

#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "touchDriver.h"


void __delay_cycle(uint32_t cycle_count)
{
	uint32_t count=0;
	for(count=0;count<cycle_count;count++)
	{
		__ASM volatile ("NOP");
	}
}


void GPIO_Touch_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	__GPIOD_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6;
	
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11; // | GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void pulse_high(GPIO_TypeDef *gpio, uint16_t pin)
{
	gpio->ODR |= pin;
	__delay_cycle(6);
	gpio->ODR &= ~pin;
	__delay_cycle(6);
}

void pulse_low(GPIO_TypeDef *gpio, uint16_t pin)
{
	gpio->ODR &= ~pin;
	__delay_cycle(6);
	gpio->ODR |= pin;
	__delay_cycle(6);
}

uint32_t read_pin(GPIO_TypeDef *gpio, uint16_t pin)
{
	if(gpio->IDR & pin)
	{
		return 1UL;
	}
	else
	{
		return 0UL;
	}
}

void write_pin(GPIO_TypeDef *gpio, uint16_t pin, uint8_t bit)
{
	if(bit)
	{
		gpio->ODR |= pin;
	}
	else
	{
		gpio->ODR &= ~pin;
	}
}

uint16_t touch_ReadData(void)
{
	uint16_t data=0;
	uint16_t count=0;
	
	for(count=0;count<12;count++)
	{
		data <<= 1;
		pulse_high(GPIOD, TOUCH_DCLK_PIN);
		if(read_pin(GPIOD, TOUCH_DOUT_PIN))
		{
			data++;
		}
	}
	return data;
}

uint32_t touch_TestRead(void)
{
	uint32_t temp_x=0;
	
	write_pin(GPIOD, TOUCH_CS_PIN, 0);
	
	if(!read_pin(GPIOD, TOUCH_PENIRQ_PIN))
	{
		touch_WriteData(0x90);
		pulse_high(GPIOD, TOUCH_DCLK_PIN);
		temp_x = touch_ReadData();
		return temp_x;
	}
	else
	{
		return 0UL;
	}
}

void touch_WriteData(uint8_t data)
{
	uint8_t count=0;
	uint8_t tmp = data;
	write_pin(GPIOD, TOUCH_DCLK_PIN, 1);
	__delay_cycle(50);
	
	for(count=0;count<8;count++)
	{
		if(tmp & 0x80)
		{
			write_pin(GPIOD, TOUCH_DIN_PIN, 1);
		}
		else
		{
			write_pin(GPIOD, TOUCH_DIN_PIN, 0);
		}
		tmp <<= 1;
		pulse_low(GPIOD, TOUCH_DCLK_PIN);
	}
}

