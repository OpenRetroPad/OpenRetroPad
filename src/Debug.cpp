
#include "Arduino.h"

#include "gamepad/Gamepad.h"

GAMEPAD_CLASS gamepad;

uint8_t c = 0;

void setup() {
	Serial.begin(115200);
	Serial.println("Starting debug work!");
	gamepad.begin();
}

void loop() {
	if (gamepad.isConnected()) {
		// test code to automate sending to test RadioReceiver (or any gamepad impl)
		Serial.println("Press buttons A and Y. Move all axes to center. Set DPAD to down right.");
		gamepad.press(c, BUTTON_A);
		gamepad.press(c, BUTTON_Y);
		gamepad.setAxis(c, 0, 0, 0, 0, 0, 0, DPAD_DOWN_RIGHT);
		delay(500);
		Serial.println("Release all buttons. Move all axes to center. Set DPAD to center.");
		gamepad.buttons(c, 0);
		gamepad.setAxis(c, 0, 0, 0, 0, 0, 0, DPAD_CENTER);
		delay(500);
	}
}
