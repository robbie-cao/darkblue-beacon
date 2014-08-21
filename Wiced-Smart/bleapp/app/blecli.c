/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2011 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: blecli.h
*
* Abstract: This file implements the BLE Generic client profile, service, application
*
*
* Functions:
*
*******************************************************************************/
#include "blecli.h"
#include "dbfw_app.h"

#define FID  FID_BLEAPP_APP__BLECLI_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blecli_ClientRsp(int len, int attr_len, UINT8 *data);


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////
typedef struct
{
    UINT16 blecli_client_hdl; //this is for GATT client

    UINT16 blecli_svc_uuid;
    UINT16 blecli_cha_uuid;
    UINT16 blecli_desc_uuid;
    UINT16 blecli_s_handle;
    UINT16 blecli_e_handle;

    UINT8 *blecli_data;

    LEATT_TRIPLE_PARAM_CB blecli_cb;

    BLE_CLIENT_HANDLE blecli_client_hdl_list[CLIENT_HANDLE_NUM_MAX];

    UINT8 blecli_action;

    UINT8 blecli_client_hdl_num;

    UINT8 blecli_len;
} tCliAppState;

tCliAppState *cliAppState = NULL;

UINT8 blecli_client_state = CLIENT_IDLE;


void blecli_Init(void)
{
    if(!cliAppState)
    {
        cliAppState = (tCliAppState *)cfa_mm_Sbrk(sizeof(tCliAppState));
        memset(cliAppState, 0x00, sizeof(tCliAppState));
    }

    cliAppState->blecli_client_hdl=0;
    cliAppState->blecli_client_hdl_num = 0;
    cliAppState->blecli_data = NULL;
    cliAppState->blecli_len = 0;
    cliAppState->blecli_cb=NULL;
    blecli_client_state=CLIENT_IDLE;
    memset(cliAppState->blecli_client_hdl_list, 0x00, sizeof(BLE_CLIENT_HANDLE)*CLIENT_HANDLE_NUM_MAX);

    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) blecli_ClientRsp);
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) blecli_ClientRsp);
    if(bleprofile_p_cfg && bleprofile_p_cfg->client_grouptype_enable)
    {
        leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) blecli_ClientRsp);
    }
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) blecli_ClientRsp);
}

UINT8 blecli_ClientButton(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid,
	UINT8 action, UINT8 *data, UINT8 len, LEATT_TRIPLE_PARAM_CB cb)
{
    if(blecli_client_state != CLIENT_IDLE)
    {
        return blecli_client_state;
    }

    cliAppState->blecli_client_hdl = blecli_ClientGetHandle(svc_uuid, cha_uuid, desc_uuid);

    cliAppState->blecli_action = action;
    cliAppState->blecli_data = data;
    cliAppState->blecli_len = len;
    cliAppState->blecli_cb = cb;

    if(!cliAppState->blecli_client_hdl) //first read
    {
        //read primary services
        cliAppState->blecli_svc_uuid = svc_uuid;
	 cliAppState->blecli_cha_uuid = cha_uuid;
	 cliAppState->blecli_desc_uuid = desc_uuid;
        cliAppState->blecli_s_handle = 1;
        cliAppState->blecli_e_handle = 0xFFFF;
        blecli_client_state = CLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
    }
    else //there is saved handle
    {
        blecli_client_state = CLIENT_WAIT_ACTION_DEFER;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
    }

    return CLIENT_IDLE;
}

void blecli_ClientHandleReset(void)
{
    cliAppState->blecli_client_hdl=0;
    cliAppState->blecli_client_hdl_num = 0;
    cliAppState->blecli_data = NULL;
    cliAppState->blecli_len = 0;
    cliAppState->blecli_cb=NULL;
    blecli_client_state=CLIENT_IDLE;
    memset(cliAppState->blecli_client_hdl_list, 0x00, sizeof(BLE_CLIENT_HANDLE)*CLIENT_HANDLE_NUM_MAX);
}

