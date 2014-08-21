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
* BLE Generic profile, service, application
*
* This is a ROM code for BLE generic device.
* An application can use any portions of this code to simplify development 
* and reduce download and startup time.  
*
* GATT DB for the device contains mandatory GATT and GAP service description
* with minimal set of characteristics.  During initialization the app 
* registers with LE stack to receive various notifications including bonding 
* complete, connection status change.  When device is successfully bonded, 
* application saves peer's Bluetooth Device address to the NVRAM.  
*
* Features demonstrated
*  - GATT database and Device configuration initialization
*  - Registration with LE stack for various events
*  - NVRAM read/write operation
*
* To demonstrate the app, work through the following steps.
* 1. Plug the WICED eval board into your computer
* 2. Build and download the application (to the WICED board)
* 3. Pair with a client
*
*/
#include "blegen.h"

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////
void blegen_Timeout(UINT32 count);
void blegen_FineTimeout(UINT32 finecount);
void blegen_handleUART(char *gen_char);
void blegen_DBInit(void);
void blegen_connUp(void);
void blegen_connDown(void);
void blegen_advStop(void);
void blegen_appTimerCb(UINT32 arg); 
void blegen_appFineTimerCb(UINT32 arg);
void blegen_smpBondResult(LESMP_PARING_RESULT  result);
void blegen_encryptionChanged(HCI_EVT_HDR *evt);
void blegen_IndicationConf(void);


//This function is for GEN test
void blegen_FakeUART(char *gen_char, UINT32 count);


extern void bleprofile_regAppEvtHandler(BLECM_APP_EVT_ENUM idx,BLECM_NO_PARAM_FUNC func);


//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

PLACE_IN_DROM const UINT8 blegen_db_data[]=
{
0x01, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x04,                       // length
    0x00, 0x28,                 // uuid - primary service 0x2800
    0x01, 0x18,                 // GATT service 0x1801
0x02, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x07,                       // length
    0x03, 0x28,                 // uuid - characteristic 0x2803
    0x10,0x03,0x00,0x05,0x2a,   // GATT_CHAR_PROP_BIT_NOTIFY, value handle 0x03, uuid 0x2a05 // Arvind I think the first byte should be 0x20
0x03, 0x00,                     // handle
    LEGATTDB_PERM_NONE,         // permission - not readable
    0x06,                       // length
    0x05, 0x2a,                 // GATT_UUID_GATT_SRV_CHGD 0x2a05
    0x00, 0x00, 0x00, 0x00,
0x14, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x04,                       // length
    0x00, 0x28,                 // uuid - primary service 0x2800
    0x00, 0x18,                 // GAP service 0x1800
0x15, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x07,                       // length
    0x03, 0x28,                 // uuid - characteristic 0x2803
    0x02,0x16,0x00,0x00,0x2a,   // GATT_CHAR_PROP_BIT_READ, value handle 0x16, uuid 0x2a00
0x16, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x12,                       // length
    0x00, 0x2a,                 // GATT_UUID_GAP_DEVICE_NAME 0x2a00
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // " "
0x17, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x07,                       // length
    0x03, 0x28,                 // uuid - characteristic 0x2803
    0x02,0x18,0x00,0x01,0x2a,   // GATT_CHAR_PROP_BIT_READ, value handle 0x16, uuid 0x2a01
0x18, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x04,                       // length
    0x01, 0x2a,                 // GATT_UUID_GAP_APPEARANCE 0x2a01
    0x00,0x00,	

0x2d, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x04,                       // length
    0x00, 0x28,                 // uuid - primary service 0x2800
    0x0a, 0x18,                 // service UUID_SERVCLASS_DEVICE_INFO 0x180a
0x2e, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x07,                       // length
    0x03, 0x28,                 // uuid - characteristic 0x2803
    0x02,0x2f,0x00,0x29,0x2a,   // GATT_CHAR_PROP_BIT_READ, value handle 0x2f, uuid 0x2a29
0x2f, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x0a,                       // length
    0x29, 0x2a,                 // uuid UUID_CHARACTERISTIC_MANUFACTURER_NAME_STRING 0x2a29
    0x42, 0x72, 0x6f, 0x61, 0x64, 0x63, 0x6f, 0x6d, // "Broadcom"
0x30, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x07,                       // length
    0x03, 0x28,                 // uuid - characteristic 0x2803
    0x02,0x31,0x00,0x24,0x2a,   // GATT_CHAR_PROP_BIT_READ, value handle 0x31, uuid 0x2a24
0x31, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x0a,                       // length
    0x24, 0x2a,                 // uuid UUID_CHARACTERISTIC_MODEL_NUMBER_STRING 0x2a24
    0x31,0x32,0x33,0x34,0x00,0x00,0x00,0x00,
0x32, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x07,                       // length
    0x03, 0x28,                 // uuid - characteristic 0x2803
    0x02,0x33,0x00,0x23,0x2a,   // GATT_CHAR_PROP_BIT_READ, value handle 0x31, uuid 0x2a23
0x33, 0x00,                     // handle
    LEGATTDB_PERM_READABLE,     // permission - peer can read only
    0x0a,                       // length
    0x23, 0x2a,                 // uuid System ID.
    0x00,0x01,0x02,0x03,0x4,0x5,0x6,0x7
};

