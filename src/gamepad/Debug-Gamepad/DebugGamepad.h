#ifndef USB_GAMEPAD_H
#define USB_GAMEPAD_H

#include <WString.h>

#ifndef GAMEPAD_CLASS
#define GAMEPAD_CLASS DebugGamepad
#endif

#include "../common.h"

void print_axis(int16_t i) {
	char buf[6];
	sprintf(buf, i < 0 ? "%05d" : "+%05d", i);
	Serial.print(buf);
}

void print_trigger(char i) {
	char buf[4] = {0};	// why the hell is this initializer required...
	// todo: probably not right
	sprintf(buf, "%03u", (uint8_t)i);
	Serial.print(buf);
}

class DebugGamepad : public AbstractGamepad {
   public:
	DebugGamepad() : AbstractGamepad() {
	}

	virtual void begin(void) {
		Serial.begin(115200);
		Serial.println("DebugGamepad.begin");
	}

	virtual void setAxis(const uint8_t cIdx, int16_t x, int16_t y, int16_t z, int16_t rZ, char rX, char rY, signed char hat) {
		Serial.print("DebugGamepad.setAxis ");
		Serial.print(cIdx);
		Serial.print(" b: ");
		Serial.print(this->isPressed(cIdx, BUTTON_A) ? "A" : "-");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_B) ? "B" : "-");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_X) ? "X" : "-");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_Y) ? "Y" : "-");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_SELECT) ? "SL" : "--");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_START) ? "ST" : "--");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_MENU) ? "M" : "-");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_TL) ? "TL" : "--");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_TR) ? "TR" : "--");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_TL2) ? "TL2" : "---");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_TR2) ? "TR2" : "---");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_THUMBL) ? "THL" : "---");
		Serial.print("+");
		Serial.print(this->isPressed(cIdx, BUTTON_THUMBR) ? "THR" : "---");
		Serial.print(" h: ");
		switch (hat) {
			case DPAD_CENTER:
				Serial.print("--");
				break;
			case DPAD_UP:
				Serial.print("U-");
				break;
			case DPAD_UP_RIGHT:
				Serial.print("UR");
				break;
			case DPAD_RIGHT:
				Serial.print("-R");
				break;
			case DPAD_DOWN_RIGHT:
				Serial.print("DR");
				break;
			case DPAD_DOWN:
				Serial.print("D-");
				break;
			case DPAD_DOWN_LEFT:
				Serial.print("DL");
				break;
			case DPAD_LEFT:
				Serial.print("-L");
				break;
			case DPAD_UP_LEFT:
				Serial.print("UL");
				break;
			default:
				Serial.print("++");
				break;
		}
		Serial.print(" X,Y: ");
		print_axis(x);
		Serial.print(",");
		print_axis(y);
		Serial.print(" Z,RZ: ");
		print_axis(z);
		Serial.print(",");
		print_axis(rZ);
		Serial.print(" rx: ");
		print_trigger(rX);
		Serial.print(" ry: ");
		print_trigger(rY);
		Serial.println();
		AbstractGamepad::setAxis(cIdx, x, y, z, rZ, rX, rY, hat);
	}

	virtual void sendHidReport(const uint8_t cIdx, const void* d, int len) {
		Serial.print("DebugGamepad.sendHidReport: ");
		Serial.println(cIdx);
	}
};

#endif	// USB_GAMEPAD_H
