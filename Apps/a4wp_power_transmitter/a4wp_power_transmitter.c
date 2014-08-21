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
* BLE A4WP Power Transfer Unit (PTU)
*
* The BLE PTU application is designed to connect and access services
* of the BLE PRU device. When the application is started, it scans for
* PRU device. It filters the A4WP devices based on RSSI and A4WP specific
* ADV data. As the A4WP ADV data contains the primary service handle, the
* PTU is not required to perform the Service discovery for finding the
* characteristic handles. PRU characteristic handles are arrived from
* primary service handle and used directly to meet the 500ms registration
* perid for A4WP connection setup. As part of the registarion procedure,
* PTU exchanges the characteristic values as per BSS 1.2.
* After the connection setup, the PTU will continue to read the dynamic
* parameter every 250ms.
* PTU application is the sample application to simulate the A4WP BLE protocol
* and the Power Transfer logic to supply power to PRU is not available. Sample
* test code is available to configure ADC, PWM, LED and control GPIOs. These
* may be implemented to suppport with the descrete component to supply power
* to PRU to charge.
*
* Features demonstrated
*  - Registration with LE stack for various events
*  - Connection setup with PRU and continue reading the dynamic parameter
*  - Connection with multiple PRU devices.
*  - As a master processing notifications/indications from the server
*  - Test code to verify the ADC, PWM, Control and LED functionslity.
*
* To demonstrate the app, work through the following steps.
* 1. Build and download the application
* 2. Make the PRU to advertise
* 3. PTU will automatically conect with PRU and starts reading dynamic parameters.
* 4. Monitor the communication between PTU and PRU throguh debug or OTA traces.
*/
#include "bleprofile.h"
#include "a4wp_power_transmitter.h"
#include "aclk.h"
#include "pwm.h"
#include "blecen.h"
#include "spar_utils.h"

/******************************************************
 *               Function Prototypes
 ******************************************************/
static void    bleptum_Create(void);
static void    bleptum_connUp(void);
static void    bleptum_connDown(void);
static void    bleptum_Timeout(UINT32 count);
static void    bleptum_FineTimeout(UINT32 count);
static void    bleptum_appTimerCb(UINT32 arg);
static void    bleptum_appFineTimerCb(UINT32 arg);
static void    bleptum_leAdvReportCb(HCIULP_ADV_PACKET_REPORT_WDATA *evt);
static void    bleptum_AttReq(UINT32 count);
static void    bleptum_AttRspCb(int len, int attr_len, UINT8 *data);
static void    bleptum_Notification(int len, int attr_len, UINT8 *data);
static void    bleptum_Indication(int len, int attr_len, UINT8 *data);
static void    bleptum_smpBondResult(LESMP_PARING_RESULT result);
static void    bleptum_encryptionChanged(HCI_EVT_HDR *evt);
static void    bleptum_start_transaction_timer(int index);
static void    bleptum_stop_transaction_timer(int index);
static void    bleptum_check_transaction_timer(UINT32 count);
#ifdef BLEPTUM_PWM_TEST
static void    bleptum_pwm_init(void);
static void    bleptum_pwm_test(UINT32 count);
static void    bleptum_pwm_off(void);
#endif
#ifdef BLEPTUM_ADC_TEST
static void    bleptum_adc_init(void);
static void    bleptum_adc_test(UINT32 count);
#endif
static void    bleptum_CenTimerCb( UINT32 arg);

extern void    blecen_connUp(void);
extern void    blecen_appTimerCb(UINT32 arg); 
extern void    blecen_appFineTimerCb(UINT32 arg);
extern void    blecen_smpBondResult(LESMP_PARING_RESULT  result);
extern void    blecen_encryptionChanged(HCI_EVT_HDR *evt);

/******************************************************
 *               Variables Definitions
 ******************************************************/
const BLE_PROFILE_CFG bleptum_cfg =
{
    /*.fine_timer_interval            =*/ 12,   // ms
    /*.default_adv                    =*/ NO_DISCOVERABLE,
    /*.button_adv_toggle              =*/ 0,    // pairing button make adv toggle (if 1) or always on (if 0)
    /*.high_undirect_adv_interval     =*/ 32,   // slots
    /*.low_undirect_adv_interval      =*/ 2048, // slots
    /*.high_undirect_adv_duration     =*/ 0,    // seconds
    /*.low_undirect_adv_duration      =*/ 0,    // seconds
    /*.high_direct_adv_interval       =*/ 0,    // seconds
    /*.low_direct_adv_interval        =*/ 0,    // seconds
    /*.high_direct_adv_duration       =*/ 0,    // seconds
    /*.low_direct_adv_duration        =*/ 0,    // seconds
    /*.local_name                     =*/ "BLE PTU", // [LOCAL_NAME_LEN_MAX];
    /*.cod                            =*/ BIT16_TO_8(0x0000),0x00, // [COD_LEN];
    /*.ver                            =*/ "1.00",         // [VERSION_LEN];
    /*.encr_required                  =*/ 0, // (SECURITY_ENABLED | SECURITY_REQUEST),
#ifdef BLEPTUM_TRANS_TO_DISCONNECTION
                                          BLEPTUM_TRANS_TO_DISCONNECTION |
#endif
#ifdef BLEPTUM_HCI_RESET
                                          BLEPTUM_HCI_RESET |
#endif
#ifdef BLEPTUM_WDTO
                                          BLEPTUM_WDTO |
#endif
    /*.disc_required                  =*/ 0,    // actions on disconnection
    /*.test_enable                    =*/ 0,    // TEST MODE is enabled when 1
    /*.tx_power_level                 =*/ 0,    // dbm
    /*.con_idle_timeout               =*/ 0,    // second  0-> no timeout
    /*.powersave_timeout              =*/ 0,    // second  0-> no timeout
    /*.hdl                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00}, // [HANDLE_NUM_MAX];
    /*.serv                           =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.cha                            =*/ {0x00, 0x00, 0x00, 0x00, 0x00},
    /*.findme_locator_enable          =*/ 0,    // if 1 Find me locator is enable
    /*.findme_alert_level             =*/ 0,    // alert level of find me
    /*.client_grouptype_enable        =*/ 1,    // if 1 grouptype read can be used
    /*.linkloss_button_enable         =*/ 0,    // if 1 linkloss button is enable
    /*.pathloss_check_interval        =*/ 0,    // second
    /*.alert_interval                 =*/ 0,    // interval of alert
    /*.high_alert_num                 =*/ 0,    // number of alert for each interval
    /*.mild_alert_num                 =*/ 0,    // number of alert for each interval
    /*.status_led_enable              =*/ 0,    // if 1 status LED is enable
    /*.status_led_interval            =*/ 0,    // second
    /*.status_led_con_blink           =*/ 0,    // blink num of connection
    /*.status_led_dir_adv_blink       =*/ 0,    // blink num of dir adv
    /*.status_led_un_adv_blink        =*/ 0,    // blink num of undir adv
    /*.led_on_ms                      =*/ 0,    // led blink on duration in ms
    /*.led_off_ms                     =*/ 0,    // led blink off duration in ms
    /*.buz_on_ms                      =*/ 0  ,  // buzzer on duration in ms
    /*.button_power_timeout           =*/ 0,    // seconds
    /*.button_client_timeout          =*/ 1,    // seconds
    /*.button_discover_timeout        =*/ 3,    // seconds
    /*.button_filter_timeout          =*/ 10,   // seconds
#ifdef BLE_UART_LOOPBACK_TRACE
    /*.button_uart_timeout            =*/ 15,   // seconds
#endif
};

