/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "defines.h"
#include "tm_stm32_fatfs.h"

#include "../touchDriver.h"

struct __FILE
{
	int dummy;
};

FILE __stdout;

#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


/* USER CODE BEGIN Includes */
__IO uint8_t flag_systick_called = RESET;
__IO uint32_t tick_count_primary = 0;
__IO uint8_t flag_uart3txcplt = SET;
uint8_t TxBuffer[] = "This is a sample text                                           \n\r";
	
char Rx3Buffer[] = "This is a sample text                                           \n\r";
char rx3buff[4];
uint8_t flag_rx3_received = RESET;
uint8_t rx3TimeOutCounter = 0;
uint8_t flag_rx3TimeOut = RESET;
uint8_t idx_rx3buff = 0;
uint32_t gen1_timeoutCounter = 0;
	
/* Fatfs structure */
FATFS FS;
FIL fil;
FRESULT fres;

/* Size structure for FATFS */
TM_FATFS_Size_t CardSize;

/* Buffer variable */
char buffer[512];
	
/* USER CODE END Includes */

void OneSecOperation(void);
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;

UART_HandleTypeDef huart3;

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART3_UART_Init(void);



int main(void)
{
	
	uint32_t fByte=0;
	
	uint32_t temp_x=0;

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM6_Init();
  
	MX_USART3_UART_Init();
	
	HAL_UART_Receive_IT(&huart3, (uint8_t *)rx3buff, 1);
	
	GPIO_Touch_Init();

  /* Infinite loop */
  while (1)
  {
		if(flag_systick_called == SET)
		{
			flag_systick_called = RESET;
			tick_count_primary++;
			if(tick_count_primary >= 300)
			{
				OneSecOperation();
				tick_count_primary = 0;
				
				temp_x = touch_TestRead();
				
				printf("X/Y: %u\r\n", temp_x);
				
			}
		}		
		
		if(flag_rx3_received)
		{
			flag_rx3_received = RESET;
			if(strstr((const char*)Rx3Buffer, "TEST") != NULL)
			{
				printf("TEST COMMAND RECEIVED\n\r");
			}
			
			if(strstr((const char*)Rx3Buffer, "ERASE1") != NULL)
			{
				printf("ERASE BLOCK\n\r");
			}
			
			if(strstr((const char*)Rx3Buffer, "MOUNTSD") != NULL)
			{
				if (f_mount(&FS, "SD:", 1) == FR_OK)
				{
					printf("SD MOUNTED\r\n");
					TM_FATFS_GetDriveSize("SD:", &CardSize);
					printf("SIZE:%u\r\n", CardSize.Total);
				}
				else
				{
					printf("SD NOT MOUNTED\n\r");
				}
			}
			
			if(strstr((const char*)Rx3Buffer, "CREATE1") != NULL)
			{
				fres = f_open(&fil, "SD:first_file.txt", FA_CREATE_NEW | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
				if(fres == FR_OK)
				{
					printf("Openning ....\r\n");
					sprintf(buffer, "Total card size: %u kBytes\n", CardSize.Total);
					
					f_puts(buffer, &fil);
					
					sprintf(buffer, "Free card size:  %u kBytes\n", CardSize.Free);
					
					f_puts(buffer, &fil);
					
					f_close(&fil);
					
				}
				else
				{
					printf("Cannot Open File : %u\r\n", fres);
				}
			}
			
			if(strstr((const char*)Rx3Buffer, "READ1") != NULL)
			{
				if(f_open(&fil, "SD:first_file.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
				{
					f_read(&fil, buffer, 12, &fByte);
					
					printf("%s\r\n", buffer);
					
					f_close(&fil);
				}
				else
				{
					printf("Cannot Open For Read\r\n");
				}
			}
			
			if(strstr((const char*)Rx3Buffer, "READ2") != NULL)
			{
				if(f_open(&fil, "SD:first_file.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE) == FR_OK)
				{
					fres = FR_OK;
					free(buffer);
					while(fres == FR_OK)
					{
						
						fres = f_read(&fil, buffer, 1, &fByte);
						printf("%s", buffer);
						if(fByte == 0)
						{
							break;
						}
					}
					
					f_close(&fil);
				}
				else
				{
					printf("Cannot Open For Read\r\n");
				}
			}
			
			if(strstr((const char*)Rx3Buffer, "APPEND1") != NULL)
			{
				fres = f_open(&fil, "SD:first_file.txt", FA_OPEN_APPEND | FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
				if(fres == FR_OK)
				{
					printf("Appending\r\n");
					sprintf(buffer, "Total card size: %u kBytes\n", CardSize.Total);
					
					f_puts(buffer, &fil);
					
					sprintf(buffer, "Free card size:  %u kBytes\n", CardSize.Free);
					
					f_puts(buffer, &fil);
					
					f_close(&fil);
				}
				else
				{
					printf("Cannot open for appending\r\n");
				}
			}
			
			if(strstr((const char*)Rx3Buffer, "UMOUNTSD") != NULL)
			{
				fres = f_mount(NULL, "SD:", 1);
				
				if(fres == FR_OK)
				{
					printf("SD Unmounted\r\n");
				}
				else
				{
					printf("Unmount Failed\r\n");
				}
			}
			
		}
  }
}

void OneSecOperation(void)
{
	if(flag_uart3txcplt)
	{
		//HAL_UART_Transmit_IT(&huart3, TxBuffer, sizeof(TxBuffer));
		flag_uart3txcplt = RESET;
	}
	
	//printf("From printf text.\r\n");
	//HAL_UART_Transmit(&huart3, (uint8_t*) TxBuffer, 10, 0x0fff);
	HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_12);
}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* TIM6 init function */
void MX_TIM6_Init(void)
{

//  TIM_MasterConfigTypeDef sMasterConfig;

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 8399;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 9999;
  HAL_TIM_Base_Init(&htim6);

//  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
//  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
//  HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig);

}

void HAL_SYSTICK_Callback(void)
{
	flag_systick_called = SET;
	
	if(gen1_timeoutCounter > 0)
	{
		gen1_timeoutCounter--;
	}
	
	if(rx3TimeOutCounter > 0)
	{
		if(rx3TimeOutCounter == 1)
		{
			flag_rx3_received = SET;
			idx_rx3buff = 0;
		}
		rx3TimeOutCounter--;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3)
	{
		flag_uart3txcplt = SET;
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	gen1_timeoutCounter = 2000;
	
	if(huart->Instance == USART3)
	{
		Rx3Buffer[idx_rx3buff] = rx3buff[0];
		Rx3Buffer[idx_rx3buff+1] = 0;
		if(idx_rx3buff < 45)
		{
			idx_rx3buff++;
		}
		rx3TimeOutCounter = 5;
		HAL_UART_Receive_IT(&huart3, (uint8_t *)rx3buff, 1);
		
		HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_14);
	}
}

/* USART1 init function */
void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
	//huart1.Init.Mode = UART_MODE_TX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  //huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart3);

}


void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __GPIOD_CLK_ENABLE();

  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
  //GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
PUTCHAR_PROTOTYPE
{
  /* Place your implementation of fputc here */
  /* e.g. write a character to the USART */
  HAL_UART_Transmit(&huart3, (uint8_t*) &ch, 1, 0x0fff);

  /* Loop until the end of transmission */
  //while (USART_GetFlagStatus(Port_USART, USART_FLAG_TC) == RESET)
  //{}

  return ch;
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
