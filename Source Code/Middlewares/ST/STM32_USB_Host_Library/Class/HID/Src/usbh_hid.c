/**
  ******************************************************************************
  * @file    usbh_hid.c
  * @author  MCD Application Team
  * @version V3.2.2
  * @date    07-July-2015
  * @brief   This file is the HID Layer Handlers for USB Host HID class.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse and Keyboard protocols
  *      
  *  @endverbatim
  *
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

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hid_parser.h"
#include "usbh_xpad.h"
extern N64ControllerData n64_data;

static uint8_t ps4_led_buffer[31] = {0xFF, 0x00, 0x00, // [0] is 0xFF
									0x00, 0x00, // [3] and [4] are rumble values... probably want 0x00
									0x00, 0x00, 0xFF, // [5] [6] and [7] are RGB values, respectively
									0xFF, 0xFF, //[8] and [9] are time to flash bright and dark (0xFF = 2.5 seconds)
									0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static uint8_t led_buffer[48] = {  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
							  0x02, /* LED_1 = 0x02, LED_2 = 0x04, ... */
							  0xff, 0x27, 0x10, 0x00, 0x32,
							  0xff, 0x27, 0x10, 0x00, 0x32,
							  0xff, 0x27, 0x10, 0x00, 0x32,
							  0xff, 0x27, 0x10, 0x00, 0x32,// 29 bytes
							  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // 48 bytes

extern uint8_t state;
uint8_t keyboardButtonPressed = 0;
uint8_t ds3ButtonPressed = 0;
extern Controls controls;
extern ControllerType type;

void ChangeButtonMappingKB(uint8_t bt);
void ChangeButtonMappingController(uint64_t bt);
void AdvanceState();
uint64_t USBH_HID_GetDS3ButtonsAndTriggers();
uint64_t USBH_HID_GetDS4ButtonsAndTriggers();

static USBH_StatusTypeDef USBH_HID_InterfaceInit  (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HID_InterfaceDeInit  (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HID_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HID_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_HID_SOFProcess(USBH_HandleTypeDef *phost);
static void  USBH_HID_ParseHIDDesc (HID_DescTypeDef *desc, uint8_t *buf);

USBH_ClassTypeDef  HID_Class = 
{
  "HID",
  USB_HID_CLASS,
  USBH_HID_InterfaceInit,
  USBH_HID_InterfaceDeInit,
  USBH_HID_ClassRequest,
  USBH_HID_Process,
  USBH_HID_SOFProcess,
  NULL,
};

uint64_t DetectButtonDS3(uint64_t buttons_and_triggers)
{
	// bit smearing so all bits to the right of the first 1 are also 1
	buttons_and_triggers |= buttons_and_triggers >> 32;
	buttons_and_triggers |= buttons_and_triggers >> 16;
	buttons_and_triggers |= buttons_and_triggers >> 8;
	buttons_and_triggers |= buttons_and_triggers >> 4;
	buttons_and_triggers |= buttons_and_triggers >> 2;
	buttons_and_triggers |= buttons_and_triggers >> 1;

	// only leave the highest 1 set
	buttons_and_triggers ^= buttons_and_triggers >> 1;

	// now it is the same as the bitmask we want to return
	return buttons_and_triggers;
}


/**
  * @brief  USBH_HID_InterfaceInit 
  *         The function init the HID class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_InterfaceInit (USBH_HandleTypeDef *phost)
{	
  uint8_t max_ep;
  uint8_t num = 0;
  uint8_t interface;
  
  USBH_StatusTypeDef status = USBH_FAIL ;
  HID_HandleTypeDef *HID_Handle;
  
  interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode, HID_BOOT_CODE, 0xFF);

  if (interface == 0xFF) // did not find KB or mouse
	  interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode, 0, 0xFF); // try looking for ds3 and ds4
  
  if(interface == 0xFF) /* No Valid Interface */
  {
    status = USBH_FAIL;  
    USBH_DbgLog ("Cannot Find the interface for %s class.", phost->pActiveClass->Name);         
  }
  else // something was found!
  {
    USBH_SelectInterface (phost, interface);
    phost->pActiveClass->pData = (HID_HandleTypeDef *)USBH_malloc (sizeof(HID_HandleTypeDef));
    HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData; 
    HID_Handle->state = HID_ERROR;
    
    /*Decode Bootclass Protocol: Mouse or Keyboard*/
    if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bInterfaceProtocol == HID_KEYBRD_BOOT_CODE)
    {
      USBH_UsrLog ("KeyBoard device found!"); 
      HID_Handle->Init =  USBH_HID_KeybdInit;
      type = CONTROLLER_KB;
    }
    else if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bInterfaceProtocol == HID_MOUSE_BOOT_CODE)
    {
      USBH_UsrLog ("Mouse device found!");         
      HID_Handle->Init =  USBH_HID_MouseInit;     
    }
    else if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bInterfaceProtocol == HID_DS3_BOOT_CODE)
	{
    	if(phost->device.DevDesc.idVendor == 0x054C && phost->device.DevDesc.idProduct == 0x0268)
    	{
			USBH_UsrLog ("DS3 device found!");
			HID_Handle->Init =  USBH_HID_DS3Init;
			type = CONTROLLER_DS3;
    	}
    	else if(phost->device.DevDesc.idVendor == 0x054C && \
    			(phost->device.DevDesc.idProduct == 0x05C4/*regular*/|| phost->device.DevDesc.idProduct == 0x09CC/*slim*/))
		{
    		USBH_UsrLog ("DS4 device found!");
    		HID_Handle->Init =  USBH_HID_DS4Init;
    		type = CONTROLLER_DS4;
		}
	}
    else
    {
      USBH_UsrLog ("Protocol not supported.");  
      return USBH_FAIL;
    }
    
    HID_Handle->state     = HID_INIT;
    HID_Handle->ctl_state = HID_REQ_INIT; 
    HID_Handle->ep_addr   = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress;
    HID_Handle->length    = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
    HID_Handle->poll      = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bInterval ;
    
    if (HID_Handle->poll  < HID_MIN_POLL)
    {
      HID_Handle->poll = HID_MIN_POLL;
    }
    
    /* Check for available number of endpoints */
    /* Find the number of EPs in the Interface Descriptor */      
    /* Choose the lower number in order not to overrun the buffer allocated */
    max_ep = ( (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bNumEndpoints <= USBH_MAX_NUM_ENDPOINTS) ? 
              phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bNumEndpoints :
                  USBH_MAX_NUM_ENDPOINTS); 
    
    
    /* Decode endpoint IN and OUT address from interface descriptor */
    for ( ;num < max_ep; num++)
    {
      if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress & 0x80)
      {
        HID_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress);
        HID_Handle->InPipe  =\
          USBH_AllocPipe(phost, HID_Handle->InEp);
        
        /* Open pipe for IN endpoint */
        USBH_OpenPipe  (phost,
                        HID_Handle->InPipe,
                        HID_Handle->InEp,
                        phost->device.address,
                        phost->device.speed,
                        USB_EP_TYPE_INTR,
                        HID_Handle->length); 
        
        USBH_LL_SetToggle (phost, HID_Handle->InPipe, 0);
        
      }
      else
      {
        HID_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress);
        HID_Handle->OutPipe  =\
          USBH_AllocPipe(phost, HID_Handle->OutEp);
        
        /* Open pipe for OUT endpoint */
        USBH_OpenPipe  (phost,
                        HID_Handle->OutPipe,
                        HID_Handle->OutEp,                            
                        phost->device.address,
                        phost->device.speed,
                        USB_EP_TYPE_INTR,
                        HID_Handle->length); 
        
        USBH_LL_SetToggle (phost, HID_Handle->OutPipe, 0);        
      }
      
    }  
    status = USBH_OK;
  }
  return status;
}

