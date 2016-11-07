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
 *	PancadariaStick main source file. Based on the LUFA Joystick demo.
 *  Modifications (C) 2016 André Luiz de Amorim, licensed under GPLv3.
 */

#include "Joystick.h"

/** Buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Joystick_HID_Interface =
	{
		.Config =
			{
				.InterfaceNumber              = INTERFACE_ID_Joystick,
				.ReportINEndpoint             =
					{
						.Address              = JOYSTICK_EPADDR,
						.Size                 = JOYSTICK_EPSIZE,
						.Banks                = 1,
					},
				.PrevReportINBuffer           = PrevJoystickHIDReportBuffer,
				.PrevReportINBufferSize       = sizeof(PrevJoystickHIDReportBuffer),
			},
	};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();

	GlobalInterruptEnable();

	for (;;)
	{
		HID_Device_USBTask(&Joystick_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	Input_Init();
	USB_Init();
}

/** Configure joystick and button pins. */
void Input_Init(void)
{
	DDRB |= (1 << PB0); // Output: LED attached to PB0
	DDRD |= (1 << PD5); // Output: LED attached to PD5

	/* Inputs with pullup. */
	DDRB &= ~MASK_INPUT_PORTB;
	PORTB |= MASK_INPUT_PORTB;
	DDRC &= ~MASK_INPUT_PORTC;
	PORTC |= MASK_INPUT_PORTC;
	DDRD &= ~MASK_INPUT_PORTD;
	PORTD |= MASK_INPUT_PORTD;
	DDRE &= ~MASK_INPUT_PORTE;
	PORTE |= MASK_INPUT_PORTE;
}

inline void LED1_on(void)
{
	PORTB &= ~(1 << PB0);
}

inline void LED1_off(void)
{
	PORTB |= (1 << PB0);
}

inline void LED2_on(void)
{
	PORTD &= ~(1 << PD5);
}

inline void LED2_off(void)
{
	PORTD |= (1 << PD5);
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Joystick_HID_Interface);

	USB_Device_EnableSOFEvents();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Joystick_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Joystick_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	USB_JoystickReport_Data_t* jsRep = (USB_JoystickReport_Data_t*)ReportData;

	if (AXIS_LEFT_PRESSED) {
		jsRep->X = 0;
	}
	else if (AXIS_RIGHT_PRESSED) {
		jsRep->X = 255;
	}
	else {
		jsRep->X = 128;
	}

	if (AXIS_DOWN_PRESSED) {
		jsRep->Y = 0;
	}
	else if (AXIS_UP_PRESSED) {
		jsRep->Y = 255;
	}
	else {
		jsRep->Y = 128;
	}

	jsRep->ButtonL = 0x00;
	jsRep->ButtonL |= (BUTTON_1_PRESSED << 0);
	jsRep->ButtonL |= (BUTTON_2_PRESSED << 1);
	jsRep->ButtonL |= (BUTTON_3_PRESSED << 2);
	jsRep->ButtonL |= (BUTTON_4_PRESSED << 3);
	jsRep->ButtonL |= (BUTTON_5_PRESSED << 4);
	jsRep->ButtonL |= (BUTTON_6_PRESSED << 5);
	jsRep->ButtonL |= (BUTTON_7_PRESSED << 6);
	jsRep->ButtonL |= (BUTTON_8_PRESSED << 7);

	jsRep->ButtonH = 0x00;
	jsRep->ButtonH |= (BUTTON_9_PRESSED << 0);
	jsRep->ButtonH |= (BUTTON_10_PRESSED << 1);

	if (jsRep->ButtonL | jsRep->ButtonH) {
		LED1_on();
	} else {
		LED1_off();
	}

	*ReportSize = sizeof(USB_JoystickReport_Data_t);
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	// Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

