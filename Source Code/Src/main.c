// Ownasaurus
// rawr

/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "usb_host.h"

/* USER CODE BEGIN Includes */
#include <string.h>
#include "usbh_hid_ds3.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
uint8_t state = NORMAL;
#define SAVE_ADDR 0x08010000 // sector 4
Controls controls;
Controls* saveData = (Controls*)SAVE_ADDR;
uint8_t blueButtonPressed = 0;
ControllerType type = CONTROLLER_NONE;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
N64ControllerData n64_data;
GCControllerData gc_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void MX_USB_HOST_Process(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void my_wait_us_asm(int n);

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void ChangeButtonMappingKB(uint8_t bt)
{
    if(state == A_UP) // state = 1 --> analog up
    {
        controls.KBControls.KEYBOARD_a_up = bt;
    }
    else if(state == A_DOWN) // state = 2 --> analog up
    {
        controls.KBControls.KEYBOARD_a_down = bt;
    }
    else if(state == A_LEFT) // state = 3 --> analog up
    {
        controls.KBControls.KEYBOARD_a_left = bt;
    }
    else if(state == A_RIGHT) // state = 4 --> analog up
    {
        controls.KBControls.KEYBOARD_a_right = bt;
    }
    else if(state == DPAD_UP) // state = 5 --> dpad up
    {
        controls.KBControls.KEYBOARD_d_up = bt;
    }
    else if(state == DPAD_DOWN) // state = 6 --> dpad down
    {
        controls.KBControls.KEYBOARD_d_down = bt;
    }
    else if(state == DPAD_LEFT) // state = 7 --> dpad left
    {
        controls.KBControls.KEYBOARD_d_left = bt;
    }
    else if(state == DPAD_RIGHT) // state = 8 --> dpad right
    {
        controls.KBControls.KEYBOARD_d_right = bt;
    }
    else if(state == BUTTON_START) // state = 9 --> start
    {
        controls.KBControls.KEYBOARD_start = bt;
    }
    else if(state == BUTTON_B) // state = 10 --> B
    {
        controls.KBControls.KEYBOARD_b = bt;
    }
    else if(state == BUTTON_A) // state = 11 --> A
    {
        controls.KBControls.KEYBOARD_a = bt;
    }
    else if(state == C_UP) // state = 12 --> c up
    {
        controls.KBControls.KEYBOARD_c_up = bt;
    }
    else if(state == C_DOWN) // state = 13 --> c down
    {
        controls.KBControls.KEYBOARD_c_down = bt;
    }
    else if(state == C_LEFT) // state = 14 --> c left
    {
        controls.KBControls.KEYBOARD_c_left = bt;
    }
    else if(state == C_RIGHT) // state = 15 --> c right
    {
        controls.KBControls.KEYBOARD_c_right = bt;
    }
    else if(state == BUTTON_L) // state = 16 --> L
    {
        controls.KBControls.KEYBOARD_l = bt;
    }
    else if(state == BUTTON_R) // state = 17 --> R
    {
        controls.KBControls.KEYBOARD_r = bt;
    }
    else if(state == BUTTON_Z) // state = 18 --> Z
    {
        controls.KBControls.KEYBOARD_z = bt;
    }
}

void ChangeButtonMappingController(uint64_t bt)
{
    if(state == DPAD_UP) // state = 5 --> dpad up
    {
    	controls.XpadControls.up = bt;
    }
    else if(state == DPAD_DOWN) // state = 6 --> dpad down
    {
    	controls.XpadControls.down = bt;
    }
    else if(state == DPAD_LEFT) // state = 7 --> dpad left
    {
        controls.XpadControls.left = bt;
    }
    else if(state == DPAD_RIGHT) // state = 8 --> dpad right
    {
        controls.XpadControls.right = bt;
    }
    else if(state == BUTTON_START) // state = 9 --> start
    {
        controls.XpadControls.start = bt;
    }
    else if(state == BUTTON_B) // state = 10 --> B
    {
        controls.XpadControls.b = bt;
    }
    else if(state == BUTTON_A) // state = 11 --> A
    {
        controls.XpadControls.a = bt;
    }
    else if(state == C_UP) // state = 12 --> c up
    {
        controls.XpadControls.c_up = bt;
    }
    else if(state == C_DOWN) // state = 13 --> c down
    {
        controls.XpadControls.c_down = bt;
    }
    else if(state == C_LEFT) // state = 14 --> c left
    {
        controls.XpadControls.c_left = bt;
    }
    else if(state == C_RIGHT) // state = 15 --> c right
    {
        controls.XpadControls.c_right = bt;
    }
    else if(state == BUTTON_L) // state = 16 --> L
    {
        controls.XpadControls.l = bt;
    }
    else if(state == BUTTON_R) // state = 17 --> R
    {
        controls.XpadControls.r = bt;
    }
    else if(state == BUTTON_Z) // state = 18 --> Z
    {
        controls.XpadControls.z = bt;
    }
}

void SaveControls()
{
	HAL_FLASH_Unlock(); //unlock flash writing
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.Sector = FLASH_SECTOR_4;
	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.NbSectors = 1;
	uint32_t SectorError = 0;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
	    HAL_FLASH_Lock();
	    return;
	}

    uint32_t* data = (uint32_t*)&controls;

    // Total size is 114 bytes + 18 bytes = 132 bytes
    // Each word is 4 bytes, so the total size is 33 Words
    for(int ct = 0;ct < 33;ct++)
    {
    	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,SAVE_ADDR+(ct*4),*data); //each SAVE_ADDR+4 is 4 bytes because it is a memory address
        data++; // each data+1 is 4 bytes because it is a 32 bit data type
    }

    HAL_FLASH_Lock(); // lock it back up
}

