/*
             LUFA Library
     Copyright (C) Dean Camera, 2015.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2015  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Header file for Joystick.c.
 */

#ifndef _JOYSTICK_H_
#define _JOYSTICK_H_

	/* Includes: */

		#include <avr/io.h>
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <string.h>

		#include "LUFA/Drivers/USB/USB.h"
		#include "Descriptors.h"


/* Type Defines: */
		/** Type define for the joystick HID report structure, for creating and sending HID reports to the host PC.
		 *  This mirrors the layout described to the host in the HID report descriptor, in Descriptors.c.
		 *
\		 *     B08 B07 B06 B05 B04 B03 B02 B01 .... Two bytes with buttons plus padding.
		 *       .   .   .   .   .   . B10 B09
		 *      X7  X6  X5  X4  X3  X2  X1  X0 .... 8 bit signed relative coordinate x.
		 *      Y7  Y6  Y5  Y4  Y3  Y2  Y1  Y0 .... 8 bit signed relative coordinate y.
		 *
		 **/
		typedef struct
		{
			uint8_t ButtonL; /**< Bit mask of the currently pressed joystick buttons */
			uint8_t ButtonH; /**< Bit mask of the currently pressed joystick buttons */
			int8_t  X; /**< Current absolute joystick X position, as a signed 8-bit integer */
			int8_t  Y; /**< Current absolute joystick Y position, as a signed 8-bit integer */
		} USB_JoystickReport_Data_t;


	/* Function Prototypes: */
		void SetupHardware(void);
		void Input_Init(void);
		void testInput(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);

		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
		                                         const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);

#endif

