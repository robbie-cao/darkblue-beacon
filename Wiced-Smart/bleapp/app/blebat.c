/*
* Copyright 2013, Broadcom Corporation
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Broadcom Corporation.
*/

/** @file
*
* This file implements Battery profile, service, application 
*
* Refer to Bluetooth SIG Battery Service 1.0 specification for details.
*
* This is ROM code plug in.  If application requires battery support it needs
* to add Battery service and corresponding characteristics to the GATT database.  
* In addition to that application need to call blebat_Init, blebat_connUp, 
* blebat_connDown, blebat_smpBondResult when corresponding event callbacks are 
* issued by the stack.  Typicallt on timer event application would call 
* blebat_pollMonitor to verify the battery level and send notification if 
* appropriate.
*
*/

#include "blebat.h"
#include "blecm.h"
#include "dbfw_app.h"
#include "legattdb.h"

#include "bleappconfig.h"

#define FID   FID_BLEAPP_APP__BLEBAT_C

//////////////////////////////////////////////////////////////////////////////
//                      local interface declaration
//////////////////////////////////////////////////////////////////////////////

void   blebat_DBInit(void);
void   blebat_set_db(int index, UINT32 value, UINT8 len);
void   blebat_set_battery_level(UINT8 value);
void   blebat_set_battery_power_state(UINT8 value);
void   blebat_set_battery_level_state(UINT16 value);
void   blebat_set_service_required(UINT8 value);
void   blebat_set_removable(UINT8 value);
UINT32 blebat_get_db(int index, UINT8 *p_len);
UINT8  blebat_get_battery_level(void);
UINT8  blebat_get_battery_power_state(void);
UINT16 blebat_get_battery_level_state(void);
UINT8  blebat_get_service_required(void);
UINT8  blebat_get_removable(void);
void   blebat_send_notification(int index);
void   blebat_send_broadcast(int index);

void   blebat_Config(void);
void   blebat_startMonitoring(void);
void   blebat_stopMonitoring(void);
void   blebat_measureAverage(void);
void   blebat_calculateCurrentReportValue(void);
void   blebat_checkCriticalLow(void);
void   blebat_lowBatteryShutdown(void);

void   blebat_connUp(void);
void   blebat_connDown(void);
void   blebat_smpBondResult(LESMP_PARING_RESULT  result);
int    blebat_writeCb(LEGATTDB_ENTRY_HDR *p);

//////////////////////////////////////////////////////////////////////////////
//                      global variables
//////////////////////////////////////////////////////////////////////////////

#define BATTERY_CHAR_INDEX_LEVEL            0
#define BATTERY_CHAR_INDEX_POWER_STATE      1
#define BATTERY_CHAR_INDEX_SERVICE_REQUIRED 2
#define BATTERY_CHAR_INDEX_REMOVABLE        3
#define BATTERY_CHAR_INDEX_LEVEL_STATE      4

#define BLEPROX_BAT_HANDLE_NUM_MAX          5

#ifndef BCM20703
extern UINT8 bleapp_hidoff_enable;
#endif

extern BLEPROFILE_SINGLE_PARAM_CB blecm_generate_advCb;

//Battery Monitor related variables
BLEBAT_BATMON_CFG blebat_batmon_cfg =
{
    /*.adcInputConnectedToBattery      =*/ ADC_INPUT_VDDIO, // P15. ADC input to be used for measurement. Note that this may not be a GPIO.
    /*.measurementInterval             =*/ 60000,           // msec. Interval between battery measurements
    /*.numberOfMeasurementsToAverage   =*/ 8,               // Number of measurements averaged for a report, max 16
    /*.fullVoltage                     =*/ 3200,            // millivolts. The nominal full battery voltage
    /*.emptyVoltage                    =*/ 1800,            // millivolts. The voltage at which the batteries are considered drained
    /*.shutdownVoltage                 =*/ 1700,            // millivolts. System should shutdown if it detects battery 
                                                            // voltage at or below this value.  0 disables shutdown. 
    /*.maxLevel                        =*/ 100,             // Sets the range of the reported number of steps
                                                            // Set it to 100 to report battery in %, 0-100.
    /*.reportID                        =*/ 3,               // ID of the battery report
    /*.reportLength                    =*/ 8,               // Length of the battery report
    /*.reportOnConnect                 =*/ TRUE,            // if TRUE report should be sent when connection is established
};

