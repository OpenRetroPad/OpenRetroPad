/*
Genesis/Atari/Megadrive:
      LOOKING AT THE PLUG ON FRONT OF CONSOLE (not coming from controller)

      1   2   3   4   5
   -----------------------
   \  o   o   o   o   o  /
    \   o   o   o   o   /
     \-----------------/
        6   7   8   9

PIN # USAGE

  5: 5V VCC
  8: GND
*/
#include "Arduino.h"

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 2
#endif

#define BUTTON_COUNT 9
#define PIN_COUNT 6

#include "gamepad/Gamepad.h"

enum
{
	// this is the button mapping, change as you wish
	SC_BTN_A = BUTTON_Y,
	SC_BTN_B = BUTTON_B,
	SC_BTN_C = BUTTON_A,
	SC_BTN_X = BUTTON_L,
	SC_BTN_Y = BUTTON_X,
	SC_BTN_Z = BUTTON_R,
	SC_BTN_START = BUTTON_START,
	SC_BTN_MODE = BUTTON_SELECT,
	SC_BTN_HOME = BUTTON_MENU,
	SC_BTN_UP = 1,
	SC_BTN_DOWN = 2,
	SC_BTN_LEFT = 4,
	SC_BTN_RIGHT = 8,
};

#include "pins.h"

static const int DATA_PIN_SELECT[GAMEPAD_COUNT] = {
	OR_PIN_5,
#if GAMEPAD_COUNT > 1
	OR_PIN_7,
#endif
};

#if CODE_PLATFORM == 2

#define OPT_PIN_READ1(X) (bitRead(reg1, DATA_PIN[c][X]))
#define OPT_PIN_READ2(X) (bitRead(reg2, DATA_PIN[c][X]))

//individual data pin BIT for each controller, they are read in bulk
static const int DATA_PIN[GAMEPAD_COUNT][PIN_COUNT] = {
	{3, 2, 1, 0, 4, 7},
#if GAMEPAD_COUNT > 1
	{7, 6, 5, 4, 3, 1},
#endif
};

#else

#define OPT_PIN_READ1(X) (digitalRead(DATA_PIN[c][X]))
#define OPT_PIN_READ2(X) (digitalRead(DATA_PIN[c][X]))

//individual data pin for each controller
static const int DATA_PIN[GAMEPAD_COUNT][PIN_COUNT] = {
	{OR_PIN_1, OR_PIN_11, OR_PIN_2, OR_PIN_3, OR_PIN_10, OR_PIN_26},
#if GAMEPAD_COUNT > 1
	{OR_PIN_18, OR_PIN_19, OR_PIN_20, OR_PIN_21, OR_PIN_28, OR_PIN_29},
#endif
};

#endif	// CODE_PLATFORM

const byte SC_CYCLE_DELAY = 10;	 // Delay (µs) between setting the select pin and reading the button pins

class SegaControllers32U4 {
   public:
	SegaControllers32U4(void);
	void readState();
	byte currentDpadState[GAMEPAD_COUNT];
	word currentState[GAMEPAD_COUNT];
	// Controller previous states
	word lastState[GAMEPAD_COUNT];
	byte lastDpadState[GAMEPAD_COUNT];

	void poll(void (*controllerChanged)(const int c)) {
		readState();
		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			if (currentState[c] != lastState[c] || currentDpadState[c] != lastDpadState[c]) {
				controllerChanged(c);
				lastState[c] = currentState[c];
				lastDpadState[c] = currentDpadState[c];
			}
		}
	}

	bool down(int controller, int button) const {
		return currentDpadState[controller] & button;
	}

	bool dpad(int controller, int button) const {
		return currentDpadState[controller] & button;
	}

#ifdef DEBUG
	void printState(byte gp) {
		auto cs = currentState[gp];
		Serial.print(_connected[gp] ? "+" : "-");
		Serial.print(_sixButtonMode[gp] ? "6" : "3");
		Serial.print((cs & SC_BTN_UP) ? "U" : "0");
		Serial.print((cs & SC_BTN_DOWN) ? "D" : "0");
		Serial.print((cs & SC_BTN_LEFT) ? "L" : "0");
		Serial.print((cs & SC_BTN_RIGHT) ? "R" : "0");
		Serial.print((cs & SC_BTN_START) ? "S" : "0");
		Serial.print((cs & SC_BTN_A) ? "A" : "0");
		Serial.print((cs & SC_BTN_B) ? "B" : "0");
		Serial.print((cs & SC_BTN_C) ? "C" : "0");
		Serial.print((cs & SC_BTN_X) ? "X" : "0");
		Serial.print((cs & SC_BTN_Y) ? "Y" : "0");
		Serial.print((cs & SC_BTN_Z) ? "Z" : "0");
		Serial.print((cs & SC_BTN_MODE) ? "M" : "0");
		Serial.print((cs & SC_BTN_HOME) ? "H" : "0");
		Serial.println(" sending");
	}
