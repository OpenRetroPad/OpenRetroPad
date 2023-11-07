/* -------------------------------------------------------------------------
Saturn controller socket (looking face-on at the front of the socket):
___________________
/ 1 2 3 4 5 6 7 8 9 \
|___________________|

Saturn controller plug (looking face-on at the front of the controller plug):
___________________
/ 9 8 7 6 5 4 3 2 1 \
|___________________|

Saturn
-----------------
1 VCC     - black
2 DATA1   - white
3 DATA0   - gray
4 SEL1    - blue
5 SEL0    - green
6 TL (5V) - yellow
7 DATA3   - orange
8 DATA2   - red
9 GND     - brown

NOTE: The receiver of the Retro Bit 2.4GHz controller needs to be plugged
      in after the adapter has been connected to USB and the RETROBIT_WL
      define needs to be uncommented.
------------------------------------------------------------------------- */
#include "Arduino.h"

#include <digitalWriteFast.h>
//#define digitalWriteFast digitalWrite
//#define digitalReadFast digitalRead

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 2
#endif

#include "gamepad/Gamepad.h"

// How many microseconds to wait after setting select lines? (2µs is enough according to the Saturn developer's manual)
// 20µs is a "safe" value that seems to work for original Saturn controllers and Retrobit wired controllers
#define SELECT_PAUSE 10

// Uncomment to support the Retro Bit 2.4GHz wireless controller (this will increase lag a lot)
//#define RETROBIT_WL

#include "pins.h"

// pins
#define P1_2 OR_PIN_2
#define P1_3 OR_PIN_3
#define P1_6 OR_PIN_4
#define P1_7 OR_PIN_1
#define P1_8 OR_PIN_11

#define PX_4 OR_PIN_6
#define PX_5 OR_PIN_5

#if GAMEPAD_COUNT == 2

#define P2_2 OR_PIN_20
#define P2_3 OR_PIN_21
#define P2_6 OR_PIN_10
#define P2_7 OR_PIN_18
#define P2_8 OR_PIN_19

#endif

#ifdef BLUERETRO_MAPPING
#undef P1_6
#undef PX_4
#define P1_6 ALT_PIN_1
#define PX_4 ALT_PIN_2
#endif

// Set up USB HID gamepads
GAMEPAD_CLASS gamepad;

ScratchGamepad currentGamepad;

// Read R, X, Y, Z
void read1() {
	// Set select outputs to 00
	digitalWriteFast(PX_4, LOW);
	digitalWriteFast(PX_5, LOW);
	delayMicroseconds(SELECT_PAUSE);
	if (!digitalReadFast(P1_3)) {
		// Z
		currentGamepad.press(0, BUTTON_R);
	}
	if (!digitalReadFast(P1_2)) {
		// Y
		currentGamepad.press(0, BUTTON_X);
	}
	if (!digitalReadFast(P1_8)) {
		// X
		currentGamepad.press(0, BUTTON_L);
	}
	if (!digitalReadFast(P1_7)) {
		// R
		currentGamepad.press(0, BUTTON_R2);
	}
#if GAMEPAD_COUNT == 2
	if (!digitalReadFast(P2_3)) {
		// Z
		currentGamepad.press(1, BUTTON_R);
	}
	if (!digitalReadFast(P2_2)) {
		// Y
		currentGamepad.press(1, BUTTON_X);
	}
	if (!digitalReadFast(P2_8)) {
		// X
		currentGamepad.press(1, BUTTON_L);
	}
	if (!digitalReadFast(P2_7)) {
		// R
		currentGamepad.press(1, BUTTON_R2);
	}
#endif
}

// Read ST, A, C, B
void read2() {
	// Toggle select outputs (01->10 or 10->01)
	digitalWriteFast(PX_4, HIGH);
	digitalWriteFast(PX_5, LOW);
	delayMicroseconds(SELECT_PAUSE);
	if (!digitalReadFast(P1_3)) {
		// B
		currentGamepad.press(0, BUTTON_B);
	}
	if (!digitalReadFast(P1_2)) {
		// C
		currentGamepad.press(0, BUTTON_A);
	}
	if (!digitalReadFast(P1_8)) {
		// A
		currentGamepad.press(0, BUTTON_Y);
	}
	if (!digitalReadFast(P1_7)) {
		// ST
		currentGamepad.press(0, BUTTON_START);
	}
#if GAMEPAD_COUNT == 2
	if (!digitalReadFast(P2_3)) {
		// B
		currentGamepad.press(1, BUTTON_B);
	}
	if (!digitalReadFast(P2_2)) {
		// C
		currentGamepad.press(1, BUTTON_A);
	}
	if (!digitalReadFast(P2_8)) {
		// A
		currentGamepad.press(1, BUTTON_Y);
	}
	if (!digitalReadFast(P2_7)) {
		// ST
		currentGamepad.press(1, BUTTON_START);
	}
#endif
}

