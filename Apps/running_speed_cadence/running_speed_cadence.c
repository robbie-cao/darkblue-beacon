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
* BLE Runners Speed and Cadence profile, service, application
*
* Refer to Bluetooth SIG Runners Speed and Cadence Profile 1.0 Runners Speed
* and Cadence Service 1.0 specifications for details.
*
* This file replaces ROM's processing of the fine timer keeping rest of the
* functionality untouched.  ROM code can be reviewed at
* WICED-Smart-SDK\Wiced-Smart\bleapp\app\blersc.c.
*
* Features demonstrated
*  - Replacing ROM application create functions with local one
*  - Accessing ROM functions
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
* 
*/

#include "bleprofile.h"
#include "bleapp.h"
#include "blersc.h"
#include "spar_utils.h"
#include "bleappevent.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Forward declarations
///////////////////////////////////////////////////////////////////////////////////////////////////

static void    running_speed_cadence_Create(void);
static void    running_speed_cadence_appFineTimerCb(UINT32 arg);
static int     running_speed_cadence_write_handler(LEGATTDB_ENTRY_HDR *p);
static int     running_speed_cadence_send_delayed_indication(void* data);

// Forward declaration of the ROM function.
extern void    blersc_appTimerCb(UINT32 arg);
extern void    blersc_connUp(void);
extern void    blersc_connDown(void);
extern void    blersc_advStop(void);
extern void    blersc_smpBondResult(LESMP_PARING_RESULT  result);
extern void    blersc_encryptionChanged(HCI_EVT_HDR *evt);
extern UINT32  blersc_RscButton(UINT32 function);
extern void    blersc_IndicationConf(void);

/******************************************************
 *               Variables Definitions
 ******************************************************/

