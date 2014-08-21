Manufacturing Bluetooth Test Tool 

Overview

The manufacturing Bluetooth test tool (MBT) is used to test and verify the RF 
performance of the BCM2073x family of SoC Bluetooth Low Energy (BLE) devices. 
Each test sends an HCI command to the device and then waits for an HCI Command 
Complete event from the device.

To run the tests:
  1. Configure the BCM2073x to run in the HCI mode. When a BCM2073x device 
     starts up it looks for the HCI UART: if detected, the device stays in the 
	 HCI mode. Configuration example: to use a WICED Smart Tag board in the 
	 HCI mode, verify that all of the SW4 DIP switches are in the ON position.

  2. Plug the BCM2073X into the computer and note the COM port assigned to 
     the HCI UART. The COM port is used to send HCI commands and receive HCI 
	 events from the device.

Note: All examples in this document use COM4 as the port assigned to the 
BCM2073x by the PC. Replace “COM4” in each test example with the actual port 
assigned to the BCM2073x under test (see [1], for details on how to determine 
the assigned port).

Reset Test

This test verifies that the device is correctly configured and connected to 
the PC.

Usage: mbt reset COMx

The example below sends HCI_Reset command to the device and processes the HCI 
Command Complete event (BLUETOOTH SPECIFICATION Version 4.1 [Vol 2], Section 
7.3.2 for details).

WICED-Smart-SDK\Tools\mbt\win32>mbt reset COM4
Sending HCI Command:
0000 < 01 03 0C 00 >
Received HCI Event:
0000 < 04 0E 04 01 03 0C 00 >
Success

The last byte of the HCI Command Complete event is the operation status, where 
0 signifies success.


LE Receiver Test

This test configures the BCM2073x to receive reference packets at a fixed 
interval. External test equipment should be used to generate the reference 
packets.

The channel on which the device listens for the packets is passed as a 
parameter. BLE devices use 40 channels, each of which is 2 MHz wide. Channel 
0 maps to 2402 MHz and Channel 39 maps to 2480 MHz (see BLUETOOTH 
SPECIFICATION Version 4.1 [Vol 2], Section 7.8.28 for details).

Usage: mbt le_receiver_test COMx <rx_channel>
where:
    rx_channel = receive frequency minus 2402 divided by 2. For example, 
	if the desired receive frequency is 2406MHz then the rx_channel = 
	(2406 – 2402) / 2 = 2.
	The channel range is 0–39 (2402–2480 MHz).

The example below starts the LE receiver test on Channel 2 (2406 MHz).

WICED-Smart-SDK\Tools\mbt\win32>mbt le_receiver_test COM4 2
Sending HCI Command:
0000 < 01 1D 20 01 02 >
Received HCI Event:
0000 < 04 0E 04 01 1D 20 00 >
Success
LE Receiver Test running, to stop execute mbt le_test_end COMx

The last byte of the HCI Command Complete event is the operation status, 
where 0 signifies success. Use mbt le_test_end COMx to complete the test and 
print the number of received packets.

Note: This test will fail if the device is running another test: use 
le_test_end or reset to put the BCM2073x in idle state before running this 
test.


LE Transmitter Test

The LE Transmitter Test configures the BCM2073x to send test packets at a 
fixed interval. External test equipment may be used to receive and analyze 
the reference packets. 

The channel on which the BCM2073x transmits the packets  is passed as a 
parameter. BLE devices use 40 channels, each of which is 2 MHz wide. Channel 0 
maps to 2402 MHz and Channel 39 maps to 2480 MHz.

The other two parameters specify the length of the test data and the data 
pattern to be used (see BLUETOOTH SPECIFICATION Version 4.1 [Vol 2], Section 
7.8.29 for details).

