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
* File Name: bletime.c
*
* Abstract: This file implements the BLE Time profile, service, application
*
* Functions:
*
*******************************************************************************/
#include "bletime.h"
#include "dbfw_app.h"

#define FID   FID_BLEAPP_APP__BLETIME_C
//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void bletime_Timeout(UINT32 count);
void bletime_FineTimeout(UINT32 finecount);
void bletime_handleUART(char *time_char);
void bletime_connUp(void);
void bletime_connDown(void);
void bletime_advStop(void);
void bletime_appTimerCb(UINT32 arg);
void bletime_appFineTimerCb(UINT32 arg);
void bletime_smpBondResult(LESMP_PARING_RESULT  result);
void bletime_encryptionChanged(HCI_EVT_HDR *evt);
void bletime_TimeRsp(int len, int attr_len, UINT8 *data);
void bletime_TimeWriteRsp(void);
void bletime_TimeNotification(int len, int attr_len, UINT8 *data);
void bletime_IntCb(UINT8 value);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const BLE_PROFILE_CFG bletime_cfg =
{
    1000, // UINT16 fine_timer_interval; //ms
    4, // HIGH_UNDIRECTED_DISCOVERABLE, // UINT8 default_adv; //default adv
    0, // UINT8 button_adv_toggle; //pairing button make adv toggle (if 1) or always on (if 0)
    32, //UINT16 high_undirect_adv_interval; //slots
    2048, //UINT16 low_undirect_adv_interval; //slots
    30, // UINT16 high_undirect_adv_duration; //seconds
    300, // UINT16 low_undirected_adv_duration; //seconds
    0, //UINT16 high_direct_adv_interval; //seconds
    0,  //UINT16 low_direct_adv_interval; //seconds
    0,  // UINT16 high_direct_adv_duration; //seconds
    0, //  UINT16 low_direct_adv_duration; //seconds
    "BLE Time watch", //char local_name[LOCAL_NAME_LEN_MAX];
    "\xC0\x00\x00", //char cod[COD_LEN];
    "1.00", //char ver[VERSION_LEN];
    0, // UINT8 encr_required; // if 1, encryption is needed before sending indication/notification
    0, // UINT8 disc_required;// if 1, disconnection after confirmation
    0, //UINT8 test_enable;   //TEST MODE is enabled when 1
    0x04,  //  UINT8 tx_power_level; //dbm
    120, // UINT8 con_idle_timeout; //second   0-> no timeout
    5, //    UINT8 powersave_timeout; //second  0-> no timeout
    {0x00,
      0x00,
      0x00,
      0x00,
      0x00}, // UINT16 hdl[HANDLE_NUM_MAX];   //GATT HANDLE number
    {UUID_SERVICE_CURRENT_TIME,
      0x00,
      0x00,
      0x00,
      0x00}, // UINT16 serv[HANDLE_NUM_MAX];  //GATT service UUID
    {0x00,
      0x00,
      0x00,
      0x00,
      0x00}, // UINT16 cha[HANDLE_NUM_MAX];   // GATT characteristic UUID
    0, // UINT8 findme_locator_enable; //if 1 Find me locator is enable
    0, // UINT8 findme_alert_level; //alert level of find me
    1, // UINT8 client_grouptype_enable; // if 1 grouptype read can be used
    0, // UINT8 linkloss_button_enable; //if 1 linkloss button is enable
    0, // UINT8 pathloss_check_interval; //second
    0, //UINT8 alert_interval; //interval of alert
    0, //UINT8 high_alert_num;     //number of alert for each interval
    0, //UINT8 mild_alert_num;     //number of alert for each interval
    0, // UINT8 status_led_enable;    //if 1 status LED is enable
    0, //UINT8 status_led_interval; //second
    0, // UINT8 status_led_con_blink; //blink num of connection
    0, // UINT8 status_led_dir_adv_blink; //blink num of dir adv
    0, //UINT8 status_led_un_adv_blink; //blink num of undir adv
    0, // UINT16 led_on_ms;  //led blink on duration in ms
    0, // UINT16 led_off_ms; //led blink off duration in ms
    0, // UINT16 buz_on_ms; //buzzer on duration in ms
    0, // UINT8 button_power_timeout; // seconds
    3, // UINT8 button_client_timeout; // seconds
    1, //UINT8 button_discover_timeout; // seconds
    10, //UINT8 button_filter_timeout; // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    15, //UINT8 button_uart_timeout; // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG bletime_puart_cfg =
{
    115200, // UINT32 baudrate;
    32, // 31, // 32, // UINT8  txpin; //GPIO pin number //20730A0 module need to use 32 instead, normally it is 31
    33, // UINT8  rxpin; //GPIO pin number
};

#if 1  // 1 : Dev board setting,  0 : module setting
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG bletime_gpio_cfg =
{
    {31, 14, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_HIGH/*LOW*/|GPIO_INT|GPIO_BUTTON,
      GPIO_INPUT|GPIO_INIT_HIGH/*LOW*/|GPIO_INT|GPIO_BUTTON2,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0}, // UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
};
#else
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG bletime_gpio_cfg =
{
    {31, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_INT|GPIO_BUTTON,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0,
      0}, // UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
};
#endif


tTimeAppState *timeAppState = NULL;


void bletime_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_Create Ver[5:2] = %02x %02x %02x %02x",  TVF_BBBB(bleprofile_p_cfg->ver[5], bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[3], bleprofile_p_cfg->ver[2]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_Create Ver[1:0] = %02x %02x", TVF_BB(bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[0]));

    timeAppState = (tTimeAppState *)cfa_mm_Sbrk(sizeof(tTimeAppState));
    memset(timeAppState, 0x00, sizeof(tTimeAppState));

    //initialize the default value of timeAppState
    timeAppState->bletime_client_state = TIMECLIENT_IDLE;
    timeAppState->bletime_ct_client_val = CCC_NONE;

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, bletime_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, bletime_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, bletime_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(bletime_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) bletime_smpBondResult);

    bletime_Init(NULL); //Callback should be set as proper function when callback function call is needed

    bleprofile_regButtonFunctionCb(bletime_TimeButton);
    bleprofile_regIntCb((BLEPROFILE_SINGLE_PARAM_CB) bletime_IntCb);

    bleprofile_regTimerCb(bletime_appFineTimerCb, bletime_appTimerCb);
    bleprofile_StartTimer();

    bletime_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}