typedef struct
{
    BLEBAT_HOSTINFO     blebat_hostinfo;
    BD_ADDR             blebat_remote_addr;
    BLE_BAT_GATT_CFG    blebat_gatt_cfg[BLEPROX_BAT_HANDLE_NUM_MAX];
    INT16               blebat_measurements[MAX_MEAS_TO_AVERAGE];   // Array that holds the last measurements taken
    INT16               blebat_measurementSum;                      // Running total of all the measurements. Note that a word is still adequate since
    INT16               blebat_measurementAverage;                  // Current average in MilleVolts
    UINT8               blebat_bat_level;
    BYTE                blebat_oldestMeasurementIndex;              // Index into the measurement array indicating the oldest measurement
} tBatAppState;

tBatAppState            *batAppState = NULL;

/// What the current measurement corresponds to in terms of reported range
BYTE        blebat_curBatteryLevelInReportUnits;
UINT8       blebat_Enabled;
UINT8       blebat_count;

void blebat_Init(void)
{
    if (!batAppState)
    {
        batAppState = (tBatAppState *)cfa_mm_Sbrk(sizeof(tBatAppState));
        memset(batAppState, 0x00, sizeof(tBatAppState));
    }

    // write DBCB
    legattdb_regWriteHandleCb((LEGATTDB_WRITE_CB)blebat_writeCb);

    blebat_DBInit();

    blebat_Config();

    blebat_startMonitoring();

    blebat_pollMonitor();
}

// when connection comes up we need to setup GATT DB in case reads descriptors
// if it is not a paired device, values of all descriptors should be 0
// For paired device we read bitmask of configured notifications and broadcasts
// from the NVRAM and create appropriate value in the GATT DB.
void blebat_connUp(void)
{
    BLEPROFILE_DB_PDU   db_pdu;
    int                 i;

    memcpy(batAppState->blebat_remote_addr, (UINT8 *)emconninfo_getPeerPubAddr(), sizeof(batAppState->blebat_remote_addr));

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebat_connUp[5:2]: %02x %02x %02x %02x", TVF_BBBB(batAppState->blebat_remote_addr[5], batAppState->blebat_remote_addr[4], batAppState->blebat_remote_addr[3], batAppState->blebat_remote_addr[2] ));
    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebat_connUp[1:0]: %02x %02x", TVF_BB(batAppState->blebat_remote_addr[1], batAppState->blebat_remote_addr[0]));

    bleprofile_ReadNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);

    for (i = 0; i < BLEPROX_BAT_HANDLE_NUM_MAX; i++)
    {
        //using default value first
        db_pdu.len    = 2;
        db_pdu.pdu[0] = 0x00;
        db_pdu.pdu[1] = 0x00;

        if ((batAppState->blebat_gatt_cfg[i].cl_hdl != 0) && 
            (memcmp(batAppState->blebat_remote_addr, batAppState->blebat_hostinfo.bdAddr, 6) == 0))
        {
            // paired device and Client Configuration descriptor exists in the database
            // set value of the descriptor to Notify if mask bit is set.
            db_pdu.pdu[0] = (batAppState->blebat_hostinfo.notify_mask  & (1 << i)) ? CCC_NOTIFICATION : 0;
        }
        bleprofile_WriteHandle(batAppState->blebat_gatt_cfg[i].cl_hdl, &db_pdu);
        ble_tracen((char *)db_pdu.pdu, db_pdu.len);

        db_pdu.pdu[0] = 0x00;

        if ((batAppState->blebat_gatt_cfg[i].ds_hdl != 0) && 
            (memcmp(batAppState->blebat_remote_addr, batAppState->blebat_hostinfo.bdAddr, 6) == 0))
        {
            // paired device and Server Configuration descriptor exists in the database
            // set value of the descriptor to Broadcast if mask bit is set.
            db_pdu.pdu[0] = (batAppState->blebat_hostinfo.broadcast_mask & (1 << i)) ? SCC_BROADCAST : 0;
        }
        bleprofile_WriteHandle(batAppState->blebat_gatt_cfg[i].ds_hdl, &db_pdu);
        ble_tracen((char *)db_pdu.pdu, db_pdu.len);
    }
}

