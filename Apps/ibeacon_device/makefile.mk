#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

########################################################################
# Add Application sources here.
########################################################################
APP_SRC = ibeacon_device.c

########################################################################
# Add requires libraries here.
# This application requires a special patch to stop RF activity when GPIO is active
########################################################################
APP_PATCHES_AND_LIBS += additional_advertisement_control.a
APP_PATCHES_AND_LIBS += ibeacon.a

