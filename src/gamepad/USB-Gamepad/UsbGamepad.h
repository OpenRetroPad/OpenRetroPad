#ifndef USB_GAMEPAD_H
#define USB_GAMEPAD_H

#include <WString.h>

#include "../common.h"

#include "HID.h"

// ATT: 20 chars max (including NULL at the end) according to Arduino source code.
// Additionally serial number is used to differentiate arduino projects to have different button maps!
const char* gp_serial = "SF30 Pro";

class Gamepad_ : public PluggableUSBModule {
   private:
	uint8_t reportId;

   protected:
	uint8_t epType[1];
	uint8_t protocol;
	uint8_t idle;

	/*
    int getInterface(uint8_t* interfaceCount);
    int getDescriptor(USBSetup& setup);
    uint8_t getShortName(char *name);
    bool setup(USBSetup& setup);
    */

	int getInterface(uint8_t* interfaceCount) {
		*interfaceCount += 1;  // uses 1
		HIDDescriptor hidInterface = {D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
									  D_HIDREPORT(sizeof(_hidReportDescriptor)),
									  D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)};
		return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
	}

	int getDescriptor(USBSetup& setup) {
		// Check if this is a HID Class Descriptor request
		if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
			return 0;
		}
		if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
			return 0;
		}

		// In a HID Class Descriptor wIndex cointains the interface number
		if (setup.wIndex != pluggedInterface) {
			return 0;
		}

		// Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
		// due to the USB specs, but Windows and Linux just assumes its in report mode.
		protocol = HID_REPORT_PROTOCOL;

		return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
	}

	bool setup(USBSetup& setup) {
		if (pluggedInterface != setup.wIndex) {
			return false;
		}

		uint8_t request = setup.bRequest;
		uint8_t requestType = setup.bmRequestType;

		if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE) {
			if (request == HID_GET_REPORT) {
				// TODO: HID_GetReport();
				return true;
			}
			if (request == HID_GET_PROTOCOL) {
				// TODO: Send8(protocol);
				return true;
			}
		}

		if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE) {
			if (request == HID_SET_PROTOCOL) {
				protocol = setup.wValueL;
				return true;
			}
			if (request == HID_SET_IDLE) {
				idle = setup.wValueL;
				return true;
			}
			if (request == HID_SET_REPORT) {
			}
		}

		return false;
	}

	uint8_t getShortName(char* name) {
		if (!next) {
			strcpy(name, gp_serial);
			return strlen(name);
		}
		return 0;
	}

   public:
	Gamepad_(void) : PluggableUSBModule(1, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1) {
		epType[0] = EP_TYPE_INTERRUPT_IN;
		PluggableUSB().plug(this);
	}

	void send(const void* d, int len) {
		USB_Send(pluggedEndpoint | TRANSFER_RELEASE, d, len);
	}
};

class UsbGamepad : public AbstractGamepad {
   public:
	String deviceManufacturer;
	String deviceName;
	Gamepad_ gamepad[GAMEPAD_COUNT];

	UsbGamepad(String deviceName = "8BitDo SN30 Pro+", String deviceManufacturer = "8Bitdo SF30 Pro") : AbstractGamepad() {
		this->deviceName = deviceName;
		this->deviceManufacturer = deviceManufacturer;
	}

	void sync(const uint8_t cIdx) {
		gamepad[cIdx].send(&gamepadReport, GAMEPAD_REPORT_LEN);
	}
};

typedef UsbGamepad Gamepad;

#endif	// USB_GAMEPAD_H
