----------------------------------------------------------
WICED Smart Application Directory - README
----------------------------------------------------------

This directory contains applications that demonstrate how to
use the WICED Smart API and how to build advanced applications.
Several applications are also provided to test WICED hardware.


Pre-Requisites for running WICED Smart Applications
---------------------------------------------
 ***PLEASE*** read through the WICED Smart Quickstart Guide located
 in the <WICED-Smart-SDK>/Doc directory before attempting to use any of 
 the applications in this directory!

    
Application Demonstration Requirements
---------------------------------------------
* A development computer with the following software:
    * The WICED Smart IDE & SDK installed
    * A terminal application (such as PuTTY). The WICED Quickstart Guide
      located in the <WICED-Smart-SDK>/Doc directory describes how to configure
      a terminal application.
    * An up-to-date web browser such as Chrome, Safari or Internet Explorer

A4WP Power Transmitter
     This application implements the A4WP sample power transmitter as per the BSS 1.2 
     specification.

A4WP Power Receiver
     This application implements the A4WP sample power receiver as per the BSS 1.2 
     specification. The application interfaces with the Broadcom Power Manangement Unit
     chip (59350) for receiving the power from PTU coil.
    
Automation I/O Application
     This application implements a digital and analog input and output signals as 
     specified in Bluetooth SIG Automation IO Profile 0.9 and Automation IO Service
     0.9 specifications
	 
Blood Pressure Monitor Application
     This application implements the Blood Pressure Monitor device as specified
     in Bluetooth SIG Blood Pressure Profile 1.0 and Blood Pressure Service 1.0 
     specifications

Cycling Speed and Cadence Application
     This application implements the Cycling Speed and Cadence Sensor as specified
     in Bluetooth SIG Cycling Speed and Cadence Profile 1.0 Cycling Speed and Cadence
     Service 1.0 specifications

Glucose Meter Application
     This application implements Glucose Service and Device Information Service 
     as as specified in Bluetooth SIG Glucose Profile 1.0 and Glucose Service 1.0 
     specifications
	 
Health Thermometer Application
     This application implements the Health Thermometer device as specified
     in Bluetooth SIG Health Thermometer Profile 1.0 and Health Thermometer Service 1.0 
     specifications

Heart Rate Monitor Application
     This application implements the Heart Rate Monitor device as specified
     in Bluetooth SIG Heart Rate Monitor Profile 1.0 and Heart Rate Monitor Service 1.0 
     specifications

Hello Client Application
     This application shows an example implementation of a vendor specific client profile.

Hello Sensor Application
     This application shows an example of a vendor specific device and service.

I2C Temperature sensor
     This application provides a sample code interfacing TI LM73 temperature
     sensor (http://www.ti.com/product/lm73) over the I2C interface.
	 
iBeacon Device
	 A simple iBeacon implementation that uses non-connectable undirected advertisements.
	 Note: This sample depends on a library that is available to MFI licensees. Please
	 contact a Broadcom sales representative for this library.
	 
iBeacon Managed
	 An iBeacon sample implementation  which also supports a Vendor
	 specific service which allows a peer device, for example a smart phone,
	 to connect and configure iBeacon parameters.  The configured information
	 is saved in the NVRAM, so that iBeacon would start up correctly after
	 the power cycle and does not need to be configured again.  For
	 security purposes iBeacon Managed device advertises an allowed peer
	 to be connected only when the button is pressed.
	 Note: This sample depends on a library that is available to MFI licensees. Please
	 contact a Broadcom sales representative for this library.
	 
Location and Navigation
	 This sample implements the Location and Navigation Sensor specified in the Bluetooth
	 SIG Location and Navigation Profile 1.0 and Location and Navigation Service 1.0.

Long Characteristic
	 This sample demonstrates how to read and write values to a characteristic value
	 larger than 512 octets in length.

My Beacon Application
     This application implements a sample beacon device. Also demonstrates how advertisements
     can be paused (with no RF activity) when any configured GPIO is asserted externally.

Over the air firmware upgrade
     This application adds Over The Air (OTA) upgrade capability to the hello sensor
     application.

Over the air Secure Firmware Upgrade
	 This application adds Secure Over The Air (SOTA) upgrade capability to the hello
	 sensor application. In addition, this application also shows how the application
	 thread stack size and the dynamic memory pools can be configured.
	 
Proximity Client
     This application is a sample implementation of a proximity client profile that
     interacts with the proximity sample application in the ROM.

Proximity Sensor Application
     This application implements the Proximity Sensor device as specified
     in Bluetooth SIG Proximity Profile 1.0 and Proximity Service 1.0 
     specifications

PWM Tones
	 This application sample uses two channels of the on-chip PWM, one to generate
	 different tones using the buzzer on the tag board for every press of the button
	 and another channel to generate random PWM signals on P26.

RTC Sample
	 This application provides the sample code for interfacing with
	 the on-chip RTC clock. The sample also shows how the application can configure
	 the RTC to wake the device from deep sleep.
	 
Running Speed and Cadence Application
     This application implements Runners Speed and Cadence Sensor as specified in 
     Bluetooth SIG Runners Speed and Cadence Profile 1.0 Runners Speed and Cadence 
     Service 1.0 specifications.

Speed test
     This is a sample application to demonstrate/test throughput at the GATT layer.

SPI communications master
     The application initializes the second SPI interface to communicate with
     the a peer device as a SPI master.

SPI communications slave
     The application initializes the second SPI interface to communicate with
     the a peer device as a SPI slave.

SPI Pressure sensor
     This application provides a sample code interfacing the MS5525DSO
     pressure sensor (http://www.meas-spec.com/product/pressure/MS5525DSO.aspx)
     over the SPI interface.

Time Application
     This application implements the Time device as specified
     in Bluetooth SIG Time Profile 1.0 and Time Service 1.0 
     specifications
     
UART firmware upgrade
     This application adds firmware/application upgrade capability over peripheral uart.

Watch
     This application provides a sample watch reference platform.
