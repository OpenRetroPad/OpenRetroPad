#ifndef USB_GAMEPAD_H
#define USB_GAMEPAD_H

#include <WString.h>

#ifndef GAMEPAD_CLASS
#define GAMEPAD_CLASS DebugGamepad
#endif

#include "../common.h"

class DebugGamepad : public AbstractGamepad {
   public:
	DebugGamepad() : AbstractGamepad() {
	}

	virtual void begin(void) {
		Serial.begin(115200);
		Serial.println("DebugGamepad.begin");
	}

	virtual void setAxes(const uint8_t cIdx, int16_t x, int16_t y, int16_t z, int16_t rZ, char rX, char rY, signed char hat) {
		Serial.println("DebugGamepad.setAxes");
		AbstractGamepad::setAxes(cIdx, x, y, z, rZ, rX, rY, hat);
	}

	virtual void sendHidReport(const uint8_t cIdx, const void* d, int len) {
		Serial.print("DebugGamepad.sendHidReport: ");
		Serial.println(cIdx);
	}
};

#endif	// USB_GAMEPAD_H
