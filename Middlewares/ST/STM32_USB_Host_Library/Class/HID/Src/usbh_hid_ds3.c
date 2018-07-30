
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

void DS3_Led(USBH_HandleTypeDef *phost, int i)
{
	uint8_t ledpattern[7] = {0x02, 0x04, 0x08, 0x10, 0x12, 0x14, 0x18 };
	uint8_t abuffer[48] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                          0x00, 0x02, 0xff, 0x27, 0x10, 0x00, 0x32, 0xff,
	                          0x27, 0x10, 0x00, 0x32, 0xff, 0x27, 0x10, 0x00,
	                          0x32, 0xff, 0x27, 0x10, 0x00, 0x32, 0x00, 0x00,
	                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	if (i < 7) abuffer[9] = ledpattern[i];
//
//	uint8_t _ss_attributes_payload[] =
//	{
//	    0x52,
//	    0x00, 0x00, 0x00, 0x00, //Rumble
//	    0xff, 0x80, //Gyro
//	    0x00, 0x00,
//	    0x00, //* LED_1 = 0x02, LED_2 = 0x04, ... */
//	    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_4 */
//	    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_3 */
//	    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_2 */
//	    0xff, 0x27, 0x10, 0x00, 0x32, /* LED_1 */
//	};
//
//	USBH_StatusTypeDef result = USBH_HID_SetReport(phost,0x02,0x01,_ss_attributes_payload,sizeof(_ss_attributes_payload)); // set the player LED?

	USBH_HID_SetReport(phost,0x02,0x01,abuffer,sizeof(abuffer)); // set the player LED?
}

USBH_StatusTypeDef USBH_HID_DS3Init(USBH_HandleTypeDef *phost)
{
	  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;

	  memset(&ds3_data,0,sizeof(HID_DS3_Info_TypeDef));
	  memset(ds3_report_data,0,sizeof(uint32_t)*12);

	  if(HID_Handle->length > (sizeof(ds3_report_data)/sizeof(uint32_t)))
	  {
	    HID_Handle->length = (sizeof(ds3_report_data)/sizeof(uint32_t));
	  }
	  HID_Handle->pData = (uint8_t*)ds3_report_data;
	  fifo_init(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE * sizeof(ds3_report_data));

	  //DS3_Led(phost, 1);

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
	//TODO: MAKE THIS WORK

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
