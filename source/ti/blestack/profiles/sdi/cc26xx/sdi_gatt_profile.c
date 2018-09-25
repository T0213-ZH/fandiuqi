/******************************************************************************

 @file       simple_gatt_profile.c

 @brief This file contains the Simple GATT profile sample GATT service profile
        for use with the BLE sample application.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2010-2017, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 Release Name: simplelink_cc2640r2_sdk_01_50_00_58
 Release Date: 2017-10-17 18:09:51
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>
#include <icall.h>
#include "util.h"
/* This Header file contains all BLE API and icall structure definition */
#include "icall_ble_api.h"

#include "sdi_gatt_profile.h"

/************************************************************************/
#define SERVAPP_NUM_ATTR_SUPPORTED    (8)
#define OTA_SERV_NUM_ATTR_SUPPORTED   (4)

/********************* user uuid define *********************************/

CONST uint8 SDI_profile_user_server_uuid[ATT_UUID_SIZE] = SDI_PROFILE_SERV_UUID;
CONST uint8 SDI_profile_user_recv_uuid[ATT_UUID_SIZE] = SDI_PROFILE_RECV_UUID;
CONST uint8 SDI_profile_user_send_uuid[ATT_UUID_SIZE] = SDI_PROFILE_SEND_UUID;

static sdiProfileCBs_t *sdiProfile_AppCBs = NULL;

// Simple Profile Service attribute
static CONST gattAttrType_t sdiProfileService = { ATT_UUID_SIZE, SDI_profile_user_server_uuid};

static uint8 SDI_profile_recv_props = GATT_PROP_WRITE_NO_RSP;  // Simple Profile Characteristic 3 Properties
static uint8 SDI_profile_recv_buf[20] = {0};                   // Characteristic 3 Value
static uint8 SDI_profile_recv_desp[10] = "SDI->SEND";          // Simple Profile Characteristic 3 User Description

static uint8 SDI_profile_send_props = GATT_PROP_NOTIFY;       // Simple Profile Characteristic 4 Properties
static uint8 SDI_profile_send_buf[20] = {0};                  // Characteristic 4 Value
static gattCharCfg_t *SDI_profile_send_config; 
static uint8 SDI_profile_send_desp[10] = "SDI->RECV";         // Simple Profile Characteristic 4 User Description


//OTA
CONST uint8 SDI_profile_ota_server_uuid[ATT_BT_UUID_SIZE] = {LO_UINT16(SDI_PROFILE_OTA_SERV_UUID), HI_UINT16(SDI_PROFILE_OTA_SERV_UUID)};
CONST uint8 SDI_profile_ota_recv_uuid[ATT_BT_UUID_SIZE] = {LO_UINT16(SDI_PROFILE_OTA_RECV_UUID), HI_UINT16(SDI_PROFILE_OTA_RECV_UUID)};

//static sdiProfileCBs_t *SDI_OTAProfile_AppCBs = NULL;

// Simple Profile Service attribute
static CONST gattAttrType_t SDI_profile_ota_service = { ATT_BT_UUID_SIZE, SDI_profile_ota_server_uuid};

static uint8 SDI_profile_ota_recv_props = GATT_PROP_WRITE_NO_RSP;  // Simple Profile Characteristic 3 Properties
static uint8 SDI_profile_ota_recv_buf[20] = {0};                   // Characteristic 3 Value
static uint8 SDI_profile_ota_recv_desp[9] = "SDI->OTA";          // Simple Profile Characteristic 3 User Description

/*********************************************************************/

uint8 g_recv_buf[21];
uint8 g_ota_recv_buf[21];

static uint8 g_send_buf[20];
static uint8 g_send_len;

/*********************************************************************
 * Profile Attributes - Table
 */

static gattAttribute_t sdiProfileAttrTbl[SERVAPP_NUM_ATTR_SUPPORTED] =
{
  // Simple Profile Service
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&sdiProfileService               /* pValue */
  },
    // Characteristic 3 Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &SDI_profile_recv_props
    },

      // Characteristic Value 3
      {
        { ATT_UUID_SIZE, SDI_profile_user_recv_uuid },
        GATT_PERMIT_WRITE,
        0,
        SDI_profile_recv_buf
      },

      // Characteristic 3 User Description
      {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        SDI_profile_recv_desp
      },

    // Characteristic 4 Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &SDI_profile_send_props
    },

      // Characteristic Value 4
      {
        { ATT_UUID_SIZE, SDI_profile_user_send_uuid },
        0,
        0,
        SDI_profile_send_buf
      },

      // Characteristic 4 configuration
      {
        { ATT_BT_UUID_SIZE, clientCharCfgUUID },
        GATT_PERMIT_READ | GATT_PERMIT_WRITE,
        0,
        (uint8 *)&SDI_profile_send_config
      },

      // Characteristic 4 User Description
      {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        SDI_profile_send_desp
      },
};


