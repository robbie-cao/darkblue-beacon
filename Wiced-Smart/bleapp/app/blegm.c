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
* File Name: blegm.c
*
* Abstract: This file implements the BLE Blood Pressure profile, service, application
*
* Functions:
*
*******************************************************************************/
#include "blegm.h"

#include "gpiodriver.h"
#include "dbfw_app.h"

#define FID    FID_BLEAPP_APP__BLEGM_C

#define BLEGM_R


#define PUART_TX_WATERLEVEL  1 //13 // 1

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blegm_Timeout(UINT32 count);
void blegm_FineTimeout(UINT32 finecount);
void blegm_handleUART(char *gm_char);
void blegm_handlemeasurement(void);
void blegm_DBInit(void);
void blegm_connUp(void);
void blegm_connDown(void);
void blegm_advStop(void);
void blegm_appTimerCb(UINT32 arg);
void blegm_appFineTimerCb(UINT32 arg);
void blegm_smpBondResult(LESMP_PARING_RESULT  result);
void blegm_encryptionChanged(HCI_EVT_HDR *evt);
void blegm_IndicationConf(void);
void blegm_transactionTimeout(void);


//This function is for GM test
void blegm_FakeUART(char *gm_char, UINT32 count);
int blegm_writeCb(LEGATTDB_ENTRY_HDR *p);

void blegm_PUARTInitTxWaterlevel(UINT8 puart_tx_waterlevel);
void blegm_PUARTSetTxWaterlevel(UINT8 puart_tx_waterlevel);
int blegm_PUARTRxMaxFail(char *data, UINT8 len, UINT8 max);

void blegm_PUART_RxIntCb(void *ptr, UINT8 portPin);
void blegm_randCb(char *rand);