/**
  * @brief  USBH_HID_InterfaceDeInit 
  *         The function DeInit the Pipes used for the HID class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_InterfaceDeInit (USBH_HandleTypeDef *phost )
{	
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData; 
  
  if(HID_Handle->InPipe != 0x00)
  {   
    USBH_ClosePipe  (phost, HID_Handle->InPipe);
    USBH_FreePipe  (phost, HID_Handle->InPipe);
    HID_Handle->InPipe = 0;     /* Reset the pipe as Free */  
  }
  
  if(HID_Handle->OutPipe != 0x00)
  {   
    USBH_ClosePipe(phost, HID_Handle->OutPipe);
    USBH_FreePipe  (phost, HID_Handle->OutPipe);
    HID_Handle->OutPipe = 0;     /* Reset the pipe as Free */  
  }
  
  if(phost->pActiveClass->pData)
  {
    USBH_free (phost->pActiveClass->pData);
  }

  type = CONTROLLER_NONE;

  return USBH_OK;
}

/**
  * @brief  USBH_HID_ClassRequest 
  *         The function is responsible for handling Standard requests
  *         for HID class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_ClassRequest(USBH_HandleTypeDef *phost)
{   
  uint8_t enable[4] = {0x42, 0x0C, 0x00, 0x00};
  USBH_StatusTypeDef status         = USBH_BUSY;
  USBH_StatusTypeDef classReqStatus = USBH_BUSY;
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;

  /* Switch HID state machine */
  switch (HID_Handle->ctl_state)
  {
  case HID_REQ_INIT:  
  case HID_REQ_GET_HID_DESC:

    /* Get HID Desc */
	if(type == CONTROLLER_DS4)
	{
		uint8_t buffer[9] = {0x09,        // bLength
				0x21,        // bDescriptorType (HID)
				0x11, 0x01,  // bcdHID 1.17
				0x00,        // bCountryCode
				0x01,        // bNumDescriptors
				0x22,        // bDescriptorType[0] (HID)
				0xF3, 0x01,  // wDescriptorLength[0] 499
				//0xD3, 0x01,  // wDescriptorLength[0] 467
				};

		USBH_HID_ParseHIDDesc(&HID_Handle->HID_Desc, buffer);
		HID_Handle->ctl_state = HID_REQ_SET_IDLE;
	}
	else if (USBH_HID_GetHIDDescriptor (phost, USB_HID_DESC_SIZE) == USBH_OK) // try this function call earlier?
    {
      
      USBH_HID_ParseHIDDesc(&HID_Handle->HID_Desc, phost->device.Data);
      HID_Handle->ctl_state = HID_REQ_GET_REPORT_DESC;
    }
    
    break;     
  case HID_REQ_GET_REPORT_DESC:
    
    
    /* Get Report Desc */ 
    if (USBH_HID_GetHIDReportDescriptor(phost, HID_Handle->HID_Desc.wItemLength) == USBH_OK)
    {
      /* The descriptor is available in phost->device.Data */
    	if(type == CONTROLLER_DS3)
    	{
    		HID_Handle->ctl_state = HID_PS3_BOOTCODE;
    	}
    	else if(type == CONTROLLER_DS4)
		{
			HID_Handle->ctl_state = HID_PS4_LED;
		}
    	else HID_Handle->ctl_state = HID_REQ_SET_IDLE;
    }
    
    break;
    
  case HID_PS3_BOOTCODE:
	  if(USBH_HID_SetReport(phost,0x03,0xF4,enable,4) == USBH_OK)  // enable ps3 communication
	  {
		  HID_Handle->ctl_state = HID_PS3_LED;
	  }
	  break;
  case HID_PS3_LED:
  	  if(USBH_HID_SetReport(phost,0x02,0x01,led_buffer,48) == USBH_OK)  // turn on p1 LED
  	  {
			HID_Handle->ctl_state = HID_REQ_IDLE; // move on to normal input processing
			phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
			status = USBH_OK;
  	  }
  	  break;
  case HID_PS4_LED:
	  // ps4 led info. write with HID report id 0x05 (HID set report?)
	  if(USBH_HID_SetReport(phost,0x02,0x05,ps4_led_buffer,31) == USBH_OK)  // turn on p1 LED
	  {
		  HID_Handle->ctl_state = HID_REQ_IDLE;
		/* all requests performed*/
		phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
		status = USBH_OK;
	  }
	  break;

  case HID_REQ_SET_IDLE:
    
    classReqStatus = USBH_HID_SetIdle (phost, 0, 0);
    
    /* set Idle */
    if (classReqStatus == USBH_OK)
    {
    	if(type == CONTROLLER_DS4)
    	{
    		 HID_Handle->ctl_state = HID_REQ_GET_REPORT_DESC;
    	}
    	else
		{
    		HID_Handle->ctl_state = HID_REQ_SET_PROTOCOL;
		}
    }
    else if(classReqStatus == USBH_NOT_SUPPORTED) 
    {
      HID_Handle->ctl_state = HID_REQ_SET_PROTOCOL; // Why if it's not supported would we set it to this state?!
    } 
    break; 
    
  case HID_REQ_SET_PROTOCOL:
    /* set protocol */
    if (USBH_HID_SetProtocol (phost, 0) == USBH_OK)
    {
      HID_Handle->ctl_state = HID_REQ_IDLE;
      
      /* all requests performed*/
      phost->pUser(phost, HOST_USER_CLASS_ACTIVE); 
      status = USBH_OK; 
    } 
    break;
    
  case HID_REQ_IDLE:
  default:
    break;
  }
  
  return status; 
}

