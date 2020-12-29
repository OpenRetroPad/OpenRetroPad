
#include "Arduino.h"

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define GAMEPAD_REPORT_ARRAY_ADD 1

#include "gamepad/Gamepad.h"

#if defined(ARDUINO_ARCH_ESP32)
// esp32
#define CE_PIN 18
#define CSN_PIN 19
#else
// micro
#define CE_PIN 7
#define CSN_PIN 8
#endif	// ARDUINO_ARCH_ESP32

RF24 radio(CE_PIN, CSN_PIN);

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