const UINT16 blegen_db_size = sizeof(blegen_db_data);

PLACE_IN_DROM const BLE_PROFILE_CFG blegen_cfg =
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
    "BLE Generic", //char local_name[LOCAL_NAME_LEN_MAX];
    "\x00\x00\x00", //char cod[COD_LEN];
    "1.00", //char ver[VERSION_LEN];
    0, // UINT8 encr_required; // if 1, encryption is needed before sending indication/notification
    0, // UINT8 disc_required;// if 1, disconnection after confirmation
    0, //UINT8 test_enable;   //TEST MODE is enabled when 1
    0x04,  //  UINT8 tx_power_level; //dbm
    120, // UINT8 con_idle_timeout; //second   0-> no timeout
    0, //    UINT8 powersave_timeout; //second  0-> no timeout    
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

PLACE_IN_DROM const BLE_PROFILE_PUART_CFG blegen_puart_cfg =
{
    115200, // UINT32 baudrate; 
    32, // 31, // 32, // UINT8  txpin; //GPIO pin number //20730A0 module need to use 32 instead, normally it is 31
    33, // UINT8  rxpin; //GPIO pin number
};


// GPIO pin 31 is used for NVRAM access, all other pins are not used
PLACE_IN_DROM const BLE_PROFILE_GPIO_CFG blegen_gpio_cfg =
{
    {31,                                
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, // UINT8 gpio_pin[GPIO_NUM_MAX];  //pin number of gpio
    {GPIO_OUTPUT|GPIO_INIT_LOW|GPIO_WP,  
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0}, // UINT16 gpio_flag[GPIO_NUM_MAX]; //flag of gpio
};

UINT32 blegen_apptimer_count=0;
UINT32 blegen_appfinetimer_count=0;
UINT16 blegen_con_handle=0;
BD_ADDR  blegen_remote_addr;

UINT16 blegen_gen_hdl=0;
UINT16 blegen_gen_client_hdl=0;
UINT8 blegen_bat_enable=0;

UINT8 blegen_measurement_done=0;

//NVRAM save area
BLEPROFILE_HOSTINFO blegen_hostinfo;

BLEGEN_GEN_DATA blegen_gen_data;




void blegen_Create(void)
{
    ble_trace0("blegen_Create()");
    ble_trace0(bleprofile_p_cfg->ver);

    // dump the database to debug uart.
    legattdb_dumpDb();

    bleprofile_Init(bleprofile_p_cfg);
    bleprofile_GPIOInit(bleprofile_gpio_p_cfg);

    blegen_DBInit(); //load handle number
        
    // register connection up and connection down handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, blegen_connUp );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, blegen_connDown );
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_ADV_TIMEOUT, blegen_advStop );

    // handler for Encryption changed.
    blecm_regEncryptionChangedHandler(blegen_encryptionChanged);
    // handler for Bond result
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) blegen_smpBondResult);
#if 0
    // handler for conf
    leatt_regHandleValueConfCb((LEATT_NO_PARAM_CB) blegen_IndicationConf);
