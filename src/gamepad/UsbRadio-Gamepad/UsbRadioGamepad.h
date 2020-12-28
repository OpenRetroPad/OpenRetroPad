#ifndef RADIO_USB_GAMEPAD_H
#define RADIO_USB_GAMEPAD_H

// number of button presses we wait for USB-HID to connect
// works within 1 for me but who knows...
#ifndef GAMEPAD_USBRADIO_DETECT_BUTTONS
#define GAMEPAD_USBRADIO_DETECT_BUTTONS 4
#endif

#ifndef GAMEPAD_CLASS
#define GAMEPAD_CLASS UsbRadioGamepad
#endif

#include "../Radio-Gamepad/RadioGamepad.h"
#include "../USB-Gamepad/UsbGamepad.h"

class UsbRadioGamepad : public AbstractGamepad {
   public:
	uint8_t checkCount = 0;

	AbstractGamepad* gamepad = new UsbGamepad();

	void (UsbRadioGamepad::*syncFunc)(const uint8_t cIdx, const void* d, int len) = &UsbRadioGamepad::syncDetectUsb;

	UsbRadioGamepad() : AbstractGamepad() {
	}

	virtual void begin(void) {
		gamepad->begin();
	}

	void syncDetectUsb(const uint8_t cIdx, const void* d, int len) {
		if (++checkCount <= GAMEPAD_USBRADIO_DETECT_BUTTONS) {
			if (gamepad->isConnected()) {
				// we are connected to PC/phone/usb-hid-capable-device, stop checking
				syncFunc = &UsbRadioGamepad::syncNoDetect;
			} else if (checkCount == GAMEPAD_USBRADIO_DETECT_BUTTONS) {
				// we assume we are connected to a power brick or power supply, and switch to radio
				delete gamepad;
				gamepad = new RadioGamepad();
				gamepad->begin();
				// stop trying to detect, it is what it is...
				syncFunc = &UsbRadioGamepad::syncNoDetect;
			}
		}
		gamepad->sendHidReport(cIdx, &gamepadReport, GAMEPAD_REPORT_LEN);
	}

	void syncNoDetect(const uint8_t cIdx, const void* d, int len) {
		gamepad->sendHidReport(cIdx, &gamepadReport, GAMEPAD_REPORT_LEN);
	}

	virtual void sendHidReport(const uint8_t cIdx, const void* d, int len) {
		(*this.*syncFunc)(cIdx, &gamepadReport, GAMEPAD_REPORT_LEN);
	}
};

#endif	// RADIO_GAMEPAD_H