void blebat_connDown(void)
{
    BLEPROFILE_DB_PDU   db_pdu;
    UINT8               writtenbyte;
    int                 i;

    bleprofile_ReadNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);

    // Save NVRAM to client characteristic descriptor
    if (memcmp(batAppState->blebat_remote_addr, batAppState->blebat_hostinfo.bdAddr, 6) == 0)
    {
        for (i = 0; i < BLEPROX_BAT_HANDLE_NUM_MAX; i++)
        {
            if (batAppState->blebat_gatt_cfg[i].cl_hdl)
            {
                bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[i].cl_hdl, &db_pdu);
                ble_tracen((char *)db_pdu.pdu, db_pdu.len);

                if (db_pdu.len == 2 && (db_pdu.pdu[0] & CCC_NOTIFICATION))
                {
                    batAppState->blebat_hostinfo.notify_mask |= (1 << i);
                }
                else
                {
                    batAppState->blebat_hostinfo.notify_mask &= ~(1 << i);
                }
            }

            if (batAppState->blebat_gatt_cfg[i].ds_hdl)
            {
                bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[i].ds_hdl, &db_pdu);
                ble_tracen((char *)db_pdu.pdu, db_pdu.len);

                if (db_pdu.len == 2 && (db_pdu.pdu[0] & SCC_BROADCAST))
                {
                    batAppState->blebat_hostinfo.broadcast_mask |= (1 << i);
                }
                else
                {
                    batAppState->blebat_hostinfo.broadcast_mask &= ~(1 << i);
                }
            }
        }

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

void blebat_smpBondResult(LESMP_PARING_RESULT  result)
{
    BLEPROFILE_DB_PDU   db_pdu;
    UINT8               writtenbyte;
    int                 i;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "blebat, bond result %02x", TVF_D(result));

    if (result == LESMP_PAIRING_RESULT_BONDED)
    {
        memcpy(batAppState->blebat_hostinfo.bdAddr, emconninfo_getPeerPubAddr(), sizeof(BD_ADDR));

        db_pdu.len    = 2;
        db_pdu.pdu[0] = 0x00;
        db_pdu.pdu[1] = 0x00;

        for (i = 0; i < BLEPROX_BAT_HANDLE_NUM_MAX; i++)
        {
            if (batAppState->blebat_gatt_cfg[i].cl_hdl)
            {
                bleprofile_WriteHandle(batAppState->blebat_gatt_cfg[i].cl_hdl, &db_pdu);
            }
            if (batAppState->blebat_gatt_cfg[i].ds_hdl)
            {
                bleprofile_WriteHandle(batAppState->blebat_gatt_cfg[i].ds_hdl, &db_pdu);
            }
        }
        batAppState->blebat_hostinfo.notify_mask     = 0;
        batAppState->blebat_hostinfo.broadcast_mask  = 0;

        writtenbyte = bleprofile_WriteNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);
        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
    }
}

int blebat_writeCb(LEGATTDB_ENTRY_HDR *p)
{
    UINT16              handle = legattdb_getHandle(p);
    BLEPROFILE_DB_PDU   db_pdu;
    UINT8               writtenbyte;
    int                 i;

    TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: handle %04x", TVF_D(handle));

    for (i = 0; i < BLEPROX_BAT_HANDLE_NUM_MAX; i++)
    {
        if (batAppState->blebat_gatt_cfg[i].cl_hdl && handle == batAppState->blebat_gatt_cfg[i].cl_hdl)
        {
            bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[i].cl_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bleprofile_ReadNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);

            if (memcmp(batAppState->blebat_remote_addr, batAppState->blebat_hostinfo.bdAddr, 6) == 0)
            {
            	UINT8 old_mask = batAppState->blebat_hostinfo.notify_mask;

                if (db_pdu.len == 2 && (db_pdu.pdu[0] & CCC_NOTIFICATION))
                {
                    batAppState->blebat_hostinfo.notify_mask |= (1 << i);
                }
                else
                {
                    batAppState->blebat_hostinfo.notify_mask &= ~(1 << i);
                }

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: notify old:%02x new %02x index %d", TVF_BBW(old_mask, batAppState->blebat_hostinfo.notify_mask, i));

                writtenbyte = bleprofile_WriteNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));
            }
            break;
        }

        else if (batAppState->blebat_gatt_cfg[i].ds_hdl && handle == batAppState->blebat_gatt_cfg[i].ds_hdl)
        {
            bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[i].ds_hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bleprofile_ReadNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);

            if (memcmp(batAppState->blebat_remote_addr, batAppState->blebat_hostinfo.bdAddr, 6) == 0)
            {
            	UINT8 old_mask = batAppState->blebat_hostinfo.broadcast_mask;

                if (db_pdu.len == 2 && (db_pdu.pdu[0] & SCC_BROADCAST))
                {
                    batAppState->blebat_hostinfo.broadcast_mask |= (1 << i);
                }
                else
                {
                    batAppState->blebat_hostinfo.broadcast_mask &= ~(1 << i);
                }

                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "WriteCb: notify old:%02x new %02x index %d", TVF_BBW(old_mask, batAppState->blebat_hostinfo.broadcast_mask, i));

                writtenbyte = bleprofile_WriteNVRAM(VS_BLE_BAT_DATA1_1, sizeof (BLEBAT_HOSTINFO), (UINT8 *)&batAppState->blebat_hostinfo);
                TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "NVRAM write:%04x", TVF_D(writtenbyte));

                //change broadcast setting with current value
                blebat_send_broadcast(BATTERY_CHAR_INDEX_LEVEL_STATE);
            }
            break;
        }
    }

    return 0;
}

