/**
  ******************************************************************************
  * @file    usbh_mtp.c
  * @author  MCD Application Team
  * @version V3.2.2
  * @date    07-July-2015
  * @brief   This file is the MTP Layer Handlers for USB Host MTP class.
  *
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
#include "usbh_xpad.h"

uint8_t LSY=0, LSX=0, RSY=0, RSX=0, Lt=0, Rt=0;

/**
@namespace tN
@brief float for storing the trigger Normalising value
@brief makes the range of the triggers 0 to 10
*/
uint8_t serial = 0;
extern uint8_t state;
uint8_t xpadButtonPressed = 0;
extern Controls controls;
extern ControllerType type;

// bit order xbox: many 0s|rt|lt|y|x|b|a|?|?|RB|LB|RAB|LAB|back|start|dright|dleft|ddown|dup

const uint64_t   A_MASK      = 0x00001000,
				 B_MASK      = 0x00002000,
				 X_MASK      = 0x00004000,
				 Y_MASK      = 0x00008000,
				 LB_MASK     = 0x00000100,
				 RB_MASK     = 0x00000200,
				 START_MASK  = 0x00000010,
				 BACK_MASK   = 0x00000020,
				 LAB_MASK    = 0x00000040,
				 RAB_MASK    = 0x00000080,
				 DUP_MASK    = 0x00000001,
				 DDOWN_MASK  = 0x00000002,
				 DLEFT_MASK  = 0x00000004,
				 DRIGHT_MASK = 0x00000008,
				 // the next two masks are special extensions to support triggers
				 LT_MASK     = 0x00010000,
				 RT_MASK     = 0x00020000;

