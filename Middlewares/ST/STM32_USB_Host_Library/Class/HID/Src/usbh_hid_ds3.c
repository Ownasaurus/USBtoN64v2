
/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_ds3.h"
#include "usbh_hid_parser.h"
#include "main.h"

HID_DS3_Info_TypeDef ds3_data;
uint32_t ds3_report_data[12];

static const HID_Report_ItemTypedef imp_0_ds3_byte={
  (uint8_t*)ds3_report_data+0, /*data*/
  8,     /*size*/
  0,     /*shift*/
  48,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  255,   /*max value read can return*/
  0,     /*min vale device can report*/
  255,   /*max value device can report*/
  1      /*resolution*/
};

USBH_StatusTypeDef USBH_HID_DS3Init(USBH_HandleTypeDef *phost)
{
	  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;

	  memset(&ds3_data,0,sizeof(HID_DS3_Info_TypeDef));
	  memset(ds3_report_data,0,sizeof(ds3_report_data));

	  if(HID_Handle->length > (sizeof(ds3_report_data)/sizeof(uint32_t)))
	  {
	    HID_Handle->length = (sizeof(ds3_report_data)/sizeof(uint32_t));
	  }
	  HID_Handle->pData = (uint8_t*)ds3_report_data;
	  fifo_init(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE * sizeof(ds3_report_data));

	  return USBH_OK;
}


static USBH_StatusTypeDef USBH_HID_DS3Decode(USBH_HandleTypeDef *phost)
{
  int x;

  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;
  if(HID_Handle->length == 0)
  {
    return USBH_FAIL;
  }
  /*Fill report */
  if(fifo_read(&HID_Handle->fifo, &ds3_report_data, HID_Handle->length) ==  HID_Handle->length)
  {
	for(x=0; x < sizeof(HID_DS3_Info_TypeDef); x++)
	{
		ds3_data.data[x]=(uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &imp_0_ds3_byte, x);
	}

    return USBH_OK;
  }
  return   USBH_FAIL;
}

HID_DS3_Info_TypeDef *USBH_HID_GetDS3Info(USBH_HandleTypeDef *phost)
{
 if(USBH_HID_DS3Decode(phost) == USBH_OK)
 {
  return &ds3_data;
 }
 else
 {
  return NULL;
 }
}

// bit order  ds3: many 0s|r2|l2|triangle|square|circle|X|?|?|R1|L1|R3|L3|select|start|dright|dleft|ddown|dup
uint64_t USBH_HID_GetDS3ButtonsAndTriggers()
{
	return ds3_data.d_up | (ds3_data.d_down << 1) | (ds3_data.d_left << 2) | (ds3_data.d_right << 3) |
			(ds3_data.start << 4) | (ds3_data.select << 5) | (ds3_data.L3 << 6) | (ds3_data.R3 << 7) |
			(ds3_data.L1 << 8) | (ds3_data.R1 << 9) | // next two bits unused
			(ds3_data.x << 12) | (ds3_data.circle << 13) | (ds3_data.square << 14) | (ds3_data.triangle << 15) |
			(ds3_data.L2 << 16) | (ds3_data.R2 << 17);
}
