/*
********************************************************************
* THIS INFORMATION IS PROPRIETARY TO
* BROADCOM CORP.
*-------------------------------------------------------------------
*
*           Copyright (c) 2011 Broadcom Corp.
*                      ALL RIGHTS RESERVED
*
********************************************************************

********************************************************************
*    File Name: adc.h
*
*    Abstract: This file defines the ADC chip specific data struct
*
*
********************************************************************
*/

#ifndef __ADC_CS__H__
#define __ADC_CS__H__

#include "types.h"

/**  \addtogroup ADC
 *  \ingroup HardwareDrivers
*/

/// supported ADC input channel selection
 typedef enum ADC_INPUT_CHANNEL_SEL {
    /// Same as GND.
    ADC_INPUT_VSS =0,
    ADC_INPUT_P38,
    ADC_INPUT_P37,
    ADC_INPUT_P36,

    ADC_INPUT_P35,
    ADC_INPUT_P34,
    ADC_INPUT_P33,
    ADC_INPUT_P32,

    ADC_INPUT_P31,
    ADC_INPUT_P30,
    ADC_INPUT_P29,
    ADC_INPUT_P28,

    ADC_INPUT_P23,
    ADC_INPUT_P22,
    ADC_INPUT_P21,
    /// P20 is the special internal bandgap.
    /// This GPIO cannot be used as an input channel.
    ADC_INPUT_P20,

    ADC_INPUT_P19,
    ADC_INPUT_P18,
    ADC_INPUT_P17,
    ADC_INPUT_P16,

    ADC_INPUT_P15,
    ADC_INPUT_P14,
    ADC_INPUT_P13,
    ADC_INPUT_P12,

    ADC_INPUT_P11,
    ADC_INPUT_P10,
    ADC_INPUT_P9,
    ADC_INPUT_P8,

    ADC_INPUT_P1,
    ADC_INPUT_P0,
    /// Vcore is always 1.2V.
    ADC_INPUT_VDD_CORE,
    /// Vio supply.
    ADC_INPUT_VDDIO,
    ADC_INPUT_CHANNEL_MASK =0x1f,
}ADC_INPUT_CHANNEL_SEL;


/// input  voltage range selection
typedef enum ADC_INPUT_RANGE_SEL
{
    /// Range 0 to 1p2V. USe when no input to any
    /// ADC channel exceeds 1.2V.
    ADC_RANGE_0_1P2V=0,

    /// Range 0 to 2p4V. USe when no input to any
    /// ADC channel exceeds 2.4V.
    ADC_RANGE_0_2P4V,

    /// Range 0 to 3p6V. USe when no input to any
    /// ADC channel exceeds 3.6V.
    ADC_RANGE_0_3P6V,
    ADC_MAX_RANGES=3,
}ADC_INPUT_RANGE_SEL;

/// ADC power type
typedef enum ADC_POWER_TYPE
{
    /// Power down the ADC.
    ADC_POWER_DOWN          = 1,

    /// Power up the ADC.
    ADC_POWER_UP            = 0,
}ADC_POWER_TYPE;

/// ADC bandgap reference.
enum
{
    /// P25 or ADC channel 15 is always the internal bandgap reference.
    ADC_INPUT_BG_REFERENCE = ADC_INPUT_P20,
};

///
/// lhl_adc_ctl_adr register
///
typedef union LHL_ADC_CTL_REG
{
    /// 32 bits regsiters
    UINT32 lhl_ctl_reg;

    struct
    {
        /// bit 4:0 -   ADC input channel select
        UINT32 inputADCChannelSel   : 5;

        /// bit 6:5 -   adc input range select
        UINT32 inputRangeSel        : 2;                // bit 6:5
                                                        //
        /// bit 7   -   power down
        UINT32 powerDown            : 1;                // bit 7

        /// reserved 31:8
        UINT32 reserved7            : 24;               // bit 31:8
    }bitmap;
}LHL_ADC_CTL_REG;


///
/// Mia ADC interface Control Register
///
// - mia_adc_intf_ctl_adr
///
///

typedef union INTF_CTL_REG
{
    UINT32 adc_intf_ctl_reg;

    struct
    {

        /// bit 0    - start ADC
        UINT32 startADC             : 1;

        /// bit 1    - one shot
        UINT32 SampleOneShot        : 1;

        /// bit 2    - notch frequencey select
        UINT32 notchFrequenceSelect : 1;

        /// bit 3    - adc dither disable
        UINT32 adcDitherDisable     : 1;

        /// bit 6:4  - ADC mode select
        UINT32 adcModeSelect        : 3;

        /// reserved bits 15:7
        UINT32 reserved0            : 9;

        /// bit 16 - ADC sample ready
        UINT32 SampleReady          : 1;
        UINT32 reserved1            : 15;
    }bitmap;
}INTF_CTL_REG;


