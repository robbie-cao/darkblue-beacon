#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

#
# BLE Time Client device.
# Refer to Bluetooth SIG Current Time Service 1.0.
#
# The makefile creates an application referencing the code in ROM.  
# See WICED-Smart-SDK\Wiced-Smart\bleapp\app\bletime.c.
#

########################################################################
# Sources in APP_SRC are delivered to Broadcom in the Application
# Release Package (make arp).
########################################################################
APP_SRC =

########################################################################
# We are reusing the ROM application as is, so use app init in ROM as
# the spar app init function.
########################################################################
MY_APP_INIT_FUNCTION := bleapp_set_cfg_time

########################################################################
################ DO NOT MODIFY FILE BELOW THIS LINE ####################
########################################################################
C_FLAGS += -Dapplication_init=$(MY_APP_INIT_FUNCTION)