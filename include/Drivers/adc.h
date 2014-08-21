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
* Defines functions to access ADC peripheral. The ADC provides one-shot conversion of
* and input analog signal to a digital value (raw or converted to Volts). The maximum
* sampling rate is 187 KHz with 10 ENOB and the minimum conversion rate is 5.859 KHz.
*/
#ifndef __ADC__H__
#define __ADC__H__

#include "types.h"
#include "adc_cs.h"
#include "20732mapa1.h"

/**  \addtogroup ADC
 *  \ingroup HardwareDrivers
*/

/// Initialize the ADC HW block.
void adc_config(void);

/// Calibrate the ADC against a known (external) voltage.
/// \param refVoltageInMicroVolts The voltage of the reference in uV.
/// \param referenceInput The ADC channel to which the reference is connected to.
void adc_adcCalibrate(UINT32 refVoltageInMicroVolts, ADC_INPUT_CHANNEL_SEL referenceInput);

/// Read the analog to digital value on a given channel. If the ADC
/// is power off, this function will power it on, read and then power it off.
/// \param channel_idx The input channel to read from.
/// \return Raw ADC output.
INT16 adc_readSampleRaw(ADC_INPUT_CHANNEL_SEL channel_idx);

/// Read the analog to digital value converted to volts on a given channel.
/// Uncalibrated readings are expected to be within +/-10% while calibrated
/// values are expected to be better than +/-3%. Uses adc_readSampleRaw
/// internally.
/// \param idx The input channel to read from.
/// \return The value read in volts, scaled according to the ENOB.
UINT32 adc_readVoltage(ADC_INPUT_CHANNEL_SEL idx);

/// Powers on/off the ADC HW block. When reading from a channel,
/// it is not required to power on the ADC before the read.
/// \param pwr On or off.
/// \return The previous state.
ADC_POWER_TYPE adc_setPower(ADC_POWER_TYPE pwr);

/// Selects the input range of the ADC input channel. This applies
/// to all channels. Choose a range that will cover all ADC input channels.
/// \param range_idx The input range of th signal being measured.
void adc_SetInputRange(ADC_INPUT_RANGE_SEL range_idx);

/// Convert GPIO to ADC input channel number.
/// \param gpio The GPIO logical pin number from gpiodriver.h.
/// \return The ADC channel number.
UINT8 adc_convertGPIOtoADCInput(UINT8 gpio);

/// Convert ADC input channel number to GPIO.
/// \param adcInput The ADC input channel number.
/// \return The GPIO logical pin number from gpiodriver.h.
UINT8 adc_convertADCInputtoGPIO(UINT8 adcInput);

/// Set the sampling frequency to use. Default is 5.859 KHz.
/// \param sample_frequency The sample frequency to use.
static INLINE void adc_setAdcSampleFrequency(ADC_SAMPLE_FREQUENCY sample_frequency)
{
	((volatile INTF_CTL_REG *)mia_adc_intf_ctl_adr)->bitmap.adcModeSelect = sample_frequency;
}

/* @} */

#endif
