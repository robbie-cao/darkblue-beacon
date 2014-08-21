/*
 * Copyright 2014, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
*/

/** @file
*
* BLE Find Me client profile, service, application
*
* Refer to Bluetooth SIG Find Me Profile1.0 specifications for details.
*
* This file implements the BLE Find Me client profile, service, application.
* Find me server role is the same as proximity profile's immediate alert 
* service and it is typically included in proximity profile.  This file 
* contains functions needed to implement Find Me while the GATT DB for the 
* service and actual calls are implemented in bleprox.c.
*
*/
#include "blefind.h"
#include "dbfw_app.h"

#define FID  FID_BLEAPP_APP__BLEFIND_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blefind_FindmeRsp(int len, int attr_len, UINT8 *data);


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT16  blefind_findme_hdl; //this is for GATT client
    UINT8   blefind_findme_state;
    UINT16  blefind_s_handle;
    UINT16  blefind_e_handle;
    UINT8   blefind_findme_prev_alert_level;
} tFindAppState;

tFindAppState *findAppState = NULL;


void blefind_Init(void)
{
    if (!findAppState)
    {
        findAppState = (tFindAppState *)cfa_mm_Sbrk(sizeof(tFindAppState));
        memset(findAppState, 0x00, sizeof(tFindAppState));

        //initialize the default value of findAppState
        findAppState->blefind_findme_state = FINDME_IDLE;
    }

    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) blefind_FindmeRsp);
    if (bleprofile_p_cfg && bleprofile_p_cfg->client_grouptype_enable)
    {
        leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) blefind_FindmeRsp);
    }
}

// Function is typically called by a proximity device when the find me button is pushed
void blefind_FindmeButton(void)
{
    // if we never performed the search for the immediate alert characteristic, set it up now
    if (findAppState->blefind_findme_hdl == 0) //first read
    {
        // Set the state to start searching for primary services.  Actual action will when FindmeReq is called.
        findAppState->blefind_s_handle     = 1;
        findAppState->blefind_e_handle     = 0xFFFF;
        findAppState->blefind_findme_state = FINDME_WAIT_READ_PRIMARY_SERVICE_DEFER;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
    }
    else 
    {
        // Set the state to write the aler level.  Actual action will when FindmeReq is called.
        findAppState->blefind_findme_state = FINDME_WAIT_WRITE_ALERTLEVEL_DEFER;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
    }
}

// Application should call this function to set handle to write immediate alert if known,
// or clear when Find Me should perform discovery
void blefind_FindmeSetHandle(UINT16 findme_hdl)
{
    findAppState->blefind_findme_hdl = findme_hdl;
}

// Findme logic performs GATT search to find the handle to be used for immediate alert.  Application
// should call this function to save appropriate handle in the NVRAM
UINT16 blefind_FindmeGetHandle(void)
{
    return (findAppState->blefind_findme_hdl);
}

// Send request based on the defered state.  Application should call this
// function periodically, so that findme logic can execute request when needed.
void blefind_FindmeReq(void)
{
    if (bleprofile_p_cfg->findme_locator_enable)
    {
        //handle deferred Read or Write
        if (findAppState->blefind_findme_state == FINDME_WAIT_READ_PRIMARY_SERVICE_DEFER)
        {
            findAppState->blefind_findme_state = FINDME_WAIT_READ_PRIMARY_SERVICE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));

            if (bleprofile_p_cfg->client_grouptype_enable)
            {
                bleprofile_sendReadByGroupTypeReq(findAppState->blefind_s_handle, findAppState->blefind_e_handle,
                    UUID_ATTRIBUTE_PRIMARY_SERVICE);
            }
            else
            {
                bleprofile_sendReadByTypeReq(findAppState->blefind_s_handle, findAppState->blefind_e_handle,
                    UUID_ATTRIBUTE_PRIMARY_SERVICE);
            }
        }
        else if (findAppState->blefind_findme_state == FINDME_WAIT_READ_CHARACTERISTIC_DEFER)
        {
            findAppState->blefind_findme_state = FINDME_WAIT_READ_CHARACTERISTIC;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
            bleprofile_sendReadByTypeReq(findAppState->blefind_s_handle, findAppState->blefind_e_handle,
						UUID_ATTRIBUTE_CHARACTERISTIC);
        }
        else if (findAppState->blefind_findme_state == FINDME_WAIT_WRITE_ALERTLEVEL_DEFER)
        {
            findAppState->blefind_findme_state = FINDME_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));

            if ((bleprofile_p_cfg->findme_locator_enable & 0x02) && findAppState->blefind_findme_prev_alert_level)
            {
                findAppState->blefind_findme_prev_alert_level = 0;
                bleprofile_sendWriteCmd(findAppState->blefind_findme_hdl, &(findAppState->blefind_findme_prev_alert_level), 1);
            }
            else
            {
                findAppState->blefind_findme_prev_alert_level = bleprofile_p_cfg->findme_alert_level;
                bleprofile_sendWriteCmd(findAppState->blefind_findme_hdl, &(bleprofile_p_cfg->findme_alert_level), 1);

                bleprofile_LEDBlink(bleprofile_p_cfg->led_off_ms, bleprofile_p_cfg->led_off_ms, bleprofile_p_cfg->findme_alert_level);
            }
        }
    }
}

