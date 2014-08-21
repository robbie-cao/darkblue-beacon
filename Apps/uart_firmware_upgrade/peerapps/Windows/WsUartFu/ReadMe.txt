========================================================================
    CONSOLE APPLICATION : WsUartFu Project Overview
========================================================================

This application is performing firmware upgrade of the Wiced Smart device over 
the peripheral UART.

WARNING:  If EEPROM or Serial Flash installed on the device is less then 64
KBytes, the memory after the upgrade might be corrupted.  Use the recovery
procedure described in the Quick Start Guide to continue using the device.

Usage:

WsUartFu <image.bin> <COMx>

The protocol for the upgrade over UART is rather simple.  

On the startup application reads the FW image file.  After the COM
port initalization application sends 3 bytes of the Start command.

0x33 <fw image length>

Device should responds with a single byte 0xF0 indicating that it
is ready for download.

Then application writes 15 byte chunks of the FW image waiting for
the one byte 0xF0 acknowledgment after ever data transfer.

After all bytes have been sent and acknowledged application sends 5
bytes of the finish command passing 32 bit checksum

0x55 <checksum>

At that time device verifies that data has been successfully committed
to EEPROM or SFLASH and acknowledge with 0xF0 if success or 0xF1 if
not all bytes were received or if checsum does not match.  In case
of success device automatically reboots after making downloaded image
active.

