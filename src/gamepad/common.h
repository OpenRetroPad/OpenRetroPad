
#ifndef GAMEPAD_COMMON_H
#define GAMEPAD_COMMON_H

#include <Arduino.h>

#define ARDUINO_ARCH_ESP32 1
#include "HIDTypes.h"

#if GAMEPAD_OUTPUT == 5	 // nintendo switch

#define BUTTON_A NSButton_A
#define BUTTON_B NSButton_B
#define BUTTON_MENU NSButton_Home
#define BUTTON_X NSButton_X
#define BUTTON_Y NSButton_Y
#define BUTTON_TL NSButton_LeftTrigger
#define BUTTON_TR NSButton_RightTrigger
#define BUTTON_TL2 NSButton_LeftThrottle
#define BUTTON_TR2 NSButton_RightThrottle
#define BUTTON_SELECT NSButton_Minus
#define BUTTON_START NSButton_Plus
#define BUTTON_THUMBL NSButton_LeftStick
#define BUTTON_THUMBR NSButton_RightStick

#define DPAD_CENTER NSGAMEPAD_DPAD_CENTERED
#define DPAD_UP NSGAMEPAD_DPAD_UP
#define DPAD_UP_RIGHT NSGAMEPAD_DPAD_UP_RIGHT
#define DPAD_RIGHT NSGAMEPAD_DPAD_RIGHT
#define DPAD_DOWN_RIGHT NSGAMEPAD_DPAD_DOWN_RIGHT
#define DPAD_DOWN NSGAMEPAD_DPAD_DOWN
#define DPAD_DOWN_LEFT NSGAMEPAD_DPAD_DOWN_LEFT
#define DPAD_LEFT NSGAMEPAD_DPAD_LEFT
#define DPAD_UP_LEFT NSGAMEPAD_DPAD_UP_LEFT

#define AXIS_CENTER 128
#define AXIS_MAX 255
#define AXIS_MIN 0

#else

#define BUTTON_A 1
#define BUTTON_B 2
#define BUTTON_MENU 4
#define BUTTON_X 8
#define BUTTON_Y 16
#define BUTTON_TL 64
#define BUTTON_TR 128
#define BUTTON_TL2 256
#define BUTTON_TR2 512
#define BUTTON_SELECT 1024
#define BUTTON_START 2048
#define BUTTON_THUMBL 8192
#define BUTTON_THUMBR 16384

#define DPAD_CENTER 0
#define DPAD_UP 1
#define DPAD_UP_RIGHT 2
#define DPAD_RIGHT 3
#define DPAD_DOWN_RIGHT 4
#define DPAD_DOWN 5
#define DPAD_DOWN_LEFT 6
#define DPAD_LEFT 7
#define DPAD_UP_LEFT 8

#define AXIS_CENTER 0
#define AXIS_MAX 32768
#define AXIS_MIN -32767

#endif	// nintendo switch

// aliases
#define BUTTON_HOME BUTTON_MENU
#define BUTTON_L BUTTON_TL
#define BUTTON_L1 BUTTON_TL
#define BUTTON_R BUTTON_TR
#define BUTTON_R1 BUTTON_TR
#define BUTTON_L2 BUTTON_TL2
#define BUTTON_R2 BUTTON_TR2
#define BUTTON_L3 BUTTON_THUMBL
#define BUTTON_R3 BUTTON_THUMBR

#define DPAD_CENTERED DPAD_CENTER
// end aliases

#ifndef AXIS_CENTER_IN
#define AXIS_CENTER_IN 0
#endif
#ifndef AXIS_MAX_IN
#define AXIS_MAX_IN 32768
#endif
#ifndef AXIS_MIN_IN
#define AXIS_MIN_IN -32767
#endif

