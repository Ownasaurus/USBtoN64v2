
/* Define to prevent recursive -----------------------------------------------*/
#ifndef __USBH_HID_DS4_H
#define __USBH_HID_DS4_H

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

// source: http://www.psdevwiki.com/ps4/DS4-USB

typedef struct __attribute__((packed))
{
  union {
	  struct{
		 uint8_t	reportID;

		 uint8_t    LAnalogX;
		 uint8_t	LAnalogY;
		 uint8_t	RAnalogX;
		 uint8_t	RAnalogY;

		 uint8_t	dpad:4;
		 uint8_t	square:1;
		 uint8_t	x:1;
		 uint8_t	circle:1;
		 uint8_t	triangle:1;

		 uint8_t	L1:1;
		 uint8_t	R1:1;
		 uint8_t	L2:1;
		 uint8_t	R2:1;
		 uint8_t	share:1;
		 uint8_t	options:1;
		 uint8_t	L3:1;
		 uint8_t	R3:1;

		 uint8_t	PS:1;
		 uint8_t	touchpad:1;
		 uint8_t	reportcounter:6;

		 // 0x00 is released and 0xFF is fully pressed
		 uint8_t	left_trigger;
		 uint8_t	right_trigger;

		 uint8_t	unknown[2];

		 uint8_t	battery;

		 uint8_t	unknown2;

		 int16_t	accZ;
		 int16_t	accY;
		 int16_t	accX;

		 int16_t	gyroX;
		 int16_t	gyroY;
		 int16_t	gyroZ;

		 uint8_t	externaldevices;

		 uint8_t	unknown3[2];

		 uint8_t	tpadevent:4;
		 uint8_t	unknown4:4;

		 uint8_t	trackpad[17];

		 uint8_t	unknown5[11];
	  };
	  uint8_t    data[64];
  };
} HID_DS4_Info_TypeDef;

USBH_StatusTypeDef USBH_HID_DS4Init(USBH_HandleTypeDef *phost);
HID_DS4_Info_TypeDef *USBH_HID_GetDS4Info(USBH_HandleTypeDef *phost);

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