void blebat_DBInit(void)
{
    //load handle number
    LEGATTDB_ENTRY_HDR  *dbPtr;
    UINT16              handle;
    int                 i, done = 0;
    UINT8               *pDataEndPtr;
    UINT16              attrUuid;

    for (i = 0; i < BLEPROX_BAT_HANDLE_NUM_MAX; i++)
    {
        batAppState->blebat_gatt_cfg[i].hdl    = 0;
        batAppState->blebat_gatt_cfg[i].cl_hdl = 0;
        batAppState->blebat_gatt_cfg[i].ds_hdl = 0;
    }

    pDataEndPtr = (UINT8 *)bleapp_init_cfg.p_db + bleapp_init_cfg.db_size;

    // First search DB for the Battery Service
    for (dbPtr = (LEGATTDB_ENTRY_HDR  *)bleapp_init_cfg.p_db;
         dbPtr < (LEGATTDB_ENTRY_HDR  *)pDataEndPtr;
         dbPtr = legattdb_nextEntry(dbPtr))
    {
        // get the handle
        handle = legattdb_getHandle(dbPtr);

        // Get the attribute UUID.
        attrUuid = legattdb_getAttrUUID16(dbPtr);

        // check attribute group type.
        if ((attrUuid == UUID_ATTRIBUTE_PRIMARY_SERVICE) || (attrUuid == UUID_ATTRIBUTE_SECONDARY_SERVICE))
        {
            // this is primary or secondary service, can be the battery

            if (legattdb_getAttrValueUUID16(dbPtr) == UUID_SERVICE_BATTERY)
            {
                dbPtr = legattdb_nextEntry(dbPtr);
                break;
            }
        }
    }

    // if we found Battery service, search for possible characteristics
    for (; (dbPtr < (LEGATTDB_ENTRY_HDR  *)pDataEndPtr) && !done; dbPtr = legattdb_nextEntry(dbPtr))
    {
        // get the handle
        handle = legattdb_getHandle(dbPtr);
        attrUuid = legattdb_getAttrUUID16(dbPtr);

        switch (attrUuid)
        {
        case UUID_CHARACTERISTIC_BATTERY_LEVEL:
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_LEVEL].hdl          = handle;
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_LEVEL].cl_hdl       = legattdb_findCharacteristicDescriptor(handle, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
            break;

        case UUID_CHARACTERISTIC_BATTERY_POWER_STATE:
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_POWER_STATE].hdl    = handle;
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_POWER_STATE].cl_hdl = legattdb_findCharacteristicDescriptor(handle, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
            break;

        case UUID_CHARACTERISTIC_SERVICE_REQUIRED:
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_SERVICE_REQUIRED].hdl    = handle;
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_SERVICE_REQUIRED].cl_hdl = legattdb_findCharacteristicDescriptor(handle, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
            break;

        case UUID_CHARACTERISTIC_REMOVABLE:
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_REMOVABLE].hdl      = handle;
            break;

        case UUID_CHARACTERISTIC_BATTERY_LEVEL_STATE:
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_LEVEL_STATE].hdl    = handle;
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_LEVEL_STATE].cl_hdl = legattdb_findCharacteristicDescriptor(handle, UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION);
            batAppState->blebat_gatt_cfg[BATTERY_CHAR_INDEX_LEVEL_STATE].ds_hdl = legattdb_findCharacteristicDescriptor(handle, UUID_DESCRIPTOR_SERVER_CHARACTERISTIC_CONFIGURATION);
            break;

        case UUID_ATTRIBUTE_PRIMARY_SERVICE:
        case UUID_ATTRIBUTE_SECONDARY_SERVICE:
        case UUID_ATTRIBUTE_INCLUDE:
            // next service is starting no more battery related characteristics
            done = TRUE;
            break;
        }
    }
}