#endif

    //data init
    memset(&blegen_gen_data, 0x00, sizeof(BLEGEN_GEN_DATA));

    bleprofile_regTimerCb(blegen_appFineTimerCb, blegen_appTimerCb);
    bleprofile_StartTimer();

    blegen_connDown();	

#if 0
    bleprofile_NVRAMCheck();
#endif

    // This is for test only	
    //bleprofile_Discoverable(HIGH_UNDIRECTED_DISCOVERABLE, NULL);
    //bleprofile_Discoverable(HIGH_DIRECTED_DISCOVERABLE, bleprofile_remote_addr);

}


void blegen_FakeUART(char *bpm_char, UINT32 count)
{
    //This function does data change or setting for test

}

void blegen_Timeout(UINT32 count)
{
    //ble_trace1("Normaltimer:%d", count);	

    if(blegen_bat_enable)
    {
        blebat_pollMonitor();
    }

    bleprofile_pollPowersave();
}


void blegen_FineTimeout(UINT32 finecount)
{
    //ble_trace1("Finetimer:%d", finecount);

    {
        char gen_char[READ_UART_LEN+1];
                
        //Reading 
        bleprofile_ReadUART(gen_char);
        //ble_trace6("UART RX: %02x %02x %02x %02x %02x %02x ", 
         //      gen_char[0], gen_char[1], gen_char[2], gen_char[3], gen_char[4], gen_char[5]);

#if 1
        if(bleprofile_p_cfg->test_enable)
        {
            //This is making faking data
            //For test only
            blegen_FakeUART(gen_char, finecount);
        }
#endif

         if(gen_char[0] == 'D' && gen_char[1] == 'D') //download start
         {
            blecm_setFilterEnable(0);
            ble_trace0("CSA_filter disabled");
         }
         else if(gen_char[0] == 'A' && gen_char[1] == 'A') //download start
         {
            blecm_setFilterEnable(1);
            ble_trace0("CSA_filter enabled");

            blegen_connDown();
         }
        else  //gen main reading
        {
            blegen_handleUART(gen_char);
        }
    }    

    // button control
    bleprofile_ReadButton();	 

}

void blegen_handleUART(char *gen_char)
{
    UINT8 writtenbyte;


    //this part can be replaced by callback function
    if(bleprofile_handleUARTCb)
    {
        blegen_measurement_done = bleprofile_handleUARTCb((UINT8 *)gen_char, (UINT8 *)&blegen_gen_data);
    }
    else
    {
        //handle UART internally
        //when it is proper packet set done
        //blegen_measurement_done = 1; //1 will write data in EEPROM
    }





    // change EEPROM and GATT DB
    if(blegen_measurement_done == 1) //if connected and encrpted, old data is sent
    {	
        //write whole in EEPROM
         writtenbyte = bleprofile_WriteNVRAM(VS_BLE_GEN_DATA, sizeof(BLEGEN_GEN_DATA), (UINT8 *)&blegen_gen_data);
        ble_trace1("NVRAM GEN write:%04x", writtenbyte);

        blegen_measurement_done = 2;
        //blegen_measurement_done = 0; //this will permit multiple measurement
    }


    //save data to GATT DB and send data with indication/notification

}


