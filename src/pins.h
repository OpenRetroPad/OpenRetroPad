
// this let's me keep the db-25 pin mapping naming the same across all inputs/outputs

#ifndef OR_PINS_H
#define OR_PINS_H

#if defined(ARDUINO_ARCH_ESP32)
// esp32
#define OR_PIN_1 1
#define OR_PIN_2 21
#define OR_PIN_3 22
#define OR_PIN_4 15
#define OR_PIN_5 16
#define OR_PIN_6 2
#define OR_PIN_7 17
#define OR_PIN_8 4
#define OR_PIN_9 35
#define OR_PIN_10 32
#define OR_PIN_11 3
#define OR_PIN_14 12
#define OR_PIN_15 14
#define OR_PIN_16 13
#define OR_PIN_18 27
#define OR_PIN_19 26
#define OR_PIN_20 25
#define OR_PIN_21 33
#else
// micro
#define OR_PIN_1 1
#define OR_PIN_2 2
#define OR_PIN_3 3
#define OR_PIN_4 4
#define OR_PIN_5 5
#define OR_PIN_6 6
#define OR_PIN_7 7
#define OR_PIN_8 8
#define OR_PIN_9 9
#define OR_PIN_10 10
#define OR_PIN_11 0
#define OR_PIN_14 14
#define OR_PIN_15 15
#define OR_PIN_16 16
#define OR_PIN_18 18
#define OR_PIN_19 19
#define OR_PIN_20 20
#define OR_PIN_21 21
#endif	// ARDUINO_ARCH_ESP32

// currently unused, reserved for future use
//#define OR_PIN_12  -
//#define OR_PIN_13  -
//#define OR_PIN_17  -
//#define OR_PIN_22  -

#endif	// OR_PINS_H