void bletime_Init(LEATT_TRIPLE_PARAM_CB cb)
{
    timeAppState->bletime_cb = cb;

    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) bletime_TimeRsp);
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) bletime_TimeRsp);
    if(bleprofile_p_cfg->client_grouptype_enable)
    {
        leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) bletime_TimeRsp);
    }
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) bletime_TimeWriteRsp);
    leatt_regNotificationCb((LEATT_TRIPLE_PARAM_CB) bletime_TimeNotification);
}


void bletime_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);

    if(timeAppState->bletime_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}



void bletime_FineTimeout(UINT32 count)
{
    //ble_trace1("Finetimer:%d", finecount);
    {
        char time_char[READ_UART_LEN+1];

        //Reading
        bleprofile_ReadUART(time_char);
        //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ",
	 //      time_char[0], time_char[1], time_char[2], time_char[3], time_char[4], time_char[5]);

	 if(time_char[0] == 'D' && time_char[1] == 'D') //download start
	 {
            blecm_setFilterEnable(0);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Disable", TVF_D(0));
	 }
	 else if(time_char[0] == 'A' && time_char[1] == 'A') //download start
	 {
            blecm_setFilterEnable(1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Enalbe", TVF_D(0));

            bletime_connDown();
	 }
        else  //time main reading
        {
            bletime_handleUART(time_char);
        }
    }

    // button control
    //bletime_ReadButton();
    bleprofile_ReadButton();

    bletime_TimeReq();

}


void bletime_handleUART(char *time_char)
{


}


void bletime_connUp(void)
{
    bletime_TimeHandleReset();

    timeAppState->bletime_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(timeAppState->bletime_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(timeAppState->bletime_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_connUp[5:2]: %02x %02x %02x %02x", TVF_BBBB(timeAppState->bletime_remote_addr[5], timeAppState->bletime_remote_addr[4], timeAppState->bletime_remote_addr[3], timeAppState->bletime_remote_addr[2]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_connDown[1:0]: %02x %02x, handle: %04x", TVF_BBW(timeAppState->bletime_remote_addr[1], timeAppState->bletime_remote_addr[0], timeAppState->bletime_con_handle));

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void bletime_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_connDown[5:2]: %02x %02x %02x %02x",TVF_BBBB(timeAppState->bletime_remote_addr[5], timeAppState->bletime_remote_addr[4], timeAppState->bletime_remote_addr[3], timeAppState->bletime_remote_addr[2]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_connDown[1:0]: %02x %02x", TVF_BB(timeAppState->bletime_remote_addr[1], timeAppState->bletime_remote_addr[0]));

    // go back to Adv.
    // should be select after read NVRAM data
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);	//for test only


    // Mandatory discovery mode
    if(bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    // check NVRAM for previously paired BD_ADDR
    else
    {
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(timeAppState->bletime_hostinfo));

        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, timeAppState->bletime_hostinfo.bdAddr);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2]: %02x %02x %02x %02x", TVF_BBBB(timeAppState->bletime_hostinfo.bdAddr[5], timeAppState->bletime_hostinfo.bdAddr[4], timeAppState->bletime_hostinfo.bdAddr[3], timeAppState->bletime_hostinfo.bdAddr[2]));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[1:0]: %02x %02x %02x %02x", TVF_BB(timeAppState->bletime_hostinfo.bdAddr[1], timeAppState->bletime_hostinfo.bdAddr[0]));
    }

    timeAppState->bletime_con_handle = 0; //reset connection handle
}