// Read DR, DL, DD, DU
void read3() {
	// Set select outputs to 10 from 11 (toggle)
	digitalWriteFast(PX_4, LOW);
	digitalWriteFast(PX_5, HIGH);
	delayMicroseconds(SELECT_PAUSE);
	if (!digitalReadFast(P1_3)) {
		// UP
		currentGamepad.pressDpad(0, DPAD_BIT_UP);
	}
	if (!digitalReadFast(P1_2)) {
		// DOWN
		currentGamepad.pressDpad(0, DPAD_BIT_DOWN);
	}
	if (!digitalReadFast(P1_8)) {
		// LEFT
		currentGamepad.pressDpad(0, DPAD_BIT_LEFT);
	}
	if (!digitalReadFast(P1_7)) {
		// RIGHT
		currentGamepad.pressDpad(0, DPAD_BIT_RIGHT);
	}
#if GAMEPAD_COUNT == 2
	if (!digitalReadFast(P2_3)) {
		// UP
		currentGamepad.pressDpad(1, DPAD_BIT_UP);
	}
	if (!digitalReadFast(P2_2)) {
		// DOWN
		currentGamepad.pressDpad(1, DPAD_BIT_DOWN);
	}
	if (!digitalReadFast(P2_8)) {
		// LEFT
		currentGamepad.pressDpad(1, DPAD_BIT_LEFT);
	}
	if (!digitalReadFast(P2_7)) {
		// RIGHT
		currentGamepad.pressDpad(1, DPAD_BIT_RIGHT);
	}
#endif
}

// Read L, *, *, *
void read4() {
	// Set select outputs to 11
	digitalWriteFast(PX_4, HIGH);
	digitalWriteFast(PX_5, HIGH);
	delayMicroseconds(SELECT_PAUSE);
	if (!digitalReadFast(P1_7)) {
		// L
		currentGamepad.press(0, BUTTON_L2);
	}
#if GAMEPAD_COUNT == 2
	if (!digitalReadFast(P2_7)) {
		// L
		currentGamepad.press(1, BUTTON_L2);
	}
#endif
}

void setup() {
	gamepad.begin();

	// Set P1 data pins  as inputs and enable pull-up resistors
	pinMode(P1_3, INPUT_PULLUP);
	pinMode(P1_2, INPUT_PULLUP);
	pinMode(P1_7, INPUT_PULLUP);
	pinMode(P1_8, INPUT_PULLUP);
	digitalWrite(P1_3, HIGH);
	digitalWrite(P1_2, HIGH);
	digitalWrite(P1_7, HIGH);
	digitalWrite(P1_8, HIGH);

	// Set P1 TL as input and enable pull-up resistor
	pinMode(P1_6, INPUT_PULLUP);
	digitalWrite(P1_6, HIGH);

#if GAMEPAD_COUNT == 2
	// Set P2 data pins as inputs and enable pull-up resistors
	pinMode(P2_3, INPUT_PULLUP);
	pinMode(P2_2, INPUT_PULLUP);
	pinMode(P2_7, INPUT_PULLUP);
	pinMode(P2_8, INPUT_PULLUP);
	digitalWrite(P2_3, HIGH);
	digitalWrite(P2_2, HIGH);
	digitalWrite(P2_7, HIGH);
	digitalWrite(P2_8, HIGH);

	// Set P2 TL as input and enable pull-up resistor
	pinMode(P2_6, INPUT_PULLUP);
	digitalWrite(P2_6, HIGH);
#endif

	// Set P1+P2 select pins as outputs and set them HIGH
	pinMode(PX_4, OUTPUT);
	pinMode(PX_5, OUTPUT);
	digitalWrite(PX_4, HIGH);
	digitalWrite(PX_5, HIGH);

	// Wait for the controller(s) to settle
	delay(100);
}

void loop() {
	// Read all button and axes states
	read3();
	read2();
	read1();
	read4();

	// Send data to USB if values have changed
	for (uint8_t gp = 0; gp < GAMEPAD_COUNT; gp++) {
		if (currentGamepad.changed(gp, gamepad)) {
			const auto hat = gamepad.getHat(gp);
			if (hat == DPAD_DOWN && gamepad.isPressed(gp, BUTTON_START)) {
				gamepad.buttons(gp, 0);
				gamepad.press(gp, BUTTON_MENU);
				gamepad.setHatSync(gp, DPAD_CENTERED);
				currentGamepad.changed(gp, gamepad);
				return;
			}
			gamepad.setHatSync(gp, hat);
		}
		// Clear button data
		currentGamepad.reset(gp);
	}

#ifdef RETROBIT_WL
	// This delay is needed for the retro bit 2.4GHz wireless controller, making it more or less useless with this adapter
	delay(17);
#endif
}