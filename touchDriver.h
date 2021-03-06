
#include "stm32f4xx_hal.h"

#define TOUCH_CS_PIN				GPIO_PIN_4
#define TOUCH_DCLK_PIN			GPIO_PIN_5

//FOLLOWING PIN IS DATA OUT FROM STM32
#define TOUCH_DIN_PIN				GPIO_PIN_6

//FOLLOWING PIN IS DATA IN TO STM32
#define TOUCH_DOUT_PIN			GPIO_PIN_10

#define TOUCH_PENIRQ_PIN		GPIO_PIN_11

#define TOUCH_PORT_REGISTER		GPIOD


void GPIO_Touch_Init(void);
uint16_t touch_ReadData(void);
void touch_WriteData(uint8_t data);
uint32_t touch_TestRead(void);
void touch_DemoPulse(void);
void touch_TestPulse(void);

