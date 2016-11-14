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

#include <avr/io.h>
#include <avr/eeprom.h>
#include <util/delay.h>
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

/* Button mapping structures: */
#define NUM_BUTTONS 10
#define NUM_INPUT 14
const InputMap_t inputMap[NUM_INPUT] = { { MAP_PORTB, _BV(5) }, { MAP_PORTB, _BV(4) }, {
		MAP_PORTE, _BV(6) }, { MAP_PORTD, _BV(7) }, { MAP_PORTC, _BV(6) }, {
		MAP_PORTD, _BV(4) }, { MAP_PORTD, _BV(0) }, { MAP_PORTD, _BV(1) }, {
		MAP_PORTD, _BV(2) }, { MAP_PORTD, _BV(3) }, { MAP_PORTB, _BV(1) }, { MAP_PORTB, _BV(3) }, {
		MAP_PORTB, _BV(2) }, { MAP_PORTB, _BV(6) } };

uint8_t buttonOrder[NUM_BUTTONS];
uint8_t eeprom_buttonOrder[NUM_BUTTONS] EEMEM = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	MapInput();
	GlobalInterruptEnable();
	for (;;)
	{
		HID_Device_USBTask(&Joystick_HID_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
static void SetupHardware(void)
{
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);

	/* Hardware Initialization */
	InputInit();
	USB_Init();
}

static bool IsMapped(uint8_t button, uint8_t orderMap[])
{
	uint8_t i;
	for (i = 0; i < NUM_BUTTONS; ++i) {
		if (orderMap[i] == button) {
			return true;
		}
	}
	return false;
}

static void MapInput(void)
{
	uint8_t input, button, count;
	uint8_t newButtonOrder[NUM_BUTTONS];

	/* Default button mapping. */
	for (input = 0; input < NUM_BUTTONS; ++input) {
		buttonOrder[input] = input;
	}
	if (!InputPressed(8) || !InputPressed(9)) {
		/* No remapping, load previous map. */
		eeprom_read_block(buttonOrder, eeprom_buttonOrder, NUM_BUTTONS);
		return;
	}

	/* Will remap buttons. Initialize blank new map. */
	for (input = 0; input < NUM_BUTTONS; ++input) {
		newButtonOrder[input] = 0xFF;
	}
	/* Blink slowly to inform the user we are remapping. */
	for (count = 0; count < 20; ++count) {
		LED_toggle();
		_delay_ms(100);
	}
	input = 0;
	while (input < NUM_BUTTONS) {
		button = 0;
		count = 0;
		/*
		 * Cycle through inputs and see if it keeps pressed for
		 * one second. Can't map the same input to another button.
		 */
		while (count < 20) {
			++count;
			_delay_ms(50);
			if (InputPressed(button) && !IsMapped(button, newButtonOrder)) {
				LED_on();
			} else {
				LED_toggle();
				count = 0;
				++button;
				if (button >= NUM_BUTTONS) {
					button = 0;
				}
			}
		}
		/* Found the new input for this button. */
		LED_off();
		_delay_ms(1000);
		newButtonOrder[input] = button;
		++input;
	}
	/* Done remapping. Use the new map and save to eeprom. */
	LED_off();
	for (input = 0; input < NUM_BUTTONS; ++input) {
		buttonOrder[input] = newButtonOrder[input];
	}
	eeprom_write_block(buttonOrder, eeprom_buttonOrder, NUM_BUTTONS);
}

/** Configure joystick and button pins. */
static void InputInit(void)
{
	InputMap_t map;
	uint8_t i;

	DDRD |= (1 << PD5); // Output: LED attached to PD5

	/* Inputs with pullup. */
	for(i = 0; i < NUM_INPUT; ++i) {
		map = inputMap[i];
		switch (map.port) {
		case MAP_PORTB:
			DDRB &= ~map.mask;
			PORTB |= map.mask;
			break;
		case MAP_PORTC:
			DDRC &= ~map.mask;
			PORTC |= map.mask;
			break;
		case MAP_PORTD:
			DDRD &= ~map.mask;
			PORTD |= map.mask;
			break;
		case MAP_PORTE:
			DDRE &= ~map.mask;
			PORTE |= map.mask;
		}
	}
}

static bool InputPressed(uint8_t input) {
	InputMap_t map;
	if (input < NUM_BUTTONS) {
		input = buttonOrder[input];
	}
	map = inputMap[input];
	switch (map.port) {
	case MAP_PORTB:
		return ~PINB & map.mask;
	case MAP_PORTC:
		return ~PINC & map.mask;
	case MAP_PORTD:
		return ~PIND & map.mask;
	case MAP_PORTE:
		return ~PINE & map.mask;
	}
	return false;

}

static inline void LED_on(void)
{
	PORTD &= ~(1 << PD5);
}

static inline void LED_off(void)
{
	PORTD |= (1 << PD5);
}

static inline void LED_toggle(void)
{
	if (~PORTD & (1 << PD5)) {
		LED_off();
	} else {
		LED_on();
	}
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

	if (InputPressed(MAP_AXIS_LEFT)) {
		jsRep->X = 0;
	}
	else if (InputPressed(MAP_AXIS_RIGHT)) {
		jsRep->X = 255;
	}
	else {
		jsRep->X = 128;
	}

	if (InputPressed(MAP_AXIS_DOWN)) {
		jsRep->Y = 0;
	}
	else if (InputPressed(MAP_AXIS_UP)) {
		jsRep->Y = 255;
	}
	else {
		jsRep->Y = 128;
	}

	jsRep->ButtonL = 0x00;
	jsRep->ButtonL |= InputPressed(0) << 0;
	jsRep->ButtonL |= InputPressed(1) << 1;
	jsRep->ButtonL |= InputPressed(2) << 2;
	jsRep->ButtonL |= InputPressed(3) << 3;
	jsRep->ButtonL |= InputPressed(4) << 4;
	jsRep->ButtonL |= InputPressed(5) << 5;
	jsRep->ButtonL |= InputPressed(6) << 6;
	jsRep->ButtonL |= InputPressed(7) << 7;

	jsRep->ButtonH = 0x00;
	jsRep->ButtonH |= InputPressed(8) << 0;
	jsRep->ButtonH |= InputPressed(9) << 1;

	if (jsRep->ButtonL || jsRep->ButtonH) {
		LED_on();
	} else {
		LED_off();
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