#if 0
#define BUTTON_1 1
#define BUTTON_2 2
#define BUTTON_3 4
#define BUTTON_4 8
#define BUTTON_5 16
#define BUTTON_6 32
#define BUTTON_7 64
#define BUTTON_8 128
#define BUTTON_9 256
#define BUTTON_10 512
#define BUTTON_11 1024
#define BUTTON_12 2048
#define BUTTON_13 4096
#define BUTTON_14 8192
#define BUTTON_15 16384
#define BUTTON_16 32768
#define BUTTON_17 65536
#define BUTTON_18 131072
#define BUTTON_19 262144
#define BUTTON_20 524288
#define BUTTON_21 1048576
#define BUTTON_22 2097152
#define BUTTON_23 4194304
#define BUTTON_24 8388608
#define BUTTON_25 16777216
#define BUTTON_26 33554432
#define BUTTON_27 67108864
#define BUTTON_28 134217728
#define BUTTON_29 268435456
#define BUTTON_30 536870912
#define BUTTON_31 1073741824
#define BUTTON_32 2147483648
#endif

#ifndef GAMEPAD_REPORT_ARRAY_ADD
// this is used by radio gamepad to send additional info
#define GAMEPAD_REPORT_ARRAY_ADD 0
#endif

// this must match _hidReportDescriptor, length to actually send
#define GAMEPAD_REPORT_LEN 15

static const uint8_t _hidReportDescriptor[] PROGMEM = {
	USAGE_PAGE(1),
	0x01,  // USAGE_PAGE (Generic Desktop)
	USAGE(1),
	0x05,  // USAGE (Gamepad)
	COLLECTION(1),
	0x01,  // COLLECTION (Application)
	USAGE(1),
	0x01,  //   USAGE (Pointer)
	COLLECTION(1),
	0x00,  //   COLLECTION (Physical)

	// ------------------------------------------------- Buttons (1 to 32)
	USAGE_PAGE(1),
	0x09,  //     USAGE_PAGE (Button)
	USAGE_MINIMUM(1),
	0x01,  //     USAGE_MINIMUM (Button 1)
	USAGE_MAXIMUM(1),
	0x20,  //     USAGE_MAXIMUM (Button 32)
	LOGICAL_MINIMUM(1),
	0x00,  //     LOGICAL_MINIMUM (0)
	LOGICAL_MAXIMUM(1),
	0x01,  //     LOGICAL_MAXIMUM (1)
	REPORT_SIZE(1),
	0x01,  //     REPORT_SIZE (1)
	REPORT_COUNT(1),
	0x20,  //     REPORT_COUNT (32)
	HIDINPUT(1),
	0x02,  //     INPUT (Data, Variable, Absolute) ;32 button bits
	// ------------------------------------------------- X/Y position, Z/rZ position
	USAGE_PAGE(1),
	0x01,  //		USAGE_PAGE (Generic Desktop)
	COLLECTION(1),
	0x00,  //		COLLECTION (Physical)
	USAGE(1),
	0x30,  //     USAGE (X)
	USAGE(1),
	0x31,  //     USAGE (Y)
	USAGE(1),
	0x32,  //     USAGE (Z)
	USAGE(1),
	0x35,  //     USAGE (rZ)
	0x16,
	0x01,
	0x80,  //LOGICAL_MINIMUM (-32767)
	0x26,
	0xFF,
	0x7F,  //LOGICAL_MAXIMUM (32767)
	REPORT_SIZE(1),
	0x10,  //		REPORT_SIZE (16)
	REPORT_COUNT(1),
	0x04,  //		REPORT_COUNT (4)
	HIDINPUT(1),
	0x02,  //     INPUT (Data,Var,Abs)
	// ------------------------------------------------- Triggers
	USAGE(1),
	0x33,  //     USAGE (rX) Left Trigger
	USAGE(1),
	0x34,  //     USAGE (rY) Right Trigger
	LOGICAL_MINIMUM(1),
	0x81,  //     LOGICAL_MINIMUM (-127)
	LOGICAL_MAXIMUM(1),
	0x7f,  //     LOGICAL_MAXIMUM (127)
	REPORT_SIZE(1),
	0x08,  //     REPORT_SIZE (8)
	REPORT_COUNT(1),
	0x02,  //     REPORT_COUNT (2)
	HIDINPUT(1),
	0x02,				//     INPUT (Data, Variable, Absolute) ;4 bytes (X,Y,Z,rZ)
	END_COLLECTION(0),	//     END_COLLECTION

	USAGE_PAGE(1),
	0x01,  //     USAGE_PAGE (Generic Desktop)
	USAGE(1),
	0x39,  //     USAGE (Hat switch)
	USAGE(1),
	0x39,  //     USAGE (Hat switch)
	LOGICAL_MINIMUM(1),
	0x01,  //     LOGICAL_MINIMUM (1)
	LOGICAL_MAXIMUM(1),
	0x08,  //     LOGICAL_MAXIMUM (8)
	REPORT_SIZE(1),
	0x04,  //     REPORT_SIZE (4)
	REPORT_COUNT(1),
	0x02,  //     REPORT_COUNT (2)
	HIDINPUT(1),
	0x02,  //     INPUT (Data, Variable, Absolute) ;1 byte Hat1, Hat2

	END_COLLECTION(0),	//     END_COLLECTION
	END_COLLECTION(0)	//     END_COLLECTION
};

