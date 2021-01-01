/*
 Copyright (c) 2014-present PlatformIO <contact@platformio.org>

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
**/

#include <cstdio>

#include <unity.h>

#define DATA_SIZE 800  // number of sample points to poll

// buffer to hold data being read from controller
bool buffer[DATA_SIZE];

#define NUM_BITS 32

// bit resolution and offsets
int bitOffsets[NUM_BITS];
int bitResolution = 0;
int bitsToRead = DATA_SIZE;

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

ControllerData controller;

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
	}
}

/* Function to extract a controller bit from the buffer of returned data */
void getBit(bool *bit, int offset, bool *data) {
	*bit = data[offset];
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
	}

// print axis values
#ifdef DEBUG
	printf("yRaw: %i %i %i %i %i %i %i %i xRaw: %i %i %i %i %i %i %i %i yAxis: %03i xAxis: %03i",
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

	/*
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
    */

	//printf("xaxis: %-3i yaxis: %-3i \n",data->xAxis,data->yAxis);
}

void load(const char *s) {
	const char *t;
	int i = 0;

	// convert test string to buffer
	for (t = s; *t != '\0'; t++) {
		buffer[i++] = *t == '1';
	}
}

void loadUpdate(const char *s) {
	load(s);

	updateOffsetsAndResolution();

	/*
	for (int i = 0; i < DATA_SIZE; i++) {
		printf(buffer[i] ? "1" : "0");
	}
	printf("\n----------------\n");
	*/
	printf("bitOffsets: ");
	for (int i = 0; i < NUM_BITS; i++) {
		//printf("%i ", bitOffsets[i]);
		printf("%i:%i ", i, bitOffsets[i]);
	}
	printf("\n");
	printf("bitResolution: %i\n", bitResolution);
	printf("bit 0: %i\n", buffer[bitOffsets[0]]);
	printf("bit buffer[14]: %i\n", buffer[14]);
	printf("bit buffer[15]: %i\n", buffer[15]);
	printf("bit buffer[16]: %i\n", buffer[16]);
}

void reset() {
	// reset all bitOffsets to 0 which is invalid
	for (int i = 0; i < NUM_BITS; i++) {
		bitOffsets[i] = 0;
	}
	bitResolution = 0;
	bitsToRead = DATA_SIZE;
}

void print(const char *s) {
	printf(s);
}

// void setUp(void) {
// // set stuff up here
// }

// void tearDown(void) {
// // clean stuff up here
// }

/*
*/
void test_function_updateOffsetsAndResolution(void) {
	reset();
	loadUpdate(
		"0000000000000000111110000000000000000011110000000000000000011111000000000000000011111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111"
		"1000000000000000001111000000000000000001111100000000000000001111100000000000000000111100000000000000000111100000000000000000111110000000000000000111110000000000000000011110000000000000000011"
		"1100000000000000000111110000000000000000111110000000000000000011110000000000000000011110000000000000000011111000000000000000011111000000000000000001111000000000000000001111000000000000000001"
		"1111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111100000000000111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");

	TEST_ASSERT_EQUAL(5, bitResolution);

	loadUpdate(
		"1000000000000001111110000000000000000011110000000000000000011111000000000000000011111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111"
		"1000000000000000001111000000000000000001111100000000000000001111100000000000000000111100000000000000000111100000000000000000111110000000000000000111110000000000000000011110000000000000000011"
		"1100000000000000000111110000000000000000111110000000000000000011110000000000000000011110000000000000000011111000000000000000011111000000000000000001111000000000000000001111000000000000000001"
		"1111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111100000000000111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");

	TEST_ASSERT_EQUAL(6, bitResolution);

	loadUpdate(
		"1000000000000000001111000000000000000011110000000000000000011111000000000000000011111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111"
		"1000000000000000001111000000000000000001111100000000000000001111100000000000000000111100000000000000000111100000000000000000111110000000000000000111110000000000000000011110000000000000000011"
		"1100000000000000000111110000000000000000111110000000000000000011110000000000000000011110000000000000000011111000000000000000011111000000000000000001111000000000000000001111000000000000000001"
		"1111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111100000000000111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");

	TEST_ASSERT_EQUAL(6, bitResolution);

	calcBitsToRead();
	TEST_ASSERT_EQUAL(670, bitsToRead);

	bool buttonA = false;

	getBit(&buttonA, bitOffsets[0], &buffer[0]);

	TEST_ASSERT_EQUAL(false, buttonA);

	load(
		"0000001111111111111110000000000000000011110000000000000000011111000000000000000011111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111"
		"1000000000000000001111000000000000000001111100000000000000001111100000000000000000111100000000000000000111100000000000000000111110000000000000000111110000000000000000011110000000000000000011"
		"1100000000000000000111110000000000000000111110000000000000000011110000000000000000011110000000000000000011111000000000000000011111000000000000000001111000000000000000001111000000000000000001"
		"1111000000000000000011111000000000000000001111000000000000000001111100000000000000001111100000000000000001111100000000000111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");

	getBit(&buttonA, bitOffsets[0], &buffer[0]);

	TEST_ASSERT_EQUAL(true, buttonA);
}