// Following structure defines UART configuration
const BLE_PROFILE_PUART_CFG bleptum_puart_cfg =
{
    /*.baudrate   =*/ 115200,
    /*.txpin      =*/ BLEPTUM_GPIO_PIN_UART_TX,
    /*.rxpin      =*/ BLEPTUM_GPIO_PIN_UART_RX,
};

// Following structure defines GPIO configuration used by the application
const BLE_PROFILE_GPIO_CFG bleptum_gpio_cfg =
{
    /*.gpio_pin =*/
    {
        BLEPTUM_GPIO_PIN_WP,    // This need to be used to enable/disable NVRAM write protect
        BLEPTUM_GPIO_PIN_ADC1,  // ADC1 GPIO used to configure for sample ADC channel
        BLEPTUM_GPIO_PIN_ADC2,  // ADC2 GPIO used to configure for sample ADC channel
        BLEPTUM_GPIO_PIN_ADC3,  // ADC3 GPIO used to configure for sample ADC channel
        BLEPTUM_GPIO_PIN_LED1,  // LED1 GPIO used to configure for sample LED
        BLEPTUM_GPIO_PIN_CTR1,  // Sample Control channel configuration
        BLEPTUM_GPIO_PIN_PWM1,  // Sample PWM channle configuration
        -1, -1, -1, -1, -1, -1, -1, -1, -1 // other GPIOs are not used
    },
    /*.gpio_flag =*/
    {
        GPIO_OUTPUT | GPIO_INIT_LOW | GPIO_WP,
        GPIO_INPUT  | GPIO_INIT_LOW,
        GPIO_INPUT  | GPIO_INIT_LOW,
        GPIO_INPUT  | GPIO_INIT_LOW,
        GPIO_OUTPUT | GPIO_INIT_HIGH,
        GPIO_OUTPUT | GPIO_INIT_LOW,
        GPIO_OUTPUT | GPIO_INIT_LOW,
        0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

BLEPTUM_APP_STATE   *bleptum_app_state  = NULL;
BLEPTUM_INFO        *bleptum_info       = NULL;

// Following variables are in ROM 
extern BLE_CEN_CFG      blecen_cen_cfg;
extern BLEAPP_TIMER_CB  blecen_usertimerCb;
extern UINT8            bleapp_trace_enable;

/******************************************************
 *               Function Definitions
 ******************************************************/

// Application initialization
APPLICATION_INIT()
{
    bleapp_set_cfg(NULL,
                   0,
                   (void *)&bleptum_cfg,
                   (void *)&bleptum_puart_cfg,
                   (void *)&bleptum_gpio_cfg,
                   bleptum_Create);

    bleapp_trace_enable = 1;
}

// Create bleptum application
static void bleptum_Create(void)
{
    // update the default parameter of blecen for bleptum application
    blecen_cen_cfg.high_conn_min_interval       = 40;   //50ms
    blecen_cen_cfg.high_conn_max_interval       = 40;   //50ms
    blecen_cen_cfg.high_supervision_timeout     = 100;  //1sec
    blecen_cen_cfg.high_scan_duration           = 5;    //5sec
    blecen_cen_cfg.low_conn_min_interval        = 40;   //50ms
    blecen_cen_cfg.low_conn_max_interval        = 40;   //50ms
    blecen_cen_cfg.low_supervision_timeout      = 100;  //1sec
    blecen_cen_cfg.low_scan_duration            = 5;    //5sec
    blecen_cen_cfg.filter_duplicates            = HCIULP_SCAN_DUPLICATE_FILTER_OFF;

    // register application timer callback with blecen to control LE scan.
    blecen_usertimerCb = bleptum_CenTimerCb;

    // Allocate the memory for bleptm application state and initialize to zero.
    bleptum_app_state = (BLEPTUM_APP_STATE *)cfa_mm_Sbrk(sizeof(BLEPTUM_APP_STATE));
    memset(bleptum_app_state, 0x00, sizeof(BLEPTUM_APP_STATE));

    // update the maximum connections supported in application
    bleptum_app_state->bleptu_max_con = BLEPTUM_MAX_CON;

    // CCC value updated for alert characteristic
    bleptum_app_state->bleptu_pru_alert_ccc = CCC_NOTIFICATION | CCC_INDICATION;

    // PRU control parameter values
    bleptum_app_state->bleptu_pru_control[0]    = 0x80;
    bleptum_app_state->bleptu_pru_control[1]    = 0;
    bleptum_app_state->bleptu_pru_control[2]    = 0;
    bleptum_app_state->bleptu_pru_control[3]    = 0;
    bleptum_app_state->bleptu_pru_control[4]    = 0;

    // PTU static parameter values
    bleptum_app_state->bleptu_ptu_static[0]     = 0;
    bleptum_app_state->bleptu_ptu_static[1]     = 0x54;
    bleptum_app_state->bleptu_ptu_static[2]     = 0;
    bleptum_app_state->bleptu_ptu_static[3]     = 0;
    bleptum_app_state->bleptu_ptu_static[4]     = 0xF7;
    bleptum_app_state->bleptu_ptu_static[5]     = 0x8F;
    bleptum_app_state->bleptu_ptu_static[6]     = 0x1;
    bleptum_app_state->bleptu_ptu_static[7]     = 0xA0;
    bleptum_app_state->bleptu_ptu_static[8]     = 0xF0;
    bleptum_app_state->bleptu_ptu_static[9]     = 0x1;
    bleptum_app_state->bleptu_ptu_static[10]    = 0x2;
    bleptum_app_state->bleptu_ptu_static[11]    = 0;
    bleptum_app_state->bleptu_ptu_static[12]    = 0;
    bleptum_app_state->bleptu_ptu_static[13]    = 0;
    bleptum_app_state->bleptu_ptu_static[14]    = 0;
    bleptum_app_state->bleptu_ptu_static[15]    = 0;
    bleptum_app_state->bleptu_ptu_static[16]    = 0;

    // Set up the config feature to change the ptu behavior
    bleptum_app_state->bleptu_config            = BLEPTUM_SMP_ERASE_KEY |
                                                // BLEPTUM_SMP_PAIRING |
                                                  0;

    // clear the bonded information on application start
    if (bleptum_app_state->bleptu_config & BLEPTUM_SMP_ERASE_KEY)
    {
        lesmpkeys_removeAllBondInfo();
        ble_trace0("Pairing Key removed");
    }

    // reserve memory and initialize for per connection structure for all connections
    bleptum_info = (BLEPTUM_INFO *)cfa_mm_Sbrk(sizeof(BLEPTUM_INFO) *
                                               bleptum_app_state->bleptu_max_con);
    memset(bleptum_info, 0x00,
           sizeof(BLEPTUM_INFO) * bleptum_app_state->bleptu_max_con);

    // enable multi connection
    blecm_ConMuxInit(bleptum_app_state->bleptu_max_con);
    blecm_enableConMux();

    blecen_Create();

    // register connection up and down callback handler.
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_UP, bleptum_connUp);
    bleprofile_regAppEvtHandler(BLECM_APP_EVT_LINK_DOWN, bleptum_connDown);

    // register callback handler for encryption changed notification
    blecm_regEncryptionChangedHandler(bleptum_encryptionChanged);

    // register handler for bond result notification
    lesmp_regSMPResultCb((LESMP_SINGLE_PARAM_CB) bleptum_smpBondResult);

    // register handler for Adv Report indications
    blecm_RegleAdvReportCb((BLECM_FUNC_WITH_PARAM)bleptum_leAdvReportCb);

    // register handlers for alert notifcation and indications from stack
    leatt_regNotificationCb((LEATT_TRIPLE_PARAM_CB) bleptum_Notification);
    leatt_regIndicationCb((LEATT_TRIPLE_PARAM_CB) bleptum_Indication);

    // register handlers for ATT client response callbacks
    leatt_regReadRspCb((LEATT_TRIPLE_PARAM_CB) bleptum_AttRspCb);
    leatt_regReadByTypeRspCb((LEATT_TRIPLE_PARAM_CB) bleptum_AttRspCb);
    leatt_regReadByGroupTypeRspCb((LEATT_TRIPLE_PARAM_CB) bleptum_AttRspCb);
    leatt_regWriteRspCb((LEATT_NO_PARAM_CB) bleptum_AttRspCb);

#ifdef BLEPTUM_PWM_TEST
    // initialize and configured the PWM
    bleptum_pwm_init();
#endif

#ifdef BLEPTUM_ADC_TEST
    // initialize the ADC, LED and control GPIOs
    bleptum_adc_init();
#endif

    // register for application and fine timer callbacks and start the timer.
    bleprofile_KillTimer();
    bleprofile_regTimerCb(bleptum_appFineTimerCb, bleptum_appTimerCb);
    bleprofile_StartTimer();
}

// Finds the free device instance when initiating the connection with PRU.
static int bleptum_FindFreeIndex(void)
{
    int i;

    for (i = 0; i < bleptum_app_state->bleptu_max_con; i ++)
    {
        // zero connection handle is the unused device instance
        if (bleptum_info[i].bleptu_conhandle == 0)
        {
            // free instance found
            return i;
        }
    }

    return -1;
}

// Finds the device instance with matching connection handle
static int bleptum_FindIndex(UINT16 conhandle)
{
    int i;

    for (i = 0; i < bleptum_app_state->bleptu_max_con; i ++)
    {
        if (bleptum_info[i].bleptu_conhandle == conhandle)
        {
            // matching device instance found
            return i;
        }
    }

    return -1;
}

// Finds the device instance with matching BD address
static int bleptum_FindIndexByBDADDR(UINT8 *bdaddr)
{
    int i;

    for (i = 0; i < bleptum_app_state->bleptu_max_con; i ++)
    {
        if (memcmp(bleptum_info[i].bleptu_remote_addr,
                   bdaddr, BD_ADDRESS_SIZE) == 0)
        {
            // matching device instance found
            return i;
        }
    }

    return -1;
}

// Marks the device instance as free and resets the details to zero
static void bleptum_DelIndex(int index)
{
    if (index < bleptum_app_state->bleptu_max_con)
    {
        memset(&(bleptum_info[index]), 0x00, sizeof(BLEPTUM_INFO));
    }
}

// LE connection complete callback
static void bleptum_connUp(void)
{
    int     index;
    int     lindex;
    UINT8   *p_remote_addr = (UINT8 *)emconninfo_getPeerAddr();

    blecen_connUp();

    // instance is already allocated and BD ADDR is updated with connection request
    lindex = bleptum_FindIndexByBDADDR(p_remote_addr);

    if (lindex < 0)
    {
        // find free instance if already not available
        lindex = bleptum_FindFreeIndex();
    }

    if (lindex >= 0)
    {
        // update the connection handle with device instance
        bleptum_info[lindex].bleptu_conhandle = emconinfo_getConnHandle();

        // get the details with same connection handle if available
        index = blecm_FindConMux(bleptum_info[lindex].bleptu_conhandle);

        if (index >= 0)
        {
            // delete the old connection information for new update
            blecm_DelConMux(index);
        }

        // find free index for update
        index = blecm_FindFreeConMux();

        // update the connection information for the connected device
        if (index >= 0)
        {
            // save dev_info
            memcpy((UINT8 *)(&(bleptum_info[lindex].bleptu_dev_info)),
                   (UINT8 *)emconinfo_getPtr(), sizeof(EMCONINFO_DEVINFO));

            // save smp_info
            memcpy((UINT8 *)(&(bleptum_info[lindex].bleptu_smp_info)),
                   (UINT8 *)lesmp_getPtr(), sizeof(LESMP_INFO));

            // update the connMux with latest details
            blecm_AddConMux(index, bleptum_info[index].bleptu_conhandle,
                            0, NULL,
                            &(bleptum_info[lindex].bleptu_dev_info),
                            &(bleptum_info[lindex].bleptu_smp_info));

            // start pairing procedure if configured in app_config
            if (bleptum_app_state->bleptu_config & BLEPTUM_SMP_PAIRING)
            {
                lesmp_startPairing(NULL);
            }
            else
            {
                // mark as SMP completed to start ATT exchange
                bleptum_info[lindex].bleptu_state = BLEPTUM_SMP_DONE;
                ble_trace2("bleptu_state[%d] = %02x",
                           lindex, bleptum_info[lindex].bleptu_state);
            }

            ble_trace2("Add %d,%d", lindex, index);
        }
    }

    bleptum_app_state->bleptu_num_con ++;

    if (bleptum_app_state->bleptu_num_con < bleptum_app_state->bleptu_max_con)
    {
        // restart scan looking for more devices
        blecen_Scan(HIGH_SCAN);
    }
}

// Disconnection callback
static void bleptum_connDown(void)
{
    UINT16  con_handle  = emconinfo_getConnHandle();
    int     lindex      = bleptum_FindIndex(con_handle);
    int     index;

    if (bleptum_app_state->bleptu_config & BLEPTUM_SMP_ERASE_KEY)
    {
        // delete the pairing info for the disconnected device
        lesmpkeys_deleteBondInfo(emconninfo_getPeerPubAddr(),
                                 emconninfo_getPeerAddrType());
        ble_trace0("Pairing Key removed");
    }

    ble_trace1("Disc_Reason: %02x", emconinfo_getDiscReason());

    if (emconinfo_getDiscReason() == BLEPTUM_HCI_ERR_CONNECTION_TIMEOUT)
    {
#ifdef BLEPTUM_HCI_RESET
        if(bleprofile_p_cfg->disc_required & BLEPTUM_HCI_RESET)
        {
            blecm_hciReset();
            ble_trace0("HCI_Reset");
        }
#endif

#ifdef BLEPTUM_WDTO
        if (bleprofile_p_cfg->disc_required & BLEPTUM_WDTO)
        {
            ble_trace0("Watchdog_timeout");
            // infinite loop to trigger WD reset
            while(1);
        }
#endif
    }

    bleptum_app_state->bleptu_num_con --;

    // start scan again
    blecen_Scan(HIGH_SCAN);

    //delete the connection structure for disconnected device
    if (lindex >= 0)
    {
        bleptum_DelIndex(lindex);
    }

    index = blecm_FindConMux(con_handle);
    if (index >= 0)
    {
        blecm_DelConMux(index);
    }

    ble_trace2("Del %d,%d", lindex, index);
}

// handles the application timeout processing, every second.
static void bleptum_Timeout(UINT32 count)
{
    // update and check for transaction timer
    bleptum_check_transaction_timer(count);
}

// handles the fine timeout application processing, every tick (12.5ms)
static void bleptum_FineTimeout(UINT32 count)
{
    // based on connection specific state, the ATT requests are triggered with slave
    bleptum_AttReq(count);

#ifdef BLEPTUM_PWM_TEST
    bleptum_pwm_test(count);
#endif

#ifdef BLEPTUM_ADC_TEST
    bleptum_adc_test(count);
#endif
}

// application timeout handler (every second)
static void bleptum_appTimerCb(UINT32 arg)
{
    switch (arg)
    {
        case BLEPROFILE_GENERIC_APP_TIMER:
        {
            bleptum_app_state->bleptu_apptimer_count ++;

            // process the application timeout
            bleptum_Timeout(bleptum_app_state->bleptu_apptimer_count);
        }
        break;
    }

    // call blecen application timer callback as the timer is configured locally
    blecen_appTimerCb(arg);
}

// Fine timeout handler
static void bleptum_appFineTimerCb(UINT32 arg)
{
    bleptum_app_state->bleptu_appfinetimer_count ++;

    // process the fine timeout in the application
    bleptum_FineTimeout(bleptum_app_state->bleptu_appfinetimer_count);

    // call blecen fine timer callback as the timer is configured locally
    blecen_appFineTimerCb(arg);
}

// LE advertise indication callback
static void bleptum_leAdvReportCb(HCIULP_ADV_PACKET_REPORT_WDATA *evt)
{
    int                 lindex;
    BLE_ADV_FIELD       *p_field;
    UINT8               *data   = (UINT8 *)(evt->data);
    UINT8               *ptr    = data;
    UINT8               dataLen = (UINT8)(evt->dataLen);
    BLEPTUM_ADV_PAYLOAD *p_adv_payload;
    UINT16              hdl;

    // report the ADV results to blecen app
    blecen_leAdvReportCb(evt);

#ifdef BLEPTUM_MIN_RSSI
    // initial filter of ADV results based on rssi value reported
    if (evt->rssi < BLEPTUM_MIN_RSSI)
    {
        return;
    }
#endif

    // parse the ADV data for A4WP specific data
    while (1)
    {
        p_field = (BLE_ADV_FIELD *)ptr;

        // A4WP payload in Service Data AD type
        // skip the length (1 byts) of AD type when comparing payload
        if ((p_field->val == ADV_SERVICE_DATA) &&
            ((p_field->len - 1) == sizeof(BLEPTUM_ADV_PAYLOAD)))
        {
            p_adv_payload = (BLEPTUM_ADV_PAYLOAD *)(p_field->data);

            ble_trace2("A4WP BLE Advertisement Received - Service UUID:0x%02X GATT:0x%04X ",
                        p_adv_payload->service_uuid,
                        p_adv_payload->gatt_primary_service_handle);

            ble_trace2("BD addr:0x%08X%04X",
                          (evt->wd_addr[5] << 24) + (evt->wd_addr[4] << 16) +
                          (evt->wd_addr[3] << 8) + evt->wd_addr[2],
                          (evt->wd_addr[1] << 8) + evt->wd_addr[0]);

            // look for A4WP PRU primary service UUID in ADV payload
            if (p_adv_payload->service_uuid == UUID_SERVCLASS_A4WP)
            {
                // make sure the device is not already found
                lindex = bleptum_FindIndexByBDADDR(evt->wd_addr);

                if (lindex < 0)
                {
                    // allocate the free available instance
                    lindex = bleptum_FindFreeIndex();

                    if (lindex >= 0)
                    {
                        memcpy(bleptum_info[lindex].bleptu_remote_addr,
                               evt->wd_addr, BD_ADDRESS_SIZE);

                        // primary service handle
                        hdl = p_adv_payload->gatt_primary_service_handle;

                        // Populate the characteristic handles from primary service handle
                        bleptum_info[lindex].bleptu_pru_control_hdl   = hdl + BLEPTUM_PRU_CONTROL_HDL_OFFSET;
                        bleptum_info[lindex].bleptu_ptu_static_hdl    = hdl + BLEPTUM_PTU_STATIC_HDL_OFFSET;
                        bleptum_info[lindex].bleptu_pru_alert_hdl     = hdl + BLEPTUM_PRU_ALERT_HDL_OFFSET;
                        bleptum_info[lindex].bleptu_pru_alert_ccc_hdl = hdl + BLEPTUM_PRU_ALERT_CCC_HDL_OFFSET;
                        bleptum_info[lindex].bleptu_pru_static_hdl    = hdl + BLEPTUM_PRU_STATIC_HDL_OFFSET;
                        bleptum_info[lindex].bleptu_pru_dynamic_hdl   = hdl + BLEPTUM_PRU_DYNAMIC_HDL_OFFSET;

                        ble_trace6("HANDLE: %04x %04x %04x %04x %04x %04x", 
                                    bleptum_info[lindex].bleptu_pru_control_hdl,
                                    bleptum_info[lindex].bleptu_ptu_static_hdl,
                                    bleptum_info[lindex].bleptu_pru_alert_hdl,
                                    bleptum_info[lindex].bleptu_pru_static_hdl,
                                    bleptum_info[lindex].bleptu_pru_dynamic_hdl,
                                    bleptum_info[lindex].bleptu_pru_alert_ccc_hdl);

                        if (blecen_GetConn() == NO_CONN)
                        {
                            ble_trace1("RSSI = %d", evt->rssi);
                            blecen_Conn(HIGH_CONN, evt->wd_addr, evt->addressType);
                            blecen_Scan(NO_SCAN);
                        }
                        else
                        {
                            //SCAN + CONN at the same time
                            ble_trace1("blecen_GetConn = %d", blecen_GetConn());
                            blecen_Conn(NO_CONN, NULL, 0);
                            blecen_Scan(HIGH_SCAN);
                        }
                    }
                }
                else
                {
                    // device alreay in connection.
                    ble_trace1("Ignoring the device: %d", lindex);
                    return;
                }
                // A4WP ADV type found
                break;
            }
        }
        // move to the next ADV structure
        ptr += (p_field->len) + 1;

        if (ptr >= data + dataLen)
        {
            // reached end of ADV data
            break;
        }
    }
}

// Function to process the requests with connected PRU(slave) devices based on the state
static void bleptum_AttReq(UINT32 count)
{
    int i;

    // traverse through each connected device instance and trigger ATT request
    for (i = 0; i < bleptum_app_state->bleptu_max_con; i ++)
    {
        if (bleptum_info[i].bleptu_conhandle)
        {
            // set connection pointer for the device
            blecm_SetPtrConMux(bleptum_info[i].bleptu_conhandle);

            // move t discovery done state as char handles are available
            if (bleptum_info[i].bleptu_state == BLEPTUM_SMP_DONE)
            {
                bleptum_info[i].bleptu_state = BLEPTUM_DISCOVER_DONE;
            }

            if (blecm_getAvailableTxBuffers() < 1)
            {
                // No Tx buffers available, can not proceed
                return;
            }

            if (bleptum_info[i].bleptu_state == BLEPTUM_DISCOVER_DONE)
            {
                bleptum_info[i].bleptu_state = BLEPTUM_W4_READ_PRU_STATIC;
                ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);

                // read the PRU static parameter
                bleprofile_sendReadReq(bleptum_info[i].bleptu_pru_static_hdl);
                bleptum_start_transaction_timer(i);
            }
            else if (bleptum_info[i].bleptu_state == BLEPTUM_READ_PRU_STATIC_DONE)
            {
                // write the PTU static parameter to PRU
                bleprofile_sendWriteCmd(bleptum_info[i].bleptu_ptu_static_hdl,
                                        bleptum_app_state->bleptu_ptu_static,
                                        BLEPTUM_PTU_STATIC_PARAM_LEN);

                bleptum_info[i].bleptu_state = BLEPTUM_WRITE_PTU_STATIC_DONE;
                ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);
            }
            else if (bleptum_info[i].bleptu_state == BLEPTUM_WRITE_PTU_STATIC_DONE)
            {
                bleptum_info[i].bleptu_state = BLEPTUM_W4_1ST_READ_PRU_DYNAMIC;
                ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);

                // first dynamic parameter read
                bleprofile_sendReadReq(bleptum_info[i].bleptu_pru_dynamic_hdl);
                bleptum_start_transaction_timer(i); 
            }
            else if (bleptum_info[i].bleptu_state == BLEPTUM_READ_1ST_PRU_DYNAMIC_DONE)
            {
                // write PRU control parameter to PRU
                bleprofile_sendWriteCmd(bleptum_info[i].bleptu_pru_control_hdl,
                                        bleptum_app_state->bleptu_pru_control,
                                        BLEPTUM_PRU_CONTROL_PARAM_LEN);

                bleptum_info[i].bleptu_state = BLEPTUM_WRITE_PRU_CONTROL_DONE;
                ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);
            }
            else if (bleptum_info[i].bleptu_state == BLEPTUM_WRITE_PRU_CONTROL_DONE)
            {
                // enable alert notifications from PRU
                bleprofile_sendWriteCmd(bleptum_info[i].bleptu_pru_alert_ccc_hdl,
                                       (UINT8 *)&(bleptum_app_state->bleptu_pru_alert_ccc),
                                       BLEPTUM_CCC_PARAM_LEN);

                bleptum_info[i].bleptu_state = BLEPTUM_WRITE_CCC_DONE;
                ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);
            }
            else if (bleptum_info[i].bleptu_state == BLEPTUM_WRITE_CCC_DONE)
            {
                bleptum_info[i].bleptu_state = BLEPTUM_W4_READ_PRU_DYNAMIC;
                ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);

                // start reading the dynamic parameter
                bleprofile_sendReadReq(bleptum_info[i].bleptu_pru_dynamic_hdl);
                bleptum_start_transaction_timer(i);
            }
            else if (bleptum_info[i].bleptu_state == BLEPTUM_READ_PRU_DYNAMIC_DONE)
            {
                // 250ms (12.5 * 20) interval
                if (count % 20 == 0)
                {
                    bleptum_info[i].bleptu_state = BLEPTUM_W4_READ_PRU_DYNAMIC;
                    ble_trace2("bleptu_state[%d] = %02x", i, bleptum_info[i].bleptu_state);

                    // continue to read the dynamic parameter on every 250ms
                    bleprofile_sendReadReq(bleptum_info[i].bleptu_pru_dynamic_hdl);
                    bleptum_start_transaction_timer(i);
                }
            }
        }
    }
}

