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

#define DATA_PIN 23

//#define PRINT_Y_AXIS_VALUES 1
//#define PRINT_X_AXIS_VALUES 1
//#define PLOT_CONSOLE_POLLING 1
#define DEBUG
#define PRINT_DATA

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 1
#endif

#define AXIS_CENTER_IN 0
#define AXIS_MAX_IN 60
#define AXIS_MIN_IN -60

#include "gamepad/Gamepad.h"
#include "util.cpp"

#define LINE_WRITE_HIGH pinMode(DATA_PIN, INPUT_PULLUP)
#define LINE_WRITE_LOW pinMode(DATA_PIN, OUTPUT)

#define MAX_INCLINE_AXIS_X 60
#define MAX_INCLINE_AXIS_Y 60

#define DATA_SIZE 1536	// number of sample points to poll
#define CALIBRATE_PASSES 5

#define NUM_BITS 32

// buffer to hold data being read from controller
bool buffer[DATA_SIZE];

// bit resolution and offsets
int bitOffsets[NUM_BITS];
int bitResolution = 0;
int bitsToRead = DATA_SIZE;

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

	// we might be further refining a previous calibration, if non-zero, remove existing bitResolution
	for (int i = 0; i < NUM_BITS; i++) {
		bitOffsets[i] += bitResolution;
	}

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
				int thisOffset = i - thisResolution + 1;
				if (bitOffsets[bitCounter] == 0 || bitOffsets[bitCounter] > thisOffset) {
					bitOffsets[bitCounter] = thisOffset;
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

	// calculate bit's beginning offsets by subtracting resolution
	// if this index is 0, button is not pressed, if 1, button is pressed
	for (int i = 0; i < NUM_BITS; i++) {
		bitOffsets[i] -= bitResolution;
	}
}

void calcBitsToRead() {
	bitsToRead = bitOffsets[NUM_BITS - 1] + 1;
	if (bitsToRead < NUM_BITS * 2) {
		// todo: not enough, error out...
		while (true) {
			printf("not enough bitsToRead, increase DATA_SIZE ???\n");
			delay(5000);
		}
	}
}

/* Function to extract a controller bit from the buffer of returned data */
void getBit(bool *bit, int offset, bool *data) {
	*bit = data[offset] == true;
}

/** Function to send a Command to the attached N64-Controller.
 *  Must be run from RAM to defy timing differences introduced from
 *  reading Code from ESP32's SPI Flash Chip.
*/
void IRAM_ATTR sendCommand(byte command) {
	// the current bit to write
	bool bit;

	// clear output buffer, todo: really need to do this????
	//memset(buffer, 0, DATA_SIZE);

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
	delayMicroseconds(2);

	// read returned data as fast as possible
	for (int i = 0; i < bitsToRead; i++) {
		//buffer[i] = digitalRead(DATA_PIN);
		// this is faster:
#if DATA_PIN < 32
		buffer[i] = (GPIO.in >> DATA_PIN) & 0x1;
#elif DATA_PIN < 40
		buffer[i] = (GPIO.in1.val >> (DATA_PIN - 32)) & 0x1;
#else

#error unsupported DATA_PIN must be <40

#endif
		//delayMicroseconds(1);
	}

	interrupts();

// plot polling process from controller if unstructed to
#ifdef PLOT_CONSOLE_POLLING
	for (int i = 0; i < bitsToRead; i++) {
		Serial.println(buffer[i] * 2500);
	}

#endif

#ifdef PRINT_DATA
	for (int i = 0; i < bitsToRead; i++) {
		printf(buffer[i] ? "1" : "0");
	}
	printf("\n----------------\n");
#endif
}

/** Function to populate the controller struct if command 0x01 was sent.
 *  Buttons are set according to data in buffer, raw axis data is written,
 *  Axis Data is correctly decoded from raw axis data by taking two's complement
 *  and checking if value if below 'MAX_INCLINE_AXIS_X' or 'MAX_INCLINE_AXIS_Y'.
 *  If values surpass the maximum incline they are set to match those values.
 */