extern void wdog_restart(void);


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blegm_db_data[]=
{
0x01, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x01,0x18,
0x02, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x10,0x03,0x00,0x05,0x2a,
0x03, 0x00, // handle
        0, // permission
        0x06, // length
        0x05, 0x2a, // uuid
	0x00,0x00,0x00,0x00,
0x14, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x00,0x18,
0x15, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x16,0x00,0x00,0x2a,
0x16, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x12, // length
        0x00, 0x2a, // uuid
	0x42,0x4c,0x45,0x20,0x47,0x6C,0x75,0x63,0x6f,0x73,0x65,0x6D,0x65,0x74,0x65,0x72, // "BLE Glucosemeter"
0x17, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x18,0x00,0x01,0x2a,
0x18, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x01, 0x2a, // uuid
	0x00,0x00,
0x28, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x08,0x18,
0x29, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x22,0x2a,0x00,0x18,0x2a,    //UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT
0x2a, 0x00, // handle
        LEGATTDB_PERM_READABLE , // permission Gluecose Measurement, readable.
        0x05, // length
        0x18, 0x2a, // uuid
	0x00,0x00,0x00,
0x2b, 0x00, // handle
        LEGATTDB_PERM_READABLE
        |LEGATTDB_PERM_WRITE_CMD
        |LEGATTDB_PERM_WRITE_REQ,  // permission
        0x04, // length
        0x02, // max length
        0x02, 0x29, // uuid
	0x00,0x00,
0x2d, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x04, // length
        0x00, 0x28, // uuid
	0x0a,0x18,
0x2e, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x2f,0x00,0x29,0x2a,
0x2f, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x0a, // length
        0x29, 0x2a, // uuid
	0x42,0x72,0x6f,0x61,0x64,0x63,0x6f,0x6d,
0x30, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x31,0x00,0x24,0x2a,
0x31, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x0a, // length
        0x24, 0x2a, // uuid
	0x31,0x32,0x33,0x34,0x00,0x00,0x00,0x00,
0x32, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x07, // length
        0x03, 0x28, // uuid
	0x02,0x33,0x00,0x23,0x2a,
0x33, 0x00, // handle
        LEGATTDB_PERM_READABLE,  // permission
        0x0a, // length
        0x23, 0x2a, // uuid System ID.
	0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const UINT16 blegm_db_size = sizeof(blegm_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blegm_cfg =
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
    "BLE Glucosemeter", //char local_name[LOCAL_NAME_LEN_MAX];
    "\x00\x09\x10", //char cod[COD_LEN];
    "1.00", //char ver[VERSION_LEN];
    0, // UINT8 encr_required; // if 1, encryption is needed before sending indication/notification
    0, // UINT8 disc_required;// if 1, disconnection after confirmation
    0, //UINT8 test_enable;   //TEST MODE is enabled when 1
    0x04,  //  UINT8 tx_power_level; //dbm
    120, // UINT8 con_idle_timeout; //second   0-> no timeout
    0, //    UINT8 powersave_timeout; //second  0-> no timeout
    {0x002a,
      0x002b,
      0x00,
      0x00,
      0x00}, // UINT16 hdl[HANDLE_NUM_MAX];   //GATT HANDLE number
    {UUID_SERVICE_GLUECOSE_CONCENTRATION,
      UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT,
      0x00,
      0x00,
      0x00}, // UINT16 serv[HANDLE_NUM_MAX];  //GATT service UUID
    {UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT,
      UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
      0x00,
      0x00,
      0x00}, // UINT16 cha[HANDLE_NUM_MAX];   // GATT characteristic UUID
    0, // UINT8 findme_locator_enable; //if 1 Find me locator is enable
    0, // UINT8 findme_alert_level; //alert level of find me
    0, // UINT8 client_grouptype_enable; // if 1 grouptype read can be used
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
    0, // UINT8 button_client_timeout; // seconds
    0, //UINT8 button_discover_timeout; // seconds
    0, //UINT8 button_filter_timeout; // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    15, //UINT8 button_uart_timeout; // seconds
#endif
};

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blegm_puart_cfg =
{
    115200, // UINT32 baudrate;
    24, //32, //24, // 31, // 32, // UINT8  txpin; //GPIO pin number //20730A0 module need to use 32 instead, normally it is 31
    25, //33, //25, // UINT8  rxpin; //GPIO pin number
};


PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blegm_gpio_cfg =
{
    {31, 1, 2, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,
      GPIO_INPUT|GPIO_INIT_HIGH|GPIO_BUTTON1|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BUTTON2|GPIO_INT,
      GPIO_INPUT|GPIO_INIT_LOW|GPIO_BUTTON3|GPIO_INT,
      GPIO_OUTPUT|GPIO_INIT_LOW,
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

UINT32 blegm_apptimer_count=0;
UINT32 blegm_appfinetimer_count=0;
UINT16 blegm_con_handle=0;
BD_ADDR  blegm_remote_addr;

UINT16 blegm_gm_hdl=0;
UINT16 blegm_gm_client_hdl=0;
UINT8 blegm_bat_enable=0;
UINT8 blegm_indication_enable=0;
UINT8 blegm_indication_defer=0;
UINT8 blegm_indication_sent=0;
UINT8 blegm_measurement_done=0;

//NVRAM save area
BLEPROFILE_HOSTINFO blegm_hostinfo;

BLEGM_GM_DATA blegm_gm_data;

UINT8 blegm_rand_received=0;


//Output GPIO
UINT8 blegm_gpio_output = 4;//should be same as blegm_gpio_cfg

#if 0 //This is test code
#include "puart.h"
void blegm_PUARTRxIntCb(void *data)
{
    char blegmroche_rxdata[16];
    char temp[3];
    UINT8 len;
    int i;

    (void)data; // We know that this is not used. The callback registration needs it.

    if(puart_rxFifoNotEmpty())
    {
        len = blegm_PUARTRxMaxWait(blegmroche_rxdata, 16, 0);

        blegm_PUARTTx(blegmroche_rxdata, len);
    }
}
#endif

#ifdef CONFIG_RAMBUF
//This function is used for loading data from SPI to rambuf
void blegm_rambuf_load(void)
{
    extern UINT8* cfa_ConfigGetRamBufferInfo(UINT16* allocated_len, UINT16* total_len);
    extern void cfa_ConfigRambufRecal(UINT16 loadlen);
    UINT8 data[12] = {0x60,0x01,0x00,0x00,
                      0x61,0x02,0x00,0x00,0x01,
                      0xfe,0x00,0x00
                     };
    UINT16 len1, len2;
    UINT8 *ptr = cfa_ConfigGetRamBufferInfo(&len1, &len2);
    UINT16 load_len = 12;

    //empty rambuf has 3 bytes in length
    //contents is 0xFE(CONFIG_ITEM_SECTION_END), 0x00, 0x00 (two bytes length)
    //rambuf should be end with these 3 bytes
    //if loaded content is not ended with these three bytes, it should be terminated with these three bytes.

    //test
    memset(ptr, 0, len1);
    memcpy(ptr, data, load_len);

    // Rambuf is prepared from ptr with size of load_len
    // len1 is the total available length
    // len2 is the currenly used length

    //recalculated ptr
    cfa_ConfigRambufRecal(load_len);
}

void blegm_rambuf_test(void)
{
#if 0
//rambuf level test
    {
        UINT8 data[100];
        int i;
        int max_item = 10;
        UINT16 length;
        UINT8 *ptr;

        //init data
        for(i=0; i<100; i++)
        {
            data[i] = i;
        }

        for(i=0; i<100; i++)
        {
            extern UINT8 config_rambuf_AddItem(UINT8 item_type, UINT8* item, UINT16 length);
            extern UINT8* config_rambuf_FindItem(UINT8 type, UINT16* length);
            extern UINT8 config_rambuf_DeleteItem(UINT8 type);

            ble_trace1("item:%02x", 0x70+i%max_item);
            config_rambuf_DeleteItem(0x70+i%max_item);
            {
                extern UINT8* cfa_ConfigGetRamBufferInfo(UINT16* allocated_len, UINT16* total_len);
                UINT16 len1, len2;
                UINT8 *ptr = cfa_ConfigGetRamBufferInfo(&len1, &len2);
                ble_trace3("RAM buf : S:%08x, alloc_len : %d, total_len : %d", (UINT32)ptr, len1, len2);
            }
            config_rambuf_AddItem(0x70+i%max_item, data, i);
            {
                extern UINT8* cfa_ConfigGetRamBufferInfo(UINT16* allocated_len, UINT16* total_len);
                UINT16 len1, len2;
                UINT8 *ptr = cfa_ConfigGetRamBufferInfo(&len1, &len2);
                ble_trace3("RAM buf : S:%08x, alloc_len : %d, total_len : %d", (UINT32)ptr, len1, len2);
            }
            ptr = config_rambuf_FindItem(0x70+i%max_item, &length);

            {
                extern UINT8 *g_config_rambuf_Start;
                ble_trace3("ptr: %08x(%08x), length: %d", (UINT32)ptr, (UINT32)(ptr-g_config_rambuf_Start), length);
            }
//            ble_tracen(ptr, length);

//          {
//              extern UINT8 *g_config_rambuf_Start;
//              extern UINT16 g_config_rambuf_TotalLength;
//              ble_tracen(g_config_rambuf_Start, g_config_rambuf_TotalLength);
//          }

        }
    }
#endif

#if 0
//cfa level test
    {
        UINT8 data[100];
        UINT8 read_data[100];
        int i;
        int max_item = 10;
        UINT16 length;

        //init data
        for(i=0; i<100; i++)
        {
            data[i] = i;
        }

        //variable length
        for(i=0; i<100; i++)
        {
            ble_trace1("item:%02x", 0x70+i%max_item);

            ble_trace1("Write:%d bytes", cfa_ConfigRambufWrite(0x70+i%max_item, i%(max_item*2), data));

            length = cfa_ConfigRambufRead(0x70+i%max_item, i, read_data);
            ble_trace1("Read:%d bytes", length);

            ble_tracen(read_data, length);
        }

        //fixed length
        for(i=0; i<100; i++)
        {
            ble_trace1("item:%02x", 0x70+i%max_item);

            ble_trace1("Write:%d bytes", cfa_ConfigRambufWrite(0x70+i%max_item, 1, &(data[i])));

            length = cfa_ConfigRambufRead(0x70+i%max_item, i, read_data);
            ble_trace1("Read:%d bytes", length);

            ble_tracen(read_data, length);
        }
    }
#endif

#if 0
//profile level test
    {
        UINT8 data[100];
        UINT8 read_data[100];
        int i;
        int max_item = 10;
        UINT16 length;

        //init data
        for(i=0; i<100; i++)
        {
            data[i] = i;
        }

        //variable length
        for(i=0; i<100; i++)
        {
            ble_trace1("item:%02x", 0x70+i%max_item);

            ble_trace1("Write:%d bytes", bleprofile_WriteNVRAM(0x70+i%max_item, i%(max_item*2), data));

            length = bleprofile_ReadNVRAM(0x70+i%max_item, i, read_data);
            ble_trace1("Read:%d bytes", length);

            ble_tracen(read_data, length);
        }

        //fixed length
        for(i=0; i<100; i++)
        {
            ble_trace1("item:%02x", 0x70+i%max_item);

            ble_trace1("Write:%d bytes", bleprofile_WriteNVRAM(0x70+i%max_item, 1, &(data[i])));

            length = bleprofile_ReadNVRAM(0x70+i%max_item, i, read_data);
            ble_trace1("Read:%d bytes", length);

            ble_tracen(read_data, length);
        }
    }
#endif

}
#endif

void blegm_Create(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_Create Ver[5:2] = %02x %02x %02x %02x",
          TVF_BBBB(bleprofile_p_cfg->ver[5], bleprofile_p_cfg->ver[4], bleprofile_p_cfg->ver[3], bleprofile_p_cfg->ver[2]));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "bletime_Create Ver[1:0] = %02x %02x",
          TVF_BB(bleprofile_p_cfg->ver[1], bleprofile_p_cfg->ver[0]));

    blegm_PUARTInitTxWaterlevel(PUART_TX_WATERLEVEL);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blegm_DBInit(); //load handle number

    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blegm_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blegm_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blegm_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blegm_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blegm_smpBondResult);
#if 0
    // handler for conf
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blegm_IndicationConf);
#endif
    // ATT timeout cb
    leatt_regTransactionTimeoutCb((LEATT_NO_PARAM_CB) blegm_transactionTimeout);

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blegm_writeCb);

#if 0
    // enable PUART Rx Interrupt
    blegm_PUART_EnableRxFIFOInt(blegm_PUARTRxIntCb);
#endif

    //data init
    memset(&blegm_gm_data, 0x00, sizeof(BLEGM_GM_DATA));
    blegm_gm_data.flag = 0;

    bleprofile_regTimerCb(blegm_appFineTimerCb, blegm_appTimerCb);
    bleprofile_StartTimer();

#if 0
{
    BLECM_SELECT_ADDR select_addr[5] =
    {
        {{0x00,0x00,0x00,0x00,0x00,0x00},0},
        {{0x01,0x00,0x00,0x00,0x00,0x00},1},
        {{0x02,0x00,0x00,0x00,0x00,0x00},0},
        {{0x03,0x00,0x00,0x00,0x00,0x00},1},
        {{0x04,0x00,0x00,0x00,0x00,0x00},0}
    };

    blecm_enableAddressSelection();
    //blecm_disableAddressSelection();
    blecm_SelectAddress(select_addr, 5);
    blecm_SelectTargetAddress(select_addr, 5);
}
#endif