void blebat_set_db(int index, UINT32 value, UINT8 len)
{
    BLEPROFILE_DB_PDU db_pdu;

    db_pdu.len = len;
    memcpy(db_pdu.pdu, &value, len);
    bleprofile_WriteHandle(batAppState->blebat_gatt_cfg[index].hdl, &db_pdu);
    ble_tracen((char *)db_pdu.pdu, db_pdu.len);
}

void blebat_set_battery_level(UINT8 value)
{
    blebat_set_db(BATTERY_CHAR_INDEX_LEVEL, (UINT32) value, 1);
}

void blebat_set_battery_power_state(UINT8 value)
{
    blebat_set_db(BATTERY_CHAR_INDEX_POWER_STATE, (UINT32) value, 1);
}

void blebat_set_battery_level_state(UINT16 value)
{
    blebat_set_db(BATTERY_CHAR_INDEX_LEVEL_STATE, (UINT32) value, 2);
}

void blebat_set_service_required(UINT8 value)
{
    blebat_set_db(BATTERY_CHAR_INDEX_SERVICE_REQUIRED, (UINT32) value, 1);
}

void blebat_set_removable(UINT8 value)
{
    blebat_set_db(BATTERY_CHAR_INDEX_REMOVABLE, (UINT32) value, 1);
}

UINT32 blebat_get_db(int index, UINT8 *p_len)
{
    BLEPROFILE_DB_PDU db_pdu;
    UINT32            value;

    if (batAppState->blebat_gatt_cfg[index].hdl)
    {
        bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[index].hdl, &db_pdu);
        ble_tracen((char *)db_pdu.pdu, db_pdu.len);

        *p_len = db_pdu.len;
        memcpy(&value, db_pdu.pdu, db_pdu.len);
        return value;
    }

    *p_len = 0;
    return 0;
}

UINT8 blebat_get_battery_level(void)
{
    UINT8 len;
    return (UINT8)blebat_get_db(BATTERY_CHAR_INDEX_LEVEL, &len);
}

UINT8 blebat_get_battery_power_state(void)
{
    UINT8 len;
    return (UINT8)blebat_get_db(BATTERY_CHAR_INDEX_POWER_STATE, &len);
}

UINT16 blebat_get_battery_level_state(void)
{
    UINT8 len;
    return (UINT16)blebat_get_db(BATTERY_CHAR_INDEX_LEVEL_STATE, &len);
}

UINT8 blebat_get_service_required(void)
{
    UINT8 len;
    return (UINT8)blebat_get_db(BATTERY_CHAR_INDEX_SERVICE_REQUIRED, &len);
}

UINT8 blebat_get_removable(void)
{
    UINT8 len;
    return (UINT8)blebat_get_db(BATTERY_CHAR_INDEX_REMOVABLE, &len);
}

void blebat_send_notification(int index)
{
    BLEPROFILE_DB_PDU db_pdu, db_cl_pdu;

    if (batAppState->blebat_gatt_cfg[index].hdl && batAppState->blebat_gatt_cfg[index].cl_hdl)
    {
        //check client char cfg
        bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[index].cl_hdl, &db_cl_pdu);
        ble_tracen((char *)db_cl_pdu.pdu, db_cl_pdu.len);

        if (db_cl_pdu.len == 2 && (db_cl_pdu.pdu[0] & CCC_NOTIFICATION))
        {
            bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[index].hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            bleprofile_sendNotification( batAppState->blebat_gatt_cfg[index].hdl,
                        (UINT8 *)db_pdu.pdu, db_pdu.len);
        }
    }
}

