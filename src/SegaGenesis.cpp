
#include "Arduino.h"

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 2
#endif

#define BUTTON_COUNT 9

#include "gamepad/Gamepad.h"

enum
{
	SC_BTN_UP = 1,
	SC_BTN_DOWN = 2,
	SC_BTN_LEFT = 4,
	SC_BTN_RIGHT = 8,
	SC_BTN_A = 16,
	SC_BTN_B = 32,
	SC_BTN_C = 64,
	SC_BTN_X = 128,
	SC_BTN_Y = 256,
	SC_BTN_Z = 512,
	SC_BTN_START = 1024,
	SC_BTN_MODE = 2048,
	SC_BTN_HOME = 4096,
	SC_BIT_SH_UP = 0,
	SC_BIT_SH_DOWN = 1,
	SC_BIT_SH_LEFT = 2,
	SC_BIT_SH_RIGHT = 3,
	SC_PIN1_BIT = 0,
	SC_PIN2_BIT = 1,
	SC_PIN3_BIT = 2,
	SC_PIN4_BIT = 3,
	SC_PIN6_BIT = 4,
	SC_PIN9_BIT = 5,
	DB9_PIN1_BIT1 = 7,
	DB9_PIN2_BIT1 = 6,
	DB9_PIN3_BIT1 = 5,
	DB9_PIN4_BIT1 = 4,
	DB9_PIN6_BIT1 = 3,
	DB9_PIN9_BIT1 = 1,
	DB9_PIN1_BIT2 = 3,
	DB9_PIN2_BIT2 = 2,
	DB9_PIN3_BIT2 = 1,
	DB9_PIN4_BIT2 = 0,
	DB9_PIN6_BIT2 = 4,
	DB9_PIN9_BIT2 = 7
};

//individual data pin for each controller
static const int DATA_PIN[GAMEPAD_COUNT][6] = {
	{DB9_PIN1_BIT1, DB9_PIN2_BIT1, DB9_PIN3_BIT1, DB9_PIN4_BIT1, DB9_PIN6_BIT1, DB9_PIN9_BIT1},
#if GAMEPAD_COUNT > 1
	{DB9_PIN1_BIT2, DB9_PIN2_BIT2, DB9_PIN3_BIT2, DB9_PIN4_BIT2, DB9_PIN6_BIT2, DB9_PIN9_BIT2},
#endif
};

// pressing one of these buttons on the controller... (read below)
static const int translateFromButton[BUTTON_COUNT] = {
	SC_BTN_A,
	SC_BTN_B,
	SC_BTN_C,
	SC_BTN_X,
	SC_BTN_Y,
	SC_BTN_Z,
	SC_BTN_START,
	SC_BTN_MODE,
	SC_BTN_HOME,
};

// ... translates to one of these buttons over HID
static const int translateToHid[BUTTON_COUNT] = {
	BUTTON_Y,
	BUTTON_B,
	BUTTON_A,
	BUTTON_L,
	BUTTON_X,
	BUTTON_R,
	BUTTON_START,
	BUTTON_SELECT,
	BUTTON_MENU,
};

const byte SC_CYCLE_DELAY = 10;	 // Delay (µs) between setting the select pin and reading the button pins

class SegaControllers32U4 {
   public:
	SegaControllers32U4(void);
	void readState();
	word currentState[GAMEPAD_COUNT];
	// Controller previous states
	word lastState[GAMEPAD_COUNT];

	void poll(void (*controllerChanged)(const int controller)) {
		readState();
		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			if (currentState[c] != lastState[c]) {
				controllerChanged(c);
				lastState[c] = currentState[c];
			}
		}
	}

	bool down(int controller, int button) const {
		return currentState[controller] & button;
	}

#ifdef DEBUG
	void printState(byte gp) {
		auto cs = currentState[gp];
		//Serial.println(cs);
		//Serial.print((cs & SC_CTL_ON)    ? "+" : "-");
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
	void readPort(byte c, byte reg1, byte reg2);
	boolean _pinSelect;

	byte _ignoreCycles[GAMEPAD_COUNT];

	boolean _connected[GAMEPAD_COUNT];
	boolean _sixButtonMode[GAMEPAD_COUNT];

	byte _inputReg1;
	byte _inputReg2;
#if GAMEPAD_COUNT > 1
	byte _inputReg3;
#endif
};