#ifdef CONFIG_RAMBUF
    // This will show smp keys
    {
        extern UINT8* cfa_ConfigGetRamBufferInfo(UINT16* allocated_len, UINT16* total_len);
        UINT16 len1, len2;
        UINT8 *ptr = cfa_ConfigGetRamBufferInfo(&len1, &len2);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RAM buf : Start:%08x", TVF_D(ptr));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RAM buf : alloc_len:%d, total_len : %d", TVF_WW(len1, len2));

        ble_tracen(ptr, len1);
    }

    // reload
    blegm_rambuf_load();

    //lesmpkeys_init(); //smp key should be reloaded with this function call

    // This will show reloaded ram buf content
    {
        extern UINT8* cfa_ConfigGetRamBufferInfo(UINT16* allocated_len, UINT16* total_len);
        UINT16 len1, len2;
        UINT8 *ptr = cfa_ConfigGetRamBufferInfo(&len1, &len2);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RAM buf (reloaded) : Start:%08x", TVF_D(ptr));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "RAM buf (reloaded) : alloc_len:%d, total_len : %d", TVF_WW(len1, len2));
    }

    // Read Write test
    blegm_rambuf_test();
#endif

    blegm_Sleep(10);

    blegm_connDown();

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);


    /* Disabling UART interrupt */
    //UART_INT_ENABLE     = 0x0000;
}


void blegm_FakeUART(char *gm_char, UINT32 count)
{
    //This is for test only
    if(count % 30 == 0 && blegm_indication_enable && gm_char[0] == 0)
    {
        //faking data measurement
        gm_char[0] = 'G';
        gm_char[1] = '8';
        gm_char[2] = '0';
        gm_char[3] = (count&0xF0)>>4;
        if(gm_char[3]<0x0A)
        {
            gm_char[3]+='0';
        }
        else
        {
            gm_char[3]+=('A'-10);
        }
        gm_char[4] = count&0x0F;
        if(gm_char[4]<0x0A)
        {
            gm_char[4]+='0';
        }
        else
        {
            gm_char[4]+=('A'-10);
        }
    }
}

void blegm_Timeout(UINT32 count)
{
    if(blegm_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();

#if 0
    if(count == 10)
    {
        BLECM_SELECT_ADDR select_addr[5] =
        {
            {{0x00,0x00,0x00,0x00,0x00,0x00},0},
            {{0x01,0x00,0x00,0x00,0x00,0x00},0},
            {{0x02,0x00,0x00,0x00,0x00,0x00},0},
            {{0x03,0x00,0x00,0x00,0x00,0x00},0},
            {{0x04,0x00,0x00,0x00,0x00,0x00},0}
        };

        blecm_enableAddressSelection();
        //blecm_disableAddressSelection();
        blecm_SelectAddress(select_addr, 5);
        blecm_SelectTargetAddress(select_addr, 5);
    }
#endif

#if 0
    {
        blegm_rand_received=0;
        lesmp_getRandwithCb(blegm_randCb);
    }
#endif
}

void blegm_FineTimeout(UINT32 finecount)
{
    {
        char gm_char[READ_UART_LEN+1];

        //Reading
        bleprofile_ReadUART(gm_char);

#if 1
        if(bleprofile_p_cfg->test_enable)
        {
            //This is making faking data
            //For test only
            blegm_FakeUART(gm_char, finecount);
        }
#endif

	 if(gm_char[0] == 'D' && gm_char[1] == 'D') //download start
	 {
            blecm_setFilterEnable(0);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Disabled", TVF_D(0));
	 }
	 else if(gm_char[0] == 'A' && gm_char[1] == 'A') //download start
	 {
            blecm_setFilterEnable(1);
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "CSA Filter Enabled", TVF_D(0));

            blegm_connDown();
	 }
        else  //gm main reading
        {
            blegm_handleUART(gm_char);
        }
    }

    // button control
    bleprofile_ReadButton();
}

void blegm_handleUART(char *gm_char)
{
    UINT16 gm_temp;
    int i;

    //this part can be replaced by callback function
    if(bleprofile_handleUARTCb)
    {
        blegm_measurement_done = bleprofile_handleUARTCb((UINT8 *)gm_char, (UINT8 *)&blegm_gm_data);
    }
    else
    {
        if(gm_char[0] == 'G' && gm_char[1] == '8' && gm_char[2] == '0')
        {
            //change ascii to hex
            for(i=3; i<=10; i++)
            {
                if((gm_char[i] >= '0') && (gm_char[i] <= '9'))
                {
                    gm_char[i] -= '0';
                }
                else if((gm_char[i] >= 'A') && (gm_char[i] <= 'F'))
                {
                    gm_char[i] = (gm_char[i] -'A'+10);
                }
                else if((gm_char[i] >= 'a') && (gm_char[i] <= 'f'))
                {
                    gm_char[i] = (gm_char[i] -'a'+10);
                }
                else
                {
                    gm_char[i] = 0; //error case
                }
            }

            //Measurement is available
            if(1)
            {
                blegm_gm_data.flag |= GM_GLUCOSE_MEASUREMENT;
            }
            else
            {
                blegm_gm_data.flag &= ~GM_GLUCOSE_MEASUREMENT;
            }

            //if unit is mmol/L
            if(0)
            {
                blegm_gm_data.flag |= GM_UNIT_MMOLL;
            }
            else
            {
                blegm_gm_data.flag &= ~GM_UNIT_MMOLL;
            }

            //set measurement
            gm_temp = (UINT16)((gm_char[3]<<4)+gm_char[4]);
            blegm_gm_data.glucosemeasurement = bleprofile_UINT16toSFLOAT(gm_temp);

            blegm_measurement_done = 1; //New measurement is done
        }
    }

    blegm_handlemeasurement();
}


void blegm_handleMeasurement(UINT8 *blegm_gm_data_ptr)
{
    memcpy((void *)&blegm_gm_data, blegm_gm_data_ptr, sizeof(BLEGM_GM_DATA));
    blegm_measurement_done = 1;

    blegm_handlemeasurement();
}

void blegm_handlemeasurement(void)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;
    UINT8 writtenbyte;

    // change EEPROM and GATT DB
    if(blegm_measurement_done == 1) //if connected and encrpted, old data is sent
    {
        //write whole in EEPROM
        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_GM_DATA, sizeof(BLEGM_GM_DATA), (UINT8 *)&blegm_gm_data);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM GM write:%04x", TVF_D(writtenbyte));

        blegm_measurement_done = 2; //this will prevent multiple writing
        //blegm_measurement_done = 0; //this will permit multiple measurement
        blegm_indication_defer =1;
    }


    //handling deferred indication
    if(blegm_indication_enable && blegm_indication_defer)
    {
        //load GM data from NVRAM and send
        if(bleprofile_ReadNVRAM(VS_BLE_GM_DATA, sizeof(BLEGM_GM_DATA), (UINT8 *)&blegm_gm_data))
        {
            int i=0;

            //write partial based on flag
            // FLAG(1) - 1 byte is mandatory
            memcpy(db_pdu.pdu, &blegm_gm_data, 1);
            i=1;

            //Optional item
            if(blegm_gm_data.flag & GM_GLUCOSE_MEASUREMENT)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blegm_gm_data.glucosemeasurement, 2);
                i+=2;
            }
            if(blegm_gm_data.flag & GM_TIME_STAMP)
            {
                memcpy(&(db_pdu.pdu[i]), blegm_gm_data.timestamp, 7);
                i+=7;
            }
            if(blegm_gm_data.flag & GM_DATE_AND_TIME_ADJUST)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&(blegm_gm_data.dateandtimeadjust), 4);
                i+=4;
            }
            if(blegm_gm_data.flag & GM_SAMPLE_LOCATION)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blegm_gm_data.samplelocationvalue, 1);
                i+=1;
            }
            if(blegm_gm_data.flag & GM_MEAL)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blegm_gm_data.mealvalue, 1);
                i+=1;
            }
            if(blegm_gm_data.flag & GM_SENSOR_ANNUNCIATION)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blegm_gm_data.sensorannunciation, 2);
                i+=2;
            }
            if(blegm_gm_data.flag & GM_SEQUENCE_NUMBER)
            {
                memcpy(&(db_pdu.pdu[i]), (UINT8 *)&blegm_gm_data.sequencenumber, 2);
                i+=2;
            }

            db_pdu.len = i;
            bleprofile_WriteHandle(blegm_gm_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            //check client char cfg
            bleprofile_ReadHandle(blegm_gm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            // this is the first indication
            if(db_cl_pdu.len==2 && (db_cl_pdu.pdu[0] & CCC_INDICATION))
            {
                if(blegm_indication_sent==0)
                {
                    bleprofile_sendIndication( blegm_gm_hdl,
                            (UINT8 *)db_pdu.pdu, db_pdu.len, blegm_IndicationConf);
                    blegm_indication_sent = 1;
                }
                else
                {
                    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "No Conf Blk Ind", TVF_D(0));
                }
            }
        }

        blegm_indication_defer = 0;
    }
}

