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

APP_SRC = a4wp_power_receiver.c

########################################################################
################ DO NOT MODIFY FILE BELOW THIS LINE ####################
########################################################################
# This application requires a special patch to access A4WP HAL 
APP_PATCHES_AND_LIBS += a4wp_hal_api.a
