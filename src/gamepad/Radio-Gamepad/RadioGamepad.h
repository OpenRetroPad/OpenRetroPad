#ifndef RADIO_GAMEPAD_H
#define RADIO_GAMEPAD_H

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define GAMEPAD_REPORT_ARRAY_ADD 1

#ifndef GAMEPAD_CLASS
#define GAMEPAD_CLASS RadioGamepad
#endif

#include "../common.h"

RF24 radio(7, 8);  // CE, CSN
const byte address[13] = "OpenRetroPad";

class RadioGamepad : public AbstractGamepad {
   public:
	RadioGamepad() : AbstractGamepad() {
	}

	virtual void begin(void) {
		Serial.println("RadioGamepad.begin");
		radio.begin();
		radio.openWritingPipe(address);
		radio.setPALevel(RF24_PA_MIN);
		radio.stopListening();
	}

	virtual void sendHidReport(const uint8_t cIdx, const void* d, int len) {
		Serial.println("RadioGamepad.sync");
		gamepadReport[15] = cIdx;
		radio.write(d, 16);
	}
};

#endif	// RADIO_GAMEPAD_H