/**
  * @brief  USBH_HID_Process 
  *         The function is for managing state machine for HID data transfers 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_Process(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_OK, getreport_result;
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;
  
  switch (HID_Handle->state)
  {
  case HID_INIT:
    HID_Handle->Init(phost); 
  case HID_IDLE:
	getreport_result = USBH_HID_GetReport (phost,
	  	                             0x01,
	  	                              0,
	  	                              HID_Handle->pData,
	  	                              HID_Handle->length);
	if(getreport_result == USBH_OK || getreport_result == USBH_NOT_SUPPORTED)
    {
      
      fifo_write(&HID_Handle->fifo, HID_Handle->pData, HID_Handle->length);  
      HID_Handle->state = HID_SYNC;
    }
    
    break;
    
  case HID_SYNC:

    /* Sync with start of Even Frame */
    if(phost->Timer & 1)
    {
      HID_Handle->state = HID_GET_DATA; 
    }
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_URB_EVENT, 0);
#endif   
    break;
    
  case HID_GET_DATA:

    USBH_InterruptReceiveData(phost, 
                              HID_Handle->pData,
                              HID_Handle->length,
                              HID_Handle->InPipe);
    
    HID_Handle->state = HID_POLL;
    HID_Handle->timer = phost->Timer;
    HID_Handle->DataReady = 0;
    break;
    
  case HID_POLL:
    
    if(USBH_LL_GetURBState(phost , HID_Handle->InPipe) == USBH_URB_DONE)
    {
      if(HID_Handle->DataReady == 0)
      {
        fifo_write(&HID_Handle->fifo, HID_Handle->pData, HID_Handle->length);
        HID_Handle->DataReady = 1;
        USBH_HID_EventCallback(phost);
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_URB_EVENT, 0);
#endif          
      }
    }
    else if(USBH_LL_GetURBState(phost , HID_Handle->InPipe) == USBH_URB_STALL) /* IN Endpoint Stalled */
    {
      
      /* Issue Clear Feature on interrupt IN endpoint */ 
      if(USBH_ClrFeature(phost,
                         HID_Handle->ep_addr) == USBH_OK)
      {
        /* Change state to issue next IN token */
        HID_Handle->state = HID_GET_DATA;
      }
    } 
    

    break;
    
  default:
    break;
  }
  return status;
}