#ifdef BLEPTUM_PRU_STATIC_INTERPRET
// Function to decode and display the PRU static parameters
static void bleptum_decode_pru_static_param(UINT8                dev_instance,
                                            BLEPTUM_PRU_STATIC   *p_pru_static)
{
    BLEPTUM_PRU_INFO pru_info;

    pru_info.pru_information = p_pru_static->pru_information;

    ble_trace0("A4WP PRU Static Parameter ");
    ble_trace1("Device Instance %02d -",dev_instance);
    ble_trace1("Protocol Rev: 0x%02X", p_pru_static->protocol_rev);

    //pru_information
    if ((p_pru_static->pru_category <= BLEPTUM_TRACE_MAX_CATEGORY) &&
        (p_pru_static->pru_category != BLEPTUM_TRACE_NO_CATEGORY))
    {
        ble_trace1("CATEGORY: 0x%01X", p_pru_static->pru_category);
    }
    else
    {
        ble_trace0("CATEGORY: Undefined");
    }

    if (pru_info.bitmap.nfc_reciever)
    {
        ble_trace0("NFC: Yes");
    }
    else
    {
        ble_trace0("NFC: No");
    }

    if (pru_info.bitmap.sep_BTLE)
    {
        ble_trace0("SepBTLE : Yes");
    }
    else
    {
        ble_trace0("SepBTLE : Yes");
    }

    if (pru_info.bitmap.power_control)
    {
        ble_trace0("PCA - MaxEff");
    }
    else
    {
        ble_trace0("PCA - MinErr ");
    }

    if (pru_info.bitmap.power_back_off)
    {
        ble_trace0("Backoff : Yes");
    }
    else
    {
        ble_trace0("Backoff : No");
    }

    ble_trace1("HW: 0x%02X", p_pru_static->hw_rev);
    ble_trace1("SW: 0x%02X", p_pru_static->fw_rev);
    ble_trace1("P_RECT_MAX: %dmW", p_pru_static->maximum_power_desired * 100);
    ble_trace1("V_RECT_MIN_STATIC: %dmV", p_pru_static->v_rect_min_static);
    ble_trace1("V_RECT_HIGH_STATIC: %dmV", p_pru_static->v_rect_high_static);
    ble_trace1("V_RECT_SET: %dmV", p_pru_static->v_rect_set);

    // Delta R1 is optional field, used only if mask is enabled
    if (p_pru_static->optional_fields & 0x80)
    {
        ble_trace2("DeltaR1 %d.%02d ohms", p_pru_static->delta_r1 / 100,
                                           p_pru_static->delta_r1 % 100);
    }
}
#endif