/// bit map definition for adc intf control register
enum
{
    INTF_CTL_SAMPLE_READY               =   0x1,

    INTF_CTL_MODE_0_6P125KHZ            =   0x0 ,
    INTF_CTL_MODE_1_12P5KHZ             =   0x1 ,
    INTF_CTL_MODE_2_50KHZ               =   0x2 ,
    INTF_CTL_MODE_3_100KHZ              =   0x3 ,
    INTF_CTL_MODE_4_200KHZ              =   0x4 ,

    INTF_CTL_ADC_DITHER_DIABLE          =   0x1,

    INTF_CTL_ADC_NOTCH_FREQUENCY_DC     =   0x1,
    INTF_CTL_ADC_SAMPLE_ONE_SHOT        =   0x1,
    INTF_CTL_ADC_SAMPLE_CONTINUOUS      =   0x1,

    INTF_CTL_START_ADC                  =   0x1,
};


///
/// MIA ADC Control 0 Register
/// - mia_adc_ctl0_adr
///
typedef union ADC_CTL0_REG
{
    UINT32 adc_ctl0_reg;
    struct
    {
        /// bit 0 - reserved
        UINT32 reserved2                : 1;

        /// bit 1:1     -   Dither power down control
        UINT32 ditherPowerDown          : 1;

        /// bit 3:2     -   Bandgap noise filter time const
        UINT32 bandgapNoiseFilter       : 2;

        /// bit 7:4     -   Trim the bandgap output current for 9uA to 53uA
        UINT32 bandgapOutCurrentTrim    : 4;

        /// bit 9:8     - Bandgap temperature constant. Default 00
        UINT32 bandgapTempConstant      : 2;

        /// bit 14:10   - RC Calibration for ADC. Default 0xc
        UINT32 rcCalibration            : 5;
        UINT32 reserved3                : 17;
    }bitmap;
}ADC_CTL0_REG;

/// Bit map definition for CTL0.
enum
{
    // bit 14:10
    CTL0_REG_RC_CALIBRATION_DEFAULT              =    0xc  ,

    // bit 9:8
    CTL0_REG_BANDGAP_TEMPERATURE_DEFAULT         =   0x00,

    // bit 7:4
    CTL0_REG_BANDGAP_OUTPUT_CURRENT_MIN_9uA      =   0x0 ,
    CTL0_REG_BANDGAP_OUTPUT_CURRENT_MIN_26uA     =   0x6 ,
    CTL0_REG_BANDGAP_OUTPUT_CURRENT_MIN_53uA     =   0xf ,

    CTL0_REG_BANDGAP_OUTPUT_CURRENT_TRIM_DEFAULT =   CTL0_REG_BANDGAP_OUTPUT_CURRENT_MIN_26uA,

    // bit 3:2
    CTL0_REG_BANDGAP_NOISE_FILTER_HIGH           =   0x0,
    CTL0_REG_BANDGAP_NOISE_FILTER_LOW            =   0x3,

    CTL0_REG_BANDGAP_NOISE_DEFAULT               =   CTL0_REG_BANDGAP_NOISE_FILTER_LOW ,

    // bit 1
    CTL0_REG_DITHER_POWER_DOWN                   =   1,     // bit 1
    CTL0_REG_DITHER_POWER_UP                     =   0,

    CTL0_REG_DITHER_POWER_UP_DEFAULT            =   CTL0_REG_DITHER_POWER_DOWN ,

};


///
/// MIA ADC Control 1 registers
/// - mia_adc_ctl1_adr
///

typedef union ADC_CTL1_REG
{
    UINT32 adc_ctl1_reg;
    struct
    {
        /// bit 2:0   - Reference voltage for OPAMP
        UINT32 OPAMPReferenceVol        : 3;

        /// bit 6:3   - Reference voltage for comparator
        UINT32 compReferenceVol         : 4;

        /// bit 10:7  - IDAC Trimming code for 1st OPAMP
        UINT32 IDACTrim1stOPAMP         : 4;

        /// bit 14:11 - IDAC Trimming code for 2nd OPAMP
        UINT32 IDACTrim2ndOPAMP         : 4;
        UINT32 reserved4                : 17;
    }bitmap;
}ADC_CTL1_REG;

/// Bitmap definition for CTL1.
enum
{
    // bit 14:11
    CTL1_REG_IDAC_TRIMMING_2ND_OPAMP_DEFAULT     =  0x6,

    // bit 10:7
    CTL1_REG_IDAC_TRIMMING_1ST_OPAMP_DEFAULT     =  0x6 ,