void bletime_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV Stop", TVF_D(0));
}

void bletime_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (timeAppState->bletime_apptimer_count)++;

                bletime_Timeout(timeAppState->bletime_apptimer_count);
            }
            break;

    }
}

void bletime_appFineTimerCb(UINT32 arg)
{
    (timeAppState->bletime_appfinetimer_count)++;

    bletime_FineTimeout(timeAppState->bletime_appfinetimer_count);
}


void bletime_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime, bond result %02x", TVF_D(result));

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
	 // saving bd_addr in nvram
	 UINT8 writtenbyte;

        memcpy(timeAppState->bletime_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(timeAppState->bletime_remote_addr));
        memcpy(timeAppState->bletime_hostinfo.bdAddr, timeAppState->bletime_remote_addr, sizeof(BD_ADDR));

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(timeAppState->bletime_hostinfo));
        //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&(timeAppState->bletime_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void bletime_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Enc Changed", TVF_D(0));

    if(bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(timeAppState->bletime_hostinfo)))
    {
        if(memcmp(timeAppState->bletime_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6)==0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2]: %02x %02x %02x %02x", TVF_BBBB(timeAppState->bletime_hostinfo.bdAddr[5], timeAppState->bletime_hostinfo.bdAddr[4], timeAppState->bletime_hostinfo.bdAddr[3], timeAppState->bletime_hostinfo.bdAddr[2]));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[1:0]: %02x %02x %02x %02x", TVF_BB(timeAppState->bletime_hostinfo.bdAddr[5], timeAppState->bletime_hostinfo.bdAddr[4]));
        }
    }
}

UINT32 bletime_TimeButton(UINT32 function)
{
    if(function == BUTTON_CLIENT)
    {
        if(timeAppState->bletime_con_handle)
        {
            if(!timeAppState->bletime_ct_hdl) //first read
            {
                //read primary services
                timeAppState->bletime_s_handle = 1;
                timeAppState->bletime_e_handle = 0xFFFF;
                timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
            }
            else //there is saved handle
            {
                //Write Immediate Alert DB as 1 or 2
                timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CURRENTTIME_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
            }
        }
    }
    else if(function == BUTTON_DISCOVER)
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Discovery Btn Reset", TVF_D(0));
        timeAppState->bletime_con_handle = 0; //reset connection handle
    }

    return 0;
}