#ifdef BLEPTUM_PRU_DYNAMIC_INTERPRET
// Function to decode and display the PRU static parameters
static void bleptum_decode_pru_dynamic_param(UINT8                dev_instance,
                                             BLEPTUM_PRU_DYNAMIC  *p_pru_dynamic)
{
    DYNAMIC_OPTIONAL_FIELDS dynamic_opt_param;
    PRU_ALERT               pru_alert_val;

    dynamic_opt_param.dyn_opt_fields  = p_pru_dynamic->optional_fields;
    pru_alert_val.pru_alert           = p_pru_dynamic->pru_alert;

    ble_trace3("\nPRUdev[%02d] Dyn\n  V_RECT: %d\n  I_RECT: %d ",
               dev_instance, p_pru_dynamic->v_rect, p_pru_dynamic->i_rect);
   
    if (dynamic_opt_param.bitmap.v_out)
    {
        ble_trace1("  V_OUT : %d", p_pru_dynamic->v_out);
    }

    if (dynamic_opt_param.bitmap.i_out)
    {
        ble_trace1("  I_OUT : %d", p_pru_dynamic->i_out);
    }

    if (dynamic_opt_param.bitmap.temperature)
    {
        ble_trace1("  TEMP  : %d", p_pru_dynamic->temperature);
    }

    if (dynamic_opt_param.bitmap.v_rect_min)
    {
        ble_trace1("  V_RECT_min : %d", p_pru_dynamic->v_rect_min_dyn);
    } 

    if (dynamic_opt_param.bitmap.v_rect_set)
    {
        ble_trace1("  V_RECT_set : %d", p_pru_dynamic->v_rect_set_dyn);
    }

    if (dynamic_opt_param.bitmap.v_rect_high)
    {
        ble_trace1("  V_RECT_high: %d", p_pru_dynamic->v_rect_high_dyn);
    }

    ble_trace1(" ALERT\n  OVP: %d", pru_alert_val.bitmap.ov);
    ble_trace1("  OCP: %d", pru_alert_val.bitmap.oc);
    ble_trace1("  OTP: %d", pru_alert_val.bitmap.ot);
    ble_trace1("   SP: %d", pru_alert_val.bitmap.self_protection);
    ble_trace1("   CC: %d", pru_alert_val.bitmap.cc);
    ble_trace1("  WCD: %d", pru_alert_val.bitmap.wcd);
    ble_trace1("  Charge Port: %d", pru_alert_val.bitmap.pru_charge_port);
}
#endif

