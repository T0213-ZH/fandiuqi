/******************************************************************************

 @file       simple_gatt_profile.h

 @brief This file contains the Simple GATT profile definitions and prototypes
        prototypes.

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

#ifndef SIMPLEGATTPROFILE_H
#define SIMPLEGATTPROFILE_H

#ifdef __cplusplus
extern "C"
{
#endif


/*********************************************************************
 * CONSTANTS
 */

//Begin: Modify By T0213-ZH for solidic ble project
#define SDIPROFILE_RECV_CHAR                  (0x02)  
#define SDIPROFILE_SEND_CHAR                  (0x03)  
#define SDIPROFILE_OTA_RECV_CHAR              (0xFE)


#define SIMPLEPROFILE_SERVICE               0x00000001

#define SDI_PROFILE_SERV_UUID                {0x8C,0xF9,0x97,0xA6,0xEE,0x94,0xE3,0xBC,0xF8,0x21,0xB2,0x60,0x00,0x80,0x00,0x00}
#define SDI_PROFILE_SEND_UUID                {0x8C,0xF9,0x97,0xA6,0xEE,0x94,0xE3,0xBC,0xF8,0x21,0xB2,0x60,0x02,0x80,0x00,0x00}
#define SDI_PROFILE_RECV_UUID                {0x8C,0xF9,0x97,0xA6,0xEE,0x94,0xE3,0xBC,0xF8,0x21,0xB2,0x60,0x01,0x80,0x00,0x00}

#define SDI_PROFILE_OTA_SERV_UUID            (0x1902)
#define SDI_PROFILE_OTA_RECV_UUID            (0x2B12)
//End

/*********************************************************************
 * Profile Callbacks
 */

// Callback when a characteristic value has changed
typedef void (*sdiProfileChange_t)( uint8 paramID);

typedef struct
{
  sdiProfileChange_t        pfnsdiProfileChange;  // Called when characteristic value changes
} sdiProfileCBs_t;


//typedef struct {
//	uint8 len;
//	uint8 data[20];
//}sdi_profile_data_buf;

//typedef struct{
//	sdi_profile_data_buf recv;
//	sdi_profile_data_buf send;
//}sdi_data_ctr;

//extern sdi_data_ctr g_sdi_data_ctr;

/*********************************************************************
 * API FUNCTIONS
 */


/*
 * SimpleProfile_AddService- Initializes the Simple GATT Profile service by registering
 *          GATT attributes with the GATT server.
 *
 * @param   services - services to add. This is a bit map and can
 *                     contain more than one service.
 */

extern bStatus_t sdiProfile_AddService( uint32 services );

/*
 * SimpleProfile_RegisterAppCBs - Registers the application callback function.
 *                    Only call this function once.
 *
 *    appCallbacks - pointer to application callbacks.
 */
extern bStatus_t sdiProfile_RegisterAppCBs( sdiProfileCBs_t *appCallbacks );

/*
 * SimpleProfile_SetParameter - Set a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    len - length of data to right
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t sdiProfile_SetParameter( uint8 param, uint8 len, void *value );

/*
 * SimpleProfile_GetParameter - Get a Simple GATT Profile parameter.
 *
 *    param - Profile parameter ID
 *    value - pointer to data to write.  This is dependent on
 *          the parameter ID and WILL be cast to the appropriate
 *          data type (example: data type of uint16 will be cast to
 *          uint16 pointer).
 */
extern bStatus_t sdiProfile_GetParameter( uint8 param, void *value );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* SIMPLEGATTPROFILE_H */