// GATT databse
const UINT8 running_speed_cadence_db_data[]=
{
    // Handle 0x01: GATT service
    PRIMARY_SERVICE_UUID16 (0x0001, UUID_SERVICE_GATT),

    // Handle 0x02: characteristic Service Changed, handle 0x03 characteristic value
    CHARACTERISTIC_UUID16  (0x0002, 0x0003, UUID_CHARACTERISTIC_SERVICE_CHANGED, LEGATTDB_CHAR_PROP_INDICATE, LEGATTDB_PERM_NONE, 4),
        0x00, 0x00, 0x00, 0x00,

    // Handle 0x14: GAP service
    PRIMARY_SERVICE_UUID16 (0x0014, UUID_SERVICE_GAP),

    // Handle 0x15: characteristic Device Name, handle 0x16 characteristic value
    CHARACTERISTIC_UUID16 (0x0015, 0x0016, UUID_CHARACTERISTIC_DEVICE_NAME, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 16),
        'B','L','E',' ','R','u','n','n','e','r','s',0x00,0x00,0x00,0x00,0x00,

    // Handle 0x17: characteristic Appearance, handle 0x18 characteristic value
    CHARACTERISTIC_UUID16 (0x0017, 0x0018, UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        BIT16_TO_8(APPEARANCE_GENERIC_RUNNING_WALKING_SENSOR),

    // Handle 0x28: Running Speed and Cadence service
    PRIMARY_SERVICE_UUID16 (0x0028, UUID_SERVICE_RSC),

    // Handle 0x29: characteristic RSC Measurement, handle 0x2a characteristic value
    CHARACTERISTIC_UUID16  (0x0029, 0x002a, UUID_CHARACTERISTIC_RSC_MEASUREMENT, LEGATTDB_CHAR_PROP_NOTIFY, LEGATTDB_PERM_NONE, 9),
        0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    // Handle 0x2b: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x002b, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00,0x00,

    // Handle 0x2c: characteristic RSC Feature, handle 0x2d characteristic value
    CHARACTERISTIC_UUID16 (0x002c, 0x002d, UUID_CHARACTERISTIC_RSC_FEATURE, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 2),
        0x13, 0x00,                 //instantaneous stride length, total distance, Multiple Sensor Location

    // Handle 0x2e: characteristic Sensor Location, handle 0x2f characteristic value
    CHARACTERISTIC_UUID16 (0x002e, 0x002f, UUID_CHARACTERISTIC_SENSOR_LOCATION, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 1),
        0x08,                       //in shoe

    // Because of Apple application problem do not use consecutive handle
    // Handle 0x32: characteristic RSC Control Point, handle 0x33 characteristic value
    CHARACTERISTIC_UUID16_WRITABLE (0x0032, 0x0033, UUID_CHARACTERISTIC_RSC_CONTROL_POINT,
                           LEGATTDB_CHAR_PROP_INDICATE | LEGATTDB_CHAR_PROP_WRITE,
                           LEGATTDB_PERM_WRITE_REQ, 19),
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,

    // Handle 0x34: Client Configuration descriptor
    CHAR_DESCRIPTOR_UUID16_WRITABLE (0x0034, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                     LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ, 2),
        0x00, 0x00,

    // Handle 0x3d: Device Info service
    PRIMARY_SERVICE_UUID16 (0x003d, UUID_SERVICE_DEVICE_INFORMATION),

    // Handle 0x3e: characteristic Manufacturer Name, handle 0x3f characteristic value
    CHARACTERISTIC_UUID16 (0x003e, 0x003f, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        'B','r','o','a','d','c','o','m',

    // Handle 0x40: characteristic Model Number, handle 0x41 characteristic value
    CHARACTERISTIC_UUID16 (0x0040, 0x0041, UUID_CHARACTERISTIC_MODEL_NUMBER_STRING, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        '1','2','3','4',0x00,0x00,0x00,0x00,

    CHARACTERISTIC_UUID16 (0x0042, 0x0043, UUID_CHARACTERISTIC_SYSTEM_ID, LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE, 8),
        0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

// Application state used by the application in the ROM.
typedef struct
{
    BLERSC_HOSTINFO     blersc_hostinfo;                        //NVRAM save area

    BLERSC_RSC_DATA     blersc_rsc_data;

    UINT32              blersc_apptimer_count;
    UINT32              blersc_appfinetimer_count;
    UINT16              blersc_con_handle;
    BD_ADDR             blersc_remote_addr;

    UINT16              blersc_rsc_hdl;
    UINT16              blersc_rsc_client_hdl;
    UINT16              blersc_rsc_cp_hdl;
    UINT16              blersc_rsc_cp_client_hdl;
    UINT16              blersc_rsc_bsl_hdl;
    UINT8               blersc_bat_enable;
    UINT8               blersc_notification_enable;
    UINT8               blersc_measurement_done;
    UINT8               blersc_indication_sent;
    UINT8               blersc_sup_senloc[SUPPORTED_SENLOC_MAX];
    UINT32              blersc_distance;
} tRscAppState;

// The actual state instance used by the ROM.
extern tRscAppState *rscAppState;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

// Application initialization
APPLICATION_INIT()
{
	// Register the GATT DB, configurations and the application create function.
    bleapp_set_cfg((UINT8 *)running_speed_cadence_db_data, 
        sizeof (running_speed_cadence_db_data), (void *)&blersc_cfg,
        (void *)&blersc_puart_cfg, (void *)&blersc_gpio_cfg, running_speed_cadence_Create);
}

// The application create function.  Do not call blersc Create because we need to overwrite
// write callback
void running_speed_cadence_Create(void)
{
    ble_trace0("running_speed_cadence_Create()");
    ble_trace0(bleprofile_p_cfg->ver);

	rscAppState = (tRscAppState *)cfa_mm_Sbrk(sizeof(tRscAppState));
	memset(rscAppState, 0x00, sizeof(tRscAppState));

	//initialize the default value of rscAppState
	rscAppState->blersc_sup_senloc[0] = RSC_LOC_TOP_OF_SHOE;
	rscAppState->blersc_sup_senloc[1] = RSC_LOC_IN_SHOE;
	rscAppState->blersc_sup_senloc[2] = RSC_LOC_HIP;
		   // {RSC_LOC_TOP_OF_SHOE, 0xFF, 0xFF};
		   // 0xFF means not used, do not add 0xFF in the middle

	// dump the database to debug uart.
	legattdb_dumpDb();

	bleprofile_Init(bleprofile_p_cfg);
	bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

	blersc_DBInit(); //load handle number

	// register connection up and connection down handler.
	bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blersc_connUp );
	bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blersc_connDown );
	bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blersc_advStop );

	// handler for Encryption changed.
	blecm_regEncryptionChangedHandler(blersc_encryptionChanged);
	// handler for Bond result
	lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blersc_smpBondResult);
	// write DBCB
	legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)running_speed_cadence_write_handler);

	bleprofile_regButtonFunctionCb(blersc_RscButton);

	//data init
	memset(&(rscAppState->blersc_rsc_data), 0x00, sizeof(BLERSC_RSC_DATA));
	rscAppState->blersc_rsc_data.flag = 0;

	bleprofile_regTimerCb(running_speed_cadence_appFineTimerCb, blersc_appTimerCb);
	bleprofile_StartTimer();

	blersc_connDown();
}