#ifdef BLEPTUM_PRU_ALERT_INTERPRET
// Function to decode the alert characteristic value
static void bleptum_decode_pru_alert_char(UINT8     dev_instance,
                                          UINT16    data_len,
                                          UINT8     *data)
{
    PRU_ALERT               pru_alert_val;

    ble_trace1("Dev instance: %d", dev_instance);

    // Alert char value and the alert parameter value in dynamic parameter
    // are same except the use of bit 0 and 1 (refer BSS 1.2 for details)
    pru_alert_val.pru_alert = data[0];

    ble_trace1(" ALERT\n  OVP: %d", pru_alert_val.bitmap.ov);
    ble_trace1("  OCP: %d", pru_alert_val.bitmap.oc);
    ble_trace1("  OTP: %d", pru_alert_val.bitmap.ot);
    ble_trace1("   SP: %d", pru_alert_val.bitmap.self_protection);
    ble_trace1("   CC: %d", pru_alert_val.bitmap.cc);
    ble_trace1("  WCD: %d", pru_alert_val.bitmap.wcd);

    // since the PRU_ALERT structure is reused to decode here,
    // interpret bits 0 an 1 for mode transition
    if (data[0] & 0x03)
    {
        switch (data[0] & 0x03)
        {
            case 1:
            {
                ble_trace0("2s Mode Transition");
                break;
            }
            case 2:
            {
                ble_trace0("3s Mode Transition");
                break;
            }
            case 3:
            {
                ble_trace0("6s Mode Transition");
                break;
            }
        }

        // Optional BD address field may be preset here
        // alert value (1 octet) + BD address (6 octets) length
        if (data_len == 7)
        {
            ble_trace2("BD addr:0x%08X%04X",
                       (data[6] << 24) + (data[5] << 16) +
                       (data[4] << 8) + data[3],
                       (data[2] << 8) + data[1]);
        }
    }
}
#endif