    CTL1_REG_REFERENCE_VOL_COMP_550mV        =   0x0 ,
    CTL1_REG_REFERENCE_VOL_COMP_600mV        =   0x1 ,
    CTL1_REG_REFERENCE_VOL_COMP_650mV        =   0x3 ,
    CTL1_REG_REFERENCE_VOL_COMP_700mv        =   0x7 ,
    CTL1_REG_REFERENCE_VOL_COMP_750mV        =   0xf ,

    CTL1_REG_REFERENCE_VOL_COMP_DEFAULT      =   CTL1_REG_REFERENCE_VOL_COMP_650mV,

    // bit 6:3
    CTL1_REG_REFERENCE_VOL_OPAMP_400mV        =   0x0,
    CTL1_REG_REFERENCE_VOL_OPAMP_450mV        =   0x1,
    CTL1_REG_REFERENCE_VOL_OPAMP_500mV        =   0x3,
    CTL1_REG_REFERENCE_VOL_OPAMP_550mv        =   0x7,

    // bit 2:0
    CTL1_REG_REFERENCE_VOLTAGE_OPAMP_DEFAULT  =   CTL1_REG_REFERENCE_VOL_OPAMP_500mV,

};



///
/// MIA ADC Control 2 registers
///
/// -mia_adc_ctl2_adr
///


typedef union ADC_CTL2_REG
{
    UINT32 adc_ctl2_reg;
    struct
    {
        //
        // this bit will be used for 20730B0
        // when do the ADC calibration use Bandgap as
        // reference input
        UINT32   bandgap_cal_enable      : 1;
                                                            //
        /// bit 3   - touch_enable3
        /// bit 2   - touch_enable2
        /// bit 1   - touch enable1
        UINT32 touch_enable             : 3;

        /// bit 4   - Bias current control for comparator
        UINT32 biasCurrentCtlComp       : 1;

        /// bit 5   - Dither Clock polarity control
        UINT32 adcDitherClkPolarity     : 1;

        /// bit 6   - Sampling clock polarity control
        UINT32 adcSampleClkPolarity     : 1;

        /// bit 7   - DAC OPAMP power down control
        UINT32 adcOPAMPpowerDown        : 1;

        /// reserved bits
        UINT32 reserved4                : 24;
    }bitmap;
}ADC_CTL2_REG;

/// Bitmap definition for CTL2.
enum
{
    // bit 7
    CTL2_REG_ADC_OPAMP_POWER_DOWN            =   0x1 ,
    CTL2_REG_ADC_OPAMP_POWER_UP              =   0x0 ,

    // bit 6
    CTL2_REG_ADC_OUTPUT_CLK_POLARITY_NEG      =   0x1,
    CTL2_REG_ADC_OUTPUT_CLK_POLARITY_POSITIVE =   0x0,
    CTL2_REG_ADC_OUTPUT_CLK_POLARITY_DEFAULT  =   CTL2_REG_ADC_OUTPUT_CLK_POLARITY_NEG,

    // bit 5
    CTL2_REG_DITHER_INPUT_CLK_POLARITY_NEG        =   0x1 ,
    CTL2_REG_DITHER_INPUT_CLK_POLARITY_POSITIVE   =   0x0 ,
    CTL2_REG_DITHER_INPUT_CLK_POLARITY_DEFAULT    =   CTL2_REG_DITHER_INPUT_CLK_POLARITY_POSITIVE,

    // bit 4
    CTL2_REG_BIAS_CURRENT_CONTROL_COMP_5P6uA     =   0x1 ,
    CTL2_REG_BIAS_CURRENT_CONTROL_COMP_4P3uA     =   0x0 ,

    // bit 3:1
    CTL2_REG_TOUCH_ENABLE_MASK                   =   0x000e,
    CTL2_REG_TOUCH_ENABLE3                       =   0x1 ,
    CTL2_REG_TOUCH_ENABLE2                       =   0x2 ,
    CTL2_REG_TOUCH_ENABLE1                       =   0x4 ,

    // bit 0
    CTL2_REG_ENABLE_BANDGAP_READ                 = 0x1,
    CTL2_REG_ENABLE_BANDGAP_MASK                 = 0x1,
    CTL2_REG_DISABLE_BANDGAP_READ                = 0,
};



/// ADC calibration mode.
enum
{
    /// ADC will auto calibrate when configured. Standard setting.
    ADC_CONFIG_AUTO_CALIBRATION             = 0,

    /// application provided calibration settings. Used with factory calibration
    ADC_CONFIG_USER_INPUT                   = 1,

};

