#ifndef RADIO_GAMEPAD_H
#define RADIO_GAMEPAD_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define GAMEPAD_REPORT_ARRAY_ADD 1

#include "../common.h"

RF24 radio(7, 8);  // CE, CSN
const byte address[13] = "OpenRetroPad";

class RadioGamepad : public AbstractGamepad {
   public:
	RadioGamepad() : AbstractGamepad() {
	}

	void begin(void) {
		Serial.println("RadioGamepad.begin");
		radio.begin();
		radio.openWritingPipe(address);
		radio.setPALevel(RF24_PA_MIN);
		radio.stopListening();
	}

	void sync(const uint8_t cIdx) {
		Serial.println("RadioGamepad.sync");
		gamepadReport[15] = cIdx;
		radio.write(&gamepadReport, 16);
	}
};

typedef RadioGamepad Gamepad;

#endif	// RADIO_GAMEPAD_H
