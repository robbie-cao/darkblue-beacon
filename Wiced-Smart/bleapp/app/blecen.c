/*******************************************************************************
* THIS INFORMATION IS PROPRIETARY TO BROADCOM CORP
*
* ------------------------------------------------------------------------------
*
* Copyright (c) 2012 Broadcom Corp.
*
*          ALL RIGHTS RESERVED
*
********************************************************************************
*
* File Name: blecen.c
*
* Abstract: This file implements the BLE Generic Central profile, service, application
*
* Functions:
*
*******************************************************************************/
#include "blecen.h"
#include "hciulp.h"
#include "blecm.h"
#include "lel2cap.h"
#include "blecli.h"
#include "lesmp.h"
#include "dbfw_app.h"

#define FID  FID_BLEAPP_APP__BLECEN_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blecen_Timeout(UINT32 count);
void blecen_FineTimeout(UINT32 finecount);
void blecen_handleUART(char *cen_char);
void blecen_connUp(void);
void blecen_connDown(void);
void blecen_appTimerCb(UINT32 arg);
void blecen_appFineTimerCb(UINT32 arg);
void blecen_smpBondResult(LESMP_PARING_RESULT  result);
void blecen_encryptionChanged(HCI_EVT_HDR *evt);
void blecen_IntCb(UINT8 value);
void blecen_leAdvReportCb(HCIULP_ADV_PACKET_REPORT_WDATA *evt);

int blecen_FindAdvInfoIndex(BD_ADDR addr, UINT8 adr_type);
void blecen_AdvDataParse(UINT8 *data, UINT8 dataLen, UINT8 index);

int blecenhandleConnParamUpdateReq(LEL2CAP_HDR *l2capHdr);