#if 0
void blecli_ClientHandleTest(void)
{
    UINT8 i, j, k;

    for(i=1; i<=4; i++)
    {
	 for(j=1; j<=4; j++)
	 {
	     for(k=0; k<3; k++)
	     {
	         if(blecli_ClientSetHandle(i, j, k, (i<<4)|(j<<2)|k))
	         {
	             ble_trace4("SetHandle(%d, %d, %d):%02x Success", i, j, k, (i<<4)|(j<<2)|k);
                    ble_trace5("GetHandle(%d, %d, %d):%02x index:%d", i, j, k,
				blecli_ClientGetHandle(i, j, k), blecli_ClientGetIndex(i, j, k));
	         }
	     }
	 }
    }
}
#endif

INT8 blecli_ClientGetIndex(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid)
{
    int i;

    for(i=0; i<cliAppState->blecli_client_hdl_num; i++)
    {
        if((cliAppState->blecli_client_hdl_list[i].svc_uuid == svc_uuid) &&
            (cliAppState->blecli_client_hdl_list[i].cha_uuid == cha_uuid))
        {
            if(desc_uuid)
            {
                if(cliAppState->blecli_client_hdl_list[i].desc_uuid == desc_uuid)
                {
                    return i;
                }
                else
                {
                    return -1;
                }
            }
            else
            {
                return i;
            }
        }
    }

    return -1;
}

UINT16 blecli_ClientGetHandle(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid)
{
    int i;

    for(i=0; i<cliAppState->blecli_client_hdl_num; i++)
    {
        if((cliAppState->blecli_client_hdl_list[i].svc_uuid == svc_uuid) &&
            (cliAppState->blecli_client_hdl_list[i].cha_uuid == cha_uuid))
        {
            if(desc_uuid)
            {
                if(cliAppState->blecli_client_hdl_list[i].desc_uuid == desc_uuid)
                {
                    return cliAppState->blecli_client_hdl_list[i].desc_hdl;
                }
                else
                {
                    return 0;
                }
            }
            else
            {
                return cliAppState->blecli_client_hdl_list[i].cha_hdl;
            }
        }
    }

    return 0;
}


UINT8 blecli_ClientSetHandle(UINT16 svc_uuid, UINT16 cha_uuid, UINT16 desc_uuid, UINT16 hdl)
{
    INT8 index = -1;

    index = blecli_ClientGetIndex(svc_uuid, cha_uuid, desc_uuid);

    if(index != -1)
    {
        return blecli_ClientReplaceHandle(index, desc_uuid, hdl);
    }
    else
    {
        //replace last one when full
        if(cliAppState->blecli_client_hdl_num >= CLIENT_HANDLE_NUM_MAX)
        {
            cliAppState->blecli_client_hdl_num = CLIENT_HANDLE_NUM_MAX-1;
        }

        if(desc_uuid == 0)
        {
            cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].svc_uuid = svc_uuid;
            cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].cha_uuid = cha_uuid;
            cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].cha_hdl = hdl;
        }
        else
        {
            //search with characteristinc UUID once again
            index = blecli_ClientGetIndex(svc_uuid, cha_uuid, 0);

            if(index != -1)
            {
                return blecli_ClientReplaceHandle(index, desc_uuid, hdl);
            }
            else
            {
                cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].svc_uuid = svc_uuid;
                cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].cha_uuid = cha_uuid;
                cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].desc_uuid = desc_uuid;
                cliAppState->blecli_client_hdl_list[cliAppState->blecli_client_hdl_num].desc_hdl = hdl;
            }
        }

        (cliAppState->blecli_client_hdl_num)++;

        return 1;
    }
}


UINT8 blecli_ClientReplaceHandle(INT8 index, UINT16 desc_uuid, UINT16 new_hdl)
{
    if(desc_uuid == 0)
    {
        cliAppState->blecli_client_hdl_list[index].cha_hdl = new_hdl;
    }
    else
    {
        cliAppState->blecli_client_hdl_list[index].desc_uuid = desc_uuid;
        cliAppState->blecli_client_hdl_list[index].desc_hdl = new_hdl;
    }

    return 1;
}


