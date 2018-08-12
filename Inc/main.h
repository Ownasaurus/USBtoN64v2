/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define N64_DATA_Pin GPIO_PIN_8
#define N64_DATA_GPIO_Port GPIOA
#define N64_DATA_EXTI_IRQn EXTI9_5_IRQn
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

typedef enum
 {
   CONTROLLER_NONE = 0,
   CONTROLLER_XPAD,
   CONTROLLER_DS3,
   CONTROLLER_KB
 }
 ControllerType;

typedef struct __attribute__((packed))
{
    unsigned int a : 1; // 1 bit wide
    unsigned int b : 1;
    unsigned int z : 1;
    unsigned int start : 1;
    unsigned int up : 1;
    unsigned int down : 1;
    unsigned int left : 1;
    unsigned int right : 1;

    unsigned int dummy1 : 1;
    unsigned int dummy2 : 1;
    unsigned int l : 1;
    unsigned int r : 1;
    unsigned int c_up : 1;
    unsigned int c_down : 1;
    unsigned int c_left : 1;
    unsigned int c_right : 1;

    char x_axis;

    char y_axis;

} N64ControllerData; // all bits are in the correct order... except for the analog

typedef enum
{
	NORMAL=0,
	A_UP,
	A_DOWN,
	A_LEFT,
	A_RIGHT,
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	BUTTON_START,
	BUTTON_B,
	BUTTON_A,
	C_UP,
	C_DOWN,
	C_LEFT,
	C_RIGHT,
	BUTTON_L,
	BUTTON_R,
	BUTTON_Z
}
STATE;

typedef struct __attribute__((packed))
{
	struct __attribute__((packed))
	{
		uint64_t a;
		uint64_t b;
		uint64_t z;
		uint64_t start;
		uint64_t up;
		uint64_t down;
		uint64_t left;
		uint64_t right;
		uint64_t l;
		uint64_t r;
		uint64_t c_up;
		uint64_t c_down;
		uint64_t c_left;
		uint64_t c_right;

	} XpadControls;

	struct __attribute__((packed))
	{
		uint8_t KEYBOARD_a;
		uint8_t KEYBOARD_b;
		uint8_t KEYBOARD_z;
		uint8_t KEYBOARD_start;

		uint8_t KEYBOARD_d_up;
		uint8_t KEYBOARD_d_down;
		uint8_t KEYBOARD_d_left;
		uint8_t KEYBOARD_d_right;

		uint8_t KEYBOARD_l;
		uint8_t KEYBOARD_r;
		uint8_t KEYBOARD_c_up;
		uint8_t KEYBOARD_c_down;

		uint8_t KEYBOARD_c_left;
		uint8_t KEYBOARD_c_right;
		uint8_t KEYBOARD_a_up;
		uint8_t KEYBOARD_a_down;

		uint8_t KEYBOARD_a_left;
		uint8_t KEYBOARD_a_right;
	} KBControls;
}
Controls;

uint8_t reverse(uint8_t b);
/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
