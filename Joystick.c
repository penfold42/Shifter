#include "Joystick.h"
#include "g27shifter.h"

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


/* yay for global variables....
 * use this as a global as ADC reads are expensive
 */
g27coordinates c;

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

void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	g27_initialize_io();
	USB_Init();
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
	USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

	uint16_t buttons = read_buttons();
	bool isShifterPressed = ((buttons & (1 << 1)) == (1<< 1)) & 1;

	bool isSequential = ((buttons & (1 << 3)) == (1<< 3)) & 1;

	uint8_t shifter = read_selected_gear(isShifterPressed, isSequential);

	if (buttons & 0x02) {
		LED_PORT &= ~(1 << LED_BIT);
	} else {
		LED_PORT |= (1 << LED_BIT);
	}

	if (shifter == shiftUp) {
		JoystickReport->Buttons[2] = 0x10;
		shifter = 0;
	} else if (shifter == shiftDown) {
		JoystickReport->Buttons[2] = 0x20;
		shifter = 0;
	} else if (shifter == neutral) {

	} else {
		shifter = 1 << (shifter-1);
	}

	uint8_t Red4Buttons = (buttons >> 4) & 0x0f;
	uint8_t Top4Buttons = (buttons >> 8) & 0x0f;
	uint8_t DPad4Buttons = (buttons >> 12) & 0x0f;

	JoystickReport->Buttons[0] = shifter | (isSequential * 0x80); // Sequential to top bit
	JoystickReport->Buttons[1] = (Red4Buttons<<4) | Top4Buttons;
	JoystickReport->Buttons[2] |= buttons & 0xf;

	// scale 10bit adc to 16bit joystick axes (and flip Y)
	JoystickReport->Xaxis  = (c.x-512)*(32768/512);
	JoystickReport->Yaxis  = (511-c.y)*(32768/512);

	JoystickReport->Clutch = (c.x-512)*(32768/512);
	JoystickReport->Brake  = (511-c.y)*(32768/512);

	// count the buttons pressed
	uint8_t bits = JoystickReport->Buttons[1];
	int cnt = 0;
	for (int i=0; i<8; i++) {
		if (bits & 0b00000001) cnt++;
		bits >>=1;
	}
	JoystickReport->Accel  = (cnt-4) * (65535/8);

	switch (DPad4Buttons) {
		case 0b1000:	// N
			JoystickReport->Hat = 1;
			break;
		case 0b1001:	// NE
			JoystickReport->Hat = 2;
			break;
		case 0b0001:	// E
			JoystickReport->Hat = 3;
			break;
		case 0b0101:	// SE
			JoystickReport->Hat = 4;
			break;
		case 0b0100:	// S
			JoystickReport->Hat = 5;
			break;
		case 0b0110:	// SW
			JoystickReport->Hat = 6;
			break;
		case 0b0010:	// W
			JoystickReport->Hat = 7;
			break;
		case 0b1010:	// NW
			JoystickReport->Hat = 8;
			break;
		default:
			JoystickReport->Hat = 0;
			break;
	}

	*ReportSize = sizeof(USB_JoystickReport_Data_t);
	return true;
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
}