/**
  * @brief  USBH_HID_SOFProcess 
  *         The function is for managing the SOF Process 
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_SOFProcess(USBH_HandleTypeDef *phost)
{
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;
  
  if(HID_Handle->state == HID_POLL)
  {
    if(( phost->Timer - HID_Handle->timer) >= HID_Handle->poll)
    {
      HID_Handle->state = HID_GET_DATA;
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_URB_EVENT, 0);
#endif       
    }
  }
  return USBH_OK;
}

/**
* @brief  USBH_Get_HID_ReportDescriptor
  *         Issue report Descriptor command to the device. Once the response 
  *         received, parse the report descriptor and update the status.
  * @param  phost: Host handle
  * @param  Length : HID Report Descriptor Length
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_GetHIDReportDescriptor (USBH_HandleTypeDef *phost,
                                                         uint16_t length)
{
  
  USBH_StatusTypeDef status;
  
  status = USBH_GetDescriptor(phost,
                              USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD,                                  
                              USB_DESC_HID_REPORT, 
                              phost->device.Data,
                              length);
  
  /* HID report descriptor is available in phost->device.Data.
  In case of USB Boot Mode devices for In report handling ,
  HID report descriptor parsing is not required.
  In case, for supporting Non-Boot Protocol devices and output reports,
  user may parse the report descriptor*/
  
  
  return status;
}

/**
  * @brief  USBH_Get_HID_Descriptor
  *         Issue HID Descriptor command to the device. Once the response 
  *         received, parse the report descriptor and update the status.
  * @param  phost: Host handle
  * @param  Length : HID Descriptor Length
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_GetHIDDescriptor (USBH_HandleTypeDef *phost,
                                            uint16_t length)
{
  
  USBH_StatusTypeDef status;
  
  status = USBH_GetDescriptor( phost,
                              USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_STANDARD,                                  
                              USB_DESC_HID,
                              phost->device.Data,
                              length);
 
  return status;
}

/**
  * @brief  USBH_Set_Idle
  *         Set Idle State. 
  * @param  phost: Host handle
  * @param  duration: Duration for HID Idle request
  * @param  reportId : Targeted report ID for Set Idle request
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_SetIdle (USBH_HandleTypeDef *phost,
                                         uint8_t duration,
                                         uint8_t reportId)
{
  
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE |\
    USB_REQ_TYPE_CLASS;
  
  
  phost->Control.setup.b.bRequest = USB_HID_SET_IDLE;
  phost->Control.setup.b.wValue.w = (duration << 8 ) | reportId;
  
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = 0;
  
  return USBH_CtlReq(phost, 0 , 0 );
}


/**
  * @brief  USBH_HID_Set_Report
  *         Issues Set Report 
  * @param  phost: Host handle
  * @param  reportType  : Report type to be sent
  * @param  reportId    : Targeted report ID for Set Report request
  * @param  reportBuff  : Report Buffer
  * @param  reportLen   : Length of data report to be send
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_SetReport (USBH_HandleTypeDef *phost,
                                    uint8_t reportType,
                                    uint8_t reportId,
                                    uint8_t* reportBuff,
                                    uint8_t reportLen)
{
  
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE |\
    USB_REQ_TYPE_CLASS;
  
  
  phost->Control.setup.b.bRequest = USB_HID_SET_REPORT;
  phost->Control.setup.b.wValue.w = (reportType << 8 ) | reportId;
  
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = reportLen;
  
  return USBH_CtlReq(phost, reportBuff , reportLen );
}


/**
  * @brief  USBH_HID_GetReport
  *         retreive Set Report 
  * @param  phost: Host handle
  * @param  reportType  : Report type to be sent
  * @param  reportId    : Targeted report ID for Set Report request
  * @param  reportBuff  : Report Buffer
  * @param  reportLen   : Length of data report to be send
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_GetReport (USBH_HandleTypeDef *phost,
                                    uint8_t reportType,
                                    uint8_t reportId,
                                    uint8_t* reportBuff,
                                    uint8_t reportLen)
{
  
  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE |\
    USB_REQ_TYPE_CLASS;
  
  
  phost->Control.setup.b.bRequest = USB_HID_GET_REPORT;
  phost->Control.setup.b.wValue.w = (reportType << 8 ) | reportId;
  
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = reportLen;
  
  return USBH_CtlReq(phost, reportBuff , reportLen );
}

/**
  * @brief  USBH_Set_Protocol
  *         Set protocol State.
  * @param  phost: Host handle
  * @param  protocol : Set Protocol for HID : boot/report protocol
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_SetProtocol(USBH_HandleTypeDef *phost,
                                            uint8_t protocol)
{
  
  
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE |\
    USB_REQ_TYPE_CLASS;
  
  
  phost->Control.setup.b.bRequest = USB_HID_SET_PROTOCOL;
  phost->Control.setup.b.wValue.w = protocol != 0 ? 0 : 1;
  phost->Control.setup.b.wIndex.w = 0;
  phost->Control.setup.b.wLength.w = 0;
  
  return USBH_CtlReq(phost, 0 , 0 );
  
}

/**
  * @brief  USBH_ParseHIDDesc 
  *         This function Parse the HID descriptor
  * @param  desc: HID Descriptor
  * @param  buf: Buffer where the source descriptor is available
  * @retval None
  */