/** @defgroup USBH_XPAD_CORE_Private_FunctionPrototypes
* @{
*/ 
static USBH_StatusTypeDef USBH_XPAD_InterfaceInit  (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_XPAD_InterfaceDeInit  (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_XPAD_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_XPAD_ClassRequest (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_XPAD_SOFProcess (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_XPAD_Start(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_XPAD_Led(USBH_HandleTypeDef *phost,XPAD_LED cmd);

void ChangeButtonMappingController(uint64_t bt);
void AdvanceState();

USBH_ClassTypeDef  XPAD_Class =
{
"XPAD",
USB_XPAD_CLASS,
USBH_XPAD_InterfaceInit,
USBH_XPAD_InterfaceDeInit,
USBH_XPAD_ClassRequest,
USBH_XPAD_Process,
USBH_XPAD_SOFProcess,
NULL,
};

extern N64ControllerData n64_data;

/** @defgroup USBH_XPAD_CORE_Private_Functions
* @{
*/

uint64_t DetectButton()
{
    uint64_t buttons_and_triggers = buttons;

    if(Lt > TRIGGER_THRESHOLD)
	{
		buttons_and_triggers |= LT_MASK;
	}
	if(Rt > TRIGGER_THRESHOLD)
	{
		buttons_and_triggers |= RT_MASK;
	}

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

void parseMessage(USBH_HandleTypeDef *phost)
{
	XPAD_HandleTypeDef *XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;
    uint8_t *data = report;

    switch (XPAD_Handle->xpad_type) {
    case XPAD_ORIGINAL:
        buttons = ((uint32_t)report[3] << 8) | report[2];
        if (report[4]) buttons |= XPAD_PAD_A;
        if (report[5]) buttons |= XPAD_PAD_B;
        if (report[6]) buttons |= XPAD_PAD_X;
        if (report[7]) buttons |= XPAD_PAD_Y;
        trigger_l = report[10];
        trigger_r = report[11];

        stick_lx = ((int16_t)report[13] << 8) | report[12];
        stick_ly = ((int16_t)report[15] << 8) | report[14];
        stick_rx = ((int16_t)report[17] << 8) | report[16];
        stick_ry = ((int16_t)report[19] << 8) | report[18];
        break;
    case XPAD_360_WIRELESS:
        data += 4;
    case XPAD_360_WIRED:
        buttons = ((uint32_t)data[3] << 8) | data[2];
        trigger_l = data[4];
        trigger_r = data[5];

        stick_lx = ((int16_t)data[7] << 8) | data[6];
        stick_ly = ((int16_t)data[9] << 8) | data[8];
        stick_rx = ((int16_t)data[11] << 8) | data[10];
        stick_ry = ((int16_t)data[13] << 8) | data[12];
        break;
    case XPAD_XBONE:
        buttons = 0;
        buttons = (report[4] & 0x0C) << 2; // correctly place start and back
        buttons |= (report[4] & 0xF0) << 8; // correctly place AXYB
        buttons |= report[5] & 0x0F; // correctly place DPad
        buttons |= (report[5] & 0x30) << 4; // correctly place bumpers
        buttons |= (report[5] & 0xC0); // correctly analog stick buttons


        trigger_l = (((uint32_t)report[7] << 8) | report[6]) >> 2; // max is 1024 instead of 256, so >> 2 to divide by 4
		trigger_r = (((uint32_t)report[9] << 8) | report[8]) >> 2; // max is 1024 instead of 256, so >> 2 to divide by 4

        stick_lx = ((int16_t)data[11] << 8) | data[10];
        stick_ly = ((int16_t)data[13] << 8) | data[12];
        stick_rx = ((int16_t)data[15] << 8) | data[14];
        stick_ry = ((int16_t)data[17] << 8) | data[16];

        break;
    default:
        return;
    }

    // normalize the trigger values to be 10 max
    Lt=trigger_l*tN;
    Rt=trigger_r*tN;

    if(state == NORMAL) //used to check state variable for changing controls
    {
    	N64ControllerData new_data;
    	memset(&new_data,0,4); // clear controller state

    	uint64_t buttons_and_triggers = buttons;

    	if(Lt > TRIGGER_THRESHOLD)
    	{
    		buttons_and_triggers |= LT_MASK;
    	}
    	if(Rt > TRIGGER_THRESHOLD)
    	{
    		buttons_and_triggers |= RT_MASK;
    	}

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
		const float N64_MAX = (sensitivity > 0) ? 127*(sensitivity/100.0f) : 0;
    	float deadzoneValue = (dead_zone/100.0f) * XPAD_MAX;
    	float deadzoneRelation = XPAD_MAX / (XPAD_MAX - deadzoneValue);

    	LSX = LSY = 0; // -128 to +127...
    	float unscaled_result = 0;

    	if(stick_lx >= deadzoneValue) // positive = right
    	{
    		unscaled_result = (stick_lx - deadzoneValue) * deadzoneRelation;
    		LSX = (uint8_t)(unscaled_result * (N64_MAX / XPAD_MAX));
    	}
    	else if(stick_lx <= (-deadzoneValue)) // negative = left
    	{
    		stick_lx++; // just in case it's -32768 it cannot be negated. otherwise the 1 is negligible.
    		stick_lx = -stick_lx; // compute as positive, then negate at the end
    		unscaled_result = (stick_lx - deadzoneValue) * deadzoneRelation;
    		LSX = (uint8_t)(unscaled_result * (N64_MAX / XPAD_MAX));
    		LSX = -LSX;
    	}

    	if(stick_ly >= deadzoneValue) // positive = up
    	{
    		unscaled_result = (stick_ly - deadzoneValue) * deadzoneRelation;
    		LSY = (uint8_t)(unscaled_result * (N64_MAX / XPAD_MAX));
    	}
    	else if(stick_ly <= (-deadzoneValue)) // negative = down
    	{
    		stick_ly++; // just in case it's -32768 it cannot be negated. otherwise the 1 is negligible.
    		stick_ly = -stick_ly; // compute as positive, then negate at the end
    		unscaled_result = (stick_ly - deadzoneValue) * deadzoneRelation;
    		LSY = (uint8_t)(unscaled_result * (N64_MAX / XPAD_MAX));
    		LSY = -LSY;
    	}
    	new_data.x_axis = reverse(LSX);
    	new_data.y_axis = reverse(LSY);

    	// ----- end nrage replication analog code -----

    	__disable_irq();
		memcpy(&n64_data, &new_data,4);
		__enable_irq();
    }
    else // state > 0 so we are in the process of changing controls
    {
    	uint64_t b = DetectButton(); // read for button presses (just do linear search)
    	if(b != 0) /*button was actually is pressed*/
    	{
    		if(xpadButtonPressed == 0)
    		{
    			xpadButtonPressed = 1;
    			ChangeButtonMappingController(b);
    			AdvanceState();
    		}
    	}
    	else
    	{
    		xpadButtonPressed = 0;
    	}
    }
}

void XPAD_360_WIRELESS_ProcessInputData(USBH_HandleTypeDef *phost)
{
	if(report[0] == 0x00)
	{
		if (report[1] == 0x01 && report[2] == 0x00 && report[3] == 0xf0 && report[4] == 0x00 && report[5] == 0x13)
		{
			// Event data
			parseMessage(phost);
		}
		else if(report[1] == 0x0f)
		{
			USBH_XPAD_Led(phost, LED1_ON);
			USBH_XPAD_Start(phost);
		}
	}
	else if(report[0] == 0x08)
	{
		 if(report[1] == 0x80)
		 {
			 //USBH_XPAD_Led(phost, LED1_ON);
			 USBH_XPAD_Start(phost);
		 }
	}
}

void XPAD_360_WIRED_ProcessInputData(USBH_HandleTypeDef *phost)
{
	if(report[0] == 0x00)
	{
		if(report[1] == 0x14)
		{
			// Event data
			parseMessage(phost);
		}
	}
}

void XPAD_XBONE_ProcessInputData(USBH_HandleTypeDef *phost)
{
	if(report[0] == 0x02) // auth
	{
		if(report[1] == 0x20) // request
		{
			USBH_XPAD_Start(phost);
		}
	}
	else if(report[0] == 0x20)
	{
		// buttons update
		parseMessage(phost);
	}
}

static USBH_StatusTypeDef USBH_XPAD_Start(USBH_HandleTypeDef *phost)
{
	XPAD_HandleTypeDef *XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;
	uint8_t odata[32];
	memset(odata, 0, sizeof(odata));
	switch(XPAD_Handle->xpad_type)
	{
		case XPAD_360_WIRED:
		case XPAD_360_WIRELESS:
			odata[3] = 0x40;
			return USBH_InterruptSendData(phost,odata,12,XPAD_Handle->OutPipe);
			break;
		case XPAD_XBONE:
			memset(odata, 0, sizeof(odata));
			odata[0] = 0x05;
			odata[1] = 0x20;
			odata[2] = serial++;
			odata[3] = 0x09;
			odata[4] = 0x06;
			odata[11] = 0x55;
			odata[12] = 0x53;
			USBH_InterruptSendData(phost,odata,13,XPAD_Handle->OutPipe);

			memset(odata, 0, sizeof(odata));
			odata[0] = 0x05;
			odata[1] = 0x20;
			odata[2] = serial++;
			odata[3] = 0x01;
			odata[4] = 0x00;
			return USBH_InterruptSendData(phost,odata,5,XPAD_Handle->OutPipe);
			break;
		default:
			break;
	}
	return USBH_OK;
}

static USBH_StatusTypeDef USBH_XPAD_Led(USBH_HandleTypeDef *phost,XPAD_LED cmd)
{
	XPAD_HandleTypeDef *XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;
	uint8_t odata[32];
	memset(odata, 0, sizeof(odata));

	switch(XPAD_Handle->xpad_type)
	{
		case XPAD_360_WIRED:
			odata[0] = 0x01;
			odata[1] = 0x03;
			odata[2] = cmd;
			return USBH_InterruptSendData(phost,odata,3,XPAD_Handle->OutPipe);
			break;
		case XPAD_360_WIRELESS:
			odata[2] = 0x08;
			odata[3] = 0x40 + (cmd % 0x0e);
			return USBH_InterruptSendData(phost,odata,4,XPAD_Handle->OutPipe);
			break;
		case XPAD_ORIGINAL:
		case XPAD_XBONE:
		default:
			break;
	}
	return USBH_OK;
}

/**
  * @brief  USBH_XPAD_InterfaceInit
  *         The function init the XPAD class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_XPAD_InterfaceInit (USBH_HandleTypeDef *phost)
{
	uint8_t max_ep;
	uint8_t num = 0;
	uint8_t interface;

	XPAD_HandleTypeDef *XPAD_Handle;
	XPAD_TypeTypeDef ctype = XPAD_UNKNOWN;

	memset(report,0,sizeof(report));

	// phost, class, subclass, protocol
	interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode, 0x5D, 0x81); // X360 wireless

	if (interface == 0xFF)
	{
		interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode, 0x5D, 0x01); // X360 wired

		if (interface == 0xFF)
		{
			  interface = USBH_FindInterface(phost, phost->pActiveClass->ClassCode, 0x47, 0xD0); // XBONE wired

			  if(interface == 0xFF) /* No Valid Interface */
			  {
				USBH_DbgLog ("Cannot Find any valid interface for %s class.", phost->pActiveClass->Name);
				return USBH_FAIL;
			  }
			  else
			  {
				  ctype = XPAD_XBONE;
			  }
		}
		else
		{
			ctype = XPAD_360_WIRED;
		}
	}
	else
	{
		ctype = XPAD_360_WIRELESS;
	}
	type = CONTROLLER_XPAD;

	// open endpoints and stuff
	serial = 0;
	USBH_SelectInterface (phost, interface);
	phost->pActiveClass->pData = (XPAD_HandleTypeDef *)USBH_malloc (sizeof(XPAD_HandleTypeDef));
	XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;

	XPAD_Handle->state     = XPAD_INIT;
	XPAD_Handle->ep_addr   = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress;
	XPAD_Handle->length    = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
	XPAD_Handle->poll      = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bInterval;
	XPAD_Handle->xpad_type = ctype;

	/*if (XPAD_Handle->poll  < 10)
	{
		XPAD_Handle->poll = 10;
	}*/

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
		  XPAD_Handle->InEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress);
		  XPAD_Handle->InPipe  =\
		  USBH_AllocPipe(phost, XPAD_Handle->InEp);

		/* Open pipe for IN endpoint */
		USBH_OpenPipe  (phost,
						XPAD_Handle->InPipe,
						XPAD_Handle->InEp,
						phost->device.address,
						phost->device.speed,
						USB_EP_TYPE_INTR,
						XPAD_Handle->length);

		USBH_LL_SetToggle (phost, XPAD_Handle->InPipe, 0);

	  }
	  else
	  {
		  XPAD_Handle->OutEp = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[num].bEndpointAddress);
		  XPAD_Handle->OutPipe  =\
		  USBH_AllocPipe(phost, XPAD_Handle->OutEp);

		/* Open pipe for OUT endpoint */
		USBH_OpenPipe  (phost,
						XPAD_Handle->OutPipe,
						XPAD_Handle->OutEp,
						phost->device.address,
						phost->device.speed,
						USB_EP_TYPE_INTR,
						XPAD_Handle->length);

		USBH_LL_SetToggle (phost, XPAD_Handle->OutPipe, 0);
	  }

	}

	return USBH_OK;
}