void blecli_ClientReq(void)
{
    //handle deferred Read or Write
    if(blecli_client_state == CLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER)
    {
        blecli_client_state = CLIENT_WAIT_READ_PRIMARY_SERVICE;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
        if(bleprofile_p_cfg->client_grouptype_enable)
        {
            bleprofile_sendReadByGroupTypeReq(cliAppState->blecli_s_handle, cliAppState->blecli_e_handle,
			UUID_ATTRIBUTE_PRIMARY_SERVICE);
        }
        else
        {
            bleprofile_sendReadByTypeReq(cliAppState->blecli_s_handle, cliAppState->blecli_e_handle,
			UUID_ATTRIBUTE_PRIMARY_SERVICE);
        }
    }
    else if(blecli_client_state == CLIENT_WAIT_READ_CHARACTERISTIC_DEFER)
    {
        blecli_client_state = CLIENT_WAIT_READ_CHARACTERISTIC;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
        bleprofile_sendReadByTypeReq(cliAppState->blecli_s_handle, cliAppState->blecli_e_handle,
			UUID_ATTRIBUTE_CHARACTERISTIC);
    }
    else if(blecli_client_state == CLIENT_WAIT_READ_DESC_DEFER)
    {
        blecli_client_state = CLIENT_WAIT_READ_DESC;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
        bleprofile_sendReadByTypeReq(cliAppState->blecli_s_handle, cliAppState->blecli_e_handle,
			cliAppState->blecli_desc_uuid);
    }
    else if(blecli_client_state == CLIENT_WAIT_ACTION_DEFER)
    {
        if(cliAppState->blecli_action == CLIENT_READREQ)
        {
            blecli_client_state = CLIENT_WAIT_READ;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));

            bleprofile_sendReadReq(cliAppState->blecli_client_hdl);
        }
        else if(cliAppState->blecli_action == CLIENT_WRITEREQ)
        {
            blecli_client_state = CLIENT_WAIT_WRITE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));

            bleprofile_sendWriteReq(cliAppState->blecli_client_hdl, cliAppState->blecli_data, cliAppState->blecli_len);
        }
        else if(cliAppState->blecli_action == CLIENT_WRITECMD)
        {
            blecli_client_state = CLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));

            bleprofile_sendWriteCmd(cliAppState->blecli_client_hdl, cliAppState->blecli_data, cliAppState->blecli_len);
        }
    }
}

