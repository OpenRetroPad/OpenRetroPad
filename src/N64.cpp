
#include "Arduino.h"

//#define PRINT_Y_AXIS_VALUES 1
//#define PRINT_X_AXIS_VALUES 1
//#define PLOT_CONSOLE_POLLING 1

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 1
#endif

#include "gamepad/Gamepad.h"
#include "util.cpp"

#define DATA_PIN 13

#define LINE_WRITE_HIGH pinMode(DATA_PIN, INPUT_PULLUP)
#define LINE_WRITE_LOW pinMode(DATA_PIN, OUTPUT)

#define DATA_SIZE 450  // number of sample points to poll
#define DATA_OFFSET 0  // number of samples to ignore after staring to poll

#define MAX_INCLINE_AXIS_X 60
#define MAX_INCLINE_AXIS_Y 60

#define BIT_THRESHOLD 6

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

// buffer to hold data being read from controller
bool buffer[DATA_SIZE + DATA_OFFSET];

// bit resolution and offsets
int bitOffsets[32];
int bitResolution;

/** Function to send a Command to the attached N64-Controller.
 *  Must be run from RAM to defy timing differences introduced from
 *  reading Code from ESP32's SPI Flash Chip.
*/
//#ifdef ARDUINO_ARCH_ESP32
#if defined(CONFIG_BT_ENABLED)
void IRAM_ATTR sendCommand(byte command)
#else
// todo: what should be done here??
void sendCommand(byte command)
#endif
{
	// the current bit to write
	bool bit;

	// clear output buffer
	memset(buffer, 0, DATA_SIZE + DATA_OFFSET);

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
	for (int i = 0; i < DATA_SIZE + DATA_OFFSET; i++) {
		buffer[i] = digitalRead(DATA_PIN);
	}

	interrupts();

// plot polling process from controller if unstructed to
#ifdef PLOT_CONSOLE_POLLING
	for (int i = 0; i < DATA_SIZE + DATA_OFFSET; i++) {
		Serial.println(buffer[i] * 2500);
	}
#endif
}

/* Function to extract a controller bit from the buffer of returned data */
void getBit(bool *bit, int offset, bool *data) {
	// sanity check offset
	if (offset < 0)
		offset = 0;

	// count
	short count = 0;

	// get count from offset to offset + length
	for (int i = offset + DATA_OFFSET; i < offset + bitResolution; i++) {
		count += *(data + i);
	}

	// if offset surpasses threshold set bit
	*bit = false;
	if (count > BIT_THRESHOLD)
		*bit = true;
}

/** Function to populate the controller struct if command 0x01 was sent.
 *  Buttons are set according to data in buffer, raw axis data is written,
 *  Axis Data is correctly decoded from raw axis data by taking two's complement
 *  and checking if value if below 'MAX_INCLINE_AXIS_X' or 'MAX_INCLINE_AXIS_Y'.
 *  If values surpass the maximum incline they are set to match those values.
 */
void populateControllerStruct(ControllerData *data) {
	// first byte
	getBit(&(data->buttonA), bitOffsets[0], &buffer[0]);
	getBit(&(data->buttonB), bitOffsets[1], &buffer[0]);
	getBit(&(data->buttonZ), bitOffsets[2], &buffer[0]);
	getBit(&(data->buttonStart), bitOffsets[3], &buffer[0]);
	getBit(&(data->DPadUp), bitOffsets[4], &buffer[0]);
	getBit(&(data->DPadDown), bitOffsets[5], &buffer[0]);
	getBit(&(data->DPadLeft), bitOffsets[6], &buffer[0]);
	getBit(&(data->DPadRight), bitOffsets[7], &buffer[0]);

	// second byte, first two bits are unused
	getBit(&(data->buttonL), bitOffsets[10], &buffer[0]);
	getBit(&(data->buttonR), bitOffsets[11], &buffer[0]);
	getBit(&(data->CUp), bitOffsets[12], &buffer[0]);
	getBit(&(data->CDown), bitOffsets[13], &buffer[0]);
	getBit(&(data->CLeft), bitOffsets[14], &buffer[0]);
	getBit(&(data->CRight), bitOffsets[15], &buffer[0]);

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

// print y axis values
#ifdef PRINT_Y_AXIS_VALUES
		Serial.printf(
			"yRaw: %i %i %i %i %i %i %i %i\n", data->yAxisRaw[0], data->yAxisRaw[1], data->yAxisRaw[2], data->yAxisRaw[3], data->yAxisRaw[4], data->yAxisRaw[5], data->yAxisRaw[6], data->yAxisRaw[7]);
		Serial.printf("yAxis: %i \n", data->yAxis);
#endif

// print x axis values
#ifdef PRINT_X_AXIS_VALUES
		Serial.printf(
			"xRaw: %i %i %i %i %i %i %i %i\n", data->xAxisRaw[0], data->xAxisRaw[1], data->xAxisRaw[2], data->xAxisRaw[3], data->xAxisRaw[4], data->xAxisRaw[5], data->xAxisRaw[6], data->xAxisRaw[7]);
		Serial.printf("xAxis: %i \n", data->xAxis);
#endif
	}

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