#endif

   private:
	boolean _pinSelect;

	byte _ignoreCycles[GAMEPAD_COUNT];

	boolean _connected[GAMEPAD_COUNT];
	boolean _sixButtonMode[GAMEPAD_COUNT];

#if CODE_PLATFORM == 2
	void readPort(byte c, byte reg1, byte reg2);
	byte _inputReg3;
#if GAMEPAD_COUNT > 1
	byte _inputReg1;
	byte _inputReg2;
#endif	// GAMEPAD_COUNT
#else
	void readPort(byte c);
#endif	// CODE_PLATFORM
};

SegaControllers32U4::SegaControllers32U4(void) {
	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		// Setup output pin
		pinMode(DATA_PIN_SELECT[c], OUTPUT);
		digitalWrite(DATA_PIN_SELECT[c], HIGH);

		// Setup input pins
		for (byte i = 0; i < PIN_COUNT; i++) {
			pinMode(DATA_PIN[c][i], INPUT_PULLUP);
		}
	}

	_pinSelect = true;
	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		currentState[c] = 0;
		lastState[c] = 0;
		currentDpadState[c] = 0;
		lastDpadState[c] = 0;
		_connected[c] = 0;
		_sixButtonMode[c] = false;
		_ignoreCycles[c] = 0;
	}
}

#if CODE_PLATFORM == 2

void SegaControllers32U4::readState() {
	// Set the select pins low/high
	_pinSelect = !_pinSelect;
	if (!_pinSelect) {
		PORTE &= ~B01000000;
		PORTC &= ~B01000000;
	} else {
		PORTE |= B01000000;
		PORTC |= B01000000;
	}

	// Short delay to stabilise outputs in controller
	delayMicroseconds(SC_CYCLE_DELAY);

	// Read all input registers
	_inputReg3 = PIND;
#if GAMEPAD_COUNT > 1
	_inputReg1 = PINF;
	_inputReg2 = PINB;
#endif

	readPort(0, _inputReg3, _inputReg3);
#if GAMEPAD_COUNT > 1
	readPort(1, _inputReg1, _inputReg2);
#endif
}

#else

void SegaControllers32U4::readState() {
	// Set the select pins low/high
	_pinSelect = !_pinSelect;
	if (!_pinSelect) {
		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			digitalWrite(DATA_PIN_SELECT[c], LOW);
		}
	} else {
		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			digitalWrite(DATA_PIN_SELECT[c], HIGH);
		}
	}

	// Short delay to stabilise outputs in controller
	delayMicroseconds(SC_CYCLE_DELAY);

	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		readPort(c);
	}
}

#endif