/// Internal state of the ADC driver.
typedef struct
{
    /// Ground offset. Obtained form user settings or auto calibration
    INT32          gndReading;

    /// Reference voltage conversion value.
    /// Obtained from user configuation setting or auto calibration
    INT32          referenceReading;

    /// Reference micro voltage
    UINT32          refMicroVolts;

    /// Whether the ADC driver has been initialized or not
    UINT8        inited;
} AdcState;

/// ADC configuration from the cgs.
typedef PACKED struct
{
    PACKED union
    {
        PACKED struct
        {
            /// bit 0 - reserved
            UINT32 reserved2                : 1;

            /// bit 1:1     -   Dither power down control
            UINT32 ditherPowerDown          : 1;

            /// bit 3:2     -   Bandgap noise filter time const
            UINT32 bandgapNoiseFilter       : 2;

            /// bit 7:4     -   Trim the bandgap output current for 9uA to 53uA
            UINT32 bandgapOutCurrentTrim    : 4;

            /// bit 9:8     - Bandgap temperature constant. Default 00
            UINT32 bandgapTempConstant      : 2;

            /// bit 14:10   - RC Calibration for ADC. Default 0xc
            UINT32 rcCalirbarion            : 5;
            UINT32 reserved3                : 17;
        }bitmap_adc_ctl0;

        UINT32 adc_ctl0;

    } adc_ctl0_reg;

    PACKED union
    {
        PACKED struct
        {
            /// bit 2:0   - Reference voltage for OPAMP
            UINT32 OPAMPReferenceVol        : 3;

            /// bit 6:3   - Reference voltage for comparator
            UINT32 compReferenceVol         : 4;

            /// bit 10:7  - IDAC Trimming code for 1st OPAMP
            UINT32 IDACTrim1stOPAMP         : 4;

            /// bit 14:11 - IDAC Trimming code for 2nd OPAMP
            UINT32 IDACTrim2ndOPAMP         : 4;
            UINT32 reserved4                : 17;

        }bitmap_adc_ctl1;

        UINT32 adc_ctl1;
    }adc_ctl1_reg;


    PACKED union
    {
        PACKED struct
        {
            UINT32 bandgap_cal_enable       : 1;                // bit 0
                                                                //
            /// bit 3   - touch_enable3
            /// bit 2   - touch_enable2
            /// bit 1   - touch enable1
            UINT32 touch_enable             : 3;

            /// bit 4   - Biaso current control for compartor
            UINT32 biasCurrentCtlComp       : 1;

            /// bit 5   - Dither Clock Polarity Control
            UINT32 adcDitherClkPolarity     : 1;

            /// bit 6   - Sampling clock polarity control
            UINT32 adcSampleClkPolarity     : 1;

            /// bit 7   - DAC OPAMP power down control
            UINT32 adcOPAMPpowerDown        : 1;

            /// reserved bits
            UINT32 reserved4                : 24;

        }bitmap_adc_ctl2;

        UINT32 adc_ctl2;

    } adc_ctl2_reg;

    /// ADC power on delay, default 20us
    UINT8       powerondelayInUs;           // us

    /// bandgap reference channel, default 16
    UINT8       bandgapReferenceChannel;

    ///  calibration average count
    UINT8       calibrationConversionCount;

    /// input range
    UINT8      inputRangeSelect;

    /// reference voltage for bandgap, default 500mV
    UINT32      internalBandgapInMicroVolts;

    /// For legacy devices, the bandgap has to be offset by a certain amount
    INT16 ADCBandgapDelta[3];

}ADCConfig;

/// Generic sample frequency/mode settings.
typedef enum ADC_SAMPLE_FREQUENCY
{
    /// Low sample frequency. Provides 13 ENOB. See datasheet.
    /// For 20737, 5.859 KHz, ~170uS latency to sample.
    ADC_SAMPLE_FREQUENCY_LOW,

    /// Medium low sample frequency. Provides ~12.6 ENOB.
    /// For 20737, 11.7KHz.  ~85uS latency to sample. See datasheet.
    ADC_SAMPLE_FREQUENCY_MEDIUM_LOW,

    /// Medium sample frequency. Provides ~12 ENOB.
    /// For 20737, 46.875 KHz. ~21uS latency to sample. See datasheet.
    ADC_SAMPLE_FREQUENCY_MEDIUM,

    /// Medium high sample frequency. Provides ~11.5 ENOB.
    /// For 20737, 93.75 KHz. ~11uS latency to sample. See datasheet.
    ADC_SAMPLE_FREQUENCY_MEDIUM_HIGH,

    /// High sample frequency .Provides ~10 ENOB.
    /// For 20737, 187 KHz. ~5uS latency to sample. See datasheet.
    ADC_SAMPLE_FREQUENCY_HIGH
} ADC_SAMPLE_FREQUENCY;

/* @} */

#endif