void updateOffsetsAndResolution() {
	// the current bit counter
	int bitCounter = 0;

	// to hold the offset of A Button's falling edge
	int bitAfallingOffset = 0;

	// iterate over buffer
	for (int i = 0; i < DATA_SIZE + DATA_OFFSET - 1; i++) {
		// if a falling edge is detected
		if (buffer[i] == true && buffer[1 + i] == false) {
			// store bit's end offset
			bitOffsets[bitCounter] = i + 1;

			// if it's the A button store offset of the falling edge
			if (bitCounter == 0)
				bitAfallingOffset = i + 1;

			// if it's the B button calculate the bit Resolution
			if (bitCounter == 1)
				bitResolution = (i + 1) - bitAfallingOffset;

			// increment bit counter
			bitCounter++;
		}
	}

	//Serial.printf("Bit resolution is %i \n", bitResolution);

	// calculate bit's beginning offsets by subtracting resolution
	for (int i = 0; i < 32; i++) {
		bitOffsets[i] -= bitResolution;
		//Serial.printf("beginning of bit %i detected @ begin+%i \n", i + 1, bitOffsets[i]);
	}
}

ControllerData controller;

void setup() {
	Serial.begin(115200);

	gamepad.begin();

	// setup io pins
	//setupIO();
	// the controller data line
	LINE_WRITE_HIGH;

#ifdef PLOT_CONSOLE_POLLING
	delay(5000);
	sendCommand(0x01);
	while (true)
		;
#endif

	delay(5000);

	sendCommand(0x01);
	updateOffsetsAndResolution();
}

void loop() {
	Serial.println("sending command to n64");
	// send command 0x01 to n64 controller
	sendCommand(0x01);

	// store received data in controller struct
	populateControllerStruct(&controller);

	// output received data to ique
	//outputToiQue(&controller);
    uint8_t c = 0; // for now just do 1 pad
    gamepad.buttons(c, 0);
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
	if (controller.buttonStart) {
        gamepad.press(c, BUTTON_START);
	}
	auto hat = calculateDpadDirection(controller.DPadUp, controller.DPadDown, controller.DPadLeft, controller.DPadRight);
	auto cHat = dpadToAxis(calculateDpadDirection(controller.CUp, controller.CDown, controller.CLeft, controller.CRight));
    // todo: need to scale max/min to our max/min
    gamepad.setAxis(c, controller.xAxis, controller.yAxis, cHat.x, cHat.y, 0, 0, hat);


	// polling must not occur faster than every 20 ms
	delay(14);

	//checkUpdateCombo(&controller);

	//Serial.printf("DPAD: %i %i %i %i \n", controller.DPadUp, controller.DPadDown, controller.DPadLeft, controller.DPadRight);
	//Serial.printf("C: %i %i %i %i \n", controller.CUp, controller.CDown, controller.CLeft, controller.CRight);
	//Serial.printf("Y: %i X: %i\n", controller.yAxis, controller.xAxis);
	//Serial.print("C: ");
	//Serial.println(controller.CUp);

	//delay(500);
}