// fine timer processing callback
void running_speed_cadence_appFineTimerCb(UINT32 arg)
{
	char rsc_char[READ_UART_LEN+1];
	BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
	
	(rscAppState->blersc_appfinetimer_count)++;
	
	    //this part can be replaced by callback function
    if (bleprofile_handleUARTCb)
    {
        rscAppState->blersc_measurement_done = bleprofile_handleUARTCb((UINT8 *)rsc_char, (UINT8 *)&(rscAppState->blersc_rsc_data));
    }
    else if (bleprofile_p_cfg->test_enable)
    {
        //This is making faking data
        //For test only
        rscAppState->blersc_rsc_data.flag                        = RSC_INSTANTANEOUS_STRIDE_LENGTH_PRESENT
                                                                  |RSC_TOTAL_DISTANCE_PRESENT;
                                                                  //|RSC_WALKING_OR_RUNNING_STATUS;
        rscAppState->blersc_rsc_data.instantaneous_speed         = rscAppState->blersc_appfinetimer_count&0xFFFF;
        rscAppState->blersc_rsc_data.instantaneous_cadence       = rscAppState->blersc_appfinetimer_count&0xFF;
        rscAppState->blersc_rsc_data.instantaneous_stride_length = rscAppState->blersc_appfinetimer_count&0xFFFF;

        rscAppState->blersc_distance += (rscAppState->blersc_rsc_data.instantaneous_stride_length / 10);

        rscAppState->blersc_measurement_done = 1; //New measurement is done
    }


    // send Notification
    if (rscAppState->blersc_notification_enable && rscAppState->blersc_measurement_done) //if connected and encrpted, old data is sent
    {
        int i;

        //write partial based on flag
        // FLAG(1) instantaneous speed(2) instantaneous cadence(1) - 4 bytes are mandatory
        memcpy(db_pdu.pdu, (UINT8 *)&(rscAppState->blersc_rsc_data), 4);
        i = 4;


        // Optional item
        if (rscAppState->blersc_rsc_data.flag & RSC_INSTANTANEOUS_STRIDE_LENGTH_PRESENT) //16 byte
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(rscAppState->blersc_rsc_data.instantaneous_stride_length), 2);
            i += 2;
        }

        if (rscAppState->blersc_rsc_data.flag & RSC_TOTAL_DISTANCE_PRESENT)
        {
            memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(rscAppState->blersc_distance), sizeof(UINT32));
            i += sizeof(UINT32);
        }

        db_pdu.len = i;

        //check client char cfg
        bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);

        if ((db_cl_pdu.len == 2) && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_sendNotification(rscAppState->blersc_rsc_hdl, (UINT8 *)db_pdu.pdu, db_pdu.len);
        }

        rscAppState->blersc_measurement_done = 0; //enable new measurement
    }
	
	bleprofile_ReadButton();
}