void populateControllerStruct(ControllerData *data) {
	// first byte
	data->buttonA = buffer[bitOffsets[0]];
	data->buttonB = buffer[bitOffsets[1]];
	data->buttonZ = buffer[bitOffsets[2]];
	data->buttonStart = buffer[bitOffsets[3]];
	data->DPadUp = buffer[bitOffsets[4]];
	data->DPadDown = buffer[bitOffsets[5]];
	data->DPadLeft = buffer[bitOffsets[6]];
	data->DPadRight = buffer[bitOffsets[7]];

	// second byte, first two bits are unused
	data->buttonL = buffer[bitOffsets[10]];
	data->buttonR = buffer[bitOffsets[11]];
	data->CUp = buffer[bitOffsets[12]];
	data->CDown = buffer[bitOffsets[13]];
	data->CRight = buffer[bitOffsets[14]];

	// third byte
	getBit(&(data->xAxisRaw[0]), bitOffsets[16], &buffer[0]);
	getBit(&(data->xAxisRaw[1]), bitOffsets[17], &buffer[0]);
	getBit(&(data->xAxisRaw[2]), bitOffsets[18], &buffer[0]);
	getBit(&(data->xAxisRaw[3]), bitOffsets[19], &buffer[0]);
	getBit(&(data->xAxisRaw[4]), bitOffsets[20], &buffer[0]);
	getBit(&(data->xAxisRaw[5]), bitOffsets[21], &buffer[0]);
	getBit(&(data->xAxisRaw[6]), bitOffsets[22], &buffer[0]);
	getBit(&(data->xAxisRaw[7]), bitOffsets[23], &buffer[0]);

	// fourth byte
	getBit(&(data->yAxisRaw[0]), bitOffsets[24], &buffer[0]);
	getBit(&(data->yAxisRaw[1]), bitOffsets[25], &buffer[0]);
	getBit(&(data->yAxisRaw[2]), bitOffsets[26], &buffer[0]);
	getBit(&(data->yAxisRaw[3]), bitOffsets[27], &buffer[0]);
	getBit(&(data->yAxisRaw[4]), bitOffsets[28], &buffer[0]);
	getBit(&(data->yAxisRaw[5]), bitOffsets[29], &buffer[0]);
	getBit(&(data->yAxisRaw[6]), bitOffsets[30], &buffer[0]);
	getBit(&(data->yAxisRaw[7]), bitOffsets[31], &buffer[0]);

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
	if (data->xAxis > MAX_INCLINE_AXIS_X)
		data->xAxis = MAX_INCLINE_AXIS_X;
	if (data->xAxis < -MAX_INCLINE_AXIS_X)
		data->xAxis = -MAX_INCLINE_AXIS_X;

	// keep y axis below maxIncline
	if (data->yAxis > MAX_INCLINE_AXIS_Y)
		data->yAxis = MAX_INCLINE_AXIS_Y;
	if (data->yAxis < -MAX_INCLINE_AXIS_Y)
		data->yAxis = -MAX_INCLINE_AXIS_Y;

	//Serial.printf("xaxis: %-3i yaxis: %-3i \n",data->xAxis,data->yAxis);
}

ControllerData controller;

void loope() {
	// polling must not occur faster than every 20 ms
	//delay(14);
	delay(30);
	//delay(2000); // todo: change to above

	//Serial.println("sending command to n64");
	// send command 0x01 to n64 controller
	sendCommand(0x01);
	//updateOffsetsAndResolution();

	// store received data in controller struct
	populateControllerStruct(&controller);

	// output received data to ique
	//outputToiQue(&controller);
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
	gamepad.setAxis(c, translateAxis(controller.xAxis), translateAxis(controller.yAxis), cHat.x, cHat.y, 0, 0, hat);

	//checkUpdateCombo(&controller);

#ifdef DEBUG
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
	Serial.print(translateAxis(controller.yAxis));
	Serial.print(" X: ");
	Serial.print(controller.xAxis);
	Serial.print(" XT: ");
	Serial.print(translateAxis(controller.xAxis));
	Serial.println();
#endif

	//delay(500);
}

void pinned_loop() {
	while (true) {
		loope();
	}
}

void setup() {
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
	calcBitsToRead();
#ifdef DEBUG
	printf("bitOffsets: ");
	for (int i = 0; i < NUM_BITS; i++) {
		printf("%i:%i ", i, bitOffsets[i]);
	}
	printf("\n");
	printf("bitOffsets: ");
	for (int i = 0; i < NUM_BITS; i++) {
		printf("%i, ", bitOffsets[i]);
	}
	printf("\n");
	printf("bitResolution: %i\n", bitResolution);
	delay(5000);
#endif

	//xTaskCreatePinnedToCore(pinned_loop, "gbuttons", 2048, NULL, 1, NULL, 0);
	//xTaskCreatePinnedToCore(pinned_loop, "gbuttons", 2048, NULL, 1, NULL, 1);
}

void loop() {
	loope();
}
