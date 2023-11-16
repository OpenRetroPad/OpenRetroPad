/*
      LOOKING AT THE PLUG FROM CONTROLLER
          /---------\
 PIN 1-> /  o  o  o  \
        /-------------\


PIN # USAGE

    GND
    DATA
    VCC +3.3V ONLY
*/

#include "Arduino.h"

#include "pins.h"

#define DATA_PIN OR_PIN_2

//#define PRINT_Y_AXIS_VALUES 1
//#define PRINT_X_AXIS_VALUES 1
//#define PLOT_CONSOLE_POLLING 1
//#define DEBUG
//#define PRINT_DATA

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 1
#endif

#define AXIS_CENTER_IN 0
#define AXIS_MAX_IN 70
#define AXIS_MIN_IN -70

#include "gamepad/Gamepad.h"
#include "util.cpp"

#define LINE_WRITE_HIGH pinMode(DATA_PIN, INPUT_PULLUP)
#define LINE_WRITE_LOW pinMode(DATA_PIN, OUTPUT)

#define DATA_SIZE 2048	// number of sample points to poll
#define CALIBRATE_PASSES 5

#define NUM_BITS 32

// buffer to hold data being read from controller
bool buffer[DATA_SIZE];

// bit resolution
int bitResolution = 0;
int lastIndex = 0;

bool returnedBits[NUM_BITS];
bool oldReport[NUM_BITS];

GAMEPAD_CLASS gamepad;

struct ControllerData {
	bool buttonA;
	bool buttonB;
	bool buttonZ;
	bool buttonL;
	bool buttonR;
	bool buttonStart;

	bool DPadUp;
	bool DPadDown;
	bool DPadLeft;
	bool DPadRight;

	bool CUp;
	bool CDown;
	bool CLeft;
	bool CRight;

	short xAxis;
	short yAxis;

	bool xAxisRaw[8];
	bool yAxisRaw[8];
};

void updateOffsetsAndResolution() {
	// the current bit counter
	int bitCounter = 0;

	// to hold the number of 1's in this bit
	int thisResolution = 0;

	// current index
	int i = 0;

	bitResolution /= 2;

	for (; i < DATA_SIZE; i++) {
		if (buffer[i] == false) {
			// we skip all leading 1's
			break;
		}
	}

	// iterate over buffer
	for (; i < DATA_SIZE - 1 && bitCounter < NUM_BITS; i++) {
		if (buffer[i] == true) {
			++thisResolution;
			// if a falling edge is detected
			if (buffer[1 + i] == false) {
				// store bit's earliest possible beginning offsets
				if (lastIndex < (i + 1)) {
					lastIndex = i + 1;
				}

				// store max resolution in bitResolution
				if (thisResolution > bitResolution) {
					bitResolution = thisResolution;
				}

				// reset thisResolution
				thisResolution = 0;
				// increment bitCounter
				++bitCounter;
			}
		}
	}

	bitResolution *= 2;
}

void checkBitsToRead() {
	if (lastIndex < NUM_BITS * 2) {
		// not enough, error out...
		while (true) {
			printf("not enough bitsToRead, increase DATA_SIZE ???\n");
			delay(5000);
		}
	}
}

/** Function to send a Command to the attached N64-Controller.
 *  Must be run from RAM to defy timing differences introduced from
 *  reading Code from ESP32's SPI Flash Chip.
*/
void IRAM_ATTR sendCommand(byte command) {
	// the current bit to write
	bool bit;

	noInterrupts();

	// for each bit
	for (int i = 0; i < 8; i++) {
		// get value
		bit = (1 << (7 - i)) & command;

		// write data
		LINE_WRITE_LOW;
		delayMicroseconds((3 - 2 * bit));
		LINE_WRITE_HIGH;
		delayMicroseconds((1 + 2 * bit));
	}

	// console stop bit
	LINE_WRITE_LOW;
	delayMicroseconds(1);
	LINE_WRITE_HIGH;
	delayMicroseconds(1);  // by spec should be 2

	// read returned data as fast as possible
	for (int i = 0; i < DATA_SIZE; i++) {
		// this is faster:
#if defined(ARDUINO_ARCH_ESP32)

#if DATA_PIN < 32
		buffer[i] = (GPIO.in >> DATA_PIN) & 0x1;
#elif DATA_PIN < 40
		buffer[i] = (GPIO.in1.val >> (DATA_PIN - 32)) & 0x1;
#else

#error unsupported DATA_PIN must be <40

#endif

#else
		// this is portable:
		buffer[i] = digitalRead(DATA_PIN);
#endif	// not defined(ARDUINO_ARCH_ESP32)
	}

	interrupts();

// plot polling process from controller if unstructed to
#ifdef PLOT_CONSOLE_POLLING
	for (int i = 0; i < DATA_SIZE; i++) {
		Serial.println(buffer[i] * 2500);
	}

#endif

#ifdef PRINT_DATA
	for (int i = 0; i < DATA_SIZE; i++) {
		printf(buffer[i] ? "1" : "0");
	}
	printf("\n----------------\n");
#endif
}