SegaControllers32U4::SegaControllers32U4(void) {
	// Setup input pins (A0,A1,A2,A3,14,15 or PF7,PF6,PF5,PF4,PB3,PB1)
	DDRF &= ~B11110000;	 // input
	PORTF |= B11110000;	 // high to enable internal pull-up
	DDRB &= ~B00001010;	 // input
	PORTB |= B00001010;	 // high to enable internal pull-up
	// Setup input pins (TXO,RXI,2,3,4,6 or PD3,PD2,PD1,PD0,PD4,PD7)
	DDRD &= ~B10011111;	 // input
	PORTD |= B10011111;	 // high to enable internal pull-up

	DDRC |= B01000000;	// Select pins as output
	DDRE |= B01000000;
	PORTC |= B01000000;	 // Select pins high
	PORTE |= B01000000;

	_pinSelect = true;
	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		currentState[c] = 0;
		lastState[c] = 0;
		_connected[c] = 0;
		_sixButtonMode[c] = false;
		_ignoreCycles[c] = 0;
	}
}

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
	_inputReg1 = PINF;
	_inputReg2 = PINB;
#if GAMEPAD_COUNT > 1
	_inputReg3 = PIND;
#endif

	readPort(0, _inputReg1, _inputReg2);
#if GAMEPAD_COUNT > 1
	readPort(1, _inputReg3, _inputReg3);
#endif
}

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
void SegaControllers32U4::readPort(byte c, byte reg1, byte reg2) {
	if (_ignoreCycles[c] <= 0) {
		if (_pinSelect)	 // Select pin is HIGH
		{
			if (_connected[c]) {
				// Check if six button mode is active
				if (_sixButtonMode[c]) {
					// Read input pins for X, Y, Z, Mode
					(bitRead(reg1, DATA_PIN[c][0]) == LOW) ? currentState[c] |= SC_BTN_Z : currentState[c] &= ~SC_BTN_Z;
					(bitRead(reg1, DATA_PIN[c][1]) == LOW) ? currentState[c] |= SC_BTN_Y : currentState[c] &= ~SC_BTN_Y;
					(bitRead(reg1, DATA_PIN[c][2]) == LOW) ? currentState[c] |= SC_BTN_X : currentState[c] &= ~SC_BTN_X;
					(bitRead(reg1, DATA_PIN[c][3]) == LOW) ? currentState[c] |= SC_BTN_MODE : currentState[c] &= ~SC_BTN_MODE;
					_sixButtonMode[c] = false;
					_ignoreCycles[c] = 2;  // Ignore the two next cycles (cycles 6 and 7 in table above)
				} else {
					// Read input pins for Up, Down, Left, Right, B, C
					(bitRead(reg1, DATA_PIN[c][0]) == LOW) ? currentState[c] |= SC_BTN_UP : currentState[c] &= ~SC_BTN_UP;
					(bitRead(reg1, DATA_PIN[c][1]) == LOW) ? currentState[c] |= SC_BTN_DOWN : currentState[c] &= ~SC_BTN_DOWN;
					(bitRead(reg1, DATA_PIN[c][2]) == LOW) ? currentState[c] |= SC_BTN_LEFT : currentState[c] &= ~SC_BTN_LEFT;
					(bitRead(reg1, DATA_PIN[c][3]) == LOW) ? currentState[c] |= SC_BTN_RIGHT : currentState[c] &= ~SC_BTN_RIGHT;
					(bitRead(reg2, DATA_PIN[c][4]) == LOW) ? currentState[c] |= SC_BTN_B : currentState[c] &= ~SC_BTN_B;
					(bitRead(reg2, DATA_PIN[c][5]) == LOW) ? currentState[c] |= SC_BTN_C : currentState[c] &= ~SC_BTN_C;
				}
			} else	// No Mega Drive controller is connected, use SMS/Atari mode
			{
				// Clear current state
				currentState[c] = 0;

				// Read input pins for Up, Down, Left, Right, Fire1, Fire2
				if (bitRead(reg1, DATA_PIN[c][0]) == LOW) {
					currentState[c] |= SC_BTN_UP;
				}
				if (bitRead(reg1, DATA_PIN[c][1]) == LOW) {
					currentState[c] |= SC_BTN_DOWN;
				}
				if (bitRead(reg1, DATA_PIN[c][2]) == LOW) {
					currentState[c] |= SC_BTN_LEFT;
				}
				if (bitRead(reg1, DATA_PIN[c][3]) == LOW) {
					currentState[c] |= SC_BTN_RIGHT;
				}
				if (bitRead(reg2, DATA_PIN[c][4]) == LOW) {
					currentState[c] |= SC_BTN_A;
				}
				if (bitRead(reg2, DATA_PIN[c][5]) == LOW) {
					currentState[c] |= SC_BTN_B;
				}
			}
		} else	// Select pin is LOW
		{
			// Check if a controller is connected
			_connected[c] = (bitRead(reg1, DATA_PIN[c][2]) == LOW && bitRead(reg1, DATA_PIN[c][3]) == LOW);

			// Check for six button mode
			_sixButtonMode[c] = (bitRead(reg1, DATA_PIN[c][0]) == LOW && bitRead(reg1, DATA_PIN[c][1]) == LOW);

			// Read input pins for A and Start
			if (_connected[c]) {
				if (!_sixButtonMode[c]) {
					(bitRead(reg2, DATA_PIN[c][4]) == LOW) ? currentState[c] |= SC_BTN_A : currentState[c] &= ~SC_BTN_A;
					(bitRead(reg2, DATA_PIN[c][5]) == LOW) ? currentState[c] |= SC_BTN_START : currentState[c] &= ~SC_BTN_START;
				}
			}
		}
	} else {
		if (_ignoreCycles[c]-- == 2)  // Decrease the ignore cycles counter and read 8bitdo home in first "ignored" cycle, this cycle is unused on normal 6-button controllers
		{
			(bitRead(reg1, DATA_PIN[c][0]) == LOW) ? currentState[c] |= SC_BTN_HOME : currentState[c] &= ~SC_BTN_HOME;
		}
	}
}

