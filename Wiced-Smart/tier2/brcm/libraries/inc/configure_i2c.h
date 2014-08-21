#ifndef _CON_I2C_H_
#define _CON_I2C_H_

#include "i2cm.h"

	typedef UINT32 (tMY_I2C_CBACK)(void);

	void registerClaimPinsForI2c(tMY_I2C_CBACK);
	void registerReleasePinsFromI2c(tMY_I2C_CBACK);

	UINT32 claimPinsForI2c();
	UINT32 releasePinsFromI2c();

#endif