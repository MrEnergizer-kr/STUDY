INTRODUCTION
------------
The SD_USB_CID.c gets and displays CID register of the SD/MMC card through Microchip's USB2660 device.

For detailed description of the CID register and its vaues referer JEDEC Standard No.JESD84-B42

For specific values check the connected eMMC card datasheet/manual.

PACKAGE CONTAINS
----------------
This package contains the following files
	1. SD_USB_CID.c
	2. sd_usb.h
	3. sd_usb_win32.h
	4. README.txt


REQUIREMENTS
------------

This file needs the following libraries to be installed
	1.sgutils2

sgutils is a scsi driver which will be used to send the SCSI commands to the device.
It can be downloaded from sg.danny.cz/sg/sg3_utils.html

COMPILATION
-----------

Use the following command to compile the program

	gcc -g -Wall -o SD_USB_CID SD_USB_CID.c -lpthread -lsgutils2

REFERENCES
----------

	1.USB82642/2642 SDIO over USB User's Guide.pdf 

The above file can be downloaded from Microchips Website.