//OTA
static gattAttribute_t SDI_profile_ota_attrtbl[OTA_SERV_NUM_ATTR_SUPPORTED] =
{
  // Simple Profile Service
  {
    { ATT_BT_UUID_SIZE, primaryServiceUUID }, /* type */
    GATT_PERMIT_READ,                         /* permissions */
    0,                                        /* handle */
    (uint8 *)&SDI_profile_ota_service         /* pValue */
  },
    // Characteristic 3 Declaration
    {
      { ATT_BT_UUID_SIZE, characterUUID },
      GATT_PERMIT_READ,
      0,
      &SDI_profile_ota_recv_props
    },

      // Characteristic Value 3
      {
        { ATT_BT_UUID_SIZE, SDI_profile_ota_recv_uuid },
        GATT_PERMIT_WRITE,
        0,
        SDI_profile_ota_recv_buf
      },

      // Characteristic 3 User Description
      {
        { ATT_BT_UUID_SIZE, charUserDescUUID },
        GATT_PERMIT_READ,
        0,
        SDI_profile_ota_recv_desp
      },
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static bStatus_t sdiProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method);
static bStatus_t sdiProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);
static bStatus_t otaProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// Simple Profile Service Callbacks
// Note: When an operation on a characteristic requires authorization and
// pfnAuthorizeAttrCB is not defined for that characteristic's service, the
// Stack will report a status of ATT_ERR_UNLIKELY to the client.  When an
// operation on a characteristic requires authorization the Stack will call
// pfnAuthorizeAttrCB to check a client's authorization prior to calling
// pfnReadAttrCB or pfnWriteAttrCB, so no checks for authorization need to be
// made within these functions.
CONST gattServiceCBs_t sdiProfileCBs =
{
  sdiProfile_ReadAttrCB,  // Read callback function pointer
  sdiProfile_WriteAttrCB, // Write callback function pointer
  NULL                       // Authorization callback function pointer
};

CONST gattServiceCBs_t otaProfileCBs =
{
  NULL,  // Read callback function pointer
  otaProfile_WriteAttrCB,  // Write callback function pointer
  NULL   // Authorization callback function pointer
};


/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleProfile_AddService
 *
 * @brief   Initializes the Simple Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 *
 * @return  Success or Failure
 */