// Function to process the ATT responses from slave devices
static void bleptum_AttRspCb(int len, int attr_len, UINT8 *data)
{
    UINT16  conhandle = emconinfo_getConnHandle();
    int     lindex    = bleptum_FindIndex(conhandle);
 
    ble_trace2("bleptum_AttRspCb: %d, %d", len, attr_len);

    if (lindex < 0)
    {
        //this should not happen
        return;
    }

    if (bleptum_info[lindex].bleptu_state == BLEPTUM_W4_READ_PRU_STATIC)
    {
        bleptum_stop_transaction_timer(lindex);

        bleptum_info[lindex].bleptu_state = BLEPTUM_READ_PRU_STATIC_DONE;
        ble_trace2("bleptu_state[%d] = %02x", lindex, bleptum_info[lindex].bleptu_state);

#ifdef BLEPTUM_PRU_STATIC_INTERPRET
        bleptum_decode_pru_static_param(lindex, (BLEPTUM_PRU_STATIC *)data);
#endif
    }
    else if (bleptum_info[lindex].bleptu_state == BLEPTUM_W4_1ST_READ_PRU_DYNAMIC)
    {
        bleptu_stop_transaction_timer(lindex);
        bleptum_info[lindex].bleptu_state = BLEPTUM_READ_1ST_PRU_DYNAMIC_DONE;

        ble_trace2("bleptu_state[%d] = %02x", lindex, bleptum_info[lindex].bleptu_state);

#ifdef BLEPTUM_PRU_DYNAMIC_INTERPRET
        bleptum_decode_pru_dynamic_param(lindex, (BLEPTUM_PRU_DYNAMIC *)data);
#endif
    }
    else if (bleptum_info[lindex].bleptu_state == BLEPTUM_W4_READ_PRU_DYNAMIC)
    {
        bleptum_stop_transaction_timer(lindex);

        bleptum_info[lindex].bleptu_state = BLEPTUM_READ_PRU_DYNAMIC_DONE;

        ble_trace2("bleptu_state[%d] = %02x", lindex, bleptum_info[lindex].bleptu_state);

#ifdef BLEPTUM_PRU_DYNAMIC_INTERPRET
        bleptum_decode_pru_dynamic_param(lindex, (BLEPTUM_PRU_DYNAMIC *)data);
#endif
    }
}

