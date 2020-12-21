#ifndef SWITCH_USB_GAMEPAD_H
#define SWITCH_USB_GAMEPAD_H

#ifndef GAMEPAD_CLASS
#define GAMEPAD_CLASS SwitchUsbGamepad
#endif

#include "../common.h"

#include "HID-Project.h"

class SwitchUsbGamepad : public AbstractGamepad {
   public:
	SwitchUsbGamepad() : AbstractGamepad() {
	}

	virtual void begin(void) {
		NSGamepad.begin();
	}

	virtual void end(void) {
		NSGamepad.end();
	}

	virtual void setAxis(const uint8_t cIdx, int16_t x, int16_t y, int16_t z, int16_t rZ, char rX, char rY, signed char hat) {
		// Move x/y Axis to a new position (16bit)
		NSGamepad.leftXAxis(x);
		NSGamepad.leftYAxis(y);
		NSGamepad.rightXAxis(z);
		NSGamepad.rightYAxis(rZ);
		NSGamepad.dPad(hat);

		this->sync(cIdx);
	}

	virtual void setHatSync(const uint8_t cIdx, signed char hat) {
		setAxis(cIdx, 0, 0, 0, 0, 0, 0, hat);
	}

	virtual void buttons(const uint8_t cIdx, uint32_t b) {
		NSGamepad.buttons(b);
	}

	virtual void press(const uint8_t cIdx, uint32_t b) {
		NSGamepad.press(b);
	}

	virtual void release(const uint8_t cIdx, uint32_t b) {
		NSGamepad.release(b);
	}

	virtual void sync(const uint8_t cIdx) {
		// todo: something to make this play nice with radio
		sendHidReport(cIdx, &gamepadReport, GAMEPAD_REPORT_LEN);
	}

	virtual void sendHidReport(const uint8_t cIdx, const void* d, int len) {
		//gamepad[cIdx].send(d, len);]]
		NSGamepad.write();
	}
};

#endif	// SWITCH_USB_GAMEPAD_H