void blebat_send_broadcast(int index)
{
    BLEPROFILE_DB_PDU db_pdu, db_ds_pdu;
    BLE_ADV_FIELD     a[5];
    UINT16            serv = UUID_SERVICE_BATTERY;
    int               num_fields;

    if (batAppState->blebat_gatt_cfg[index].hdl && batAppState->blebat_gatt_cfg[index].ds_hdl)
    {
        //check client char cfg
        bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[index].ds_hdl, &db_ds_pdu);
        ble_tracen((char *)db_ds_pdu.pdu, db_ds_pdu.len);

        if (db_ds_pdu.len == 2 && (db_ds_pdu.pdu[0] & SCC_BROADCAST))
        {
            bleprofile_ReadHandle(batAppState->blebat_gatt_cfg[index].hdl, &db_pdu);
            ble_tracen((char *)db_pdu.pdu, db_pdu.len);

            if (blecm_generate_advCb)
            {
                int result = blecm_generate_advCb(2);

                if (result >= 0)
                {
                    return;
                }
            }

            // flags
            a[0].len     = FLAGS_LEN + 1;
            a[0].val     = ADV_FLAGS;
            a[0].data[0] = LE_GENERAL_DISCOVERABLE | BR_EDR_NOT_SUPPORTED;
            num_fields = 1;

            // Appearance
            if ((bleprofile_p_cfg->cod[0] || bleprofile_p_cfg->cod[1]))
            {
                a[num_fields].len = APPEARANCE_LEN + 1;
                a[num_fields].val = ADV_APPEARANCE;
                memcpy(a[num_fields].data, bleprofile_p_cfg->cod, APPEARANCE_LEN); //appearance is in cod[0,1]
                num_fields++;
            }
            // service UUID
            if (bleprofile_p_cfg->serv[0] != 0)
            {
                a[num_fields].len = UUID_LEN + 1;
                a[num_fields].val = ADV_SERVICE_UUID16_COMP;
                memcpy(a[num_fields].data, (void *)&(bleprofile_p_cfg->serv[0]), UUID_LEN);
                num_fields++;
            }
            // Service Data
            a[num_fields].len = 2 + db_pdu.len + 1; // UUID(2), power_level(5)
            a[num_fields].val = ADV_SERVICE_DATA;
            memcpy(&(a[num_fields].data[0]), &serv, 2);
            memcpy(&(a[num_fields].data[2]), db_pdu.pdu, db_pdu.len);
            num_fields++;

            // Local name
            a[num_fields].len = strlen(bleprofile_p_cfg->local_name) + 1;
            if (a[num_fields].len > LOCAL_NAME_LEN_MAX + 1)
            {
                a[num_fields].len = LOCAL_NAME_LEN_MAX + 1;
                a[num_fields].val = ADV_LOCAL_NAME_SHORT;
            }
            else
            {
                a[num_fields].val = ADV_LOCAL_NAME_COMP;
            }
            memcpy(a[num_fields].data, bleprofile_p_cfg->local_name, a[num_fields].len - 1);
            num_fields++;

            bleprofile_GenerateADVData(a, num_fields);
        }
        else
        {
            extern BLE_ADV_FIELD *bleprofile_p_adv;
            extern UINT8 bleprofile_adv_num;

            if (blecm_generate_advCb)
            {
                int result = blecm_generate_advCb(3);

                if (result >= 0)
                {
                    return;
                }
            }
            bleprofile_GenerateADVData(bleprofile_p_adv, bleprofile_adv_num);
        }
    }
}


/////////////////////////////////////////////////////////////////////
/// Config Battery Monitor
///
/////////////////////////////////////////////////////////////////////
void blebat_Config(void)
{
    //init ADC
    adc_config();

    // Limit maximum number of measurements to the room we have.
    // Don't want to crash and burn....
    if (blebat_batmon_cfg.numberOfMeasurementsToAverage > MAX_MEAS_TO_AVERAGE)
    {
        blebat_batmon_cfg.numberOfMeasurementsToAverage = MAX_MEAS_TO_AVERAGE;
    }

    // Clear battery monitor data
    blebat_curBatteryLevelInReportUnits         = 0;
    batAppState->blebat_measurementSum          = 0;
    batAppState->blebat_oldestMeasurementIndex  = 0;
    batAppState->blebat_measurementAverage      = 0;
    memset(batAppState->blebat_measurements, 0, sizeof(batAppState->blebat_measurements));

}


