/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_ds4.h"
#include "usbh_hid_parser.h"
#include "main.h"

HID_DS4_Info_TypeDef ds4_data;
uint32_t ds4_report_data[16];

static const HID_Report_ItemTypedef imp_0_ds4_byte={
  (uint8_t*)ds4_report_data+0, /*data*/
  8,     /*size*/
  0,     /*shift*/
  64,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  255,   /*max value read can return*/
  0,     /*min vale device can report*/
  255,   /*max value device can report*/
  1      /*resolution*/
};

enum DPADEnum {
		DS4_DPAD_UP = 0x0,
		DS4_DPAD_UP_RIGHT = 0x1,
		DS4_DPAD_RIGHT = 0x2,
		DS4_DPAD_RIGHT_DOWN = 0x3,
		DS4_DPAD_DOWN = 0x4,
		DS4_DPAD_DOWN_LEFT = 0x5,
		DS4_DPAD_LEFT = 0x6,
		DS4_DPAD_LEFT_UP = 0x7,
		DS4_DPAD_OFF = 0x8,
};

USBH_StatusTypeDef USBH_HID_DS4Init(USBH_HandleTypeDef *phost)
{
	  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;

	  memset(&ds4_data,0,sizeof(HID_DS4_Info_TypeDef));
	  memset(ds4_report_data,0,sizeof(ds4_report_data));

	  if(HID_Handle->length > (sizeof(ds4_report_data)/sizeof(uint32_t)))
	  {
	    HID_Handle->length = (sizeof(ds4_report_data)/sizeof(uint32_t));
	  }
	  HID_Handle->pData = (uint8_t*)ds4_report_data;
	  fifo_init(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE * sizeof(ds4_report_data));

	  return USBH_OK;
}


static USBH_StatusTypeDef USBH_HID_DS4Decode(USBH_HandleTypeDef *phost)
{
  int x;

  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;
  if(HID_Handle->length == 0)
  {
    return USBH_FAIL;
  }
  /*Fill report */
  if(fifo_read(&HID_Handle->fifo, &ds4_report_data, HID_Handle->length) ==  HID_Handle->length)
  {
	for(x=0; x < sizeof(HID_DS4_Info_TypeDef); x++)
	{
		ds4_data.data[x]=(uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_ds4_byte, x);
	}

    return USBH_OK;
  }
  return   USBH_FAIL;
}

HID_DS4_Info_TypeDef *USBH_HID_GetDS4Info(USBH_HandleTypeDef *phost)
{
 if(USBH_HID_DS4Decode(phost) == USBH_OK)
 {
  return &ds4_data;
 }
 else
 {
  return NULL;
 }
}

// bit order  ds4: many 0s|r2|l2|triangle|square|circle|X|?|?|R1|L1|R3|L3|share|options|dright|dleft|ddown|dup
uint64_t USBH_HID_GetDS4ButtonsAndTriggers()
{
	uint8_t dpad_data = 0; // the lower 4 bits need to indicate up down left and right

	// up - set bit 0
	if(ds4_data.dpad == DS4_DPAD_LEFT_UP || ds4_data.dpad == DS4_DPAD_UP || ds4_data.dpad == DS4_DPAD_UP_RIGHT)
	{
		dpad_data |= 1;
	}
	// right - set bit 3
	if(ds4_data.dpad == DS4_DPAD_UP_RIGHT || ds4_data.dpad == DS4_DPAD_RIGHT || ds4_data.dpad == DS4_DPAD_RIGHT_DOWN)
	{
		dpad_data |= 8;
	}
	// down - set bit 1
	if(ds4_data.dpad == DS4_DPAD_RIGHT_DOWN || ds4_data.dpad == DS4_DPAD_DOWN || ds4_data.dpad == DS4_DPAD_DOWN_LEFT)
	{
		dpad_data |= 2;
	}
	// left - set bit 2
	if(ds4_data.dpad == DS4_DPAD_DOWN_LEFT || ds4_data.dpad == DS4_DPAD_LEFT || ds4_data.dpad == DS4_DPAD_LEFT_UP)
	{
		dpad_data |= 4;
	}
	return	dpad_data |
			(ds4_data.options << 4) | (ds4_data.share << 5) | (ds4_data.L3 << 6) | (ds4_data.R3 << 7) |
			(ds4_data.L1 << 8) | (ds4_data.R1 << 9) | // next two bits unused
			(ds4_data.x << 12) | (ds4_data.circle << 13) | (ds4_data.square << 14) | (ds4_data.triangle << 15) |
			(ds4_data.L2 << 16) | (ds4_data.R2 << 17);
}