// PRU alert notification handler
static void bleptum_Notification(int len, int attr_hdl, UINT8 *data)
{
    UINT16 conhandle = emconinfo_getConnHandle();
    int    lindex    = bleptum_FindIndex(conhandle);

    ble_trace1("[%d]============================================================", lindex);
    ble_trace2("\rbleptu, Notification handle:%02x, %d", (UINT16)attr_hdl, len);

#ifdef BLEPTUM_PRU_ALERT_INTERPRET
    bleptum_decode_pru_alert_char(lindex, len, data);
#endif
}

// PRU alert indication handler
static void bleptum_Indication(int len, int attr_hdl, UINT8 *data)
{
    UINT16 conhandle = emconinfo_getConnHandle();
    int    lindex    = bleptum_FindIndex(conhandle);

    ble_trace1("[%d]============================================================", lindex);
    ble_trace2("\rbleptu, Indication:%02x, %d", (UINT16)attr_hdl, len);

#ifdef BLEPTUM_PRU_ALERT_INTERPRET
    bleptum_decode_pru_alert_char(lindex, len, data);
#endif

    // send confirmation for indication
    bleprofile_sendHandleValueConf();
}

// SMP result callback
static void bleptum_smpBondResult(LESMP_PARING_RESULT  result)
{
    blecen_smpBondResult(result);
}