void blegm_DBInit(void)
{
    BLEPROFILE_DB_PDU db_pdu;
    int i;
    //load handle number

    for(i=0; i<HANDLE_NUM_MAX; i++)
    {
        if(bleprofile_p_cfg->serv[i] == UUID_SERVICE_GLUECOSE_CONCENTRATION&&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT)
        {
            blegm_gm_hdl=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_gm_hdl:%04x", TVF_D(blegm_gm_hdl));
            bleprofile_ReadHandle(blegm_gm_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        if(bleprofile_p_cfg->serv[i] == UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT&&
            bleprofile_p_cfg->cha[i] == UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION)
        {
            blegm_gm_client_hdl=bleprofile_p_cfg->hdl[i];
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_gm_client_hdl:%04x", TVF_D(blegm_gm_client_hdl));
            bleprofile_ReadHandle(blegm_gm_client_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);
        }
        else if(bleprofile_p_cfg->serv[i] == UUID_SERVICE_BATTERY &&
                bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            blegm_bat_enable = 1;
            blebat_Init();
        }
    }
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(blegm_gm_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif


}

void blegm_connUp(void)
{
    BLEPROFILE_DB_PDU db_cl_pdu;

    blegm_con_handle = (UINT16)emconinfo_getConnHandle();

    // print the bd address.
    memcpy(blegm_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(blegm_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_connUp[5:2]: %02x %02x %02x %02x",
          TVF_BBBB(blegm_remote_addr[5], blegm_remote_addr[4],
                   blegm_remote_addr[3], blegm_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_connUp[1:0]: %02x %02x, handle: %04x",
          TVF_BBW(blegm_remote_addr[1], blegm_remote_addr[0], blegm_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo);

    //using default value first
    db_cl_pdu.len = 2;
    db_cl_pdu.pdu[0] = 0x00;
    db_cl_pdu.pdu[1] = 0x00;

    // Save NVRAM to client characteristic descriptor
    if(memcmp(blegm_remote_addr, blegm_hostinfo.bdAddr, 6) == 0)
    {
        if((blegm_hostinfo.serv == UUID_SERVICE_GLUECOSE_CONCENTRATION) &&
            (blegm_hostinfo.cha == UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT))
        {
            db_cl_pdu.pdu[0] = blegm_hostinfo.cli_cha_desc & 0xFF;
            db_cl_pdu.pdu[1] = blegm_hostinfo.cli_cha_desc >>8;
        }
    }

    //reset client char cfg
    if(blegm_gm_client_hdl)
    {
        bleprofile_WriteHandle(blegm_gm_client_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);
    }

    if(bleprofile_p_cfg->encr_required==0)
    {
        blegm_indication_enable = 1; //indication enable
        blegm_indication_defer = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ind On", TVF_D(0));
    }

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blegm_connDown(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_conDown[5:2]: %02x %02x %02x %02x",
          TVF_BBBB(blegm_remote_addr[5], blegm_remote_addr[4],
                   blegm_remote_addr[3], blegm_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_connDown[1:0]: %02x %02x, handle: %04x",
          TVF_BBW(blegm_remote_addr[1], blegm_remote_addr[0], blegm_con_handle));

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo);

    // Save client characteristic descriptor to NVRAM
    if(memcmp(blegm_remote_addr, blegm_hostinfo.bdAddr, 6) == 0)
    {
        BLEPROFILE_DB_PDU db_cl_pdu;
	 UINT8 writtenbyte;

        if(blegm_gm_client_hdl)
        {
            bleprofile_ReadHandle(blegm_gm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            blegm_hostinfo.serv = UUID_SERVICE_GLUECOSE_CONCENTRATION;
            blegm_hostinfo.cha = UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT;
            blegm_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo);
            //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blegm_hostinfo);

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
        }
    }

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
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, blegm_hostinfo.bdAddr);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[5:2]: %02x %02x %02x %02x",
              TVF_BBBB(blegm_hostinfo.bdAddr[5], blegm_hostinfo.bdAddr[4],
                       blegm_hostinfo.bdAddr[3], blegm_hostinfo.bdAddr[2] ));
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV start[1:0]: %02x %02x, handle %04x",
              TVF_BBW(blegm_hostinfo.bdAddr[1], blegm_hostinfo.bdAddr[0], blegm_con_handle));
    }

    blegm_con_handle = 0; //reset connection handle
    blegm_indication_enable = 0; //notification enable
    blegm_indication_defer = 0;
    blegm_indication_sent = 0;
    blegm_measurement_done = 0;
}

void blegm_advStop(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ADV Stop", TVF_D(0));
}

void blegm_appTimerCb(UINT32 arg)
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                blegm_apptimer_count++;

                blegm_Timeout(blegm_apptimer_count);
            }
            break;

    }
}

void blegm_appFineTimerCb(UINT32 arg)
{
    blegm_appfinetimer_count++;

    blegm_FineTimeout(blegm_appfinetimer_count);
}


void blegm_smpBondResult(LESMP_PARING_RESULT  result)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm, bond result %02x", TVF_D(result));

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
	 // saving bd_addr in nvram
	 UINT8 writtenbyte;

        memcpy(blegm_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(blegm_remote_addr));
        memcpy(blegm_hostinfo.bdAddr, blegm_remote_addr, sizeof(BD_ADDR));

        if(blegm_gm_client_hdl)
        {
            BLEPROFILE_DB_PDU db_cl_pdu;

            bleprofile_ReadHandle(blegm_gm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            blegm_hostinfo.serv = UUID_SERVICE_GLUECOSE_CONCENTRATION;
            blegm_hostinfo.cha = UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT;
            blegm_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo);
        //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blegm_hostinfo);

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blegm_encryptionChanged(HCI_EVT_HDR *evt)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Enc Change", TVF_D(0));

    if(bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo))
    {
        if(memcmp(blegm_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), 6)==0)
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[5:2]: %02x %02x %02x %02x",
              TVF_BBBB(blegm_hostinfo.bdAddr[5], blegm_hostinfo.bdAddr[4],
                       blegm_hostinfo.bdAddr[3], blegm_hostinfo.bdAddr[2] ));
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "EncOn for Last paired device[1:0]: %02x %02x",
                  TVF_BB(blegm_hostinfo.bdAddr[1], blegm_hostinfo.bdAddr[0]));
        }
    }

    if(bleprofile_p_cfg->encr_required)
    {
        blegm_indication_enable = 1; //indication enable
        blegm_indication_defer = 1;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Ind On", TVF_D(0));
    }
}