void blecen_readCb(int len, int attr_len, UINT8 *data);
void blecen_writeCb(int len, int attr_len, UINT8 *data);
void blecen_Notification(int len, int attr_len, UINT8 *data);


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const BLE_PROFILE_CFG blecen_cfg =
{
    500, // UINT16 fine_cenr_interval; //ms
    0, // NO_DISCOVERABLE, // UINT8 default_adv; //default adv
    0, // UINT8 button_adv_toggle; //pairing button make adv toggle (if 1) or always on (if 0)
    32, //UINT16 high_undirect_adv_interval; //slots
    2048, //UINT16 low_undirect_adv_interval; //slots
    0, // UINT16 high_undirect_adv_duration; //seconds
    0, // UINT16 low_undirected_adv_duration; //seconds
    0, //UINT16 high_direct_adv_interval; //seconds
    0,  //UINT16 low_direct_adv_interval; //seconds
    0,  // UINT16 high_direct_adv_duration; //seconds
    0, //  UINT16 low_direct_adv_duration; //seconds
    "BLE Central", //char local_name[LOCAL_NAME_LEN_MAX];
    "\x00\x00\x00", //char cod[COD_LEN];
    "1.00", //char ver[VERSION_LEN];
    0, // UINT8 encr_required; // if 1, encryption is needed before sending indication/notification
    0, // UINT8 disc_required;// if 1, disconnection after confirmation
    0, //UINT8 test_enable;   //TEST MODE is enabled when 1
    0x00,  //  UINT8 tx_power_level; //dbm
    120, // UINT8 con_idle_timeout; //second   0-> no cenout
    0, //    UINT8 powersave_timeout; //second  0-> no cenout
    {0x00,
      0x00,
      0x00,
      0x00,
      0x00}, // UINT16 hdl[HANDLE_NUM_MAX];   //GATT HANDLE number
    {0x00,
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
    0, // UINT8 button_power_cenout; // seconds
    1, // UINT8 button_client_cenout; // seconds
    3, //UINT8 button_discover_cenout; // seconds
    10, //UINT8 button_filter_cenout; // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    15, //UINT8 button_uart_cenout; // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blecen_puart_cfg =
{
    115200, // UINT32 baudrate;
    32, // 31, // 32, // UINT8  txpin; //GPIO pin number //20730A0 module need to use 32 instead, normally it is 31
    33, // UINT8  rxpin; //GPIO pin number
};

PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blecen_gpio_cfg =
{
    {31, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_HIGH/*LOW*/|GPIO_INT|GPIO_BUTTON,
      GPIO_INPUT|GPIO_INIT_HIGH/*LOW*/|GPIO_INT|GPIO_BUTTON2,
      GPIO_INPUT|GPIO_INIT_HIGH/*LOW*/|GPIO_INT|GPIO_BUTTON3,
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

BLE_CEN_CFG blecen_cen_cfg =
{
    HCIULP_ACTIVE_SCAN, //HCIULP_PASSIVE_SCAN, // UINT8 scan_type;
    HCIULP_PUBLIC_ADDRESS, // UINT8 scan_adr_type;
    HCIULP_SCAN_FILTER_POLICY_WHITE_LIST_NOT_USED, // UINT8 scan_filter_policy;
    HCIULP_SCAN_DUPLICATE_FILTER_ON, //HCIULP_SCAN_DUPLICATE_FILTER_OFF, // UINT8 filter_duplicates;
    HCIULP_INITIATOR_FILTER_POLICY_WHITE_LIST_NOT_USED, // UINT8 init_filter_policy;
    HCIULP_PUBLIC_ADDRESS, // UINT8 init_addr_type;
    96, // UINT16 high_scan_interval; //slots
    2048, // UINT16 low_scan_interval; //slots
    48, // UINT16 high_scan_window; //slots
    18, // UINT16 low_scan_window; //slots
    30, // UINT16 high_scan_duration; //seconds
    300,// UINT16 low_scan_duration; //seconds
    40, //UINT16 high_conn_min_interval;  //frames
    400, //UINT16 low_conn_min_interval;  //frames
    56, //UINT16 high_conn_max_interval; //frames
    560, //UINT16 low_conn_max_interval; //frames
    0, //UINT16 high_conn_latency; //number of connection event
    0, //UINT16 low_conn_latency; //number of connection event
    10, //UINT16 high_supervision_timeout; // N * 10ms
    100, //UINT16 low_supervision_timeout; // N * 10ms
    0, //UINT16 conn_min_event_len; // slots
    0, //UINT16 conn_max_event_len; // slots
} ;

UINT16 blecen_con_handle;

typedef struct
{
    //NVRAM save area
    BLEPROFILE_HOSTINFO blecen_hostinfo;

    UINT32 blecen_apptimer_count;
    UINT32 blecen_appfinetimer_count;
    UINT16 blecen_cli_cfg;
    BD_ADDR  blecen_remote_addr;
    UINT8 blecen_remote_addr_type;

    UINT8 blecen_scan;
    UINT8 blecen_conn;
    UINT8 blecen_conn_speed;
    int blecen_scantimer_id;
    int blecen_conntimer_id;

    UINT32 blecen_adv_info_valid;
    UINT8 blecen_adv_info_index;
    BLE_CEN_ADV_INFO blecen_adv_info[ADV_INFO_MAX];
} tCenAppState;

tCenAppState *cenAppState = NULL;

BLEAPP_TIMER_CB blecen_usertimerCb=NULL;

typedef int (*LEL2CAP_MSGHANDLER)(LEL2CAP_HDR*);
extern LEL2CAP_MSGHANDLER lel2cap_handleConnParamUpdateReq;


void blecen_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecen_Create Ver[0:3]:%d %d %d %d", TVF_BBBB(bleprofile_p_cfg->ver[0],  bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[2], bleprofile_p_cfg->ver[3]));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ver[4:5]:%d %d", TVF_BB(bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[5]));

    cenAppState = (tCenAppState *)cfa_mm_Sbrk(sizeof(tCenAppState));
    memset(cenAppState, 0x00, sizeof(tCenAppState));

    //initialize the default value of cenAppState
    cenAppState->blecen_scantimer_id = -1;
    cenAppState->blecen_conntimer_id = -1;
    cenAppState->blecen_cli_cfg = CCC_NOTIFICATION;

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blecen_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blecen_connDown );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blecen_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blecen_smpBondResult);

    bleprofile_regButtonFunctionCb(blecen_CenButton);
    bleprofile_regIntCb((BLEPROFILE_SINGLE_PARAM_CB) blecen_IntCb);

    // handler for Adv Report
    blecm_RegleAdvReportCb((BLECM_FUNC_WITH_PARAM)blecen_leAdvReportCb);

    // handler for connection update req
    lel2cap_handleConnParamUpdateReq = blecenhandleConnParamUpdateReq;

    // blecli initialization
    blecli_Init();

    // notification callback register
    leatt_regNotificationCb((LEATT_TRIPLE_PARAM_CB) blecen_Notification);

    // smp role
    lesmp_setSMPRole(LESMP_ROLE_INITIATOR);

    bleprofile_regTimerCb(blecen_appFineTimerCb, blecen_appTimerCb);
    bleprofile_StartTimer();

    blecen_connDown();
}