bool calcReturnedBits() {
	// the current bit counter
	int bitCounter = 0;

	// to hold the number of 1's in this bit
	int thisResolution = 0;

	// current index
	int i = 0;
	for (; i < DATA_SIZE; i++) {
		if (buffer[i] == false) {
			// we skip all leading 1's
			break;
		}
	}

	// iterate over buffer
	for (; i < DATA_SIZE - 1 && bitCounter < NUM_BITS; i++) {
		if (buffer[i] == true) {
			++thisResolution;
			// if a falling edge is detected
			if (buffer[1 + i] == false) {
				returnedBits[bitCounter] = thisResolution >= bitResolution;

				// reset thisResolution
				thisResolution = 0;
				// increment bitCounter
				++bitCounter;
			}
		}
	}

#ifdef DEBUG
	if (bitCounter != 32) {
		printf("%i != 32, not enough bitsToRead, increase DATA_SIZE ???\n", bitCounter);
		// not enough, error out...
		/*
        for (int i = 0; i < bitsToRead; i++) {
            printf(buffer[i] ? "1" : "0");
        }
        printf("\n----------------\n");
	*/
	}
#endif

	return bitCounter == 32;
}

/** Function to populate the controller struct if command 0x01 was sent.
 *  Buttons are set according to data in buffer, raw axis data is written,
 *  Axis Data is correctly decoded from raw axis data by taking two's complement
 *  and checking if value if below 'MAX_INCLINE_AXIS_X' or 'MAX_INCLINE_AXIS_Y'.
 *  If values surpass the maximum incline they are set to match those values.
 */
void populateControllerStruct(ControllerData *data) {
	// first byte
	data->buttonA = returnedBits[0];
	data->buttonB = returnedBits[1];
	data->buttonZ = returnedBits[2];
	data->buttonStart = returnedBits[3];
	data->DPadUp = returnedBits[4];
	data->DPadDown = returnedBits[5];
	data->DPadLeft = returnedBits[6];
	data->DPadRight = returnedBits[7];

	// second byte, first two bits are unused
	data->buttonL = returnedBits[10];
	data->buttonR = returnedBits[11];
	data->CUp = returnedBits[12];
	data->CDown = returnedBits[13];
	data->CLeft = returnedBits[14];
	data->CRight = returnedBits[15];

	for (int i = 0; i < 8; ++i) {
		// third byte
		data->xAxisRaw[i] = returnedBits[i + 16];
		// fourth byte
		data->yAxisRaw[i] = returnedBits[i + 24];
	}

	// sum up bits to get axis bytes
	data->xAxis = 0;
	data->yAxis = 0;
	for (int i = 0; i < 8; i++) {
		data->xAxis += (data->xAxisRaw[i] * (0x80 >> (i)));
		data->yAxis += (data->yAxisRaw[i] * (0x80 >> (i)));
	}

// print axis values
#ifdef DEBUG
	Serial.printf("yRaw: %i %i %i %i %i %i %i %i xRaw: %i %i %i %i %i %i %i %i yAxis: %03i xAxis: %03i",
				  data->yAxisRaw[0],
				  data->yAxisRaw[1],
				  data->yAxisRaw[2],
				  data->yAxisRaw[3],
				  data->yAxisRaw[4],
				  data->yAxisRaw[5],
				  data->yAxisRaw[6],
				  data->yAxisRaw[7],
				  data->xAxisRaw[0],
				  data->xAxisRaw[1],
				  data->xAxisRaw[2],
				  data->xAxisRaw[3],
				  data->xAxisRaw[4],
				  data->xAxisRaw[5],
				  data->xAxisRaw[6],
				  data->xAxisRaw[7],
				  data->yAxis,
				  data->xAxis);
#endif

	// decode xAxis two's complement
	if (data->xAxis & 0x80) {
		data->xAxis = -1 * (0xff - data->xAxis);
	}

	// decode yAxis two's complement
	if (data->yAxis & 0x80) {
		data->yAxis = -1 * (0xff - data->yAxis);
	}

	// keep x axis below maxIncline
	if (data->xAxis > AXIS_MAX_IN)
		data->xAxis = AXIS_MAX_IN;
	if (data->xAxis < AXIS_MIN_IN)
		data->xAxis = AXIS_MIN_IN;

	// keep y axis below maxIncline
	if (data->yAxis > AXIS_MAX_IN)
		data->yAxis = AXIS_MAX_IN;
	if (data->yAxis < AXIS_MIN_IN)
		data->yAxis = AXIS_MIN_IN;

	//Serial.printf("xaxis: %-3i yaxis: %-3i \n",data->xAxis,data->yAxis);
}

