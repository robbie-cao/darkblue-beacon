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
# BLE Proximity device
# Refer to Bluetooth SIG Proximity Profile 1.0, Immediate Alert Service
# 1.0, Link Loss Service 1.0, Tx Power Service, and Battery Service 1.0 
# specifications for details.
#
# The makefile creates an application referencing the code in ROM.  
# See WICED-Smart-SDK\Wiced-Smart\bleapp\app\bleprox.c.
#

########################################################################
# Sources in APP_SRC are delivered to Broadcom in the Application
# Release Package (make arp).
########################################################################

APP_SRC = proximity.c

########################################################################
################ DO NOT MODIFY FILE BELOW THIS LINE ####################
########################################################################
