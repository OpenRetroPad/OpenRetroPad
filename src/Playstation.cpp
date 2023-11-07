/*
        LOOKING AT THE PLUG
        -------------------------------
 PIN 1->| o  o  o | o  o  o | o  o  o |
        \_____________________________/


PIN # USAGE (colors from my extension cable, check your own)

    DATA                 - brown
    CMD/COMMAND          - orange
    N/C (9 Volts unused) - white
    GND                  - black
    5V VCC               - red
    ATT                  - yellow
    CLK/CLOCK            - blue
    N/C                  - -
    ACK                  - green

*/

#include "Arduino.h"

#include "pins.h"

#define DATA1 OR_PIN_2
#define CMD1 OR_PIN_3
#define ATT1 OR_PIN_4
#define CLK1 OR_PIN_5

#ifdef BLUERETRO_MAPPING
#undef ATT1
#define ATT1 OR_PIN_10
#endif

#if defined(ARDUINO_ARCH_ESP32)

#define CTRL_BYTE_DELAY 18

#else

#define CTRL_BYTE_DELAY 6

#endif	// esp32 vs micro delay

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 4
#endif

// not counting dpad
#define BUTTON_COUNT 12

#define JOYSTICK_STATE_SIZE 6

#define AXIS_CENTER_IN 128
#define AXIS_MAX_IN 255
#define AXIS_MIN_IN 0

//#define DEBUG

#include "gamepad/Gamepad.h"
#include "util.cpp"

GAMEPAD_CLASS gamepad;

enum
{
	// in data[1]
	// in digital mode, right joysticks are these 4 buttons too
	PS_BTN_O = 8192,
	PS_BTN_X = 16384,
	PS_BTN_SQUARE = 32768,
	PS_BTN_TRIANGLE = 4096,
	// shoulder
	PS_BTN_R1 = 2048,
	PS_BTN_R2 = 512,
	PS_BTN_L1 = 1024,
	PS_BTN_L2 = 256,

	// in data[2]
	PS_BTN_START = 8,
	PS_BTN_SELECT = 1,

	// in digital mode, left joysticks are the d-pad
	PS_BTN_UP = 16,
	PS_BTN_DOWN = 64,
	PS_BTN_LEFT = 128,
	PS_BTN_RIGHT = 32,

	PS_BTN_R3 = 4,
	PS_BTN_L3 = 2,
};

// pressing one of these buttons on the controller... (read below)
static const uint16_t translateFromButton[BUTTON_COUNT] = {
	PS_BTN_O,
	PS_BTN_X,
	PS_BTN_SQUARE,
	PS_BTN_TRIANGLE,
	PS_BTN_R1,
	PS_BTN_R2,
	PS_BTN_L1,
	PS_BTN_L2,
	PS_BTN_START,
	PS_BTN_SELECT,
	PS_BTN_R3,
	PS_BTN_L3,
};

// ... translates to one of these buttons over HID
static const uint32_t translateToHid[BUTTON_COUNT] = {
	BUTTON_A,
	BUTTON_B,
	BUTTON_Y,
	BUTTON_X,
	BUTTON_R1,
	BUTTON_R2,
	BUTTON_L1,
	BUTTON_L2,
	BUTTON_START,
	BUTTON_SELECT,
	BUTTON_R3,
	BUTTON_L3,
};

class Joystick_ {
   private:
	uint8_t olddata[JOYSTICK_STATE_SIZE];

   public:
	uint8_t type;
	uint8_t data[JOYSTICK_STATE_SIZE];

	Joystick_() {
		data[0] = 0;
		data[1] = 0;
		data[2] = 127;
		data[3] = 127;
		data[4] = 127;
		data[5] = 127;
		memcpy(olddata, data, JOYSTICK_STATE_SIZE);
	}

	bool down(int button) const {
		return *(uint16_t*)(data)&button;
	}

	void updateState(uint8_t c) {
		if (type != 0x73 && type != 0x53) {
			data[2] = 127;
			data[3] = 127;
			data[4] = 127;
			data[5] = 127;
		}
		if (type == 0x41 || type == 0x73 || type == 0x53) {
			if (memcmp(olddata, data, JOYSTICK_STATE_SIZE)) {
				memcpy(olddata, data, JOYSTICK_STATE_SIZE);
				sendState(c);
			}
		}
	}

	signed char getHat() {
		if (down(PS_BTN_DOWN)) {
			if (down(PS_BTN_RIGHT)) {
				return DPAD_DOWN_RIGHT;
			} else if (down(PS_BTN_LEFT)) {
				return DPAD_DOWN_LEFT;
			} else {
				return DPAD_DOWN;
			}
		} else if (down(PS_BTN_UP)) {
			if (down(PS_BTN_RIGHT)) {
				return DPAD_UP_RIGHT;
			} else if (down(PS_BTN_LEFT)) {
				return DPAD_UP_LEFT;
			} else {
				return DPAD_UP;
			}
		} else if (down(PS_BTN_RIGHT)) {
			return DPAD_RIGHT;
		} else if (down(PS_BTN_LEFT)) {
			return DPAD_LEFT;
		} else {
			return DPAD_CENTERED;
		}
	}

#ifdef DEBUG
	void printBin(uint8_t c) {
		//Serial.print(c);
		uint8_t mask = 1;
		for (uint8_t _i = 0; _i <= 7; _i++) {
			Serial.print((c & mask) ? "1" : "0");
			mask *= 2;
		}
	}
#endif