void blecen_Timeout(UINT32 count)
{
    bleprofile_pollPowersave();

#if 0
{
    int i;
    for(i=0; i<ADV_INFO_MAX; i++)
    {
        ble_tracen((char *)(&cenAppState->blecen_adv_info[i]), sizeof(BLE_CEN_ADV_INFO));
    }
}
#endif

#if 0
{
    extern LESMP_STATE lesmp_state;
    ble_trace1("lesmp_state=%d", lesmp_state);
}
#endif
}

void blecen_FineTimeout(UINT32 count)
{
    {
        char cen_char[READ_UART_LEN+1];

        //Reading
        bleprofile_ReadUART(cen_char);

        if(cen_char[0] == 'D' && cen_char[1] == 'D') //download start
        {
            blecm_setFilterEnable(0);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Disable", TVF_D(0));
        }
        else if(cen_char[0] == 'A' && cen_char[1] == 'A') //download start
        {
            blecm_setFilterEnable(1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA FilterEnable", TVF_D(0));
            blecen_connDown();
        }
        else  //cen main reading
        {
            blecen_handleUART(cen_char);
        }
    }

    // button control
    bleprofile_ReadButton();

    // client request is sending with this one
    blecli_ClientReq();
}


void blecen_handleUART(char *cen_char)
{


}


void blecen_connUp(void)
{
    blecen_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(cenAppState->blecen_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(cenAppState->blecen_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecen_connUp[5:2]: %02x %02x %02x %02x", TVF_BBBB(cenAppState->blecen_remote_addr[5], cenAppState->blecen_remote_addr[4], cenAppState->blecen_remote_addr[3], cenAppState->blecen_remote_addr[2] ) );
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecen_connUp[1:0]: %02x %02x, handle: %04x", TVF_BBW(cenAppState->blecen_remote_addr[1], cenAppState->blecen_remote_addr[0], blecen_con_handle));

    blecen_Conn(NO_CONN, cenAppState->blecen_remote_addr, cenAppState->blecen_remote_addr_type);

    // reset GATT handle
    blecli_ClientHandleReset();

#if 0
    lesmp_startPairing(NULL);
#endif
}

void blecen_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecen_connDown[5:2]: %02x %02x %02x %02x", TVF_BBBB(cenAppState->blecen_remote_addr[5], cenAppState->blecen_remote_addr[4], cenAppState->blecen_remote_addr[3], cenAppState->blecen_remote_addr[2] ) );
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecen_connDown[1:0]: %02x %02x, handle: %04x", TVF_BBW(cenAppState->blecen_remote_addr[1], cenAppState->blecen_remote_addr[0], blecen_con_handle));

    blecen_con_handle = 0; //reset connection handle

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ScanOn", TVF_D(0));

    blecen_Scan(HIGH_SCAN);
    //blecm_startScan(HCIULP_PASSIVE_SCAN, 96, 48, HCIULP_PUBLIC_ADDRESS,
    //    HCIULP_SCAN_FILTER_POLICY_WHITE_LIST_NOT_USED, HCIULP_SCAN_DUPLICATE_FILTER_OFF);
}

void blecen_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                (cenAppState->blecen_apptimer_count)++;

                blecen_Timeout(cenAppState->blecen_apptimer_count);
            }
            break;

    }
}

void blecen_appFineTimerCb(UINT32 arg)
{
    (cenAppState->blecen_appfinetimer_count)++;

    blecen_FineTimeout(cenAppState->blecen_appfinetimer_count);
}


