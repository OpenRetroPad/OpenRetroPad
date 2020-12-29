
// this let's me keep the db-25 pin mapping naming the same across all inputs/outputs

#ifndef OR_PINS_H
#define OR_PINS_H

#if defined(ARDUINO_ARCH_ESP32)
// esp32
#define OR_PIN_1 15
#define OR_PIN_2 2
#define OR_PIN_3 4
#define OR_PIN_4 16
#define OR_PIN_5 17
#define OR_PIN_6 5
#define OR_PIN_7 18
#define OR_PIN_8 19
#define OR_PIN_9 35
#define OR_PIN_10 3
#define OR_PIN_11 1
//#define OR_PIN_12 22
//#define OR_PIN_13 23
#define OR_PIN_14 12
#define OR_PIN_15 14
#define OR_PIN_16 13
//#define OR_PIN_17 27
#define OR_PIN_18 26
#define OR_PIN_19 25
#define OR_PIN_20 33
#define OR_PIN_21 32
//#define OR_PIN_22 22
//#define OR_PIN_23 23
//#define OR_PIN_24 24
//#define OR_PIN_25 25
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
//#define OR_PIN_12 12
//#define OR_PIN_13 13
#define OR_PIN_14 14
#define OR_PIN_15 15
#define OR_PIN_16 16
//#define OR_PIN_17 17
#define OR_PIN_18 18
#define OR_PIN_19 19
#define OR_PIN_20 20
#define OR_PIN_21 21
//#define OR_PIN_22 22
//#define OR_PIN_23 23
//#define OR_PIN_24 24
//#define OR_PIN_25 25
#endif	// ARDUINO_ARCH_ESP32

#endif	// OR_PINS_H