	void sendState(uint8_t c) {
#ifdef DEBUG
		Serial.print(c);
		Serial.print(": type: 0x");
		Serial.print(type, HEX);
		Serial.print(" hex: 0x");
		Serial.print(data[0], HEX);
		Serial.print(" 0x");
		Serial.print(data[1], HEX);
		Serial.print(" 0x");
		Serial.print(data[2], HEX);
		Serial.print(" 0x");
		Serial.print(data[3], HEX);
		Serial.print(" 0x");
		Serial.print(data[4], HEX);
		Serial.print(" 0x");
		Serial.print(data[5], HEX);

		Serial.print(" dec: ");
		printBin(data[0]);
		Serial.print(" ");
		printBin(data[1]);
		Serial.print(" ");
		printBin(data[2]);
		Serial.print(" ");
		printBin(data[3]);
		Serial.print(" ");
		printBin(data[4]);
		Serial.print(" ");
		printBin(data[5]);
		Serial.println();

		uint8_t mask = 1;
		for (uint8_t _i = 0; _i <= 8; _i++) {
			if (data[0] & mask) {
				Serial.print("0 db: ");
				Serial.println(mask);
			}
			if (data[1] & mask) {
				Serial.print("1 db: ");
				Serial.println(mask);
			}
			mask *= 2;
		}

		uint16_t mask2 = 1;
		for (uint8_t _i = 0; _i <= 32; _i++) {
			if (down(mask2)) {
				Serial.print("db: ");
				Serial.println(mask2);
			}
			mask2 *= 2;
		}

		Serial.flush();
#endif

		//gamepad.buttons(i, *(uint16_t*)(&data[0]));
		//gamepad.setHatSync(i, DPAD_CENTERED);
		gamepad.buttons(c, 0);
		// if start and select are held at the same time, send menu and only menu
		if (down(PS_BTN_START) && down(PS_BTN_SELECT)) {
			gamepad.press(c, BUTTON_MENU);
		} else {
			// actually send buttons held
			for (uint8_t btn = 0; btn < BUTTON_COUNT; btn++) {
				if (down(translateFromButton[btn])) {
					gamepad.press(c, translateToHid[btn]);
				}
			}
		}

		gamepad.setAxis(c, translateAxis(data[4]), translateAxis(data[5]), translateAxis(data[2]), translateAxis(data[3]), 0, 0, getHat());
	}
};

Joystick_ Joystick[GAMEPAD_COUNT];

uint8_t shift(uint8_t _dataOut)	 // Does the actual shifting, both in and out simultaneously
{
	uint8_t _temp = 0;
	uint8_t _dataIn = 0;

	delayMicroseconds(100);	 //max acknowledge waiting time 100us
	for (uint8_t _i = 0; _i <= 7; _i++) {
		if (_dataOut & (1 << _i))  // write bit
			digitalWrite(CMD1, HIGH);
		else
			digitalWrite(CMD1, LOW);

		digitalWrite(CLK1, LOW);  // read bit
		delayMicroseconds(CTRL_BYTE_DELAY);
		_temp = digitalRead(DATA1);
		if (_temp) {
			_dataIn = _dataIn | (B00000001 << _i);
		}

		digitalWrite(CLK1, HIGH);
		delayMicroseconds(CTRL_BYTE_DELAY);
	}
	return _dataIn;
}

void setup() {
#ifdef DEBUG
	Serial.begin(115200);
#endif
	gamepad.begin();

	pinMode(DATA1, INPUT_PULLUP);
	pinMode(CMD1, OUTPUT);
	pinMode(ATT1, OUTPUT);
	pinMode(CLK1, OUTPUT);
}

void loop() {
	// http://problemkaputt.de/psx-spx.htm#controllerandmemorycardsignals
	uint8_t head, padding, multitap;

	// first: read gamepad normally
	digitalWrite(ATT1, LOW);
	//digitalWrite(ATT2, LOW);
	head = shift(0x01);
	Joystick[0].type = shift(0x42);
	padding = shift(0x01);				 //read multitap in next command
	Joystick[0].data[0] = ~shift(0x00);	 //buttons
	Joystick[0].data[1] = ~shift(0x00);	 //buttons
	Joystick[0].data[2] = shift(0x00);	 //right analog
	Joystick[0].data[3] = shift(0x00);	 //right analog
	Joystick[0].data[4] = shift(0x00);	 //left analog
	Joystick[0].data[5] = shift(0x00);	 //left analog
	digitalWrite(ATT1, HIGH);
	//digitalWrite(ATT2, HIGH);

	//delay(100);

	// second: check and read multitap
	digitalWrite(ATT1, LOW);
	head = shift(0x01);
	multitap = shift(0x42);
	padding = shift(0x00);	//next time normal read
	if (multitap == 0x80) {
		for (uint8_t i = 0; i < GAMEPAD_COUNT; i++) {
			Joystick[i].type = shift(0x00);
			padding = shift(0x00);
			Joystick[i].data[0] = ~shift(0x00);	 //buttons
			Joystick[i].data[1] = ~shift(0x00);	 //buttons
			Joystick[i].data[2] = shift(0x00);	 //right analog
			Joystick[i].data[3] = shift(0x00);	 //right analog
			Joystick[i].data[4] = shift(0x00);	 //left analog
			Joystick[i].data[5] = shift(0x00);	 //left analog
		}
	}
	digitalWrite(ATT1, HIGH);

#ifdef DEBUGE
	Serial.print(" multitap: ");
	Serial.println(multitap, HEX);
#endif

	for (uint8_t i = 0; i < GAMEPAD_COUNT; i++) {
		Joystick[i].updateState(i);
	}

	delayMicroseconds(500);	 // todo: proper value for this... does it depend on number of gamepads/multitap ?
}