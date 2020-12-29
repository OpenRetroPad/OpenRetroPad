
#include "Arduino.h"

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define GAMEPAD_REPORT_ARRAY_ADD 1

#include "gamepad/Gamepad.h"

#include "pins.h"

RF24 radio(OR_PIN_7, OR_PIN_8);	 // CE, CSN

const byte address[13] = "OpenRetroPad";

GAMEPAD_CLASS gamepad;

void setup() {
	gamepad.begin();
	radio.begin();
	radio.openReadingPipe(0, address);
	radio.setPALevel(RF24_PA_MIN);
	radio.startListening();
}

void loop() {
	if (radio.available() && gamepad.isConnected()) {
		radio.read(&gamepad.gamepadReport, 16);
		gamepad.sync(gamepad.gamepadReport[15]);
	}
}
