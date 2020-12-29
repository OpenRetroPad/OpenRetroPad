
/*
 https://www.circuitbasics.com/arduino-ohm-meter/

micro values:

82: 81.31
100: 98.71-99.89

*/

#if defined(ARDUINO_ARCH_ESP32)

float Vin = 3.3;
float steps = 4095.0;

#else

float Vin = 5;
float steps = 1024.0;

#endif	// ARDUINO_ARCH_ESP32

// value of the known resistor
float R1 = 1000;

#include "Arduino.h"

#include "pins.h"

int raw = 0;
float Vout = 0;
float R2 = 0;
float buffer = 0;

void setup() {
	Serial.begin(115200);
}

void loop() {
	raw = analogRead(OR_PIN_9);
	if (raw) {
		buffer = raw * Vin;
		Vout = (buffer) / steps;
		buffer = (Vin / Vout) - 1;
		Serial.print("raw: ");
		Serial.print(raw);
		Serial.print(" Vout: ");
		Serial.print(Vout);

		// this is R2 when it's connected to + and R1 is connected to ground
		R2 = R1 * buffer;
		Serial.print(" R2: ");
		Serial.print(R2);

		// this is R2 when it's connected to ground, and R1 is connected to +
		R2 = R1 / buffer;
		Serial.print(" R2: ");
		Serial.println(R2);
	} else {
		Serial.println("disconnected");
	}
	delay(1000);
}