// This function is called after LE stack has processed previous event, which was write command from the
// application and sent confirmation for the write.  Now it is safe to send indication.
int running_speed_cadence_send_delayed_indication(void* data)
{
	BLEPROFILE_DB_PDU *p_db_pdu = (BLEPROFILE_DB_PDU *)data;

	ble_tracen((char *)p_db_pdu->pdu, p_db_pdu->len);
	bleprofile_sendIndication(rscAppState->blersc_rsc_cp_hdl, (UINT8 *)p_db_pdu->pdu, p_db_pdu->len, blersc_IndicationConf);

	// See the note above the return value below.
	cfa_mm_Free(data);

	// Note: The serialized call should return either BLE_APP_EVENT_NO_ACTION or
	// BLE_APP_EVENT_FREE_BUFFER in which case the stack will free data
	// using cfa_mm_Free.
	// The other option in this function would be to return BLE_APP_EVENT_FREE_BUFFER
	// and remove the cfa_mm_Free call above.
	return BLE_APP_EVENT_NO_ACTION;
}


// this function replaces blersc_writecb to send indication for the control point write
// after write response goes out
int running_speed_cadence_write_handler(LEGATTDB_ENTRY_HDR *p)
{
    UINT16  handle   = legattdb_getHandle(p);
    int     len      = legattdb_getAttrValueLen(p);
    UINT8   *attrPtr = legattdb_getAttrValue(p);

    ble_trace1("WriteCb: handle %04x", handle);

    if ((rscAppState->blersc_rsc_client_hdl) && (handle == rscAppState->blersc_rsc_client_hdl))
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(rscAppState->blersc_rsc_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if ((db_cl_pdu.len == 2) && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_StopConnIdleTimer();
        }
        else
        {
            bleprofile_StartConnIdleTimer(bleprofile_p_cfg->con_idle_timeout, bleprofile_appTimerCb);
        }

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            rscAppState->blersc_hostinfo.serv[0]         = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[0]          = UUID_CHARACTERISTIC_RSC_MEASUREMENT;
            rscAppState->blersc_hostinfo.cli_cha_desc[0] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

            ble_trace1("NVRAM write:%04x", writtenbyte);
        }
    }
    else if (rscAppState->blersc_rsc_cp_client_hdl && handle == rscAppState->blersc_rsc_cp_client_hdl)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;

        bleprofile_ReadHandle(rscAppState->blersc_rsc_cp_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

        // Save client characteristic descriptor to NVRAM
        if (memcmp(rscAppState->blersc_remote_addr, rscAppState->blersc_hostinfo.bdAddr, 6) == 0)
        {
            UINT8 writtenbyte;

            rscAppState->blersc_hostinfo.serv[1]         = UUID_SERVICE_RSC;
            rscAppState->blersc_hostinfo.cha[1]          = UUID_CHARACTERISTIC_RSC_CONTROL_POINT;
            rscAppState->blersc_hostinfo.cli_cha_desc[1] = db_cl_pdu.pdu[0] + (db_cl_pdu.pdu[1] << 8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLERSC_HOSTINFO), (UINT8 *)&(rscAppState->blersc_hostinfo));

            ble_trace1("NVRAM write:%04x", writtenbyte);
        }
    }
    else if (rscAppState->blersc_rsc_cp_hdl && (handle == rscAppState->blersc_rsc_cp_hdl))
    {
        BLERSC_CP_HDR       *cpHdr      = (BLERSC_CP_HDR *) attrPtr ;
        BLEPROFILE_DB_PDU   db_pdu;
        BLERSC_CP_RSP_HDR   *cprspHdr   = (BLERSC_CP_RSP_HDR *)(db_pdu.pdu);
        UINT8               response;
        UINT8               length      = 3; //default is 3

        if (blersc_checkClientConfigBeforeCP())
        {
            if (cpHdr->opcode == RSC_CP_RESET_TOTAL_DISTANCE)
            {
                // this is set cumulative value procedure
                if (len < 5)
                {
                    response = RSC_CP_INVALID_PARAMETER;
                }
                else
                {
                    rscAppState->blersc_distance = cpHdr->parameter[0] + (cpHdr->parameter[1] << 8) + (cpHdr->parameter[2] << 16) + (cpHdr->parameter[3] << 24);
                    response = RSC_CP_SUCCESS;
                }
            }
            else if (cpHdr->opcode == RSC_CP_START_SENSOR_CALIBRATION)
            {
                // This may time consuming job when the real sensor is used.
                response = RSC_CP_OPCODE_NOT_SUPPORTED; //RSC_CP_SUCCESS;
            }
            else if (cpHdr->opcode == RSC_CP_UPDATE_SENSOR_LOCATION)
            {
                //check location value
                if ((cpHdr-> parameter[0] >= RSC_LOC_START) && (cpHdr-> parameter[0] <= RSC_LOC_END))
                {
                    //write GATT DB with changed value
                    if (rscAppState->blersc_rsc_bsl_hdl)
                    {
                        BLEPROFILE_DB_PDU db_pdu_bsc;
                        db_pdu_bsc.pdu[0] = cpHdr-> parameter[0];
                        db_pdu_bsc.len = 1;

                        bleprofile_WriteHandle(rscAppState->blersc_rsc_bsl_hdl, &db_pdu_bsc);
                        ble_tracen((char *)db_pdu_bsc.pdu, db_pdu_bsc.len);
                    }
                    response = RSC_CP_SUCCESS;
                }
                else
                {
                    response = RSC_CP_INVALID_PARAMETER;
                }
            }
            else if (cpHdr->opcode == RSC_CP_REQUEST_SUPPORTED_SENSOR_LOCATIONS)
            {
                // support all cases
                int i;

                for (i = 0; i < SUPPORTED_SENLOC_MAX; i++)
                {
                    cprspHdr->response_parameter[i] = rscAppState->blersc_sup_senloc[i];
                    length++;
                }
                response = RSC_CP_SUCCESS;
            }
            else
            {
                // error response
                response = RSC_CP_OPCODE_NOT_SUPPORTED;
            }

            //client char cfg is already checked

            cprspHdr->opcode = RSC_CP_RESPONSE_CODE;
            cprspHdr->request_opcode = cpHdr->opcode;
            cprspHdr->response_value = response;

            if (rscAppState->blersc_indication_sent == 0)
            {
            	// some clients do not like if we send indication before write confirm.
            	// delay sending indication for later
            	BLEPROFILE_DB_PDU *p_db_pdu = (BLEPROFILE_DB_PDU *)cfa_mm_Alloc(sizeof(BLEPROFILE_DB_PDU));
                if (p_db_pdu != NULL)
                {
                	rscAppState->blersc_indication_sent = 1;
					p_db_pdu->len = length < sizeof (p_db_pdu->pdu) ? length : sizeof (p_db_pdu->pdu);
					memcpy(p_db_pdu->pdu, cprspHdr, p_db_pdu->len);
					bleappevt_serialize(running_speed_cadence_send_delayed_indication, p_db_pdu);
                }
            }
            else
            {
                ble_trace0("No Confirmation blocks Indication");
                return BLERSC_CP_PROCEDURE_ALREADY_IN_PROGRESS;
            }
        }
        else
        {
            return BLERSC_CP_CLIENT_CHAR_CONF_IMPROPERLY;
        }
    }
    return 0;
}