static void  USBH_HID_ParseHIDDesc (HID_DescTypeDef *desc, uint8_t *buf)
{
  
  desc->bLength                  = *(uint8_t  *) (buf + 0);
  desc->bDescriptorType          = *(uint8_t  *) (buf + 1);
  desc->bcdHID                   =  LE16  (buf + 2);
  desc->bCountryCode             = *(uint8_t  *) (buf + 4);
  desc->bNumDescriptors          = *(uint8_t  *) (buf + 5);
  desc->bReportDescriptorType    = *(uint8_t  *) (buf + 6);
  desc->wItemLength              =  LE16  (buf + 7);
} 

/**
  * @brief  USBH_HID_GetDeviceType
  *         Return Device function.
  * @param  phost: Host handle
  * @retval HID function: HID_MOUSE / HID_KEYBOARD
  */
HID_TypeTypeDef USBH_HID_GetDeviceType(USBH_HandleTypeDef *phost)
{
  
  if(phost->gState == HOST_CLASS)
  {
    
    if(type == CONTROLLER_KB)
    {
      return HID_KEYBOARD;
    }
    else if(type == CONTROLLER_DS3)
	{
		return HID_DS3;
	}
    else if(type == CONTROLLER_DS4)
	{
		return HID_DS4;
	}
    else if(phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].bInterfaceProtocol \
          == HID_MOUSE_BOOT_CODE)
	{
	  return HID_MOUSE;
	}
  }
  return HID_UNKNOWN;
}


/**
  * @brief  USBH_HID_GetPollInterval
  *         Return HID device poll time
  * @param  phost: Host handle
  * @retval poll time (ms)
  */
uint8_t USBH_HID_GetPollInterval(USBH_HandleTypeDef *phost)
{
  HID_HandleTypeDef *HID_Handle =  (HID_HandleTypeDef *) phost->pActiveClass->pData;
    
    if((phost->gState == HOST_CLASS_REQUEST) ||
       (phost->gState == HOST_INPUT) ||
         (phost->gState == HOST_SET_CONFIGURATION) ||
           (phost->gState == HOST_CHECK_CLASS) ||           
             ((phost->gState == HOST_CLASS)))
  {
    return (HID_Handle->poll);
  }
  else
  {
    return 0;
  }
}
/**
  * @brief  fifo_init
  *         Initialize FIFO.
  * @param  f: Fifo address
  * @param  buf: Fifo buffer
  * @param  size: Fifo Size
  * @retval none
  */
void fifo_init(FIFO_TypeDef * f, uint8_t * buf, uint16_t size)
{
     f->head = 0;
     f->tail = 0;
     f->lock = 0;
     f->size = size;
     f->buf = buf;
}

/**
  * @brief  fifo_read
  *         Read from FIFO.
  * @param  f: Fifo address
  * @param  buf: read buffer 
  * @param  nbytes: number of item to read
  * @retval number of read items
  */
uint16_t  fifo_read(FIFO_TypeDef * f, void * buf, uint16_t  nbytes)
{
  uint16_t  i;
  uint8_t * p;
  p = (uint8_t*) buf;
  
  if(f->lock == 0)
  {
    f->lock = 1;
    for(i=0; i < nbytes; i++)
    {
      if( f->tail != f->head )
      { 
        *p++ = f->buf[f->tail];  
        f->tail++;  
        if( f->tail == f->size )
        {  
          f->tail = 0;
        }
      } else 
      {
        f->lock = 0;
        return i; 
      }
    }
  }
  f->lock = 0;
  return nbytes;
}
 
