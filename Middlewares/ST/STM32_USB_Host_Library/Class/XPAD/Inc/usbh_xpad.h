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
/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_TEMPLATE_CLASS
* @{
*/

/** @defgroup USBH_TEMPLATE_CLASS
* @brief This file is the Header file for usbh_template.c
* @{
*/ 


/**
  * @}
  */ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Types
* @{
*/ 

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




/* States for TEMPLATE State Machine */


/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Defines
* @{
*/ 

/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Macros
* @{
*/ 
/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Variables
* @{
*/ 
extern USBH_ClassTypeDef  XPAD_Class;
#define USBH_XPAD_CLASS    &XPAD_Class

/**
* @}
*/ 

/** @defgroup USBH_TEMPLATE_CLASS_Exported_FunctionsPrototype
* @{
*/ 
USBH_StatusTypeDef USBH_XPAD_Process (USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_XPAD_InterfaceInit (USBH_HandleTypeDef *phost);
/**
* @}
*/ 

#ifdef __cplusplus
}
#endif

#endif /* __USBH_TEMPLATE_H */

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/ 

/**
* @}
*/ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