void blecli_ClientRsp(int len, int attr_len, UINT8 *data)
{
    UINT16 uuid=0;
    int i, found;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli, ClientRsp:%d, %d", TVF_WW(len, attr_len));

    if(blecli_client_state != CLIENT_WAIT_WRITE)
    {
        ble_tracen((char *)data, len);
    }

    if(blecli_client_state == CLIENT_WAIT_READ_PRIMARY_SERVICE)
    {
        if(len)
        {
            if(bleprofile_p_cfg->client_grouptype_enable)
            {
                //search UUID
                //S_HANDLE(2) E_HANDLE(2) UUID(2-16)

                found = -1;
                for(i=0; i<len; i+=attr_len)
                {
                    uuid = data[i+4] + (data[i+5]<<8);
                    //ble_trace1("uuid=%04x", uuid);
                    if(uuid == cliAppState->blecli_svc_uuid)
                    {
                        found = i; //start position of uuid
		          break;
                    }
                }

                if(found != -1)
                {
                    //start handle calculation
                    cliAppState->blecli_s_handle = data[found] + (data[found+1]<<8);
                    cliAppState->blecli_e_handle = data[found+2] + (data[found+3]<<8);

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found service - uuid:%04x, s_hdl:%04x", TVF_WW(uuid, cliAppState->blecli_s_handle));
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "e_hdl: %04x", TVF_D(cliAppState->blecli_e_handle));

                    //using defered way
                    blecli_client_state = CLIENT_WAIT_READ_CHARACTERISTIC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
                }
                else
                {
                    //retry reading
                    //cli last handle
                    cliAppState->blecli_s_handle = data[len-attr_len+2] + (data[len-attr_len+3]<<8)+1;
                    cliAppState->blecli_e_handle = 0xFFFF;
                    blecli_client_state = CLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
                }
            }
            else
            {
                //search uuid
                // HANDLE(2) UUID(2-16)
                found = -1;
                for(i=0; i<len; i+=attr_len)
                {
                    uuid = data[i+2] + (data[i+3]<<8);
                    //ble_trace1("uuid=%04x", uuid);
                    if(uuid == cliAppState->blecli_svc_uuid)
                    {
                        found = i; //start position of uuid
		          break;
                    }
                }

                if(found != -1)
                {
                    //start handle calculation
                    cliAppState->blecli_s_handle = data[found] + (data[found+1]<<8);

                    //end handle calculation
                    if(found +attr_len< len)
                    {
                        cliAppState->blecli_e_handle = data[found+attr_len] + (data[found+attr_len+1]<<8) -1;
                    }
                    else
                    {
                        cliAppState->blecli_e_handle = 0xFFFF;
                    }
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found service - uuid:%04x, s_hdl:%04x", TVF_WW(uuid, cliAppState->blecli_s_handle));
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "e_hdl: %04x", TVF_D(cliAppState->blecli_e_handle));

                    //using defered way
                    blecli_client_state = CLIENT_WAIT_READ_CHARACTERISTIC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
                }
                else
                {
                    //retry reading
                    //cli last handle
                    cliAppState->blecli_s_handle = data[len-attr_len] + (data[len-(attr_len-1)]<<8)+1;
                    cliAppState->blecli_e_handle = 0xFFFF;
                    blecli_client_state = CLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
                }
            }
        }
        else
        {
            blecli_client_state = CLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
        }
    }
    else if(blecli_client_state == CLIENT_WAIT_READ_CHARACTERISTIC)
    {
        if(len)
        {
            // HANDLE(2) PERM(1) HANDLE(2) UUID(2-16)

            //search uuid
            found = -1;
            for(i=0; i<len; i+=attr_len)
            {
                uuid = data[i+5] + (data[i+6]<<8);
                //ble_trace1("uuid=%04x", uuid);
                if(uuid == cliAppState->blecli_cha_uuid)
                {
                    found = i; //start position of uuid
                    break;
                }
            }

            if(found != -1)
            {
                //calculate handle
                cliAppState->blecli_client_hdl = data[found+3] + (data[found+4]<<8);

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found cha - uuid:%04x, hdl:%04x", TVF_WW(uuid, cliAppState->blecli_client_hdl));

                //save handle
                blecli_ClientSetHandle(cliAppState->blecli_svc_uuid, cliAppState->blecli_cha_uuid, cliAppState->blecli_desc_uuid, cliAppState->blecli_client_hdl);

                if(cliAppState->blecli_desc_uuid)
                {
                    //find descriptor
                    cliAppState->blecli_s_handle = cliAppState->blecli_client_hdl+1;
                    //cliAppState->blecli_e_handle = ; //using previous value
                    blecli_client_state = CLIENT_WAIT_READ_DESC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
                }
                else
                {
                    //Action
                    blecli_client_state = CLIENT_WAIT_ACTION_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
                }
            }
            else
            {
                //try again with defered way
                cliAppState->blecli_s_handle = data[len-attr_len] + (data[len-(attr_len-1)]<<8)+1;
                //cliAppState->blecli_e_handle = ; //using previous value
                blecli_client_state = CLIENT_WAIT_READ_CHARACTERISTIC_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
            }
        }
        else
        {
            blecli_client_state = CLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
        }
    }
    else if(blecli_client_state == CLIENT_WAIT_READ_DESC)
    {
        if(len)
        {
            cliAppState->blecli_client_hdl = data[0] + (data[1]<<8);

            //save handle
            blecli_ClientSetHandle(cliAppState->blecli_svc_uuid, cliAppState->blecli_cha_uuid, cliAppState->blecli_desc_uuid, cliAppState->blecli_client_hdl);

            //Action
            if(cliAppState->blecli_action == CLIENT_READREQ)
            {
                // show the received data
                //ble_tracen(data, len);

                if(cliAppState->blecli_cb)
                {
                    cliAppState->blecli_cb(len-2, attr_len, data+2);//eliminate handle
                }
                blecli_client_state = CLIENT_IDLE;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
            }
            else
            {
                blecli_client_state = CLIENT_WAIT_ACTION_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
            }
        }
    }
    else if(blecli_client_state == CLIENT_WAIT_READ)
    {
        if(len)
        {
            // show the received data
            //ble_tracen(data, len);

            if(cliAppState->blecli_cb)
            {
                cliAppState->blecli_cb(len, attr_len, data);
            }

            blecli_client_state = CLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
        }
    }
    else if(blecli_client_state == CLIENT_WAIT_WRITE)
    {
        if(cliAppState->blecli_cb)
        {
            cliAppState->blecli_cb(0, 0, NULL);
        }

        blecli_client_state = CLIENT_IDLE;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecli_client_state = %02x", TVF_D(blecli_client_state));
    }
}