/**
  * @brief  USBH_XPAD_InterfaceDeInit
  *         The function DeInit the Pipes used for the XPAD class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_XPAD_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
  XPAD_HandleTypeDef *XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;

  if(XPAD_Handle->InPipe != 0x00)
  {
	USBH_ClosePipe  (phost, XPAD_Handle->InPipe);
	USBH_FreePipe  (phost, XPAD_Handle->InPipe);
	XPAD_Handle->InPipe = 0;     /* Reset the pipe as Free */
  }

  if(XPAD_Handle->OutPipe != 0x00)
  {
	USBH_ClosePipe(phost, XPAD_Handle->OutPipe);
	USBH_FreePipe  (phost, XPAD_Handle->OutPipe);
	XPAD_Handle->OutPipe = 0;     /* Reset the pipe as Free */
  }

  if(phost->pActiveClass->pData)
  {
	USBH_free(phost->pActiveClass->pData);
  }

  serial = 0;
  type = CONTROLLER_NONE;

  return USBH_OK;
}

/**
  * @brief  USBH_XPAD_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for XPAD class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_XPAD_ClassRequest (USBH_HandleTypeDef *phost)
{
	// should be fine to keep this blank since there are no standard class requests for a custom class!
	return USBH_OK;
}

/**
  * @brief  USBH_XPAD_Process
  *         The function is for managing state machine for XPAD data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_XPAD_Process (USBH_HandleTypeDef *phost)
{
	XPAD_HandleTypeDef *XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;

	switch(XPAD_Handle->state)
	{
		case XPAD_INIT:
			USBH_InterruptReceiveData(phost,report,32,XPAD_Handle->InPipe);

			USBH_Delay(100);
			switch(XPAD_Handle->xpad_type)
			{
				case XPAD_360_WIRED:
					USBH_XPAD_Led(phost, LED1_ON);
					break;
				default:
					USBH_XPAD_Led(phost, LED_OFF);
					break;
			}
			XPAD_Handle->state = XPAD_IDLE;
			break;
		case XPAD_IDLE:
			USBH_InterruptReceiveData(phost,report,32,XPAD_Handle->InPipe);
			USBH_Delay(1);
			switch(XPAD_Handle->xpad_type)
			{
				case XPAD_360_WIRELESS:
					XPAD_360_WIRELESS_ProcessInputData(phost);
					break;
				case XPAD_360_WIRED:
					XPAD_360_WIRED_ProcessInputData(phost);
					break;
				case XPAD_XBONE:
					XPAD_XBONE_ProcessInputData(phost);
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
 
	return USBH_OK;
}

static USBH_StatusTypeDef USBH_XPAD_SOFProcess (USBH_HandleTypeDef *phost)
{
  
  return USBH_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