void blecen_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blecen, bond result %02x", TVF_D(result));

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
	 // saving bd_addr in nvram
        UINT8 writtenbyte;

        memcpy(cenAppState->blecen_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(cenAppState->blecen_remote_addr));
        memcpy(cenAppState->blecen_hostinfo.bdAddr, cenAppState->blecen_remote_addr, sizeof(BD_ADDR));

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(cenAppState->blecen_hostinfo));
        //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&(cenAppState->blecen_hostinfo));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blecen_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "enc Chng", TVF_D(0));

    if(bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&(cenAppState->blecen_hostinfo)))
    {
        if(memcmp(cenAppState->blecen_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6)==0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2]: %02x %02x %02x %02x", TVF_BBBB(cenAppState->blecen_hostinfo.bdAddr[5], cenAppState->blecen_hostinfo.bdAddr[4], cenAppState->blecen_hostinfo.bdAddr[3], cenAppState->blecen_hostinfo.bdAddr[2] ));

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[1:0]: %02x %02x", TVF_BB(cenAppState->blecen_hostinfo.bdAddr[1], cenAppState->blecen_hostinfo.bdAddr[0]));
        }
    }
}

UINT32 blecen_CenButton(UINT32 function)
{
    if(function == BUTTON_CLIENT)
    {
        if(blecen_con_handle)
        {
#if 0
            if(blecli_ClientButton(UUID_SERVICE_DEVICE_INFORMATION, UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING,
                                   0, CLIENT_READREQ, NULL, 0, (LEATT_TRIPLE_PARAM_CB)blecen_readCb))
#else
            if(blecli_ClientButton(UUID_SERVICE_HEART_RATE, UUID_CHARACTERISTIC_HEART_RATE_MEASUREMENT,
                                   UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                   CLIENT_WRITEREQ, (UINT8 *)&(cenAppState->blecen_cli_cfg), 2, (LEATT_TRIPLE_PARAM_CB)blecen_writeCb))
#endif
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Client Button Failed", TVF_D(0));
            }
        }
    }
    else if(function == BUTTON_DISCOVER)
    {
        blecen_Scan(HIGH_SCAN);

        //connDown() function is not called when button is pressed during connection state
        //ADV is turned on at the same time, but turned off after high+low duration later.
        // high_duration = 0, low_duration = 0, then ADV will stop 2 seconds later.
        blecen_con_handle = 0; //reset connection handle
    }

    return 0;
}

void blecen_IntCb(UINT8 value)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "(INT)But1:%d But2:%d But3:%d", TVF_BBW(value&0x01, (value&0x02)>>1, (value&0x04)>>2));

    if(value&0x02)
    {
        blecen_Scan(NO_SCAN);
        blecen_Conn(HIGH_CONN, cenAppState->blecen_remote_addr, cenAppState->blecen_remote_addr_type);
    }

    if(value&0x04)
    {
        if(blecen_con_handle)
        {
            // change connection interval

            if(cenAppState->blecen_conn_speed == FAST_CONN)
            {
                cenAppState->blecen_conn_speed = SLOW_CONN;
            }
            else
            {
                cenAppState->blecen_conn_speed = FAST_CONN;
            }
            blecen_ConnUpdate(cenAppState->blecen_conn_speed);
        }
        else
        {
            //select connecting device
            if(cenAppState->blecen_adv_info_valid&(1<<cenAppState->blecen_adv_info_index))
            {
                memcpy(cenAppState->blecen_remote_addr, cenAppState->blecen_adv_info[cenAppState->blecen_adv_info_index].peer_addr, 6);
                cenAppState->blecen_remote_addr_type = cenAppState->blecen_adv_info[cenAppState->blecen_adv_info_index].peer_addr_type;

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "adv info selected[5:2]: %02x %02x %02x %02x", TVF_BBBB(cenAppState->blecen_remote_addr[5], cenAppState->blecen_remote_addr[4], cenAppState->blecen_remote_addr[3], cenAppState->blecen_remote_addr[2] ));
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "adv info selected[1:0]: %02x %02x, handle: %04x", TVF_BBW(cenAppState->blecen_remote_addr[1], cenAppState->blecen_remote_addr[0], cenAppState->blecen_remote_addr_type));
            }
            else
            {
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "noAdvInfo", TVF_D(0));
            }

            (cenAppState->blecen_adv_info_index)++;
            if(cenAppState->blecen_adv_info_index == ADV_INFO_MAX)
            {
                cenAppState->blecen_adv_info_index = 0;
            }
        }
    }

}

