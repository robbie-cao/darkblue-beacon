#
# Copyright 2014, Broadcom Corporation
# All Rights Reserved.
#
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#

# This target should be build for 20737 platform because it requires
# RSA library.  The rsa_pub.c should be generated using WsRsaKeyGen
# tool and placed in the project directory.  See Secure Over the Air
# Firmware Upgrade application note for details.

########################################################################
# Add Application sources here.
########################################################################
APP_SRC = hello_sensor.c ws_sec_upgrade_ota.c ws_upgrade.c rsa_pub.c

########################################################################
# Add requires libraries here.
# This application requires a special patch to change stack size and 
# memory usage to support RSA functionality
########################################################################
APP_PATCHES_AND_LIBS += thread_and_mem_mgmt.a
