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
* Define functions to access Low Dropout Regulator (LDO) peripheral
*/

/**  \addtogroup ldo
 *  \ingroup HardwareDrivers
 */

/*! @{ */
/**
* Defines the LDO driver.
*
*/

#ifndef __LDO__H__
#define __LDO__H__

#include "types.h"
#include "hidddriversconfig.h"


/*
* Low Dropout Regulator (LDO)
*
*   steps to program the ldo_ctl
*
*   (1) write ldo_ctl_en_reg to 0  -  disable ldo_ctl_en first
*   (2) program ldo_ctl[31:0]     -   write ldo value
*   (3) write ldo_ctl_en_reg to 1 - enable ldo_ctl_en
*
*/

//
// LDO control Enable Register
//
// - ldo_ctl_en_adr
//
// When ldo_ctl_en=1, LDO module will use ldo_ctl# regsiter values to control LDO
// LDO analog IP internal default values are all zeros, if any of the ldo_ctl# is not 0,
// ldo_ctl_en need to set to 1
//
typedef union LDO_CTL_EN_REG
{
    UINT32                  ldo_ctl_en_reg;

    struct 
    {
        //
        // bit 0 -   LDO Control enable
        //
        UINT32              ldo_ctl_en_bit          : 1;

        //
        // bit 1 -   enable powerdown of the bandgap in HIDDOFF mode
        //           set this bit, bandgap will be power down when HIDOFF mode
        UINT32              ldo_bandgap_pd_en       : 1;


        //
        // bit 4:2 - LDO output level when 24MHz crystal is powered down
        // program with the same value as ldo_ctl1[3:1], if no votage step is needed
        //
        UINT32              ldo_sleep_output_level  : 3;

        //
        // reserved 31:5
        //
        UINT32              reserved7               : 27;               // bit 31:5

    }bitmap;
}LDO_CTL_EN_REG;

enum
{
    LDO_CTL_EN_ENABLE                           =   1,
    LDO_CTL_EN_DISABLE                          =   0,
    LDO_CTL_EN_BG_AUTO_HIDOFF_PD_ENABLE         =   1,
    LDO_CTL_EN_BG_AUTO_HIDOFF_PD_DISABLE        =   0,
};




//
// ldo_ctl[31:0] = {ldo_ctl1_reg, ldo_ctl0_reg}
//
//  This register settings will only valid if ldo_ctl_en_bit set to 1
//
//  * ldo_ctl0_reg *
//
// ldo_ctl[0]   - BG low power mode, default 0
// ldo_ctl[2:1] - LPF enable/adjust to fitler off BG noise , default disabled for faster startup
// ldo_ctl[3]   - BG power down, default power up
// ldo_ctl[7:4] - BG PTAT Current adjustment
// ldo_ctl[11:8]    - BG CTAT current adjustment
// ldo_ctl[14:12]   - BG Vref Adjustment
// ldo_ctl[15]      - reserved
//
//  * ldo_ctl1_reg *
//
// ldo_ctl[16]      - LDO Power down    , defautl power up
// ldo_ctl[19:17]   - LDO Output Voltage Adjustment
// ldo_ctl[20]      - Enable VDS matching amplifier for low dropout, default enabled
// ldo_ctl[21]      - Enable LDO current limite, default disable
// ldo_ctl[22]      - Select between constant/feedback current limit mode for LDO
// ldo_ctl[23]      - LDO current limit adjustment
// ldo_ctl[25:24]   - adjust LDO feedback pole to compensate ESR zero
// ldo_ctl[31:26]   - reserved
//

//
// LDO_CTL0
//
// define lower 16 bits for the ldo_ctl[15:0]
//
typedef union LDO_CTL0_REG
{

    UINT32          ldo_ctl0_reg;
    struct
    {
        // bit 0    -  dsiable BG lower power mode
        UINT32      bgLowerPowerModeDisable : 1;

        // bit 2:1  - LPF enable to filter off BD noise
        UINT32      lpfEnableFilterBGNoise  : 2;

        // bit 3    - power down the bandgap
        UINT32      bgPowerDown             : 1;

        // bit 7:4  - bandgap ptat current adjustment
        UINT32      bgPTATCurrentAdj        : 4;

        // bit 11:8 - bandgap CTAT current adjustment
        UINT32      bgCTATCurrentAdj        : 4;

        // bit 14:12 - BG Vref adjustment
        UINT32      bgVrefAdjust            : 3;

        // bit 31:15 -reserved
        UINT32      reserved                : 17;

    }bitmap;
}LDO_CTL0_REG;


// LDO_CTL0
// define higher 16 bits for the ldo_ctl[31:16]
typedef union LDO_CTL1_REG
{

    struct
    {
        // bit 0    - LDO Power down
        UINT32      ldoPowerDown             : 1;

        // bit 3:1  - LPF enable to filter off BD noise
        UINT32      ldoOutputVoltageAdj     : 3;

        // bit 4    - Disable VDS matching amplifier for low dropout
        UINT32      ldoVDSdisable           : 1;

        // bit 5   - Enable LDO current limite
        UINT32      ldocurrentLimitenable   : 1;

        // bit 6    - LDO limit mode/loopback mode 0:limite 1:loopback
        UINT32      ldoCurrentMode          : 1;

        // bit 7    - limit mode adjustment
        UINT32      ldoCurrentLimitModeAdj  : 1;

        // bit 9:8    - current mode adjustment
        UINT32      ldoCurrentLoopbackModeAdj   : 2;

        // reserved
        UINT32      reserved                :22;

    }bitmap;

    UINT32          ldo_ctl1_reg;
}LDO_CTL1_REG;

enum
{
    BG_LOW_POWERMODE_ENABLE         =   0,
    BG_LOW_POWERMODE_DISABLE        =   1,

    BG_POWER_DOWN                   =   1,
    BG_POWER_UP                     =   0,

    LDO_POWER_DOWN                  =   1,
    LDO_POWER_UP                    =   0,

    LDO_VDS_ENABLE                  =   0,
    LDO_VDS_DISABLE                 =   1,

    LDO_CURRENT_LIMIT_ENABLE        =   1,
    LDO_CURRENT_LIMIT_DISABLE       =   0,

    // valid only when LIMIT_MODE enable
    LDO_CURRENT_LIMIT_CONSTANT_MODE       =   0,
    LDO_CURRENT_LIMIT_LOOPBACK_MODE       =   1,

};

// LPO output voltage
// deault  1.2v
enum LDO_OUTPUT_VOL_ADJ
{
    LDO_VOUT_0P88V                   =   0x4,
    LDO_VOUT_0P96V                   =   0x5,
    LDO_VOUT_1P04V                   =   0x6,
    LDO_VOUT_1P12V                   =   0x7,
    LDO_VOUT_1P20V                   =   0x0,
    LDO_VOUT_1P24V                   =   0x1,
    LDO_VOUT_1P28V                   =   0x2,
    LDO_VOUT_1P32V                   =   0x3,
};

/// Initialize the LDO. Internal.
void ldo_init(void);

/// Configure the LDO. Internal.
void ldo_config(LDOconfig  *ldoCfg);

/* @} */

#endif