void blecen_leAdvReportCb(HCIULP_ADV_PACKET_REPORT_WDATA *evt)
{
    int index;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV:EvtType:%d, AddrType:%d, Len:%d, RSSI: %d", TVF_BBBB(evt->eventType, evt->addressType, evt->dataLen, evt->rssi));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BDADDR[5:2]: %02x %02x %02x %02x", TVF_BBBB(evt->wd_addr[5], evt->wd_addr[4], evt->wd_addr[3], evt->wd_addr[2]));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BDADDR[1:0]: %02x %02x", TVF_BB(evt->wd_addr[1], evt->wd_addr[0]));

    ble_tracen((char *)evt->data, evt->dataLen);

    index = blecen_FindAdvInfoIndex((evt->wd_addr), evt->addressType);
    blecen_AdvDataParse((UINT8 *)(evt->data), (UINT8)(evt->dataLen), index);
}

int blecen_FindAdvInfoIndex(BD_ADDR addr, UINT8 adr_type)
{
    int i;

    //find valid and same bd_addr
    for(i=0; i<ADV_INFO_MAX; i++)
    {
        if(cenAppState->blecen_adv_info_valid&(1<<i))
        {
            if(memcmp(cenAppState->blecen_adv_info[i].peer_addr, addr, 6)==0)
            {
                if(cenAppState->blecen_adv_info[i].peer_addr_type == adr_type)
                {
                     return i;
                }
            }
        }
    }

    // not existing case, find first one and send
    for(i=0; i<ADV_INFO_MAX; i++)
    {
        if((cenAppState->blecen_adv_info_valid&(1<<i))==0)
        {
            cenAppState->blecen_adv_info_valid |= (1<<i);
            memcpy(cenAppState->blecen_adv_info[i].peer_addr, addr, 6);
            cenAppState->blecen_adv_info[i].peer_addr_type = adr_type;

            return i;
        }
    }

    // all used up, return 0
    //cenAppState->blecen_adv_info_valid |= 1; // this is already set
    memcpy(cenAppState->blecen_adv_info[0].peer_addr, addr, 6);
    cenAppState->blecen_adv_info[0].peer_addr_type = adr_type;

    return 0;
}

