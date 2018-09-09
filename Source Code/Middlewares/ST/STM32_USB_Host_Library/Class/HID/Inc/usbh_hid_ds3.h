
/* Define to prevent recursive -----------------------------------------------*/
#ifndef __USBH_HID_DS3_H
#define __USBH_HID_DS3_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */
  
/** @defgroup USBH_HID_KEYBD
  * @brief This file is the Header file for usbh_hid_keybd.c
  * @{
  */ 


/** @defgroup USBH_HID_KEYBD_Exported_Types
  * @{
  */ 

// source: http://eleccelerator.com/wiki/index.php?title=DualShock_3
typedef struct __attribute__((packed))
{
  union {
	  struct{
		 uint8_t    reportID;
		 uint8_t    reserved;

		 uint8_t	select:1;
		 uint8_t	L3:1;
		 uint8_t	R3:1;
		 uint8_t	start:1;
		 uint8_t	d_up:1;
		 uint8_t	d_right:1;
		 uint8_t	d_down:1;
		 uint8_t	d_left:1;

		 uint8_t    L2:1;
		 uint8_t    R2:1;
		 uint8_t    L1:1;
		 uint8_t    R1:1;
		 uint8_t    triangle:1;
		 uint8_t    circle:1;
		 uint8_t    x:1;
		 uint8_t    square:1;

		 uint8_t    PS; // only bit 0 is used
		 uint8_t    unknown_button_data;

		 // top left is 0 for analog values
		 uint8_t    LAnalogX;
		 uint8_t    LAnalogY;
		 uint8_t    RAnalogX;
		 uint8_t    RAnalogY;

		 uint8_t	unknown2[3];

		 // 0x00 is released and 0xFF is fully pressed
		 uint8_t    analog_select;
		 uint8_t    analog_L3;
		 uint8_t    analog_R3;
		 uint8_t    analog_start;
		 uint8_t    analog_L2;
		 uint8_t    analog_R2;
		 uint8_t    analog_L1;
		 uint8_t    analog_R1;
		 uint8_t    analog_triangle;
		 uint8_t    analog_circle;
		 uint8_t    analog_x;
		 uint8_t    analog_square;

		 uint8_t	unknown3[15];

		 // little endian 10 bit unsigned
		 uint16_t	accelerometer_x;
		 uint16_t	accelerometer_y;
		 uint16_t	accelerometer_z;
		 uint16_t	gyroscope;
	  };
	  uint8_t    data[48];
  };
} HID_DS3_Info_TypeDef;

USBH_StatusTypeDef USBH_HID_DS3Init(USBH_HandleTypeDef *phost);
HID_DS3_Info_TypeDef *USBH_HID_GetDS3Info(USBH_HandleTypeDef *phost);

/**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif /* __USBH_HID_KEYBD_H */

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