void blegen_DBInit(void)
{
    int i;
    //load handle number
    
    for(i=0; i<HANDLE_NUM_MAX; i++)
    {
        if(bleprofile_p_cfg->serv[i]==UUID_SERVICE_BATTERY &&
            bleprofile_p_cfg->cha[i] == UUID_CHARACTERISTIC_BATTERY_LEVEL)
        {
            blegen_bat_enable = 1;
            blebat_Init();
        }
    }	
#if 0
    //writing test
    memset(&(db_pdu.pdu[1]), 0x01, db_pdu.len-1);
    bleprofile_WriteHandle(blegen_gen_hdl, &db_pdu);
    bleprofile_ReadHandle(blegen_gen_hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
#endif   	


}

void blegen_connUp(void)
{
    blegen_con_handle = (UINT16)emconinfo_getConnHandle();

    if(bleprofile_p_cfg->encr_required==0)  
    {
        //set variable when encryption is not requested
    }    
                
    // print the bd address.
    memcpy(blegen_remote_addr, (UINT8 *)emconninfo_getPeerAddr(), sizeof(blegen_remote_addr));

    ble_trace3("\rblegen_connUp: %08x%04x %d", 
                      (blegen_remote_addr[5]<<24)+(blegen_remote_addr[4]<<16)+
                      (blegen_remote_addr[3]<<8)+blegen_remote_addr[2], 
                      (blegen_remote_addr[1]<<8)+blegen_remote_addr[0], 
                      blegen_con_handle);

    bleprofile_Discoverable(NO_DISCOVERABLE, NULL);
}

void blegen_connDown(void)
{
    ble_trace3("\rblegen_connDown: %08x%04x %d", 
                      (blegen_remote_addr[5]<<24)+(blegen_remote_addr[4]<<16)+
                      (blegen_remote_addr[3]<<8)+blegen_remote_addr[2], 
                      (blegen_remote_addr[1]<<8)+blegen_remote_addr[0], 
                      blegen_con_handle);
        

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
        bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegen_hostinfo);
                
        bleprofile_Discoverable(bleprofile_p_cfg->default_adv, blegen_hostinfo.bdAddr);

        ble_trace3("\rADV start: %08x%04x %d", 
                      (blegen_hostinfo.bdAddr[5]<<24)+(blegen_hostinfo.bdAddr[4]<<16)+
                      (blegen_hostinfo.bdAddr[3]<<8)+blegen_hostinfo.bdAddr[2],
                      (blegen_hostinfo.bdAddr[1]<<8)+blegen_hostinfo.bdAddr[0], 
                      blegen_con_handle);
    }

    blegen_con_handle = 0; //reset connection handle
    blegen_measurement_done = 0;
}

void blegen_advStop(void)
{
    ble_trace0("ADV stop!!!!");
}

void blegen_appTimerCb(UINT32 arg) 
{
    switch(arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
            {
                blegen_apptimer_count++;

                blegen_Timeout(blegen_apptimer_count);
            }
            break;

    }
}

void blegen_appFineTimerCb(UINT32 arg)
{
    blegen_appfinetimer_count++;

    blegen_FineTimeout(blegen_appfinetimer_count);
}


void blegen_smpBondResult(LESMP_PARING_RESULT  result)
{
    ble_trace1("\rblegen, bond result %02x", result);

    if(result == LESMP_PAIRING_RESULT_BONDED)
    {
         // saving bd_addr in nvram	 
        UINT8 *bda;
         UINT8 writtenbyte;
         
        bda =(UINT8 *)emconninfo_getPeerAddr();

        memcpy(blegen_hostinfo.bdAddr, bda, sizeof(BD_ADDR));
                 
        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegen_hostinfo);
        //writtenbyte = bleprofile_WriteNVRAM(0x70, 6, (UINT8 *)&blegen_hostinfo);

        ble_trace1("NVRAM write:%04x", writtenbyte);    	  
    }
}

void blegen_encryptionChanged(HCI_EVT_HDR *evt)
{
    ble_trace0("\rblegen, encryption changed");

    if(bleprofile_ReadNVRAM(VS_BLE_HOST_LIST, sizeof(BLEPROFILE_HOSTINFO), (UINT8 *)&blegen_hostinfo))
    {
        if(memcmp(blegen_hostinfo.bdAddr, emconninfo_getPeerAddr(), 6)==0)
        {
            ble_trace2("\rEncOn for Last paired device: %08x%04x", 
                      (blegen_hostinfo.bdAddr[5]<<24)+(blegen_hostinfo.bdAddr[4]<<16)+
                      (blegen_hostinfo.bdAddr[3]<<8)+blegen_hostinfo.bdAddr[2],
                      (blegen_hostinfo.bdAddr[1]<<8)+blegen_hostinfo.bdAddr[0]);            
        }
    }
        
    if(bleprofile_p_cfg->encr_required)  
    {
        // set variable when encryption is requested
    } 
}



void blegen_IndicationConf(void)
{
    //handle disconnection when needed
}