void blecen_AdvDataParse(UINT8 *data, UINT8 dataLen, UINT8 index)
{
    BLE_ADV_FIELD *p_field;

    UINT8 *ptr = data;

    while(1)
    {
        p_field = (BLE_ADV_FIELD *)ptr;

        if(p_field->val == ADV_FLAGS)
        {
            memcpy(&(cenAppState->blecen_adv_info[index].flags), p_field->data, 1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "FLAGS", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID16_MORE)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "16 MORE", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID16_COMP)
        {
            memcpy((void *)&(cenAppState->blecen_adv_info[index].uuid16), p_field->data, 2);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "16 COMP", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID32_MORE)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "32 MORE", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID32_COMP)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "32 COMP", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID128_MORE)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "128 MORE", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID128_COMP)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "128 COMP", TVF_D(0));
        }
        else if(p_field->val == ADV_LOCAL_NAME_SHORT)
        {
            if(p_field->len > ADV_LEN_MAX-2)
            {
                p_field->len = ADV_LEN_MAX-2;
            }
            memcpy(cenAppState->blecen_adv_info[index].name, p_field->data, p_field->len-1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NSHRT", TVF_D(0));
        }
        else if(p_field->val == ADV_LOCAL_NAME_COMP)
        {
            if(p_field->len > ADV_LEN_MAX-2)
            {
                p_field->len = ADV_LEN_MAX-2;
            }
            memcpy(cenAppState->blecen_adv_info[index].name, p_field->data, p_field->len-1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NCMP", TVF_D(0));
        }
        else if(p_field->val == ADV_TX_POWER_LEVEL)
        {
            memcpy(&(cenAppState->blecen_adv_info[index].tx_level), p_field->data, 1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "PWRLVL", TVF_D(0));
        }
        else if(p_field->val == ADV_CLASS_OF_DEVICE)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "COD", TVF_D(0));
        }
        else if(p_field->val == ADV_SIMPLE_PAIRING_HASH_C)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "HSHC", TVF_D(0));
        }
        else if(p_field->val == ADV_SIMPLE_PAIRING_RANDOMIZER_R)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RANDR", TVF_D(0));
        }
        else if(p_field->val == ADV_TK_VALUE)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "TKVAL", TVF_D(0));
        }
        else if(p_field->val == ADV_OOB_FLAGS)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "OOBFLGS", TVF_D(0));
        }
        else if(p_field->val == ADV_SLAVE_CONNECTION_INTERVAL_RANGE)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "INTRANGE", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID16)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "UUID16", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_UUID128)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "UUID128", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_DATA)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "SRVDATA", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_TARGET_PUBLIC_ADR)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "PUBADR", TVF_D(0));
        }
        else if(p_field->val == ADV_SERVICE_TARGET_RANDOM_ADR)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RANDADR", TVF_D(0));
        }
        else if(p_field->val == ADV_MANUFACTURER_DATA)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "MANUF", TVF_D(0));
        }

        if((p_field->len < ADV_LEN_MAX-2) && (p_field->len > 1))
        {
            if((p_field->val == ADV_LOCAL_NAME_SHORT)||
                (p_field->val == ADV_LOCAL_NAME_COMP))
            {
                ble_tracen(p_field->data, p_field->len);
            }
            else
            {
                ble_tracen((char *)(p_field->data), p_field->len-1);
            }
        }

        ptr += (p_field->len)+1;

        if(ptr >= data+dataLen)
        {
            break;
        }
    }
}

UINT8 blecen_GetScan(void)
{
    // return NO_SCAN or LOW_SCAN or HIGH_SCAN
    return (cenAppState->blecen_scan&0x03);
}

void blecen_Scan(UINT8 scan_mode)
{
    if( cenAppState->blecen_scantimer_id >= 0 )
    {
        // we are going to stop the timer.
        bleapptimer_stopAppTimer(cenAppState->blecen_scantimer_id);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ScanTimer stopped: %d", TVF_D(cenAppState->blecen_scantimer_id));
        cenAppState->blecen_scantimer_id = -1;
    }

    // turn off SCAN
    blecm_setScanEnable(HCIULP_SCAN_MODE_OFF, blecen_cen_cfg.filter_duplicates);

    if (scan_mode == NO_SCAN)
    {
        cenAppState->blecen_scan = NO_SCAN;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NoScan", TVF_D(0));
    }
    else if (scan_mode == HIGH_SCAN)
    {
        ///////////////////////////////////////////////////////////////////
        // start a timer for SCAN
        cenAppState->blecen_scantimer_id = bleapptimer_startAppTimer(
                blecen_ScanTimerCb,
                blecen_cen_cfg.high_scan_duration,
                BLEAPP_APP_TIMER_SCAN);

        // start scan
        blecm_startScan(blecen_cen_cfg.scan_type,
            blecen_cen_cfg.high_scan_interval, blecen_cen_cfg.high_scan_window,
            blecen_cen_cfg.scan_adr_type, blecen_cen_cfg.scan_filter_policy,
            blecen_cen_cfg.filter_duplicates);

        cenAppState->blecen_scan = HIGH_SCAN;
        cenAppState->blecen_scan |= ((blecen_cen_cfg.scan_type) <<2); //adding active or passive scan bit

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BLE_high_scan:timer(%d)", TVF_D(cenAppState->blecen_scantimer_id));
    }
    else if (scan_mode == LOW_SCAN)
    {
        ///////////////////////////////////////////////////////////////////
        // start a timer for SCAN
        cenAppState->blecen_scantimer_id = bleapptimer_startAppTimer(
                blecen_ScanTimerCb,
                blecen_cen_cfg.low_scan_duration,
                BLEAPP_APP_TIMER_SCAN );

        // start scan
        blecm_startScan(blecen_cen_cfg.scan_type,
            blecen_cen_cfg.low_scan_interval, blecen_cen_cfg.low_scan_window,
            blecen_cen_cfg.scan_adr_type, blecen_cen_cfg.scan_filter_policy,
            blecen_cen_cfg.filter_duplicates);

        cenAppState->blecen_scan = LOW_SCAN;
        cenAppState->blecen_scan |= ((blecen_cen_cfg.scan_type) <<2); //adding active or passive scan bit

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BLE_low_scan:timer(%d)", TVF_D(cenAppState->blecen_scantimer_id));
    }
}