class AbstractGamepad {
   public:
	uint32_t _buttons[GAMEPAD_COUNT];
	uint8_t gamepadReport[GAMEPAD_REPORT_LEN + GAMEPAD_REPORT_ARRAY_ADD];

	AbstractGamepad() {
	}

	virtual void begin(void) {
	}

	virtual void end(void) {
	}

	virtual bool isConnected(void) {
		return true;
	}

	virtual void setAxis(const uint8_t cIdx, int16_t x, int16_t y, int16_t z, int16_t rZ, char rX, char rY, signed char hat) {
		if (x == -32768) {
			x = -32767;
		}
		if (y == -32768) {
			y = -32767;
		}
		if (z == -32768) {
			z = -32767;
		}
		if (rZ == -32768) {
			rZ = -32767;
		}

		gamepadReport[0] = _buttons[cIdx];
		gamepadReport[1] = (_buttons[cIdx] >> 8);
		gamepadReport[2] = (_buttons[cIdx] >> 16);
		gamepadReport[3] = (_buttons[cIdx] >> 24);
		gamepadReport[4] = x;
		gamepadReport[5] = (x >> 8);
		gamepadReport[6] = y;
		gamepadReport[7] = (y >> 8);
		gamepadReport[8] = z;
		gamepadReport[9] = (z >> 8);
		gamepadReport[10] = rZ;
		gamepadReport[11] = (rZ >> 8);
		gamepadReport[12] = (signed char)(rX - 128);
		gamepadReport[13] = (signed char)(rY - 128);
		gamepadReport[14] = hat;
		if (gamepadReport[12] == -128) {
			gamepadReport[12] = -127;
		}
		if (gamepadReport[13] == -128) {
			gamepadReport[13] = -127;
		}

		this->sync(cIdx);
	}

	virtual void setHatSync(const uint8_t cIdx, signed char hat) {
		setAxis(cIdx, AXIS_CENTER, AXIS_CENTER, AXIS_CENTER, AXIS_CENTER, 0, 0, hat);
	}

	virtual void buttons(const uint8_t cIdx, uint32_t b) {
		_buttons[cIdx] = b;
	}

	virtual void press(const uint8_t cIdx, uint32_t b) {
		buttons(cIdx, _buttons[cIdx] | b);
	}

	virtual void release(const uint8_t cIdx, uint32_t b) {
		buttons(cIdx, _buttons[cIdx] & ~b);
	}

	virtual bool isPressed(const uint8_t cIdx, uint32_t b) {
		return ((b & _buttons[cIdx]) > 0);
	}

	virtual void sync(const uint8_t cIdx) {
		sendHidReport(cIdx, &gamepadReport, GAMEPAD_REPORT_LEN);
	}

	virtual void sendHidReport(const uint8_t cIdx, const void* d, int len);	 // actually sends report

	virtual ~AbstractGamepad() {
	}
};

#endif	// GAMEPAD_COMMON_H