bStatus_t sdiProfile_AddService( uint32 services )
{
	uint8 status;

	// Allocate Client Characteristic Configuration table
	SDI_profile_send_config = (gattCharCfg_t *)ICall_malloc( sizeof(gattCharCfg_t) *
                                                            linkDBNumConns );
	if ( SDI_profile_send_config == NULL )
	{
		return ( bleMemAllocError );
	}

	// Initialize Client Characteristic Configuration attributes
	GATTServApp_InitCharCfg( INVALID_CONNHANDLE, SDI_profile_send_config );

	status = GATTServApp_RegisterService( sdiProfileAttrTbl,
                                          GATT_NUM_ATTRS( sdiProfileAttrTbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &sdiProfileCBs );
	if(status != SUCCESS) return status;

	status = GATTServApp_RegisterService( SDI_profile_ota_attrtbl,
                                          GATT_NUM_ATTRS( SDI_profile_ota_attrtbl ),
                                          GATT_MAX_ENCRYPT_KEY_SIZE,
                                          &otaProfileCBs );

	return ( status );
}

/*********************************************************************
 * @fn      SimpleProfile_RegisterAppCBs
 *
 * @brief   Registers the application callback function. Only call
 *          this function once.
 *
 * @param   callbacks - pointer to application callbacks.
 *
 * @return  SUCCESS or bleAlreadyInRequestedMode
 */
bStatus_t sdiProfile_RegisterAppCBs( sdiProfileCBs_t *appCallbacks )
{
  if ( appCallbacks )
  {
    sdiProfile_AppCBs = appCallbacks;

    return ( SUCCESS );
  }
  else
  {
    return ( bleAlreadyInRequestedMode );
  }
}

/*********************************************************************
 * @fn      SimpleProfile_SetParameter
 *
 * @brief   Set a Simple Profile parameter.
 *
 * @param   param - Profile parameter ID
 * @param   len - length of data to write
 * @param   value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 *
 * @return  bStatus_t
 */
bStatus_t sdiProfile_SetParameter( uint8 param, uint8 len, void *value )
{
	bStatus_t ret = SUCCESS;

	if(len){
        // See if Notification has been enabled
        uint8 i;
		uint8 *ptr = value;

		for(i=0; i<len; i++)
			g_send_buf[i] = *ptr++;
		g_send_len = len;
		
        GATTServApp_ProcessCharCfg( SDI_profile_send_config, &SDI_profile_send_buf[0], FALSE,
                                    sdiProfileAttrTbl, GATT_NUM_ATTRS( sdiProfileAttrTbl ),
                                    INVALID_TASK_ID, sdiProfile_ReadAttrCB );
	}else{
		ret = bleInvalidRange;
	}

  return ( ret );
}

/*********************************************************************
 * @fn          simpleProfile_ReadAttrCB
 *
 * @brief       Read an attribute.
 *
 * @param       connHandle - connection message was received on
 * @param       pAttr - pointer to attribute
 * @param       pValue - pointer to data to be read
 * @param       pLen - length of data to be read
 * @param       offset - offset of the first octet to be read
 * @param       maxLen - maximum length of data to be read
 * @param       method - type of read message
 *
 * @return      SUCCESS, blePending or Failure
 */
static bStatus_t sdiProfile_ReadAttrCB(uint16_t connHandle,
                                          gattAttribute_t *pAttr,
                                          uint8_t *pValue, uint16_t *pLen,
                                          uint16_t offset, uint16_t maxLen,
                                          uint8_t method)
{
  bStatus_t status = SUCCESS;

  // Make sure it's not a blob operation (no attributes in the profile are long)
  if ( offset > 0 )
  {
    return ( ATT_ERR_ATTR_NOT_LONG );
  }

	*pLen = g_send_len;
	VOID memcpy(pValue, (void *)&g_send_buf[0], *pLen);

	return ( status );
}

/*********************************************************************
 * @fn      simpleProfile_WriteAttrCB
 *
 * @brief   Validate attribute data prior to a write operation
 *
 * @param   connHandle - connection message was received on
 * @param   pAttr - pointer to attribute
 * @param   pValue - pointer to data to be written
 * @param   len - length of data
 * @param   offset - offset of the first octet to be written
 * @param   method - type of write message
 *
 * @return  SUCCESS, blePending or Failure
 */
static bStatus_t sdiProfile_WriteAttrCB(uint16_t connHandle,
                                           gattAttribute_t *pAttr,
                                           uint8_t *pValue, uint16_t len,
                                           uint16_t offset, uint8_t method)
{
  bStatus_t status = SUCCESS;
  uint8 notifyApp = 0xFF;

  if ( pAttr->type.len == ATT_BT_UUID_SIZE )
  {
    // 16-bit UUID
    uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);

    if(uuid == GATT_CLIENT_CHAR_CFG_UUID){
        status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
                                                 offset, GATT_CLIENT_CFG_NOTIFY );
		
     }else{
        // Should never get here! (characteristics 2 and 4 do not have write permissions)
        status = ATT_ERR_ATTR_NOT_FOUND;
    }
  }else{
    // 128-bit UUID
	uint8 uuid = pAttr->type.uuid[12];

	if(uuid == 0x01){
		if(offset == 0){
			if(len == 0) status = ATT_ERR_INVALID_HANDLE;
		}else{
			status = ATT_ERR_ATTR_NOT_LONG;
		}

		if(status == SUCCESS){
			notifyApp = SDIPROFILE_RECV_CHAR;
			//...... process recv data
			uint8 i;
			
			g_recv_buf[0] = len;
			for(i=1; i<=len; i++)
				g_recv_buf[i] = *pValue++;	
		}
	}else{	
    	status = ATT_ERR_INVALID_HANDLE;
	}
  }

  // If a characteristic value changed then callback function to notify application of change
  if ( (notifyApp != 0xFF ) && sdiProfile_AppCBs && sdiProfile_AppCBs->pfnsdiProfileChange )
  {
    sdiProfile_AppCBs->pfnsdiProfileChange( notifyApp);
  }

  return ( status );
}

/*********************************************************************
*********************************************************************/
static bStatus_t otaProfile_WriteAttrCB(uint16_t connHandle,
										   gattAttribute_t *pAttr,
										   uint8_t *pValue, uint16_t len,
										   uint16_t offset, uint8_t method)
{
	bStatus_t status = SUCCESS;
	uint8 notifyApp = 0xFF;
	
	if ( pAttr->type.len == ATT_BT_UUID_SIZE )
	{
		// 16-bit UUID
		uint16 uuid = BUILD_UINT16( pAttr->type.uuid[0], pAttr->type.uuid[1]);
	
		if(uuid == GATT_CLIENT_CHAR_CFG_UUID){
			status = GATTServApp_ProcessCCCWriteReq( connHandle, pAttr, pValue, len,
													 offset, GATT_CLIENT_CFG_NOTIFY );
			
		 }else if(uuid == SDI_PROFILE_OTA_RECV_UUID){

		 	if(offset == 0){
				if(len == 0) status = ATT_ERR_INVALID_HANDLE;
			}else{
				status = ATT_ERR_ATTR_NOT_LONG;
			}
	
			if(status == SUCCESS){
				notifyApp = SDIPROFILE_OTA_RECV_CHAR;
				//...... process recv data
				uint8 i;
				
				g_ota_recv_buf[0] = len;
				for(i=1; i<=len; i++)
					g_ota_recv_buf[i] = *pValue++;	
			}			
		}
	}else{
		status = ATT_ERR_ATTR_NOT_FOUND;
	}
	
	  // If a characteristic value changed then callback function to notify application of change
	if ( (notifyApp != 0xFF ) && sdiProfile_AppCBs && sdiProfile_AppCBs->pfnsdiProfileChange )
	{
		sdiProfile_AppCBs->pfnsdiProfileChange( notifyApp);
	}
	
	return ( status );
}