void blegm_IndicationConf(void)
{
    if(blegm_indication_sent)
    {
        blegm_indication_sent = 0;
    }
    else
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Wrong Conf", TVF_D(0));
        return;
    }

    if((bleprofile_p_cfg->disc_required) & DISC_AFTER_CONFIRM)
    {
        if(blegm_con_handle && blegm_indication_enable && blegm_measurement_done)
        //Encryption and data measurement is done
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            blegm_con_handle= 0;
            blegm_indication_enable = 0;
            blegm_indication_defer = 0;
            blegm_measurement_done = 0;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "DiscAfterConf", TVF_D(0));
        }
    }
}

void blegm_transactionTimeout(void)
{
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "ATT TimeOut", TVF_D(0));

    if((bleprofile_p_cfg->disc_required) & DISC_ATT_TIMEOUT)
    {
        {
            blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);

            blegm_con_handle= 0;
            blegm_indication_enable = 0;
            blegm_indication_defer = 0;
            blegm_measurement_done = 0;
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Disconnect ATT Timeout", TVF_D(0));
        }
    }
}

int blegm_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16 handle = legattdb_getHandle(p);

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: handle %04x",TVF_D( handle));

    if(blegm_gm_client_hdl && handle == blegm_gm_client_hdl)
    {
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo);

        // Save client characteristic descriptor to NVRAM
        if(memcmp(blegm_remote_addr, blegm_hostinfo.bdAddr, 6) == 0)
        {
            BLEPROFILE_DB_PDU db_cl_pdu;
	     UINT8 writtenbyte;

            bleprofile_ReadHandle(blegm_gm_client_hdl, &db_cl_pdu);
            ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

            blegm_hostinfo.serv = UUID_SERVICE_GLUECOSE_CONCENTRATION;
            blegm_hostinfo.cha = UUID_CHARACTERISTIC_GLUCOSE_MEASUREMENT;
            blegm_hostinfo.cli_cha_desc = db_cl_pdu.pdu[0]+(db_cl_pdu.pdu[1]<<8);

            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegm_hostinfo);
            //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blegm_hostinfo);

            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));

        }
    }

    return 0;
}


#define BLE_PUART_MUX
#define BLE_UART_DEBUG

#if defined(BLE_PUART_MUX) ||defined (BLE_UART_DEBUG)
#include "uart.h"

//#define DEBUG_UART_TX_FIFO_REAL_EMPTY()   (REG32(DEBUG_UART_LINE_STATUS_ADDR) & (DEBUG_UART_LINE_STATUS_TX_FIFO_NOT_EMPTY)== 0)
#endif

#define PUART_INT_STATUS_ADDR                sr_ptu_status_adr5
#define PUART_INT_STATUS()               UART_REG(PUART_INT_STATUS_ADDR)
#define PUART_INT_CLEAR(x)               UART_REG(PUART_INT_STATUS_ADDR) = (x)

#include "hw.h"
#include "puart.h"
void blegm_PUARTInitTxWaterlevel(UINT8 puart_tx_waterlevel)
{
    bleprofile_PUARTInitTxWaterlevel(puart_tx_waterlevel);
}


void blegm_PUARTSetTxWaterlevel(UINT8 puart_tx_waterlevel)
{
    bleprofile_PUARTSetTxWaterlevel(puart_tx_waterlevel);
}


void blegm_DUARTtx (char *p_str)
{
#ifdef BLE_PUART_MUX
    UINT8 delay=5; //0.5ms //12; // 1.2 ms for 9600
    char temp[16];
    int i;
    int length;
    UINT32 dword;

//blegm_SetOutput(1);

    //disable
    bleprofile_PUART_RxInt_Enable = 0;

    gpio_configurePin((bleprofile_puart_p_cfg->rxpin)>>4, (bleprofile_puart_p_cfg->rxpin)&0x0F,
        GPIO_INPUT_DISABLE, 0);
    gpio_configurePin((bleprofile_puart_p_cfg->txpin)>>4, (bleprofile_puart_p_cfg->txpin)&0x0F,
        GPIO_INPUT_DISABLE, 0);
    //gpio_setPinOutput((bleprofile_puart_p_cfg->txpin)>>4, (bleprofile_puart_p_cfg->txpin)&0x0F, 1);

    //temporary change to 32, 33
    puart_selectUartPads(((33&(~0x0f))<<1)|(33&0x0f),
			((32&(~0x0f))<<1)|(32&0x0f), 0, 0/*35, 30*/); //proximity key fob should use 32 as TX, no P31 pin


    blegm_PUARTSetTxWaterlevel(bleprofile_puart_tx_waterlevel);

//    cfa_debug_PrintString(p_str);

    // dc_ptu_uart2_lsr_adr - 2:2 TX_data_avail seems not working.
    // cfa_debug_xxx function relies on DEBUG_UART_TX_FIFO_EMPTY()
    // and checking DEBUG_UART_TX_FIFO_EMPTY() is not enough
    // using sr_ptu_status_adr5 - 1:1 uart_aef (UART output FIFO almost empty flag) is working well.
    // Before using PUART, setting TX waterlevel is needed.
    // P_UART_WATER_MARK_TX_LEVEL(1); will setting TX waterlevel as 1.
    // This seems if there is 1 or less in TX FIFO, it will detect as almost empty.
    // Default TX waterlevel is 6.

    length = strlen(p_str);

    while(length)
    {
        PUART_INT_CLEAR(UART_ISR_TX_FAE_MASK);

        if( PUART_INT_STATUS() & UART_ISR_TX_FAE_MASK )
        {
            /* write the data over the TX UART */
	     dword = *p_str;

            while(!cfa_debug_WriteDWORD(dword)){}
            p_str++;
            length--;
        }
        else
        {
            continue;
        }
    }

#if 0
{
    UINT32 dword;

    UINT16 timeout = 5;
    int i;
    UINT16 len = strlen(p_str);

    while(len >0)
    {
        /* write the data over the TX UART */
	 dword = *p_str;

        while(!cfa_debug_WriteDWORD(dword)){}
        data++;
        len--;

        for(i=0; i<timeout; i++)
        {
            wdog_restart(); //to prevent watch dog reset
            utilslib_delayUs(100);	//microsec with overhead 18us at 16MHz.
        }
    }
}
#endif

//blegm_SetOutput(0);

    // add some delay
    for(i=0; i<delay; i++)
    {
        extern void bleapputils_delayUs(UINT32 delay);

        wdog_restart(); //to prevent watch dog reset
        bleapputils_delayUs(100);	//microsec with overhead 18us at 16MHz.
    }

//blegm_SetOutput(1);

    //wait until UART is empty
    while(1)
    {
#if 0
       //clear bit
       UINT32 dulsa = REG32(DEBUG_UART_LINE_STATUS_ADDR);

	dulsa &= (~DEBUG_UART_LINE_STATUS_TX_FIFO_IDLE);
	dulsa |= DEBUG_UART_LINE_STATUS_TX_FIFO_NOT_EMPTY;
       REG32(DEBUG_UART_LINE_STATUS_ADDR) = dulsa;
#endif
	if(DEBUG_UART_TX_FIFO_EMPTY() == TRUE)
	{
           break;
	}
    }

#if 0
//blegm_SetOutput(0);

    // add some delay
    for(i=0; i<delay; i++)
    {
        wdog_restart(); //to prevent watch dog reset
        utilslib_delayUs(100);	//microsec with overhead 18us at 16MHz.
    }

//blegm_SetOutput(1);

    //wait again until UART is empty
    while(1)
    {
	if(DEBUG_UART_TX_FIFO_EMPTY() == TRUE)
	{
           break;
	}
    }
#endif

//blegm_SetOutput(0);

    // disable
    gpio_configurePin(33>>4, 33&0x0F, GPIO_INPUT_DISABLE, 0);
    gpio_configurePin(32>>4, 32&0x0F, GPIO_INPUT_DISABLE, 0);
    //gpio_setPinOutput(32>>4, 33, 1);

    //gpio_configurePin((bleprofile_puart_p_cfg->txpin)>>4, (bleprofile_puart_p_cfg->txpin)&0x0F,
    //				GPIO_PULL_UP, GPIO_PIN_OUTPUT_HIGH);

    // return to the original pin
    puart_selectUartPads((((bleprofile_puart_p_cfg->rxpin)&(~0x0f))<<1)|((bleprofile_puart_p_cfg->rxpin)&(0x0f)),
            (((bleprofile_puart_p_cfg->txpin)&(~0x0f))<<1)|((bleprofile_puart_p_cfg->txpin)&(0x0f)), 0, 0);

    gpio_configurePin((bleprofile_puart_p_cfg->rxpin)>>4, (bleprofile_puart_p_cfg->rxpin)&0x0F,
					GPIO_EN_INT_FALLING_EDGE, GPIO_PIN_OUTPUT_LOW);

    //read unwanted glitch
    blegm_PUARTRxMaxWait(temp, 15, 1);

    bleprofile_PUART_RxInt_Enable = 1;

//blegm_SetOutput(1);
//blegm_SetOutput(0);
#endif
}