void test_function_realControllerRead(void) {
	reset();
	loadUpdate(
		"0000000000000011111000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000111110000000000000001111100000000000000011111000000000000"
		"0001111100000000000000011111000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000011100000"
		"0000000000111100000000000000011110000000000000000111100000000000000001111000000000000000111110000000000000001111100000000000000011111000000000000000111110000000000000001111100000000000000011"
		"1100000000000000001111000000000000000011110000000000000000111100000000000111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");
	loadUpdate(
		"0000000000000011111000000000000000111110000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000"
		"0000111100000000000000001111000000000000000111110000000000000001111100000000000000011111000000000000000111110000000000000001111100000000000000011110000000000000000111100000000000000001111000"
		"0000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000011111000000000000000111110000000000000"
		"0011111000000000000000111110000000000000001111000000000000000011110000000000011111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");
	loadUpdate(
		"0000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000"
		"0000011111000000000000000111110000000000000001111100000000000000011111000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110"
		"0000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000111110000000000000001111100000000000000011111000000000000000111110000000000000001111100000000000"
		"0000111100000000000000001111000000000000000011110000000000000000111100000000000111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");
	loadUpdate(
		"0000000000000001111100000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000"
		"0000011110000000000000000111100000000000000011111000000000000000111110000000000000001111100000000000000011111000000000000000111100000000000000001111000000000000000011110000000000000000111100"
		"0000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000111110000000000000001111100000000000000011111000000000000"
		"0001111100000000000000011110000000000000000111100000000000000001111000000000001111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");
	loadUpdate(
		"0000000000000011111000000000000000111110000000000000001111100000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000"
		"0000111100000000000000001111000000000000000011110000000000000001111100000000000000011111000000000000000111110000000000000001111100000000000000011111000000000000000111100000000000000001111000"
		"0000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000111110000000000000"
		"0011111000000000000000111110000000000000001111100000000000000011110000000000011111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111"
		"1111111111111111111111111111111111111111");

	TEST_ASSERT_EQUAL(5, bitResolution);

	calcBitsToRead();
	TEST_ASSERT_EQUAL(624, bitsToRead);

	load(
		"0000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000001111000000000000000011110000000000000000111100000000000000011111000000000000"
		"0001111100000000000000011111000000000000000111110000000000000001111100000000000000011110000000000000000111100000000000000001111000000111111111111110000001111111111111100000011111111111111000"
		"0000000000000111100000000000000001111000000111111111111110000001111111111111100000000000000011111000000000000000111110000011111111111111100000000000000011111000000000000000111100000000000000"
		"001111000000111111111111110000001111111111111100000011");

	bool buttonA = true;

	getBit(&buttonA, bitOffsets[0], &buffer[0]);

	TEST_ASSERT_EQUAL(false, buttonA);

	populateControllerStruct(&controller);

	print(" buttons: ");
	/*
	*/
	print(controller.buttonA ? "A" : "-");
	print(controller.buttonB ? "B" : "-");
	print(controller.buttonZ ? "Z" : "-");
	print(controller.buttonL ? "L" : "-");
	print(controller.buttonR ? "R" : "-");
	print(controller.buttonStart ? "S" : "-");
	print(" DPAD: ");
	print(controller.DPadUp ? "U" : "-");
	print(controller.DPadDown ? "D" : "-");
	print(controller.DPadLeft ? "L" : "-");
	print(controller.DPadRight ? "R" : "-");
	print(" C: ");
	print(controller.CUp ? "U" : "-");
	print(controller.CDown ? "D" : "-");
	print(controller.CLeft ? "L" : "-");
	print(controller.CRight ? "R" : "-");
	print(" Y: ");
	//print(controller.yAxis);
	//print(" YT: ");
	//print(translateAxis(controller.yAxis));
	print(" X: ");
	//print(controller.xAxis);
	//print(" XT: ");
	//print(translateAxis(controller.xAxis));
	printf("\n");
}

int main(int argc, char **argv) {
	UNITY_BEGIN();
	//RUN_TEST(test_function_updateOffsetsAndResolution);
	RUN_TEST(test_function_realControllerRead);
	UNITY_END();

	return 0;
}
