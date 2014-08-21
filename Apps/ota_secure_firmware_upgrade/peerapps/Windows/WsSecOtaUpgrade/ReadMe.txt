========================================================================
    CONSOLE APPLICATION : WsSecOtaUpgrade Project Overview
========================================================================

This application is performing secure firmware upgrade of the Wiced Smart 
device over the air.  It uses GATT Vendor Specific Service to send 
commands and data packets, and to receive status notifications.

WARNING:  If EEPROM or Serial Flash installed on the device is less then 64
KBytes, the memory after the upgrade might be corrupted.  Use the Recovery
procedure described in the Quick Start Guide to continue using the device.

See Secure Over the Air Firmware Upgrade application note for more details.

Usage:

WsSecOtaUpgrade <image.bin>

The <image.bin> file is build when application is being built and then post
processed to add version info and signed SHA hash.  

The Private/Public Keys are generated using WsRsaKeyGen utility.  The Public
Key is used to build the image.  The Private Key is used to sign the image.

For the ota_secure_firmware_upgrade sample following file should be used

C:\Users\<username>\Documents\WICED\WICED-Smart-SDK-x.x.x\WICED-Smart-
SDK\build\ota_secure_firmware_upgrade-BCM920736TAG_Q32-rom-ram-Wiced-release\
ota_secure_firmware_upgrade-BCM920736TAG_Q32-rom-ram-Wiced-release.ota.bin

and then signed with
WsRsaSign.exe <Private key file> <file.ota.bin > <project_id> <major> <minor>

where 
That will create file.ota.bin.signed which can be used for upgrade.

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
completed application sends WS_UPGRADE_COMMAND_VERIFY.

At that time device verifies that data has been successfully committed
to EEPROM or SFLASH and acknowledge with WS_UPGRADE_RESPONSE_OK if 
success or WS_UPGRADE_RESPONSE_FAILED if not all bytes were received 
or if signature does not match.  In case of success device automatically 
reboots after making downloaded image active.