/////////////////////////////////////////////////////////////////////////
/// Start the battery monitor
/////////////////////////////////////////////////////////////////////////
void blebat_startMonitoring(void)
{
    UINT8 i;

    // Fill our measurement array by taking the maximum number of requested measurements
    for (i = 0; i < blebat_batmon_cfg.numberOfMeasurementsToAverage; i++)
    {
        blebat_measureAverage();
    }

    // Calculate the current report value
    blebat_calculateCurrentReportValue();

    bleprofile_BatOff();

    // Put the batt level in the report. This gives us a starting value for the report
    batAppState->blebat_bat_level = blebat_curBatteryLevelInReportUnits;

    blebat_Enabled  = TRUE;
    blebat_count    = 0;
}

/////////////////////////////////////////////////////////////////////////
/// Stop the battery monitor
/////////////////////////////////////////////////////////////////////////
void blebat_stopMonitoring(void)
{
    blebat_Enabled  = FALSE;
}

/////////////////////////////////////////////////////////////////////////
/// Poll the battery monitor to measure the battery level, and if it's lower
/// than the previous value, transmit the report
/// \param transport pointer to transport on which to send the response, if any
/////////////////////////////////////////////////////////////////////////
void blebat_pollMonitor(void)
{
    UINT8               level;
    UINT16              level_state;

    blebat_count++;

    if (blebat_Enabled && (blebat_count % (blebat_batmon_cfg.measurementInterval / 1000)) == 1)
    {
        bleprofile_BatOn();

        // Measure the current battery voltage
        blebat_measureAverage();

        bleprofile_BatOff();

        // Convert current reading to report format
        blebat_calculateCurrentReportValue();

        // If the new reading is less than the shutdown threshold, shutdown
        blebat_checkCriticalLow();

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "Battery level: %d/100", TVF_D(blebat_curBatteryLevelInReportUnits));

        // send notification
        level = blebat_curBatteryLevelInReportUnits;
        blebat_set_battery_level(level);

        blebat_send_notification(BATTERY_CHAR_INDEX_LEVEL);

        level_state = blebat_get_battery_level_state();
        level_state = (level_state & 0xFF00) | blebat_curBatteryLevelInReportUnits;
        blebat_set_battery_level_state(level_state);

        blebat_send_notification(BATTERY_CHAR_INDEX_LEVEL_STATE);
        blebat_send_broadcast(BATTERY_CHAR_INDEX_LEVEL_STATE);


        // If reporting is enabled, we are connected and the current reading is not
        // the last reported value, send a report
        if (blebat_curBatteryLevelInReportUnits != batAppState->blebat_bat_level)
        {
            // Update the report with the latest reading
            batAppState->blebat_bat_level = blebat_curBatteryLevelInReportUnits;

// TODO: Make this unconditional
#if defined(BLE_KB_APP) || defined (BLE_MOUSE_APP)
            // Send the report out
            blehidtran_sendRpt((UINT8*)blebat_curBatteryLevelInReportUnits, 1, BLEHIDTRAN_RT_INPUT, BLEHIDTRAN_P_INTERRUPT);
#endif
       }
    }
}