void blefind_FindmeRsp(int len, int attr_len, UINT8 *data)
{
    UINT16  uuid = 0;
    int     i, found;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind, FindmeRsp:%d, %d", TVF_WW(len, attr_len));
    ble_tracen((char *)data, len);

    if (findAppState->blefind_findme_state == FINDME_WAIT_READ_PRIMARY_SERVICE)
    {
        if (len)
        {
            if (bleprofile_p_cfg->client_grouptype_enable)
            {
                //search UUID
                //S_HANDLE(2) E_HANDLE(2) UUID(2-16)

                found = -1;
                for (i = 0; i < len; i += attr_len)
                {
                    uuid = data[i + 4] + (data[i + 5] << 8);
                    if (uuid == UUID_SERVICE_IMMEDIATE_ALERT)
                    {
                        found = i; //start position of uuid
                        break;
                    }
                }

                if (found != -1)
                {
                    //start handle calculation
                    findAppState->blefind_s_handle = data[found]     + (data[found + 1] << 8);
                    findAppState->blefind_e_handle = data[found + 2] + (data[found + 3] << 8);

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found alertservice - uuid:%04x, s_hdl:%04x", TVF_WW(uuid, findAppState->blefind_s_handle));
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "e_hdl: %04x", TVF_D(findAppState->blefind_e_handle));

                    //using defered way
                    findAppState->blefind_findme_state = FINDME_WAIT_READ_CHARACTERISTIC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
                }
                else
                {
                    //retry reading
                    //find last handle
                    findAppState->blefind_s_handle = data[len - attr_len + 2] + (data[len - attr_len + 3] << 8) + 1;
                    findAppState->blefind_e_handle = 0xFFFF;
                    findAppState->blefind_findme_state = FINDME_WAIT_READ_PRIMARY_SERVICE_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
                }
            }
            else
            {
                //search uuid
                // HANDLE(2) UUID(2-16)
                found = -1;
                for (i = 0; i < len; i += attr_len)
                {
                    uuid = data[i + 2] + (data[i + 3] << 8);
                    if (uuid == UUID_SERVICE_IMMEDIATE_ALERT)
                    {
                        found = i; //start position of uuid
                        break;
                    }
                }

                if (found != -1)
                {
                    //start handle calculation
                    findAppState->blefind_s_handle = data[found] + (data[found + 1] << 8);

                    //end handle calculation
                    if (found +attr_len < len)
                    {
                        findAppState->blefind_e_handle = data[found + attr_len] + (data[found+attr_len+1] << 8) - 1;
                    }
                    else
                    {
                        findAppState->blefind_e_handle = 0xFFFF;
                    }
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found alertservice - uuid:%04x, s_hdl:%04x", TVF_WW(uuid, findAppState->blefind_s_handle));
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "e_hdl: %04x", TVF_D(findAppState->blefind_e_handle));

                    //using defered way
                    findAppState->blefind_findme_state = FINDME_WAIT_READ_CHARACTERISTIC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
                }
                else
                {
                    //retry reading
                    //find last handle
                    findAppState->blefind_s_handle = data[len-attr_len] + (data[len-(attr_len-1)] << 8)+1;
                    findAppState->blefind_e_handle = 0xFFFF;
                    findAppState->blefind_findme_state = FINDME_WAIT_READ_PRIMARY_SERVICE_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
                }
            }
        }
        else
        {
            findAppState->blefind_findme_state = FINDME_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
        }
    }
    else if (findAppState->blefind_findme_state == FINDME_WAIT_READ_CHARACTERISTIC)
    {
        if (len)
        {
            // HANDLE(2) PERM(1) HANDLE(2) UUID(2-16)

            //search uuid
            found = -1;
            for (i = 0; i < len; i += attr_len)
            {
                uuid = data[i + 5] + (data[i + 6] << 8);
                if (uuid == UUID_CHARACTERISTIC_ALERT_LEVEL)
                {
                    found = i; //start position of uuid
                    break;
                }
            }

            if (found != -1)
            {
                //calculate handle
                findAppState->blefind_findme_hdl = data[found + 3] + (data[found + 4] << 8);

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found alertlevel_cha - uuid:%04x, hdl:%04x", TVF_WW(uuid, findAppState->blefind_findme_hdl));

                //Write Immediate Alert DB as 1 or 2
                findAppState->blefind_findme_state = FINDME_WAIT_WRITE_ALERTLEVEL_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
            }
            else
            {
                //try again with defered way
                findAppState->blefind_s_handle     = data[len - attr_len] + (data[len - (attr_len - 1)] << 8) + 1;
                //findAppState->blefind_e_handle = ; //using previous value
                findAppState->blefind_findme_state = FINDME_WAIT_READ_CHARACTERISTIC_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
            }
        }
        else
        {
            findAppState->blefind_findme_state = FINDME_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blefind_findme_state = %02x", TVF_D(findAppState->blefind_findme_state));
        }
    }
}