// LE encryption changed callback
static void bleptum_encryptionChanged(HCI_EVT_HDR *evt)
{
    UINT8   status      = *((UINT8 *)(evt + 1));
    UINT16  conhandle   = emconinfo_getConnHandle();
    int     lindex      = bleptum_FindIndex(conhandle);

    ble_trace1("\rbleptu, encryption changed: %02x", status);

    blecen_encryptionChanged(evt);

    if (status == 0)
    {
        bleptum_info[lindex].bleptu_state = BLEPTUM_SMP_DONE;
        ble_trace2("bleptu_state[%d] = %02x", lindex, bleptum_info[lindex].bleptu_state);
    }
}

// start the transation timer on initiation of ATT transation
static void bleptum_start_transaction_timer(int index)
{
    bleptum_info[index].bleptu_timeout        = bleptum_app_state->bleptu_apptimer_count + BLEPTUM_TRANSACTION_TIMEOUT;
    bleptum_info[index].bleptu_timeout_enable = 1;
}

// stop the transation timer on response for ATT request
static void bleptum_stop_transaction_timer(int index)
{
    bleptum_info[index].bleptu_timeout_enable = 0;
}

// function to check for transation timer expiry and to initiate further action on link
static void bleptum_check_transaction_timer(UINT32 count)
{
    int i;

    for (i = 0; i < bleptum_app_state->bleptu_max_con; i ++)
    {
        if (bleptum_info[i].bleptu_conhandle)
        {
            if (bleptum_info[i].bleptu_timeout_enable)
            {
                if (bleptum_info[i].bleptu_timeout == count)
                {
                    ble_trace2("Trans TO[%d](%02x)---------------------------",
                               i, bleptum_info[i].bleptu_state);

                    bleptum_info[i].bleptu_state = BLEPTUM_FAIL;
                    bleptum_info[i].bleptu_timeout_enable = 0;

#ifdef BLEPTUM_TRANS_TO_DISCONNECTION
                    if (bleprofile_p_cfg->disc_required & BLEPTUM_TRANS_TO_DISCONNECTION)
                    {
                        blecm_disconnect(BT_ERROR_CODE_CONNECTION_TERMINATED_BY_LOCAL_HOST);
                    }
#endif
                }
            }
        }
    }
}

#ifdef BLEPTUM_PWM_TEST
// sample init sequence for PWM
static void bleptum_pwm_init(void)
{
    UINT16 init_value1 = 0x3FF - 500; //0x200; //0x0; 
    UINT16 toggle_val1 = 0x3FF - 100; //0x300; //0x200; 
    UINT16 init_value2 = 0x3C0; 
    UINT16 toggle_val2 = 0x3DF; // This will use base as 125kHz and use only 32 steps makes 2.1 Khz
    UINT16 init_value3 = 0x380; 
    UINT16 toggle_val3 = 0x3C0; // This will use base as 125kHz and use only 64 steps makes 1.05 Khz
    int    i;

    aclk_configure(24000000, ACLK1, ACLK_FREQ_24_MHZ); // base clock 23.46 kHz, use 0x200-0x300 makes 46.78 kHz

    pwm_start(BLEPTUM_PWM1_ID, PMU_CLK, toggle_val1, init_value1);
}

// test code to change the control, LED and PMW duty cycle values
static void bleptum_pwm_test(UINT32 count)
{
    int value; // this should be set 0-100

    //this will test GPIO output
    gpio_setPinOutput(BLEPTUM_GPIO_PIN_CTR1 >> 4, BLEPTUM_GPIO_PIN_CTR1 & 0x0F, count & 0x01);
    gpio_setPinOutput(BLEPTUM_GPIO_PIN_LED1 >> 4, BLEPTUM_GPIO_PIN_LED1 & 0x0F, (count & 0x04) >> 2);

    value = count % 200;
    if (value % 2 == 0)
    {
        // even number will use duty cycle 0%
        value = 100;
    }
    else
    {
        // if value is 0 duty cycle is 20%, if value is 100, duty cycle is 0%
        // 100 steps are supported from 0-20% with 0.2% step
        value /= 2;
    }

    pwm_transitionToSubstituteValues(BLEPTUM_PWM1_ID, (0x3FF - 100) + value, 0x3FF - 500);
}

// stops the PMW signal
static void bleptum_pwm_off(void)
{
    // turn off PWM
    pwm_disableChannel(1 << BLEPTUM_PWM1_ID);
}
#endif

#ifdef BLEPTUM_ADC_TEST
// initialise and configure the ADC channels
static void bleptum_adc_init(void)
{
    // init ADC
    adc_config();
    adc_SetInputRange(ADC_RANGE_0_3P6V);
}

// test code to read from ADC channel
static void bleptum_adc_test(UINT32 count)
{
    if (count % 80 == 0)
    {
        UINT32 adc1 = adc_readVoltage(BLEPTUM_ADC_CH1);
        UINT32 adc2 = adc_readVoltage(BLEPTUM_ADC_CH2);
        UINT32 adc3 = adc_readVoltage(BLEPTUM_ADC_CH3);

        ble_trace3("%d, %d, %d", adc1, adc2, adc3);
    }
}
#endif

// blecen timer callback to process the scan conditions
static void bleptum_CenTimerCb( UINT32 arg)
{
    switch (arg)
    {
        case BLEAPP_APP_TIMER_SCAN:
        {
            if (blecen_GetScan() == HIGH_SCAN)
            {
                // switch from High scan to Low scan
                blecen_Scan(LOW_SCAN);
            }
            else if (blecen_GetScan() == LOW_SCAN)
            {
                // restart the high scan after low scan timeout
                blecen_Scan(HIGH_SCAN);

                ble_trace0("Restart Scan");
            }
            break;
        }

        case BLEAPP_APP_TIMER_CONN:
        {
            if ((blecen_GetConn() == HIGH_CONN) ||
                (blecen_GetConn() == LOW_CONN))
            {
                // connection request timeout, cancel connection and restart scan
                blecen_Conn(NO_CONN, NULL, 0);
                blecen_Scan(HIGH_SCAN);
                ble_trace0("Conn Fail, Restart Scan");
            }
            break;
        }
    }
}

