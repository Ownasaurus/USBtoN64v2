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
#include "../Inc/usbh_xpad.h"

/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_XPAD_CLASS
* @{
*/

/** @defgroup USBH_XPAD_CORE
* @brief    This file includes XPAD Layer Handlers for USB Host XPAD class.
* @{
*/ 

/** @defgroup USBH_XPAD_CORE_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_XPAD_CORE_Private_Defines
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_XPAD_CORE_Private_Macros
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_XPAD_CORE_Private_Variables
* @{
*/
/**
* @}
*/ 


/** @defgroup USBH_XPAD_CORE_Private_FunctionPrototypes
* @{
*/ 

USBH_StatusTypeDef USBH_XPAD_InterfaceInit  (USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_XPAD_InterfaceDeInit  (USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_XPAD_Process(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_XPAD_ClassRequest (USBH_HandleTypeDef *phost);

USBH_StatusTypeDef USBH_XPAD_SOFProcess (USBH_HandleTypeDef *phost);


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
/**
* @}
*/ 


/** @defgroup USBH_XPAD_CORE_Private_Functions
* @{
*/ 

void XPAD_360_WIRELESS_ProcessInputData(USBH_HandleTypeDef *phost)
{
	__disable_irq();
	memset(&n64_data,0,4);
	__enable_irq();
}

void XPAD_360_WIRED_ProcessInputData(USBH_HandleTypeDef *phost)
{
	__disable_irq();
	memset(&n64_data,0,4);
	__enable_irq();
}

void XPAD_XBONE_ProcessInputData(USBH_HandleTypeDef *phost)
{
	__disable_irq();
	memset(&n64_data,0,4);
	__enable_irq();
}

USBH_StatusTypeDef USBH_XPAD_Init(USBH_HandleTypeDef *phost)
{
	// clear data structures out
	// send appropriate start packet depending on XPAD_TypeTypeDef
	return USBH_OK;
}

/**
  * @brief  USBH_XPAD_InterfaceInit
  *         The function init the XPAD class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_XPAD_InterfaceInit (USBH_HandleTypeDef *phost)
{
	uint8_t max_ep;
	uint8_t num = 0;
	uint8_t interface;

	XPAD_HandleTypeDef *XPAD_Handle;
	XPAD_TypeTypeDef ctype = XPAD_UNKNOWN;

	// class, subclass, protocol
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

	// open endpoints and stuff
	USBH_SelectInterface (phost, interface);
	phost->pActiveClass->pData = (XPAD_HandleTypeDef *)USBH_malloc (sizeof(XPAD_HandleTypeDef));
	XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;

	XPAD_Handle->Init = USBH_XPAD_Init;

	XPAD_Handle->ep_addr   = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress;
	XPAD_Handle->length    = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;
	XPAD_Handle->poll      = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bInterval ;
	XPAD_Handle->xpad_type = ctype;

	if (XPAD_Handle->poll  < 10)
	{
		XPAD_Handle->poll = 10;
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

  return USBH_OK;
}

/**
  * @brief  USBH_XPAD_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for XPAD class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_XPAD_ClassRequest (USBH_HandleTypeDef *phost)
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
USBH_StatusTypeDef USBH_XPAD_Process (USBH_HandleTypeDef *phost)
{
	XPAD_HandleTypeDef *XPAD_Handle =  (XPAD_HandleTypeDef *) phost->pActiveClass->pData;
	// should depend on state

	// if ready for polling

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
 
	return USBH_OK;
}


/**
  * @brief  USBH_XPAD_IOProcess
  *         XPAD XPAD process
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_XPAD_SOFProcess (USBH_HandleTypeDef *phost)
{
  
  return USBH_OK;
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