ControllerData controller;

void setup() {
	setupBrLed();
#ifdef DEBUG
	Serial.begin(115200);
	delay(5000);
#endif

	gamepad.begin();

	// setup io pins
	//setupIO();
	// the controller data line
	LINE_WRITE_HIGH;

#ifdef PLOT_CONSOLE_POLLING
	sendCommand(0x01);
	while (true)
		;
#endif

	for (int i = 0; i < CALIBRATE_PASSES; ++i) {
		sendCommand(0x01);
		updateOffsetsAndResolution();
		if (i != CALIBRATE_PASSES) {
			delay(14);
		}
	}
	checkBitsToRead();
#ifdef DEBUG
	printf("bitResolution: %i, lastIndex: %i\n", bitResolution, lastIndex);
	delay(5000);
#endif
}

void loop() {
	// polling must not occur faster than every 20 ms
	delay(14);

	// send command 0x01 to n64 controller
	sendCommand(0x01);

	if (calcReturnedBits() && memcmp(oldReport, returnedBits, sizeof(returnedBits))) {
		memcpy(oldReport, returnedBits, sizeof(returnedBits));
	} else {
		// nothing changed
		return;
	}

	// store received data in controller struct
	populateControllerStruct(&controller);

	uint8_t c = 0;	// for now just do 1 pad
	gamepad.buttons(c, 0);
	if (controller.buttonStart) {
		if (controller.DPadDown) {
			// then only send menu, nothing else
			gamepad.press(c, BUTTON_MENU);
			gamepad.setHatSync(c, DPAD_CENTER);
			return;
		}
		gamepad.press(c, BUTTON_START);
	}
	if (controller.buttonA) {
		gamepad.press(c, BUTTON_A);
	}
	if (controller.buttonB) {
		gamepad.press(c, BUTTON_B);
	}
	if (controller.buttonZ) {
		gamepad.press(c, BUTTON_TR);
	}
	if (controller.buttonL) {
		gamepad.press(c, BUTTON_L);
	}
	if (controller.buttonR) {
		gamepad.press(c, BUTTON_R);
	}
	auto hat = calculateDpadDirection(controller.DPadUp, controller.DPadDown, controller.DPadLeft, controller.DPadRight);
	auto cHat = dpadToAxis(calculateDpadDirection(controller.CUp, controller.CDown, controller.CLeft, controller.CRight));
	gamepad.setAxis(c, translateAxis(controller.xAxis), -translateAxis(controller.yAxis), cHat.x, cHat.y, 0, 0, hat);

#ifdef DEBUG
	/*
	for (int i = 0; i < bitsToRead; i++) {
		Serial.print(buffer[i] ? "1" : "0");
	}
	Serial.println("\n----------------");
*/

	Serial.print(" buttons: ");
	Serial.print(controller.buttonA ? "A" : "-");
	Serial.print(controller.buttonB ? "B" : "-");
	Serial.print(controller.buttonZ ? "Z" : "-");
	Serial.print(controller.buttonL ? "L" : "-");
	Serial.print(controller.buttonR ? "R" : "-");
	Serial.print(controller.buttonStart ? "S" : "-");
	Serial.print(" DPAD: ");
	Serial.print(controller.DPadUp ? "U" : "-");
	Serial.print(controller.DPadDown ? "D" : "-");
	Serial.print(controller.DPadLeft ? "L" : "-");
	Serial.print(controller.DPadRight ? "R" : "-");
	Serial.print(" C: ");
	Serial.print(controller.CUp ? "U" : "-");
	Serial.print(controller.CDown ? "D" : "-");
	Serial.print(controller.CLeft ? "L" : "-");
	Serial.print(controller.CRight ? "R" : "-");
	Serial.print(" Y: ");
	Serial.print(controller.yAxis);
	Serial.print(" YT: ");
	Serial.print(-translateAxis(controller.yAxis));
	Serial.print(" X: ");
	Serial.print(controller.xAxis);
	Serial.print(" XT: ");
	Serial.print(translateAxis(controller.xAxis));
	Serial.println();
#endif
}
