========================================================================
    CONSOLE APPLICATION : WsOtaFu Project Overview
========================================================================

This application is performing firmware upgrade of the Wiced Smart 
device over the air.  It uses GATT Vendor Specific Service to send 
commands and data packets, and to receive status notifications.

WARNING:  If EEPROM or Serial Flash installed on the device is less then 64
KBytes, the memory after the upgrade might be corrupted.  Use the Recovery
procedure described in the Quick Start Guide to continue using the device.

Usage:

WsOtaUpgrade <image.bin>

The <image.bin> file is build when application is being built.  For the
ota_firmware_upgrade sample following file should be used

C:\Users\<username>\Documents\WICED\WICED-Smart-SDK-x.x.x\WICED-Smart-
SDK\build\ota_firmware_upgrade-BCM920736TAG_Q32-rom-ram-Wiced-release\
ota_firmware_upgrade-BCM920736TAG_Q32-rom-ram-Wiced-release.ota.bin

The protocol for the upgrade over air is rather simple.  

On the startup application reads the FW image file.  Then 
application searches for a paired device which exposes the Vendor
Specific Wiced Smart Upgrade service.  

The Upgrade service exposes Control Point characteristic which 
application can use to send commands and receive notifications, and 
a Data characteristic which application uses to send chunks of data 
to the device.  

To start the upgrade application writes one byte
WS_UPGRADE_COMMAND_PREPARE_DOWNLOAD command.  And device replies with
WS_UPGRADE_RESPONSE_OK.

Next application writes WS_UPGRADE_COMMAND_DOWNLOAD with 2 bytes
of the patch lenght.  After application receive OK response it starts
writing 20 bytes chunks of data.  After all the data has been 
completed application sends WS_UPGRADE_COMMAND_VERIFY command
passing passing 32 bit checksum.

At that time device verifies that data has been successfully committed
to EEPROM or SFLASH and acknowledge with WS_UPGRADE_RESPONSE_OK if 
success or WS_UPGRADE_RESPONSE_FAILED if not all bytes were received 
or if checsum does not match.  In case of success device automatically 
reboots after making downloaded image active.