void blegm_UARTtx (char *p_str)
{
#ifdef BLE_UART_DEBUG
    UINT8 length = strlen(p_str);

    /* begin writing the payload to the TX FIFO */

    while(length)
    {
        UART_INT_CLEAR(UART_ISR_TX_FAE_MASK);

        if( UART_INT_STATUS() & UART_ISR_TX_FAE_MASK )
        {
            length--;
            UART_TX_FIFO(*p_str++);
        }
        else
        {
            continue;
        }
    }
#endif

}

int blegm_ReadPUART(char *data)
{
    int i=0;

    //read from UART
    //while(cfa_debug_ReadChar((UINT8 *)(&data[i])))
    for(i=0; i<READ_UART_LEN; i++)
    {
        if(cfa_debug_ReadChar((UINT8 *)(&data[i])))
            ; // do nothing
	 else
            break;
    }
    data[i] = 0; //add NULL character

    return i;
}

int blegm_PUARTRx(char *data, UINT8 len)
{
    return bleprofile_PUARTRx(data, len);
}

int blegm_PUARTRxMaxFail(char *data, UINT8 len, UINT8 max)
{
    return bleprofile_PUARTRxMaxFail(data, len, max);
}

int blegm_PUARTRxMaxWait(char *data, UINT8 len, UINT8 ms)
{
    return bleprofile_PUARTRxMaxWait(data, len, ms);
}

#ifdef BLEGM_R
void blegm_PUART_ConfigRTS(BOOL enabled, UINT8 rtsPortPin, UINT8 assertvalue)
{
    bleprofile_PUART_ConfigRTS(enabled, rtsPortPin, assertvalue);
}
#endif

int blegm_PUARTTx(char *data, UINT8 len)
{
    return bleprofile_PUARTTx(data, len);
}


int blegm_PUARTTxMaxWait(char *data, UINT8 len, UINT8 ms)
{
    return bleprofile_PUARTTxMaxWait(data, len, ms);
}


int blegm_PUARTTxEchoMaxWait(char *data, UINT8 len, UINT8 tx_ms, UINT8 rx_ms, UINT8 flush)
{
    return bleprofile_PUARTTxEchoMaxWait(data, len, tx_ms, rx_ms, flush);
}


void blegm_Sleep(UINT8 hund_us)
{
    extern void bleapputils_Sleep(UINT8 hund_us);
    bleapputils_Sleep(hund_us);
}

void blegm_SetOutput(UINT8 value)
{
    gpio_setPinOutput(blegm_gpio_output>>4, blegm_gpio_output&0x0F, value);
}

void blegm_PUART_EnableRxInt(UINT8 rxpin, void (*userfn)(void*))
{
    bleprofile_PUART_EnableRxInt(rxpin, userfn);
}

void blegm_PUART_EnableRxFIFOInt(void (*userfn)(void*))
{
    bleprofile_PUART_EnableRxFIFOInt(userfn);
}


// set watermark level, 1-15 should be used.
void blegm_PUARTSetRxWaterlevel(UINT8 puart_rx_waterlevel)
{
    bleprofile_PUARTSetRxWaterlevel(puart_rx_waterlevel);
}

//This function is interrupt handler level callback function register
//If this callback function is not finished fast enough, system crash can happen.
void blegm_PUARTRegisterRxHandler(void (*RxHandler)(void))
{
    blegm_PUARTRegisterRxHandler(RxHandler);
}

//This function is CSA level interrupt handler function.
//Holding this function does not make any harm.
void blegm_PUARTRegisterCSARxHandler(void (*userfn)(void*))
{
    bleprofile_PUARTRegisterCSARxHandler(userfn);
}

// enable Rx AFF interrupt
void blegm_PUARTEnableAFFInt(void)
{
    bleprofile_PUARTEnableAFFInt();
}

// deassert CTS pin
void blegm_PUART_deassertCts(void)
{
    bleprofile_PUART_deassertCts();
}

//assert CTS pin
void blegm_PUART_assertCts(void)
{
    bleprofile_PUART_assertCts();
}

// enable FIFO level interrupt for CTS deassert
// disableINT usually set as 0
// If this one is set as 1, interrupt will be re-enabled in the interrupt handler level
// FIFO is not reduced fast enough, system will crash.
// If this one is set as 0, interrupt is enabled in the CSA level by blegm_PUARTCTS or blegm_PUARTMaxWaitCTS function
void blegm_PUART_EnableRxFIFOCTSInt(UINT8 ctsPortPin, UINT8 waterLevel, UINT8 assertvalue, UINT8 disableINT)
{
    bleprofile_PUART_EnableRxFIFOCTSInt(ctsPortPin, waterLevel, assertvalue, disableINT);
}

// CTS assert at the end of Rx
INT32 blegm_PUARTRxCTS(char *data, UINT8 len)
{
    return bleprofile_PUARTRxCTS(data, len);
}

// CTS assert at the end of Rx
INT32 blegm_PUARTRxMaxWaitCTS(char *data, UINT8 len, UINT16 us)
{
    return bleprofile_PUARTRxMaxWaitCTS(data, len, us);
}


void blegm_PUART_RxIntCb(void *ptr, UINT8 portPin)
{
    bleprofile_PUART_RxIntCb(ptr, portPin);
}

//#define THO_DEBUG

#ifdef THO_DEBUG
#define THO_STAT
#endif

#include "bleappconfig.h"
#ifndef BCM20703
extern DeviceLpmConfig devLpmConfig;
#endif

#ifdef THO_STAT
UINT32 blegm_timedHIDoffReadStat(void)
{
    UINT32 count = 0;
    UINT8 writtenbyte;

    bleprofile_ReadNVRAM(VS_BLE_HOST_LIST+4, 4, (UINT8 *)&count);

    return count;
}

void blegm_timedHIDoffWriteStat(UINT32 count)
{
    UINT8 writtenbyte;

    writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST+4, 4, (UINT8 *)&count);
}
#endif

UINT8 blegm_isHIDoffWake(void)
{
    extern BOOLEAN mia_isResetReasonPor(void);
    // Did we get here via a Power On Reset (as opposed to HID-OFF)
    if (mia_isResetReasonPor())
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void blegm_timedHIDoff(UINT32 count)
{
    UINT8 writtenbyte;

    writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST+3, 4, (UINT8 *)&count);
}