SegaControllers32U4 controllers;

GAMEPAD_CLASS gamepad;

void setup() {
	Serial.begin(115200);
	gamepad.begin();
}

void controllerChanged(const int c) {
#ifdef DEBUG
	controllers.printState(c);
#endif

	gamepad.buttons(c, 0);
	// if start and select are held at the same time, send menu and only menu
	if (controllers.down(c, SC_BTN_START) && controllers.down(c, SC_BTN_DOWN)) {
		gamepad.press(c, BUTTON_MENU);
	} else {
		// actually send buttons held
		for (uint8_t btn = 0; btn < BUTTON_COUNT; btn++) {
			if (controllers.down(c, translateFromButton[btn])) {
				gamepad.press(c, translateToHid[btn]);
			}
		}
	}
	if (controllers.down(c, SC_BTN_DOWN)) {
		if (controllers.down(c, SC_BTN_RIGHT)) {
			gamepad.setHatSync(c, DPAD_DOWN_RIGHT);
		} else if (controllers.down(c, SC_BTN_LEFT)) {
			gamepad.setHatSync(c, DPAD_DOWN_LEFT);
		} else {
			gamepad.setHatSync(c, DPAD_DOWN);
		}
	} else if (controllers.down(c, SC_BTN_UP)) {
		if (controllers.down(c, SC_BTN_RIGHT)) {
			gamepad.setHatSync(c, DPAD_UP_RIGHT);
		} else if (controllers.down(c, SC_BTN_LEFT)) {
			gamepad.setHatSync(c, DPAD_UP_LEFT);
		} else {
			gamepad.setHatSync(c, DPAD_UP);
		}
	} else if (controllers.down(c, SC_BTN_RIGHT)) {
		gamepad.setHatSync(c, DPAD_RIGHT);
	} else if (controllers.down(c, SC_BTN_LEFT)) {
		gamepad.setHatSync(c, DPAD_LEFT);
	} else {
		gamepad.setHatSync(c, DPAD_CENTERED);
	}
}

void loop() {
	if (gamepad.isConnected()) {
		controllers.poll(controllerChanged);
	}
}
