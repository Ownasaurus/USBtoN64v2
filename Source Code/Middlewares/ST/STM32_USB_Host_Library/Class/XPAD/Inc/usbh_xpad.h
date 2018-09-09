/**
  ******************************************************************************
  * @file    usbh_template.h
  * @author  MCD Application Team
  * @version V3.2.2
  * @date    07-July-2015
  * @brief   This file contains all the prototypes for the usbh_template.c
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_XPAD_H
#define __USBH_XPAD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"

#define USB_XPAD_CLASS                                   0xFF

 typedef enum {
	 LED_OFF    = 0x00,
	 LED_BLINK  = 0x01,
	 LED1_FLASH = 0x02,
	 LED2_FLASH = 0x03,
	 LED3_FLASH = 0x04,
	 LED4_FLASH = 0x05,
	 LED1_ON    = 0x06,
	 LED2_ON    = 0x07,
	 LED3_ON    = 0x08,
	 LED4_ON    = 0x09,
	 LED_ROTATE = 0x0a,
	 LED_ALTERNATE = 0x0d,
 }
 XPAD_LED;

 typedef enum {
	 XPAD_HAT_UP    = 0x0001,
	 XPAD_HAT_DOWN  = 0x0002,
	 XPAD_HAT_LEFT  = 0x0004,
	 XPAD_HAT_RIGHT = 0x0008,
	 XPAD_START     = 0x0010,
	 XPAD_BACK      = 0x0020,
	 XPAD_STICK_L   = 0x0040,
	 XPAD_STICK_R   = 0x0080,
	 XPAD_PAD_LB    = 0x0100,
	 XPAD_PAD_RB    = 0x0200,
	 XPAD_XLOGO     = 0x0400,
	 XPAD_PAD_A     = 0x1000,
	 XPAD_PAD_B     = 0x2000,
	 XPAD_PAD_X     = 0x4000,
	 XPAD_PAD_Y     = 0x8000,
	 XPAD_BUTTONS   = 0x10000,
	 XPAD_STICK_LX,
	 XPAD_STICK_LY,
	 XPAD_STICK_RX,
	 XPAD_STICK_RY,
	 XPAD_TRIGGER_L,
	 XPAD_TRIGGER_R,
	 XPAD_BATTERY,
 }
 XPAD_PAD;

 typedef enum
 {
   XPAD_INIT= 0,
   XPAD_IDLE,
 }
 XPAD_StateTypeDef;

 typedef enum
 {
   XPAD_ORIGINAL    = 0x01,
   XPAD_360_WIRELESS = 0x02,
   XPAD_360_WIRED = 0x03,
   XPAD_XBONE = 0x04,
   XPAD_UNKNOWN = 0xFF,
 }
 XPAD_TypeTypeDef;

 typedef struct XPAD_Process
 {
   uint8_t              OutPipe;
   uint8_t              InPipe;
   XPAD_StateTypeDef    state;
   uint8_t              OutEp;
   uint8_t              InEp;
   uint8_t              *pData;
   uint16_t             length;
   uint8_t              ep_addr;
   uint16_t             poll;
   uint32_t             timer;
   uint8_t              DataReady;
   USBH_StatusTypeDef  ( * Init)(USBH_HandleTypeDef *phost);
   XPAD_TypeTypeDef		xpad_type;
 }
 XPAD_HandleTypeDef;

 uint8_t report[32];
 uint32_t buttons;
 int16_t stick_lx, stick_ly, stick_rx, stick_ry;
 uint8_t trigger_l, trigger_r;

extern USBH_ClassTypeDef  XPAD_Class;
#define USBH_XPAD_CLASS    &XPAD_Class


#ifdef __cplusplus
}
#endif

#endif /* __USBH_TEMPLATE_H */