Usage: mbt le_transmitter_test COMx <tx_channel> <data_length> <packet_payload>
where:
	tx_channel = transmit frequency minus 2402 divided by 2. For example, if 
	the transmit frequency is 2404 MHz then the tx_channel = (2404 – 2402) / 2 = 1.
	The channel range is 0–39 (2402–2480 MHz).

	data_length = 0–37

	data_pattern = 0–7
		0 = Pseudo-random bit sequence 9
		1 = Pattern of alternating bits: 11110000
		2 = Pattern of alternating bits: 10101010
		3 = Pseudo-random bit sequence 15
		4 = Pattern of all 1s
		5 = Pattern of all 0s
		6 = Pattern of alternating bits: 00001111
		7 = Pattern of alternating bits: 0101

The example below starts the test and instructs the device to transmit packets 
on Channel 2 (2406 MHz), with a 10-byte payload of all ones (1s).

WICED-Smart-SDK\Tools\mbt\win32>mbt le_transmitter_test COM4 2 10 4
Sending HCI Command:
0000 < 01 1E 20 03 02 0A 04 >
Received HCI Event:
0000 < 04 0E 04 01 1E 20 00 >
Success
LE Transmitter Test running, to stop execute mbt le_test_end COMx

The last byte of the HCI Command Complete event is the status of the operation, 
where 0 signifies the success.  Use mbt le_test_end COMx to complete the test.

Note: This test will fail if the device is running another test: use le_test_end 
or reset to put the BCM2073x in idle state before running this test.


LE Test End

This command stops the LE Transmitter or LE Receiver Test that is in progress 
on the BCM2073x.  The number of packets received during the test is reported 
by the device and printed out. The value will always be zero (0) if the LE 
Transmitter Test was active (see BLUETOOTH SPECIFICATION Version 4.1 [Vol 2], 
Section 7.8.30 for details).

Usage: mbt le_test_end COMx

The example below stops the active test.

WICED-Smart-SDK\Tools\mbt\win32>mbt le_test_end COM4
Sending HCI Command:
0000 < 01 1F 20 00 >
Received HCI Event:
0000 < 04 0E 06 01 1F 20 00 00 00 >
Success num_packets_received 0


Continuous Transmit Test

Note: Unlike the LE tests, this test uses 79 frequencies, each 1 MHz wide.

This test configures the BCM2073x to turn the carrier ON or OFF. When the 
carrier is ON the device transmits an unmodulated pattern on the specified 
frequency at a specified power level.

The frequency to be used by the BCM2073x is passed as a parameter.

The PA table for the BLE BCM2073x family of devices is hard-coded to +3, –1, 
–5, –9, –13, –17, –21, and –25 dB. This table conforms to a power control type 
compliance format (4 dB steps). If a different value is specified in the test, 
the device will round the transmit power to the nearest value in the PA table.

Usage: mbt set_tx_frequency_arm COMx <carrier on/off> <tx_frequency> <tx_power>
where:

	carrier on/off:
		1 = carrier ON
		0 = carrier OFF
	tx_frequency = (2402 – 2480) transmit frequency, in MHz
	tx_power = (–25 to +3) transmit power, in dBm

The example below turns the carrier ON and instructs the BCM2073x to transmit an 
unmodulated pattern on 2402 MHz at 3 dBm.

WICED-Smart-SDK\Tools\mbt\win32> mbt set_tx_frequency_arm COM4 1 2402 3
Sending HCI Command:
0000 < 01 14 FC 07 00 02 00 03 08 03 00 >
Received HCI Event:
0000 < 04 0E 04 01 14 FC 00 >
Success

To stop the test, send the command a second time to the same COM port with the 
carrier on/off parameter set to zero (0). No other parameters are used.

WICED-Smart-SDK\Tools\mbt\win32>mbt set_tx_frequency_arm COM4 0 0 0
Sending HCI Command:
0000 < 01 14 FC 07 00 02 00 03 08 03 00 >
Received HCI Event:
0000 < 04 0E 04 01 14 FC 00 >
Success