void blecen_ScanTimerCb( UINT32 arg)
{
    //use user defined timerCb
    if(blecen_usertimerCb)
    {
        blecen_usertimerCb(arg);

        return;
    }
	
    switch(arg)
    {
        case BLEAPP_APP_TIMER_SCAN:
            {
                if(blecen_GetScan() == HIGH_SCAN)
                {
                    blecen_Scan(LOW_SCAN);
                }
                else if(blecen_GetScan() == LOW_SCAN)
                {
                    blecen_Scan(NO_SCAN);

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ScanStop", TVF_D(0));
                }
            }
            break;
    }
}

UINT8 blecen_GetConn(void)
{
    return (cenAppState->blecen_conn);
}

void blecen_Conn(UINT8 conn_mode, BD_ADDR peer_addr, UINT8 peer_addr_type)
{
    if( cenAppState->blecen_conntimer_id >= 0 )
    {
        // we are going to stop the timer.
        bleapptimer_stopAppTimer(cenAppState->blecen_conntimer_id);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnTimer stopped: %d", TVF_D(cenAppState->blecen_conntimer_id));
        cenAppState->blecen_conntimer_id = -1;
    }

    // cancel connection
    blecm_CreateConnectionCancel();

    if (conn_mode == NO_CONN)
    {
        cenAppState->blecen_conn = NO_CONN;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NoConn", TVF_D(0));
    }
    else if (conn_mode == HIGH_CONN)
    {
        ///////////////////////////////////////////////////////////////////
        // start a timer for conn
        cenAppState->blecen_conntimer_id = bleapptimer_startAppTimer(
                blecen_ConnTimerCb,
                blecen_cen_cfg.high_scan_duration,
                BLEAPP_APP_TIMER_CONN);

        // start conn
        blecm_CreateConnection(
            blecen_cen_cfg.high_scan_interval,
            blecen_cen_cfg.high_scan_window,
            blecen_cen_cfg.init_filter_policy,
            peer_addr_type,
            (char *)peer_addr,
            blecen_cen_cfg.init_addr_type,
            blecen_cen_cfg.high_conn_min_interval,
            blecen_cen_cfg.high_conn_max_interval,
            blecen_cen_cfg.high_conn_latency,
            blecen_cen_cfg.high_supervision_timeout,
            blecen_cen_cfg.conn_min_event_len,
            blecen_cen_cfg.conn_max_event_len
        );

        cenAppState->blecen_conn = HIGH_CONN;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BLE_high_conn:timer(%d)", TVF_D(cenAppState->blecen_conntimer_id));
    }
    else if (conn_mode == LOW_CONN)
    {
        ///////////////////////////////////////////////////////////////////
        // start a timer for SCAN
        cenAppState->blecen_conntimer_id = bleapptimer_startAppTimer(
                blecen_ConnTimerCb,
                blecen_cen_cfg.low_scan_duration,
                BLEAPP_APP_TIMER_CONN );

        // start conn
        // start conn
        blecm_CreateConnection(
            blecen_cen_cfg.low_scan_interval,
            blecen_cen_cfg.low_scan_window,
            blecen_cen_cfg.init_filter_policy,
            peer_addr_type,
            (char *)peer_addr,
            blecen_cen_cfg.init_addr_type,
            blecen_cen_cfg.high_conn_min_interval, //initially connection uses high connection parameter
            blecen_cen_cfg.high_conn_max_interval,
            blecen_cen_cfg.high_conn_latency,
            blecen_cen_cfg.high_supervision_timeout,
            blecen_cen_cfg.conn_min_event_len,
            blecen_cen_cfg.conn_max_event_len
        );

        cenAppState->blecen_conn = LOW_CONN;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BLE_low_conn:timer(%d)", TVF_D(cenAppState->blecen_conntimer_id));
    }
}