void bletime_IntCb(UINT8 value)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(INT)But1:%d But2:%d But3:%d", TVF_BBBB(value&0x01, (value&0x02)>>1, (value&0x04)>>2, 0));

    if(value&0x02)
    {
        if(timeAppState->bletime_con_handle)
        {
            if(timeAppState->bletime_ct_client_hdl)
            {
                if(timeAppState->bletime_ct_client_val==CCC_NONE)
                {
                    timeAppState->bletime_ct_client_val=CCC_NOTIFICATION;
                }
                else
                {
                    timeAppState->bletime_ct_client_val=CCC_NONE;
                }

                bleprofile_sendWriteReq(timeAppState->bletime_ct_client_hdl, (UINT8 *)&(timeAppState->bletime_ct_client_val), 2);
                //bleprofile_sendWriteCmd(timeAppState->bletime_ct_client_hdl, (UINT8 *)&(timeAppState->bletime_ct_client_val), 2);
            }
        }
    }
}

void bletime_TimeHandleReset(void)
{
    timeAppState->bletime_ct_hdl = 0;
    timeAppState->bletime_ct_client_hdl = 0;
}


void bletime_TimeReq(void)
{
    //handle deferred Read or Write
    if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER)
    {
        timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_PRIMARY_SERVICE;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
        if(bleprofile_p_cfg->client_grouptype_enable)
        {
            bleprofile_sendReadByGroupTypeReq(timeAppState->bletime_s_handle, timeAppState->bletime_e_handle,
					UUID_ATTRIBUTE_PRIMARY_SERVICE);
        }
        else
        {
            bleprofile_sendReadByTypeReq(timeAppState->bletime_s_handle, timeAppState->bletime_e_handle,
					UUID_ATTRIBUTE_PRIMARY_SERVICE);
        }
    }
    else if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_CHARACTERISTIC_DEFER)
    {
        timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CHARACTERISTIC;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
        bleprofile_sendReadByTypeReq(timeAppState->bletime_s_handle, timeAppState->bletime_e_handle,
					UUID_ATTRIBUTE_CHARACTERISTIC);
    }
    else if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_DESC_DEFER)
    {
        timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_DESC;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
        bleprofile_sendReadByTypeReq(timeAppState->bletime_s_handle, timeAppState->bletime_e_handle,
			UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
    }
    else if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_CURRENTTIME_DEFER)
    {
        timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CURRENTTIME;
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
        bleprofile_sendReadReq(timeAppState->bletime_ct_hdl);
    }
}

