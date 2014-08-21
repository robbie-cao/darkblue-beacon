=====================================================================
Broadcom WICED Smart Software Development Kit 2.1.0 - README
=====================================================================

The WICED Smart SDK provides systems and APIs needed to build, design 
and implement applications for Broadcom Bluetooth Low Energy devices.

Major features of the WICED Smart SDK include ...

- API to access Bluetooth Smart stack including GAP, ATT, GATT and SMP in the ROM.
- A generic profile level API that abstracts stack layer API.
- Reference applications defined by the Bluetooth SIG.
- API and drivers to access on board peripherals like I2C, SPI, UART, ADC, PWM, Keyscan and IR HW blocks.
- Support for BCM20736 and BCM20737 based platforms.
	* For BCM20732A0 based platforms, use SDK 1.x. This SDK does not support BCM20732A0 based platforms.
- Segger J-Link debugger using J-Link GDB server and GCC GDB client.
 
The WICED Smart SDK release is structured as follows:
Apps          : Example & Test Applications
Doc           : API & Reference Documentation, Eval Board & Module Schematics
Include       : WICED API, constants, and defaults
Platforms     : Configuration files and information for supported hardware platforms
Tools         : Build tools, compilers, programming tools etc.
WICED Smart   : WICED core Bluetooth components
README.txt    : This file
CHANGELOG.txt : A log of changes for each SDK revision
LICENSE.txt   : Licenses applicable to the SDK & IDE

 
Getting Started
---------------------------------------------------------------------
If you are unfamiliar with the WICED Smart SDK, please refer to the 
WICED Smart Quickstart Guide located here: <WICED Smart-SDK>/Doc/WICED Smart-QSG1xx-R.pdf [TBA]
The WICED Smart Quickstart Guide documents the process to setup a computer for
use with the WICED Smart SDK, IDE and WICED Smart Evaluation Board. 

The WICED Smart SDK includes lots of sample applications in the <WICED Smart-SDK>/Apps directory.
Applications included with the SDK are outlined below.
 Apps : Applications in the ROM, applications that extend the built in ROM apps and
    sample applications that are run from on-chip RAM, and may also access on-chip ROM functions
 
To obtain a complete list of build commands and options, enter the following text in the
base WICED Smart SDK directory on a command line:
$> make

To compile, download and run the automation_io application on the Broadcom BCM920737TAG_Q32 evaluation platform, 
enter the following text on a command line (a period character is used to reference applications 
in sub-directories) :
$> make automation_io-BCM920737TAG_Q32 download

Header files and reference information for supported platforms is available 
in the <WICED Smart-SDK>/platforms directory.
Platform implementations are available in the <WICED Smart-SDK>/Platform directory.


Supported Features
---------------------------------------------------------------------
Application Features
 * Peripheral interfaces
   * GPIO
   * Timer (Software)
   * PWM
   * UART (two instances - one for application download and another for application use).
   * SPI (two instances - one for serial flash and another for application use).
   * I2C (master only).
   * RTC (Real Time Clock)
   * Keyscan
   * ADC (12 bit)
 * Generic profile level abstraction API
 * API to access NV storage areas.

* WICED Smart Application Framework
   * OTA upgrade
   * Overlay support to load code from NV storage on demand (NV storage dependent latency and power).

Toolchains
 * GNU make
 * ARM RealView compiler toolchain and debugger

Hardware Platforms
 * BCM920737TAG_Q32  : Broadcom BCM20737 based evaluation Tag board.


Known Limitations & Notes
---------------------------------------------------------------------
  * SDK File Permissions
      In Linux, the SDK is extracted using the default permissions 
      for the current user. Users may wish to change the access permissions on the 
      SDK files. This can be done either on a one-time basis using ‘chmod –R’, or more 
      permanently for all user programs using the ‘umask’ command in a shell startup 
      script such as .bashrc or /etc/launchd-user.conf
         eg. At a prompt  : $WICED Smart-SDK> chmod -R g+w
         eg. In ~/.bashrc : umask u=rwx,g=rwx,o=
  * Programming and Debugging
      Programming is currently enabled with Broadcom download tools included with
      WICED Smart SDK. Debugging is enabled by ARM RealView with Serial Wire Debug
      interface. A future SDK update will include support for programming and debugging
      with OpenOCD, http://openocd.berlios.de.
  * Application download via USB-serial/serial port and application mode are mutually
      exclusive. The serial port must be disconnected from the board for the application
      to initialize
  * ARM RealView is not currently supported out of the box with the WICED Smart SDK.
    

Tools
---------------------------------------------------------------------
The GNU ARM toolchain is from Yagarto, http://yagarto.de

The SDK also supports ARM RealView 4.1 and above compiler toolchain, http://www.arm.com

The standard WICED Smart Evaluation board (BCM920737TAG_Q32) provides single USB-serial 
port for programming.

The debug interface is ARM Serial Wire Debug (SWD) and shares pins with download
serial lines TXd (SWDCLK) and RXd (SWDIO).

Building, programming and debugging of applications is achieved using either a 
command line interface or the WICED Smart IDE as described in the Quickstart Guide.

                     
WICED Technical Support
---------------------------------------------------------------------
WICED support is available on the Broadcom forum at http://community.broadcom.com
Access to the WICED forum is restricted to bona-fide WICED customers only.

Broadcom provides customer access to a wide range of additional information, including 
technical documentation, schematic diagrams, product bill of materials, PCB layout 
information, and software updates through its customer support portal. For a CSP account, 
contact your Broadcom Sales or Engineering support representative.

                     
Further Information
---------------------------------------------------------------------
Further information about WICED and the WICED Development System is
available on the WICED website at http://go.broadcom.com/wiced or
by e-mailing Broadcom support at support@broadcom.com