void blecen_ConnTimerCb( UINT32 arg)
{
    //use user defined timerCb
    if(blecen_usertimerCb)
    {
        blecen_usertimerCb(arg);

        return;
    }
	
    switch(arg)
    {
        case BLEAPP_APP_TIMER_CONN:
            {
                if(blecen_GetConn() == HIGH_CONN)
                {
                    blecen_Conn(LOW_CONN, cenAppState->blecen_remote_addr, cenAppState->blecen_remote_addr_type);
                }
                else if(blecen_GetConn() == LOW_CONN)
                {
                    blecen_Conn(NO_CONN, cenAppState->blecen_remote_addr, cenAppState->blecen_remote_addr_type);

                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnStop", TVF_D(0));
                }
            }
            break;
    }
}


void blecen_ConnUpdate(UINT8 conn_speed)
{
    if(blecen_con_handle)
    {
        if(conn_speed == FAST_CONN)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Fast Connection:%d", TVF_D(blecen_con_handle));
            blecm_ConnectionUpdate(
                blecen_con_handle,
                blecen_cen_cfg.high_conn_min_interval,
                blecen_cen_cfg.high_conn_max_interval,
                blecen_cen_cfg.high_conn_latency,
                blecen_cen_cfg.high_supervision_timeout,
                blecen_cen_cfg.conn_min_event_len,
                blecen_cen_cfg.conn_max_event_len
            );
        }
        else if(conn_speed == SLOW_CONN)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Slow Connection: %d", TVF_D(blecen_con_handle));
            blecm_ConnectionUpdate(
                blecen_con_handle,
                blecen_cen_cfg.low_conn_min_interval,
                blecen_cen_cfg.low_conn_max_interval,
                blecen_cen_cfg.low_conn_latency,
                blecen_cen_cfg.low_supervision_timeout,
                blecen_cen_cfg.conn_min_event_len,
                blecen_cen_cfg.conn_max_event_len
            );
        }
    }
}

int blecenhandleConnParamUpdateReq(LEL2CAP_HDR *l2capHdr)
{
    LEL2CAP_COMMAND_HDR *cmdPkt = (LEL2CAP_COMMAND_HDR *) l2capHdr;

    extern void lel2cap_sendConnParamUpdateRsp(UINT16 identifier, UINT16 result);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdateReq", TVF_D(0));

    if(cmdPkt->len == 8)
    {
        LEL2CAP_CMD_CONN_PARAM_UPDATE_REQ *req =
		                                              (LEL2CAP_CMD_CONN_PARAM_UPDATE_REQ *) (&(cmdPkt->code));

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdate:Min Interval: %d, Max Interval: %d", TVF_WW(req->minInterval, req->maxInterval));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ConnParamUpdate: Slave LAtency: %d, timeout: %d", TVF_WW(req->slaveLatency, req->timeout));

        blecm_ConnectionUpdate(
                blecen_con_handle,
                req->minInterval,
                req->maxInterval,
                req->slaveLatency,
                req->timeout,
                blecen_cen_cfg.conn_min_event_len,
                blecen_cen_cfg.conn_max_event_len
        );

        lel2cap_sendConnParamUpdateRsp(req->identifier, 0); //always accept

        return 0;
    }

    return -1;
}

void blecen_readCb(int len, int attr_len, UINT8 *data)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "readCb:%d,%d", TVF_WW(len, attr_len));
    ble_tracen((char *)data, len);
}

void blecen_writeCb(int len, int attr_len, UINT8 *data)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "writeCb:%d,%d", TVF_WW(len, attr_len));
    ble_tracen((char *)data, len);
}

void blecen_Notification(int len, int attr_len, UINT8 *data)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Noti:%02x,%d", TVF_WW((UINT16)attr_len, len));
    ble_tracen((char *)data, len);
}