void LoadControls()
{
    memcpy(&controls,saveData,sizeof(Controls));
}

void AdvanceState()
{
    state++;
    if((state == STATE_SENSITIVITY && type == CONTROLLER_KB) || state > STATE_DEADZONE) // we're done mapping the controls
    {
		SaveControls(); // write directly to flash
		state = NORMAL; // back to normal controller operation
    }
    else if(state == STATE_SENSITIVITY)
    {
    	controls.XpadControls.range = DEFAULT_RANGE;
    }
    else if(state == STATE_DEADZONE)
    {
    	controls.XpadControls.deadzone = DEFAULT_DEADZONE;
    }
}

uint8_t reverse(uint8_t b)
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}

void SetN64DataInputMode()
{
	// port A8 to input mode
	GPIOA->MODER &= ~(1 << 17);
	GPIOA->MODER &= ~(1 << 16);
}

void SetN64DataOutputMode()
{
	// port A8 to output mode
	GPIOA->MODER &= ~(1 << 17);
	GPIOA->MODER |= (1 << 16);
}

void write_1()
{
	GPIOA->BSRR = (1 << 24);
	my_wait_us_asm(1);
	GPIOA->BSRR = (1 << 8);
    my_wait_us_asm(3);
}

void write_0()
{
	GPIOA->BSRR = (1 << 24);
	my_wait_us_asm(3);
	GPIOA->BSRR = (1 << 8);
    my_wait_us_asm(1);
}

void SendStop()
{
	GPIOA->BSRR = (1 << 24);
	my_wait_us_asm(1);
	GPIOA->BSRR = (1 << 8);
}

// send a byte from LSB to MSB (proper serialization)
void SendByte(unsigned char b)
{
    for(int i = 0;i < 8;i++) // send all 8 bits, one at a time
    {
        if((b >> i) & 1)
        {
            write_1();
        }
        else
        {
            write_0();
        }
    }
}

void SendIdentityN64()
{
    // reply 0x05, 0x00, 0x02
    SendByte(0x05);
    SendByte(0x00);
    SendByte(0x02);
    SendStop();
}


void SendIdentityGC()
{
    SendByte(0x90);
    SendByte(0x00);
    SendByte(0x0C);
    SendStop();
}

void SendControllerDataN64()
{
    unsigned long data = *(unsigned long*)&n64_data;
    unsigned int size = sizeof(data) * 8; // should be 4 bytes * 8 = 32 bits

    for(unsigned int i = 0;i < size;i++)
    {
        if((data >> i) & 1)
        {
            write_1();
        }
        else
        {
            write_0();
        }
    }

    SendStop();
}

void SendControllerDataGC()
{
    uint64_t data = *(uint64_t*)&gc_data;
    unsigned int size = sizeof(data) * 8; // should be 8 bytes * 8 = 64 bits

    for(unsigned int i = 0;i < size;i++)
    {
        if((data >> i) & 1)
        {
            write_1();
        }
        else
        {
            write_0();
        }
    }

    SendStop();
}

void SendOriginGC()
{
	gc_data.a_x_axis = reverse(128);
	gc_data.a_y_axis = reverse(128);
	gc_data.c_x_axis = reverse(128);
	gc_data.c_y_axis = reverse(128);
	gc_data.l_trigger = 0;
	gc_data.r_trigger = 0;

	uint64_t data = *(uint64_t*)&gc_data;
	unsigned int size = sizeof(data) * 8; // should be 8 bytes * 8 = 64 bits

	for(unsigned int i = 0;i < size;i++)
	{
		if((data >> i) & 1)
		{
			write_1();
		}
		else
		{
			write_0();
		}
	}

	SendByte(0x00);
	SendByte(0x00);
	SendStop();
}