UINT32 blegm_checkHIDoffTimer(void)
{
    UINT32 count = 0xFFFFFFFF;
    UINT8 writtenbyte;

    if(bleprofile_ReadNVRAM(VS_BLE_HOST_LIST+3, 4, (UINT8 *)&count))
    {
        if(count>0)
        {
            count--;
            writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST+3, 4, (UINT8 *)&count);
        }
    }

    return count;
}

void blegm_timedHIDoffSetting(UINT32 min, UINT32 resolution)
{
    UINT32 count=0;

    extern BOOLEAN mia_isResetReasonTimedWake(void);

#ifdef THO_STAT
    UINT32 stat = blegm_timedHIDoffReadStat();

    UINT8 reason = 0;
    UINT32 stat_count;

#ifdef THO_DEBUG
    {
        char str[100];
	 memset(str, 0, 100);
        sprintf(str, "Tot:Tim:But:Pow=%08x ", stat);
        //blegm_DUARTtx(str);
        blegm_PUARTTxMaxWait(str, strlen(str),1);
    }
#endif

#endif

#ifdef BCM20703
    bleAppConfig.devLpmConfig.wakeFromHidoffInMs = 1000*60*resolution;

    miscDriverConfig.miaDriverConfig.timedWakeIntPortPin = 0x47; // this is reset to 0x00 for certain reason
#else
    devLpmConfig.wakeFromHidoffInMs = 1000*60*resolution;

    miaDriverConfig.timedWakeIntPortPin = 0x47; // this is reset to 0x00 for certain reason
#endif

    if(!blegm_isHIDoffWake())
    {
#ifdef THO_STAT
        reason = 0; //power wake up
#endif

#ifdef THO_DEBUG
        {
            char str[100];
	     memset(str, 0, 100);
            sprintf(str, "Power Up ");
            //blegm_DUARTtx(str);
            blegm_PUARTTxMaxWait(str, strlen(str),1);
        }
#endif
    }
    else if(mia_isResetReasonTimedWake()) // this function is not working (button press reset also return TRUE
    {
#ifdef THO_STAT
        reason = 2; //timed wake up
#endif

#if 0 //def THO_DEBUG
        {
	     char str[100];
	     memset(str, 0, 100);
            //sprintf(str, "lhl=%08x ", REG32(lhl_adc_ctl_adr)); // d0d4 always, not useful
            //sprintf(str, "cnt0=%08x, cnt1=%08x ", REG32(hidoff_cnt0_adr), REG32(hidoff_cnt1_adr));
            //sprintf(str, "port0=%04x, port1=%04x, port2=%04x",
            //    gpio_getPortInterruptStatus(0),
            //    gpio_getPortInterruptStatus(1),
            //    gpio_getPortInterruptStatus(2));
#ifdef BCM20703
            sprintf(str, "wakepin = %02x ", miscDriverConfig.miaDriverConfig.timedWakeIntPortPin);
#else
            sprintf(str, "wakepin = %02x ", miaDriverConfig.timedWakeIntPortPin);
#endif
            //blegm_DUARTtx(str);
            blegm_PUARTTxMaxWait(str, strlen(str),1);
        }
#endif
        count = blegm_checkHIDoffTimer();
    }
    else
    {
#ifdef THO_STAT
        reason = 1; //button wake up
#endif

#ifdef THO_DEBUG
        {
            char str[100];
	     memset(str, 0, 100);
            sprintf(str, "Button Wake ");
            //blegm_DUARTtx(str);
            blegm_PUARTTxMaxWait(str, strlen(str),1);
        }
#endif


        count = 0;
    }

#ifdef THO_STAT
    if(reason == 0)
    {
        stat_count = stat&0xFF;
        stat_count++;

        stat &= 0xFFFFFF00;
        stat |= stat_count;
    }
    else if(reason == 1)
    {
        stat_count = (stat&0xFF00)>>8;
        stat_count++;

        stat &= 0xFFFF00FF;
        stat |= (stat_count<<8);
    }
    else if(reason == 2)
    {
        stat_count = (stat&0xFF0000)>>16;
        stat_count++;

        stat &= 0xFF00FFFF;
        stat |= (stat_count<<16);
    }

    stat_count = (stat&0xFF000000)>>24;
    stat_count++;

    stat &= 0x00FFFFFF;
    stat |= (stat_count<<24);

    blegm_timedHIDoffWriteStat(stat);

#ifdef THO_DEBUG
    {
        char str[100];
	 memset(str, 0, 100);
        sprintf(str, "Tot:Tim:But:Pow=%08x ", stat);
        //blegm_DUARTtx(str);
        blegm_PUARTTxMaxWait(str, strlen(str),1);
    }
#endif
#endif


#ifdef THO_DEBUG
    {
	 char str[100];
	 memset(str, 0, 100);
        sprintf(str, "count=%d ", count);
         //blegm_DUARTtx(str);
        blegm_PUARTTxMaxWait(str, strlen(str),1);
    }
#endif

    if(count)
    {
        // shut down immediately
        bleprofile_PrepareHidOff();
    }
    else
    {
        blegm_timedHIDoff(min/resolution);//reset with original value
    }

#ifdef THO_DEBUG
    {
	 char str[100];
	 int h,m,s,ms,temp;

	 memset(str, 0, 100);

#ifdef BCM20703
        ms = bleAppConfig.devLpmConfigdevLpmConfig.wakeFromHidoffInMs % 1000;
        temp = bleAppConfig.devLpmConfigdevLpmConfig.wakeFromHidoffInMs /1000; //this will be total second
#else
        ms = devLpmConfig.wakeFromHidoffInMs % 1000;
        temp = devLpmConfig.wakeFromHidoffInMs /1000; //this will be total second
#endif
        s = temp % 60;
        temp = temp /60; //this will be total minutes
        m = temp % 60;
        h = temp / 60;

        sprintf(str, "%d:%d:%d.%d\n", h, m, s, ms);

        //blegm_DUARTtx(str);
        blegm_PUARTTxMaxWait(str, strlen(str),1);
    }
#endif
}

void blegm_gpio_debug_init(void)
{
    //(p21:TxD, 22:RxD, 23:TxFSM, 24:RxFSM)
    // phy debug
    REG32(di_test_ctl_adr) &= ~(0x0170);
    REG32(di_test_ctl_adr) |= 0x0010;    //### di_test_ctl_adr
    //gpio_configurePin(21>>4, 21&0x0F, GPIO_OUTPUT_FN_SEL_MASK, GPIO_PIN_OUTPUT_LOW);
    //gpio_configurePin(22>>4, 22&0x0F, GPIO_OUTPUT_FN_SEL_MASK, GPIO_PIN_OUTPUT_LOW);
    //gpio_configurePin(23>>4, 23&0x0F, GPIO_OUTPUT_FN_SEL_MASK, GPIO_PIN_OUTPUT_LOW);
    gpio_configurePin(24>>4, 24&0x0F, GPIO_OUTPUT_FN_SEL_MASK, GPIO_PIN_OUTPUT_LOW);
}

#if 1

#include "devicelpm.h"

typedef struct
{
    UINT8    sleepMode;
    UINT8    idleThresholdHost;
    UINT8    idleThresholdHC;

    UINT8    btWakeActiveMode:1;                    // Bit 0
    UINT8    hostWakeActiveMode:1;                  // Bit 1
    UINT8    allowHostSleepDuringSCO:1;             // Bit 2
    UINT8    combineSleepModeAndLPM:1;              // Bit 3
    UINT8    enableTristateControlofUART_TxLine:1;  // Bit 4
    UINT8    activeConnectionHandlingOnSuspend:1;   // Bit 5
    UINT8    pulsedHostWake:1;                      // Bit 6
    UINT8    enableBreakToHost:1;                   // Bit 7

    UINT8    resumeTimeout;
    UINT8    sleepEnable;
    UINT8    btWakeGpio;

}PMU_SLEEP_MODE_CONFIGURE;

