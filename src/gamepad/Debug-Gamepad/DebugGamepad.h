#ifndef USB_GAMEPAD_H
#define USB_GAMEPAD_H

#include <WString.h>

#include "../common.h"

class DebugGamepad : public AbstractGamepad {
   public:
	DebugGamepad() : AbstractGamepad() {
	}

	void begin(void) {
		Serial.begin(115200);
		Serial.println("DebugGamepad.begin");
	}

	void setAxes(const uint8_t cIdx, int16_t x, int16_t y, int16_t z, int16_t rZ, char rX, char rY, signed char hat) {
		Serial.println("DebugGamepad.setAxes");
		AbstractGamepad::setAxes(cIdx, x, y, z, rZ, rX, rY, hat);
	}

	void sync(const uint8_t cIdx) {
		Serial.print("DebugGamepad.sync: ");
		Serial.println(cIdx);
	}
};

typedef DebugGamepad Gamepad;

#endif	// USB_GAMEPAD_H