void bletime_TimeRsp(int len, int attr_len, UINT8 *data)
{
    UINT16 uuid=0;
    int i, found;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime, ReadRsp:%d, %d", TVF_WW(len, attr_len));
    ble_tracen((char *)data, len);

    if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_PRIMARY_SERVICE)
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
                    if(uuid == UUID_SERVICE_CURRENT_TIME)
                    {
                        found = i; //start position of uuid
		          break;
                    }
                }

                if(found != -1)
                {
                    //start handle calculation
                    timeAppState->bletime_s_handle = data[found] + (data[found+1]<<8);
                    timeAppState->bletime_e_handle = data[found+2] + (data[found+3]<<8);

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found alertservice - uuid:%04x, s_hdl:%04x", TVF_WW(uuid, timeAppState->bletime_s_handle));

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP,  "Found alertservice - e_hdl:%04x", TVF_D(timeAppState->bletime_e_handle));

                    //using defered way
                    timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CHARACTERISTIC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
                }
                else
                {
                    //retry reading
                    //find last handle
                    timeAppState->bletime_s_handle = data[len-attr_len+2] + (data[len-attr_len+3]<<8)+1;
                    timeAppState->bletime_e_handle = 0xFFFF;
                    timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
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
                    if(uuid == UUID_SERVICE_CURRENT_TIME)
                    {
                        found = i; //start position of uuid
                        break;
                    }
                }

                if(found != -1)
                {
                    //start handle calculation
                    timeAppState->bletime_s_handle = data[found] + (data[found+1]<<8);

                    //end handle calculation
                    if(found +attr_len< len)
                    {
                        timeAppState->bletime_e_handle = data[found+attr_len] + (data[found+attr_len+1]<<8) -1;
                    }
                    else
                    {
                        timeAppState->bletime_e_handle = 0xFFFF;
                    }
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found alertservice - uuid:%04x, s_hdl:%04x",  TVF_WW(uuid, timeAppState->bletime_s_handle));
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found alertservice - ehdl::%04x", TVF_D(timeAppState->bletime_e_handle));

                    //using defered way
                    timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CHARACTERISTIC_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
                }
                else
                {
                    //retry reading
                    //find last handle
                    timeAppState->bletime_s_handle = data[len-attr_len] + (data[len-(attr_len-1)]<<8)+1;
                    timeAppState->bletime_e_handle = 0xFFFF;
                    timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_PRIMARY_SERVICE_DEFER;
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
                }
            }
        }
        else
        {
            timeAppState->bletime_client_state = TIMECLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state = %02x", TVF_D(timeAppState->bletime_client_state));
        }
    }
    else if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_CHARACTERISTIC)
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
                if(uuid == UUID_CHARACTERISTIC_CURRENT_TIME)
                {
                    found = i; //start position of uuid
                    break;
                }
            }

            if(found != -1)
            {
                //calculate handle
                timeAppState->bletime_ct_hdl = data[found+3] + (data[found+4]<<8);

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Found currenttime_cha - uuid:%04x, hdl:%04x", TVF_WW(uuid, timeAppState->bletime_ct_hdl));

                //find descriptor
                timeAppState->bletime_s_handle = timeAppState->bletime_ct_hdl+1;
                //blecli_e_handle = ; //using previous value
                timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_DESC_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state=%02x", TVF_D(timeAppState->bletime_client_state));
            }
            else
            {
                //try again with defered way
                timeAppState->bletime_s_handle = data[len-attr_len] + (data[len-(attr_len-1)]<<8)+1;
                //timeAppState->bletime_e_handle = ; //using previous value
                timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CHARACTERISTIC_DEFER;
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state=%02x", TVF_D(timeAppState->bletime_client_state));
            }
        }
        else
        {
            timeAppState->bletime_client_state = TIMECLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state=%02x", TVF_D(timeAppState->bletime_client_state));
        }
    }
    else if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_DESC)
    {
        if(len)
        {
            timeAppState->bletime_ct_client_hdl = data[0] + (data[1]<<8);

            // show the received data
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_ct_client_hdl=%04x", TVF_D(timeAppState->bletime_ct_client_hdl));

            //Read Current Time
            timeAppState->bletime_client_state = TIMECLIENT_WAIT_READ_CURRENTTIME_DEFER;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state=%02x", TVF_D(timeAppState->bletime_client_state));
        }
    }
    else if(timeAppState->bletime_client_state == TIMECLIENT_WAIT_READ_CURRENTTIME)
    {
        if(len)
        {
            // show the received time
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(Date) %d/%d/%d",  TVF_BBW(data[2], data[3], data[0]+((data[1])<<8)));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(Time) %d:%d:%d.(%d/256)", TVF_BBBB(data[4], data[5], data[6], data[7]));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(Day) %d,  Reason:%x", TVF_WW(data[8], data[9]));

            if(timeAppState->bletime_cb)
            {
                timeAppState->bletime_cb(len, attr_len, data);
            }

            timeAppState->bletime_client_state = TIMECLIENT_IDLE;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_client_state=%02x", TVF_D(timeAppState->bletime_client_state));
        }
    }
}


void bletime_TimeWriteRsp(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Write Rsp", TVF_D(0));
}

void bletime_TimeNotification(int len, int attr_len, UINT8 *data)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime, Notification:%02x, %d", TVF_WW((UINT16)attr_len, len));
    ble_tracen((char *)data, len);

    if(len)
    {
        // show the received time
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(Date) %d/%d/%d", TVF_BBW(data[2], data[3], data[0]+((data[1])<<8)));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(Time) %d:%d:%d.(%d/256)", TVF_BBBB(data[4], data[5], data[6], data[7]));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(Day) %d,  Reason:%x", TVF_WW(data[8], data[9]));
    }
}