/**
  * @brief  fifo_write
  *         Read from FIFO.
  * @param  f: Fifo address
  * @param  buf: read buffer 
  * @param  nbytes: number of item to write
  * @retval number of written items
  */
uint16_t  fifo_write(FIFO_TypeDef * f, const void * buf, uint16_t  nbytes)
{
  uint16_t  i;
  const uint8_t * p;
  p = (const uint8_t*) buf;
  if(f->lock == 0)
  {
    f->lock = 1;
    for(i=0; i < nbytes; i++)
    {
      if( (f->head + 1 == f->tail) ||
         ( (f->head + 1 == f->size) && (f->tail == 0)) )
      {
        f->lock = 0;
        return i;
      } 
      else 
      {
        f->buf[f->head] = *p++;
        f->head++;
        if( f->head == f->size )
        {
          f->head = 0;
        }
      }
    }
  }
  f->lock = 0;
  return nbytes;
}


/**
* @brief  The function is a callback about HID Data events
*  @param  phost: Selected device
* @retval None
*/

__weak void USBH_HID_EventCallback(USBH_HandleTypeDef *phost)
{
	HID_TypeTypeDef type = HID_UNKNOWN;
	HID_KEYBD_Info_TypeDef* kb_state = NULL;
	HID_DS3_Info_TypeDef* ds3_state = NULL;
	HID_DS4_Info_TypeDef* ds4_state = NULL;
	N64ControllerData new_data;
	uint64_t buttons_and_triggers;

	type = USBH_HID_GetDeviceType(phost);

	switch(type)
	{
		case HID_KEYBOARD:
			kb_state = USBH_HID_GetKeybdInfo(phost);

			// the buttons all become 1 if overflow, i think.  or in short, [2] == [3] == 1
			if(kb_state->keys[2] == kb_state->keys[3] && kb_state->keys[2] == 1)
				return;

			if(state == NORMAL) //used to check state variable for changing controls
			{
				memset(&new_data,0,4);

				for(int index = 0;index < 6;index++)
				{
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_a_up)
					{
						new_data.y_axis = 0xFE; // -128 bit reversed (100% range)
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_a_down)
					{
						new_data.y_axis = 0x01; // +127 bit reversed (100% range)
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_a_left)
					{
						new_data.x_axis = 0x01; // +127 bit reversed (100% range)
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_a_right)
					{
						new_data.x_axis = 0xFE; // -128 bit reversed (100% range)
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_d_up)
					{
						new_data.up = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_d_down)
					{
						new_data.down = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_d_left)
					{
						new_data.left = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_d_right)
					{
						new_data.right = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_a)
					{
						new_data.a = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_b)
					{
						new_data.b = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_l)
					{
						new_data.l = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_r)
					{
						new_data.r = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_z)
					{
						new_data.z = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_start)
					{
						new_data.start = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_c_up)
					{
						new_data.c_up = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_c_down)
					{
						new_data.c_down = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_c_left)
					{
						new_data.c_left = 1;
						continue;
					}
					if(kb_state->keys[index] == controls.KBControls.KEYBOARD_c_right)
					{
						new_data.c_right = 1;
						continue;
					}
				}

				// atomic update of n64 state
				__disable_irq();
				memcpy(&n64_data, &new_data,4);
				__enable_irq();
			}
			else
			{
				uint8_t b = kb_state->keys[0]; // read for button presses (just take first pressed if many are pressed)
				if(b != 0) /*button was actually is pressed*/
				{
					if(keyboardButtonPressed == 0)
					{
						keyboardButtonPressed = 1;
						ChangeButtonMappingKB(b);
						AdvanceState();
					}
				}
				else
				{
					keyboardButtonPressed = 0;
				}
			}
			break;
		case HID_DS3:
			ds3_state = USBH_HID_GetDS3Info(phost);
			buttons_and_triggers = USBH_HID_GetDS3ButtonsAndTriggers();

			if(state == NORMAL)
			{
				memset(&new_data,0,4);

				if(buttons_and_triggers & controls.XpadControls.up)
				{
					new_data.up = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.down)
				{
					new_data.down = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.left)
				{
					new_data.left = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.right)
				{
					new_data.right = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_up)
				{
					new_data.c_up = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_down)
				{
					new_data.c_down = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_left)
				{
					new_data.c_left = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_right)
				{
					new_data.c_right = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.l)
				{
					new_data.l = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.r)
				{
					new_data.r = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.z)
				{
					new_data.z = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.a)
				{
					new_data.a = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.b)
				{
					new_data.b = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.start)
				{
					new_data.start = 1;
				}

				// ----- begin nrage replication analog code -----
				const float N64_MAX = 127*(controls.XpadControls.range/100.0f);
				float deadzoneValue = (controls.XpadControls.deadzone/100.0f) * DS3_MAX;
				float deadzoneRelation = DS3_MAX / (DS3_MAX - deadzoneValue);

				int8_t LSX = 0, LSY = 0; // -128 to +127...
				float unscaled_result = 0;
				int8_t stick_lx = ds3_state->LAnalogX - 128;
				int8_t stick_ly = ds3_state->LAnalogY - 128;

				if(stick_lx >= deadzoneValue) // positive = right
				{
					unscaled_result = (stick_lx - deadzoneValue) * deadzoneRelation;
					LSX = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
				}
				else if(stick_lx <= (-deadzoneValue)) // negative = left
				{
					stick_lx++; // just in case it's -128 it cannot be negated. otherwise the 1 is negligible.
					stick_lx = -stick_lx; // compute as positive, then negate at the end
					unscaled_result = (stick_lx - deadzoneValue) * deadzoneRelation;
					LSX = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
					LSX = -LSX;
				}

				if(stick_ly >= deadzoneValue) // DS3 positive = down
				{
					unscaled_result = (stick_ly - deadzoneValue) * deadzoneRelation;
					LSY = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
					LSY = -LSY; // for n64 down is negative
				}
				else if(stick_ly <= (-deadzoneValue)) // DS3 negative = up
				{
					stick_ly++; // just in case it's -128 it cannot be negated. otherwise the 1 is negligible.
					stick_ly = -stick_ly; // compute as positive
					unscaled_result = (stick_ly - deadzoneValue) * deadzoneRelation;
					LSY = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
				}
				new_data.x_axis = reverse((uint8_t)LSX);
				new_data.y_axis = reverse((uint8_t)LSY);
				// end of analog code

				// atomic update of n64 state
				__disable_irq();
				memcpy(&n64_data, &new_data,4);
				__enable_irq();
			}
			else if(state == STATE_SENSITIVITY)
			{
				uint64_t b = DetectButtonDS3(buttons_and_triggers); // read for button presses (just do linear search)
				if(b == XPAD_HAT_UP) // +5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range < 95 ? controls.XpadControls.range+5 : 100;
					}
				}
				else if(b == XPAD_HAT_DOWN) // -5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range > 5 ? controls.XpadControls.range-5 : 0;
					}
				}
				else if(b == XPAD_HAT_LEFT) // -1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range > 1 ? controls.XpadControls.range-1 : 0;
					}
								}
				else if(b == XPAD_HAT_RIGHT)// +1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range < 99 ? controls.XpadControls.range+1 : 100;
					}
				}
				else if(b == XPAD_PAD_A) // OK
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						AdvanceState();
					}
				}
				else
				{
					ds3ButtonPressed = 0;
				}
			}
			else if(state == STATE_DEADZONE)
			{
				uint64_t b = DetectButtonDS3(buttons_and_triggers); // read for button presses (just do linear search)
				if(b == XPAD_HAT_UP) // +5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone < 95 ? controls.XpadControls.deadzone+5 : 100;
					}
				}
				else if(b == XPAD_HAT_DOWN) // -5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone > 5 ? controls.XpadControls.deadzone-5 : 0;
					}
				}
				else if(b == XPAD_HAT_LEFT) // -1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone > 1 ? controls.XpadControls.deadzone-1 : 0;
					}
								}
				else if(b == XPAD_HAT_RIGHT)// +1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone < 99 ? controls.XpadControls.deadzone+1 : 100;
					}
				}
				else if(b == XPAD_PAD_A) // OK
				{
					if(ds3ButtonPressed == 0)
					{
						//keep ds3ButtonPressed at 0 for next time since we're done
						AdvanceState();
					}
				}
				else
				{
					ds3ButtonPressed = 0;
				}
			}
			else
			{
				uint64_t b = DetectButtonDS3(buttons_and_triggers); // read for button presses (just do linear search)
				if(b != 0) /*button was actually is pressed*/
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						ChangeButtonMappingController(b);
						AdvanceState();
					}
				}
				else
				{
					ds3ButtonPressed = 0;
				}
			}
			break;
		case HID_DS4:
			ds4_state = USBH_HID_GetDS4Info(phost);

			buttons_and_triggers = USBH_HID_GetDS4ButtonsAndTriggers();

			if(state == NORMAL)
			{
				memset(&new_data,0,4);

				if(buttons_and_triggers & controls.XpadControls.up)
				{
					new_data.up = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.down)
				{
					new_data.down = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.left)
				{
					new_data.left = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.right)
				{
					new_data.right = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_up)
				{
					new_data.c_up = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_down)
				{
					new_data.c_down = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_left)
				{
					new_data.c_left = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.c_right)
				{
					new_data.c_right = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.l)
				{
					new_data.l = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.r)
				{
					new_data.r = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.z)
				{
					new_data.z = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.a)
				{
					new_data.a = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.b)
				{
					new_data.b = 1;
				}
				if(buttons_and_triggers & controls.XpadControls.start)
				{
					new_data.start = 1;
				}

				// ----- begin nrage replication analog code -----
				const float N64_MAX = 127*(controls.XpadControls.range/100.0f);
				float deadzoneValue = (controls.XpadControls.deadzone/100.0f) * DS3_MAX;
				float deadzoneRelation = DS3_MAX / (DS3_MAX - deadzoneValue);

				int8_t LSX = 0, LSY = 0; // -128 to +127...
				float unscaled_result = 0;

				// conversion from [0,255] to [-128,+127]
				int8_t stick_lx = ds4_state->LAnalogX - 128;
				int8_t stick_ly = ds4_state->LAnalogY - 128;

				if(stick_lx >= deadzoneValue) // positive = right
				{
					unscaled_result = (stick_lx - deadzoneValue) * deadzoneRelation;
					LSX = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
				}
				else if(stick_lx <= (-deadzoneValue)) // negative = left
				{
					stick_lx++; // just in case it's -128 it cannot be negated. otherwise the change of 1 is negligible.
					stick_lx = -stick_lx; // compute as positive, then negate at the end
					unscaled_result = (stick_lx - deadzoneValue) * deadzoneRelation;
					LSX = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
					LSX = -LSX;
				}

				if(stick_ly >= deadzoneValue) // DS3 positive = down
				{
					unscaled_result = (stick_ly - deadzoneValue) * deadzoneRelation;
					LSY = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
					LSY = -LSY; // for n64 down is negative
				}
				else if(stick_ly <= (-deadzoneValue)) // DS3 negative = up
				{
					stick_ly++; // just in case it's -128 it cannot be negated. otherwise the change of 1 is negligible.
					stick_ly = -stick_ly; // compute as positive
					unscaled_result = (stick_ly - deadzoneValue) * deadzoneRelation;
					LSY = (int8_t)(unscaled_result * (N64_MAX / DS3_MAX));
				}
				new_data.x_axis = reverse((uint8_t)LSX);
				new_data.y_axis = reverse((uint8_t)LSY);
				// end of analog code

				// atomic update of n64 state
				__disable_irq();
				memcpy(&n64_data, &new_data,4);
				__enable_irq();
			}
			else if(state == STATE_SENSITIVITY)
			{
				uint64_t b = DetectButtonDS3(buttons_and_triggers); // read for button presses (just do linear search)
				if(b == XPAD_HAT_UP) // +5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range < 95 ? controls.XpadControls.range+5 : 100;
					}
				}
				else if(b == XPAD_HAT_DOWN) // -5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range > 5 ? controls.XpadControls.range-5 : 0;
					}
				}
				else if(b == XPAD_HAT_LEFT) // -1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range > 1 ? controls.XpadControls.range-1 : 0;
					}
								}
				else if(b == XPAD_HAT_RIGHT)// +1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.range = controls.XpadControls.range < 99 ? controls.XpadControls.range+1 : 100;
					}
				}
				else if(b == XPAD_PAD_A) // OK
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						AdvanceState();
					}
				}
				else
				{
					ds3ButtonPressed = 0;
				}
			}
			else if(state == STATE_DEADZONE)
			{
				uint64_t b = DetectButtonDS3(buttons_and_triggers); // read for button presses (just do linear search)
				if(b == XPAD_HAT_UP) // +5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone < 95 ? controls.XpadControls.deadzone+5 : 100;
					}
				}
				else if(b == XPAD_HAT_DOWN) // -5
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone > 5 ? controls.XpadControls.deadzone-5 : 0;
					}
				}
				else if(b == XPAD_HAT_LEFT) // -1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone > 1 ? controls.XpadControls.deadzone-1 : 0;
					}
								}
				else if(b == XPAD_HAT_RIGHT)// +1
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						controls.XpadControls.deadzone = controls.XpadControls.deadzone < 99 ? controls.XpadControls.deadzone+1 : 100;
					}
				}
				else if(b == XPAD_PAD_A) // OK
				{
					if(ds3ButtonPressed == 0)
					{
						//keep ds3ButtonPressed at 0 for next time since we're done
						AdvanceState();
					}
				}
				else
				{
					ds3ButtonPressed = 0;
				}
			}
			else
			{
				uint64_t b = DetectButtonDS3(buttons_and_triggers); // read for button presses (just do linear search)
				if(b != 0) /*button was actually is pressed*/
				{
					if(ds3ButtonPressed == 0)
					{
						ds3ButtonPressed = 1;
						ChangeButtonMappingController(b);
						AdvanceState();
					}
				}
				else
				{
					ds3ButtonPressed = 0;
				}
			}
			break;
		default:
			break;
	}
}
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


/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