//////////////////////////////////////////////////////////////////////////
/// Take a measurement, save it in the measurement array and
/// keep running total and average.
/////////////////////////////////////////////////////////////////////////
void blebat_measureAverage(void)
{
   INT16 newMeasurement;

    // Take a measurement
    adc_SetInputRange((ADC_INPUT_RANGE_SEL)adcConfig.inputRangeSelect);

    newMeasurement  = adc_readVoltage((ADC_INPUT_CHANNEL_SEL)blebat_batmon_cfg.adcInputConnectedToBattery);

    // Subtract old measurement from the running total
    batAppState->blebat_measurementSum -= batAppState->blebat_measurements[batAppState->blebat_oldestMeasurementIndex];

    // Put new measurement in the measurement array and add it to the running sum
    batAppState->blebat_measurements[batAppState->blebat_oldestMeasurementIndex] = newMeasurement;
    batAppState->blebat_measurementSum += newMeasurement;

    // Adjust the oldest index pointer to point to the next entry in the measurement array
    // Handle wraparound
    if (++(batAppState->blebat_oldestMeasurementIndex) >= blebat_batmon_cfg.numberOfMeasurementsToAverage)
    {
        batAppState->blebat_oldestMeasurementIndex = 0;
    }

    // Calculate the new average, round the result
    batAppState->blebat_measurementAverage = batAppState->blebat_measurementSum / blebat_batmon_cfg.numberOfMeasurementsToAverage;
    if (batAppState->blebat_measurementSum % blebat_batmon_cfg.numberOfMeasurementsToAverage >= blebat_batmon_cfg.numberOfMeasurementsToAverage / 2)
    {
        (batAppState->blebat_measurementAverage)++;
    }
}

//////////////////////////////////////////////////////////////////////////
/// Convert the current measurement into reportable format
//////////////////////////////////////////////////////////////////////////
void blebat_calculateCurrentReportValue(void)
{
    UINT32 tmp;

    // Take the current average and convert it into report units. Check ranges
    if (batAppState->blebat_measurementAverage >= blebat_batmon_cfg.fullVoltage)
    {
        blebat_curBatteryLevelInReportUnits = blebat_batmon_cfg.maxLevel;
    }
    else if (batAppState->blebat_measurementAverage <= blebat_batmon_cfg.emptyVoltage)
    {
        blebat_curBatteryLevelInReportUnits = 0;
    }
    else
    {
        // The actual value is in between the full-empty range. Calculate using DWORD math and then assign
        // to new value
        tmp = (batAppState->blebat_measurementAverage - blebat_batmon_cfg.emptyVoltage);
        tmp *= blebat_batmon_cfg.maxLevel;
        tmp /= (blebat_batmon_cfg.fullVoltage - blebat_batmon_cfg.emptyVoltage);
        blebat_curBatteryLevelInReportUnits = (BYTE)tmp;
    }
}

/////////////////////////////////////////////////////////////////////////////////
/// Checks the battery for critical low state. If battery is critically low or below,
/// it calls battLowBatteryShutdown. This function should be called after
/// application initialization is complete to ensure that all the drivers
/// are initialized and the system can correctly enter low battery state.
/////////////////////////////////////////////////////////////////////////////////
void blebat_checkCriticalLow(void)
{
    // If battery is below critical low, shutdown
    if (batAppState->blebat_measurementAverage <= blebat_batmon_cfg.shutdownVoltage)
    {
        UINT8 power_state;
        UINT16 level_state;

        TRACE(TRACE_INFO, MODULE_ID_BLEAPP, "BatteryLevelCritical,ShuttingDown", TVF_D(0));

        power_state = blebat_get_battery_power_state();
        power_state &= BLEBAT_POWERSTATE_LEVEL_UNKNOWN;
        power_state |= BLEBAT_POWERSTATE_LEVEL_CRITICALLYLOWLEVEL;
        blebat_set_battery_power_state(power_state);

        blebat_send_notification(BATTERY_CHAR_INDEX_POWER_STATE);

        level_state = blebat_get_battery_level_state();
        level_state = (level_state&0xFF)|(power_state << 8);
        blebat_set_battery_power_state(level_state);

        blebat_send_notification(BATTERY_CHAR_INDEX_LEVEL_STATE);
        blebat_send_broadcast(BATTERY_CHAR_INDEX_LEVEL_STATE);

        // This function should never return
        blebat_lowBatteryShutdown();
    }
}


/////////////////////////////////////////////////////////////////////////////////
/// Called when the battery voltage drops below the configured threshold
/////////////////////////////////////////////////////////////////////////////////
void blebat_lowBatteryShutdown(void)
{
#ifdef BLE_BAT_HIDOFF_ENABLE
#ifdef BCM20703
    if (bleAppConfig.bleapp_hidoff_enable & BAT_HIDOFF_ENABLE)
#else
    if (bleapp_hidoff_enable & BAT_HIDOFF_ENABLE)
#endif
    {
        bleprofile_PrepareHidOff();
    }
    else
#endif
    {
        bleprofile_PreparePowersave();
        bleprofile_StartPowersave();
    }
}