typedef void (*HCIUTIL_TOTALTIME_CB)(UINT32 sleepTime);

extern UINT8 bleprofile_powersave;
extern HCIUTIL_TOTALTIME_CB hciutil_updateTotalSleepTimeCb;
extern BLEPROFILE_QUERY_PARAM_CB bleprofile_queryPowersaveCb;
extern PMU_SLEEP_MODE_CONFIGURE pmu_sleepModeConfig;
UINT64 blegm_longtimeout = 0;
UINT8 *blegm_p_longsleepwake = NULL;
UINT8 blegm_sleepEnable=0;

void blegm_updatelongtimedSleep(UINT32 sleepTime);
UINT32 blegm_queryPowersave(LowPowerModePollType type, UINT32 context);

void blegm_longtimedSleep(UINT16 min, UINT8 *p_longsleepwake)
{
    if(bleprofile_powersave) //eliminate another sleep during sleep
    {
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "long timed Sleep Blocked", TVF_D(0));
        return;
    }

    blegm_p_longsleepwake = p_longsleepwake;
    blegm_longtimeout = ((UINT64)min)*60*1000*1000; //us
    blegm_sleepEnable = pmu_sleepModeConfig.sleepEnable;
    pmu_sleepModeConfig.sleepEnable = TRUE;
    hciutil_updateTotalSleepTimeCb = (HCIUTIL_TOTALTIME_CB)blegm_updatelongtimedSleep;
    bleprofile_queryPowersaveCb = (BLEPROFILE_QUERY_PARAM_CB)blegm_queryPowersave;
    bleprofile_PreparePowersave();
    bleprofile_StartPowersave();

#if 0
    //for test only should be removed
    //bleprofile_GPIOOn();
    bleprofile_puart_p_cfg->txpin |= PUARTDISABLE;
    bleprofile_PUARTTxOn();
    bleprofile_puart_p_cfg->txpin &= ~PUARTDISABLE;
#endif
}

void blegm_longtimedWake(UINT8 reason)
{
    if(reason == 2)
    {
        //other button is pressed
        // bleprofile_powersave = 0 already
        // bleprofile_StopPowersave(); already called
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "long timed Wake En", TVF_D(0));
    }
    else
    {
        if(bleprofile_powersave == 0) //eliminate another wakeup during active
        {
            TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "long timed Wake Blocked", TVF_D(0));
            return;
        }
    }

    pmu_sleepModeConfig.sleepEnable = blegm_sleepEnable;
    hciutil_updateTotalSleepTimeCb= NULL;
    bleprofile_queryPowersaveCb = NULL;
    *blegm_p_longsleepwake=reason+1;

    if(reason > 0)
    {
        //button wakeup
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Button Wake", TVF_D(0));
    }
    else
    {
        //timed wakeup
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Timed Wake", TVF_D(0));
    }


    if(bleprofile_powersave)
    {
        bleprofile_StopPowersave();
    }

    //blegmroche_connDown();

    // turn on ADV only
    if(bleprofile_p_cfg->default_adv == MANDATORY_DISCOVERABLE)
    {
        bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    }
    else
    {
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, NULL);
    }
}

void blegm_updatelongtimedSleep(UINT32 sleepTime)
{
    if(blegm_longtimeout >= sleepTime)
    {
        blegm_longtimeout -= sleepTime;
    }
    else
    {
        blegm_longtimeout = 0;
    }

    //small amount sleep is not permitted
    //if(sleepTime == 0)
    if(blegm_longtimeout < 3000) // 3ms	// problem found with 2.162ms
    {
        blegm_longtimeout = 0;
    }
}

UINT32 blegm_queryPowersave(LowPowerModePollType type, UINT32 context)
{
    if(bleprofile_powersave)
    {
        if(type == LOW_POWER_MODE_POLL_TYPE_SLEEP)
        {
            if(blegm_longtimeout == 0)
            {
                blegm_longtimedWake(0);
                return 0;
            }
            else
            {
                UINT32 longtimeout;
                if(blegm_longtimeout>>32)
                {
                    longtimeout = 0xFFFFFFFF;
                }
                else
                {
                    longtimeout = blegm_longtimeout&0xFFFFFFFF;
                }

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blegm_queryPowersave(SLEEP): %08x", TVF_D(longtimeout));
                return longtimeout;
            }
        }
    }

    return 0;
}

#endif

#if 1 //application timer implementation
// This timer has 12.5ms resolution
// Using BUZTimer for this purpose
// When using GMTimer, BUZTimer is not usable
#include "timer.h"

void blegm_startGMTimer( BLEAPP_TIMER_CB cb, UINT16 ms_timeout )
{
    UINT16 call_per_sec = 1000/ms_timeout;
    INT16 timeOut;

    if(call_per_sec == 0)
    {
        call_per_sec = 1;
    }
    else if(call_per_sec > TIMER_1_TICKS_SEC) //TIMER_1_TICKS_SEC = 80 for 20732A0
    {
        call_per_sec = TIMER_1_TICKS_SEC;
    }

    timeOut = TIMER_1_TICKS_SEC/call_per_sec;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "GM Timer(%d ms, %d/sec)", TVF_WW(ms_timeout, call_per_sec));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "GM Timer(%d timeout)", TVF_D(timeOut));

    bleapptimer_startBUZTimer(cb, timeOut);
}

void blegm_stopGMTimer( void )
{
    bleapptimer_stopBUZTimer();
}
#endif

#if 1 //ADV data with seqence number
#define MANUFACTURE_DATA_LEN 4 // 2 bytes company id + seq number
void blegm_GenerateADVData(UINT16 company_id, UINT16 seq_num)
{
    BLE_ADV_FIELD adv[4];

    // flags
    adv[0].len = FLAGS_LEN+1;
    adv[0].val = ADV_FLAGS;
    adv[0].data[0] = LE_GENERAL_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;

    // name
    adv[1].len = strlen(bleprofile_p_cfg->local_name) + 1;
    if(adv[1].len > (LOCAL_NAME_LEN_MAX+COD_LEN-MANUFACTURE_DATA_LEN)+1) //remove COD, add manufacture data
    {
        adv[1].len = (LOCAL_NAME_LEN_MAX+COD_LEN-MANUFACTURE_DATA_LEN)+1;
        adv[1].val = ADV_LOCAL_NAME_SHORT;
    }
    else
    {
        adv[1].val = ADV_LOCAL_NAME_COMP;
    }
    memcpy(adv[1].data, bleprofile_p_cfg->local_name, adv[1].len-1);

    // service UUID
    adv[2].len = UUID_LEN + 1;
    adv[2].val = ADV_SERVICE_UUID16_COMP;
    memcpy(adv[2].data, (void *) &(bleprofile_p_cfg->serv[0]), UUID_LEN);

    // manufacture data
    adv[3].len = MANUFACTURE_DATA_LEN + 1;
    adv[3].val = ADV_MANUFACTURER_DATA;
    adv[3].data[0] = company_id&0xFF;
    adv[3].data[1] = company_id>>8;
    adv[3].data[2] = seq_num&0xFF;
    adv[3].data[3] = seq_num>>8;

    bleprofile_GenerateADVData(adv, 4);
}
#endif


void blegm_randCb(char *rand)
{
    blegm_rand_received++;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "<random number %d>", TVF_D(blegm_rand_received));
    ble_tracen(rand, 8);

    if(blegm_rand_received < 4)
    {
        lesmp_getRandwithCb(blegm_randCb);
    }
}
