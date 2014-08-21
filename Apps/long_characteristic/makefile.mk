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
APP_SRC = long_characteristic.c

########################################################################
# Add requires libraries here.
# This application requires a special patch to support characteristics
# larger than 23 octets in length
########################################################################
APP_PATCHES_AND_LIBS += long_characteristic_support.a
APP_PATCHES_AND_LIBS += thread_and_mem_mgmt.a