// 0 is 3 microseconds low followed by 1 microsecond high
// 1 is 1 microsecond low followed by 3 microseconds high
// if either of these while loops is going on 4us or more, break out of the function

uint8_t GetMiddleOfPulse()
{
	uint8_t ct = 0;
    // wait for line to go high
    while(1)
    {
        if(GPIOA->IDR & 0x0100) break;

        ct++;
        if(ct == 200) // failsafe limit TBD
        	return 5; // error code
    }

    ct = 0;

    // wait for line to go low
    while(1)
    {
        if(!(GPIOA->IDR & 0x0100)) break;

        ct++;
		if(ct == 200) // failsafe limit TBD
			return 5; // error code
    }

    // now we have the falling edge

    // wait 2 microseconds to be in the middle of the pulse, and read. high --> 1.  low --> 0.
    my_wait_us_asm(2);

    return (GPIOA->IDR & 0x0100) ? 1U : 0U;
}



uint32_t readCommand()
{
	uint8_t retVal;

	// we are already at the first falling edge
	// get middle of first pulse, 2us later
	my_wait_us_asm(2);
	uint32_t command = (GPIOA->IDR & 0x0100) ? 1U : 0U, bits_read = 1;

    while(1) // read at least 9 bits (1 byte + stop bit)
    {
        command = command << 1; // make room for the new bit
        retVal = GetMiddleOfPulse();
        if(retVal == 5) // timeout
        {
        	if(bits_read >= 8)
        	{
				command = command >> 2; // get rid of the stop bit AND the room we made for an additional bit
				return command;
        	}
        	else // there is no possible way this can be a real command
        	{
        		return 5; // dummy value
        	}
        }
        command += retVal;

        bits_read++;

        if(bits_read >= 25) // this is the longest known command length
        {
        	command = command >> 1; // get rid of the stop bit (which is always a 1)
        	return command;
        }
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  output_type = OUTPUT_UNDEFINED;
  memset(&n64_data,0,4); // clear controller state
  memset(&gc_data,0,8); // clear controller state
  gc_data.beginning_one = 1;
  LoadControls();

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

  /* USER CODE END WHILE */
	// check for button pressed to begin control re-map

	  if(state == NORMAL)
	  {
		  if(!(GPIOC->IDR & 0x2000)) // user wants to change controls
		  {
			  if(blueButtonPressed == 0) // make sure it's a separate button press
			  {

				  if(type == CONTROLLER_KB)
					  state = 1;
				  else if(type != CONTROLLER_NONE)
					  state = 5;

				  if(type != CONTROLLER_NONE)
				  {
					  // enter programming mode
					  GPIOA->BSRR = (1 << 5); // LED ON
					  blueButtonPressed = 1;
					  continue;
				  }
			  }
		  }
		  else
		  {
			  blueButtonPressed = 0;
		  }

		  MX_USB_HOST_Process();
	  }
	  else
	  {
		  if(!(GPIOC->IDR & 0x2000)) // user wants to cancel and return to regular mode
		  {
			  if(blueButtonPressed == 0) // make sure it's a separate button press
			  {
				  GPIOA->BSRR = (1 << 21); // LED OFF
				  blueButtonPressed = 1;
				  state = NORMAL;
				  continue;
			  }
		  }
		  else
		  {
			  blueButtonPressed = 0;
		  }

		  MX_USB_HOST_Process();

		  if(state == NORMAL) // about to return to normal operation, make sure the LED turns off
		  {
			  GPIOA->BSRR = (1 << 21); // LED OFF
			  blueButtonPressed = 0;
		  }
	  }

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) // 168 MHz
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  //HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  HAL_NVIC_SetPriority(SysTick_IRQn, 1, 0);
}

/* USART2 init function */
static void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
        * Free pins are configured automatically as Analog (this feature is enabled through 
        * the Code Generation settings)
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin (blue pushbutton) */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3 
                           PC4 PC5 PC6 PC7 PC8
                           PC9 PC10 PC11 PC12 PC14 PC15*/
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8
                          |GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA4 PA6 
                           PA7 PA9 PA10 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_6 
                          |GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin (green LED) */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10 
                           PB12 PB13 PB14 PB15 
                           PB4 PB5 PB6 PB7 
                           PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15 
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : N64_DATA_Pin */
  GPIO_InitStruct.Pin = N64_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(N64_DATA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