// "Normal" Six button controller reading routine, done a bit differently in this project
// Cycle  TH out  TR in  TL in  D3 in  D2 in  D1 in  D0 in
// 0      LO      Start  A      0      0      Down   Up
// 1      HI      C      B      Right  Left   Down   Up
// 2      LO      Start  A      0      0      Down   Up      (Check connected and read Start and A in this cycle)
// 3      HI      C      B      Right  Left   Down   Up      (Read B, C and directions in this cycle)
// 4      LO      Start  A      0      0      0      0       (Check for six button controller in this cycle)
// 5      HI      C      B      Mode   X      Y      Z       (Read X,Y,Z and Mode in this cycle)
// 6      LO      ---    ---    ---    ---    ---    Home    (Home only for 8bitdo wireless gamepads)
// 7      HI      ---    ---    ---    ---    ---    ---
void SegaControllers32U4::readPort(byte c
#if CODE_PLATFORM == 2
								   ,
								   byte reg1,
								   byte reg2
#endif
) {
	if (_ignoreCycles[c] <= 0) {
		if (_pinSelect)	 // Select pin is HIGH
		{
			if (_connected[c]) {
				// Check if six button mode is active
				if (_sixButtonMode[c]) {
					// Read input pins for X, Y, Z, Mode
					(OPT_PIN_READ1(0) == LOW) ? currentState[c] |= SC_BTN_Z : currentState[c] &= ~SC_BTN_Z;
					(OPT_PIN_READ1(1) == LOW) ? currentState[c] |= SC_BTN_Y : currentState[c] &= ~SC_BTN_Y;
					(OPT_PIN_READ1(2) == LOW) ? currentState[c] |= SC_BTN_X : currentState[c] &= ~SC_BTN_X;
					(OPT_PIN_READ1(3) == LOW) ? currentState[c] |= SC_BTN_MODE : currentState[c] &= ~SC_BTN_MODE;
					_sixButtonMode[c] = false;
					_ignoreCycles[c] = 2;  // Ignore the two next cycles (cycles 6 and 7 in table above)
				} else {
					// Read input pins for Up, Down, Left, Right, B, C
					(OPT_PIN_READ1(0) == LOW) ? currentDpadState[c] |= SC_BTN_UP : currentDpadState[c] &= ~SC_BTN_UP;
					(OPT_PIN_READ1(1) == LOW) ? currentDpadState[c] |= SC_BTN_DOWN : currentDpadState[c] &= ~SC_BTN_DOWN;
					(OPT_PIN_READ1(2) == LOW) ? currentDpadState[c] |= SC_BTN_LEFT : currentDpadState[c] &= ~SC_BTN_LEFT;
					(OPT_PIN_READ1(3) == LOW) ? currentDpadState[c] |= SC_BTN_RIGHT : currentDpadState[c] &= ~SC_BTN_RIGHT;
					(OPT_PIN_READ2(4) == LOW) ? currentState[c] |= SC_BTN_B : currentState[c] &= ~SC_BTN_B;
					(OPT_PIN_READ2(5) == LOW) ? currentState[c] |= SC_BTN_C : currentState[c] &= ~SC_BTN_C;
				}
			} else	// No Mega Drive controller is connected, use SMS/Atari mode
			{
				// Clear current state
				currentState[c] = currentDpadState[c] = 0;

				// Read input pins for Up, Down, Left, Right, Fire1, Fire2
				if (OPT_PIN_READ1(0) == LOW) {
					currentDpadState[c] |= SC_BTN_UP;
				}
				if (OPT_PIN_READ1(1) == LOW) {
					currentDpadState[c] |= SC_BTN_DOWN;
				}
				if (OPT_PIN_READ1(2) == LOW) {
					currentDpadState[c] |= SC_BTN_LEFT;
				}
				if (OPT_PIN_READ1(3) == LOW) {
					currentDpadState[c] |= SC_BTN_RIGHT;
				}
				if (OPT_PIN_READ2(4) == LOW) {
					currentState[c] |= SC_BTN_A;
				}
				if (OPT_PIN_READ2(5) == LOW) {
					currentState[c] |= SC_BTN_B;
				}
			}
		} else	// Select pin is LOW
		{
			// Check if a controller is connected
			_connected[c] = OPT_PIN_READ1(2) == LOW && OPT_PIN_READ1(3) == LOW;

			// Check for six button mode
			_sixButtonMode[c] = OPT_PIN_READ1(0) == LOW && OPT_PIN_READ1(1) == LOW;

			// Read input pins for A and Start
			if (_connected[c]) {
				if (!_sixButtonMode[c]) {
					(OPT_PIN_READ2(4) == LOW) ? currentState[c] |= SC_BTN_A : currentState[c] &= ~SC_BTN_A;
					(OPT_PIN_READ2(5) == LOW) ? currentState[c] |= SC_BTN_START : currentState[c] &= ~SC_BTN_START;
				}
			}
		}
	} else {
		if (_ignoreCycles[c]-- == 2)  // Decrease the ignore cycles counter and read 8bitdo home in first "ignored" cycle, this cycle is unused on normal 6-button controllers
		{
			(OPT_PIN_READ1(0) == LOW) ? currentState[c] |= SC_BTN_HOME : currentState[c] &= ~SC_BTN_HOME;
		}
	}
}

SegaControllers32U4 controllers;

GAMEPAD_CLASS gamepad;

void controllerChanged(const int c) {
#ifdef DEBUG
	controllers.printState(c);
#endif

	// if start and down are held at the same time, send menu and only menu
	gamepad.buttons(c, controllers.currentState[c]);
	if (controllers.down(c, SC_BTN_START) && controllers.dpad(c, SC_BTN_DOWN)) {
		gamepad.buttons(c, 0);
		gamepad.press(c, BUTTON_MENU);
		gamepad.setHatSync(c, DPAD_CENTERED);
		return;
	}
	if (controllers.dpad(c, SC_BTN_DOWN)) {
		if (controllers.dpad(c, SC_BTN_RIGHT)) {
			gamepad.setHatSync(c, DPAD_DOWN_RIGHT);
		} else if (controllers.dpad(c, SC_BTN_LEFT)) {
			gamepad.setHatSync(c, DPAD_DOWN_LEFT);
		} else {
			gamepad.setHatSync(c, DPAD_DOWN);
		}
	} else if (controllers.dpad(c, SC_BTN_UP)) {
		if (controllers.dpad(c, SC_BTN_RIGHT)) {
			gamepad.setHatSync(c, DPAD_UP_RIGHT);
		} else if (controllers.dpad(c, SC_BTN_LEFT)) {
			gamepad.setHatSync(c, DPAD_UP_LEFT);
		} else {
			gamepad.setHatSync(c, DPAD_UP);
		}
	} else if (controllers.dpad(c, SC_BTN_RIGHT)) {
		gamepad.setHatSync(c, DPAD_RIGHT);
	} else if (controllers.dpad(c, SC_BTN_LEFT)) {
		gamepad.setHatSync(c, DPAD_LEFT);
	} else {
		gamepad.setHatSync(c, DPAD_CENTERED);
	}
}

void setup() {
#ifdef DEBUG
	Serial.begin(115200);
#endif
	gamepad.begin();
}

void loop() {
	if (gamepad.isConnected()) {
		controllers.poll(controllerChanged);
	}
}
