
#include "Arduino.h"

#ifndef GAMEPAD_COUNT
#define GAMEPAD_COUNT 2
#endif
#define BUTTON_COUNT 12	 // SNES has 12, NES only has 8

//individual data pin for each controller
static const int DATA_PIN[GAMEPAD_COUNT] = {18, 19};  // grey, 20, 21 are the next logical ones

//shared pins between all controllers
#if defined(CONFIG_BT_ENABLED)
// ESP32
static const int LATCH_PIN = 16;  // brown
static const int CLOCK_PIN = 17;  // white
#else
// micro
static const int LATCH_PIN = 2;	 // brown
static const int CLOCK_PIN = 3;	 // white
#endif
// power red, ground black

#include "gamepad/Gamepad.h"

static const int translateToNES[12] = {1, 8, 2, 3, 4, 5, 6, 7, 0, 8, 8, 8};

class GameControllers {
   public:
	enum Type
	{
		NES = 0,
		SNES = 1,
	};

	enum Button
	{
		B = 0,
		Y = 1,
		SELECT = 2,
		START = 3,
		UP = 4,
		DOWN = 5,
		LEFT = 6,
		RIGHT = 7,
		A = 8,
		X = 9,
		L = 10,
		R = 11,
	};

	Type types[GAMEPAD_COUNT];
	int latchPin;
	int clockPin;
	int dataPins[GAMEPAD_COUNT];
	long buttons[GAMEPAD_COUNT][12];

	int changedControllers[GAMEPAD_COUNT];

	///This has to be initialized once for the shared pins latch and clock
	void init(int latch, int clock) {
		latchPin = latch;
		clockPin = clock;
		pinMode(latchPin, OUTPUT);
		digitalWrite(latchPin, LOW);
		pinMode(clockPin, OUTPUT);
		digitalWrite(clockPin, HIGH);
		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			for (int i = 0; i < 12; i++) {
				buttons[c][i] = -1;
			}
			types[c] = NES;	 // todo: if SNES button is ever pressed, change type to SNES, maybe buttons to manually toggle?
			dataPins[c] = -1;
		}
	}

	///This sets the controller type and initializes its individual data pin
	void setController(int controller, Type type, int dataPin) {
		types[controller] = type;
		dataPins[controller] = dataPin;
		pinMode(dataPins[controller], INPUT_PULLUP);
	}

	void poll(void (*controllerChanged)(const int controller)) {
		memset(changedControllers, 0, GAMEPAD_COUNT);

		digitalWrite(latchPin, HIGH);
		delayMicroseconds(12);
		digitalWrite(latchPin, LOW);
		delayMicroseconds(6);
		for (int i = 0; i < 12; i++) {
			for (int c = 0; c < GAMEPAD_COUNT; c++)
				if (dataPins[c] > -1) {
					if (digitalRead(dataPins[c])) {
						if (-1 != buttons[c][i]) {
							buttons[c][i] = -1;
							changedControllers[c] = 1;
						}
					} else {
						//++buttons[c][i];
						//changedControllers[c] = 1;
						if (0 != buttons[c][i]) {
							buttons[c][i] = 0;
							changedControllers[c] = 1;
						}
					}
				}
			digitalWrite(clockPin, LOW);
			delayMicroseconds(6);
			digitalWrite(clockPin, HIGH);
			delayMicroseconds(6);
		}

		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			// have any buttons changed state?
			if (1 == changedControllers[c]) {
				controllerChanged(c);
			}
		}
	}

	int translate(int controller, Button b) const {
		if (types[controller] == SNES)
			return b;
		return translateToNES[b];
	}

	///returns if button is currently down
	bool down(int controller, Button b) const {
		return buttons[controller][translate(controller, b)] >= 0;
	}
};

static const int translateToHid[12] = {
	BUTTON_B,
	BUTTON_Y,
	BUTTON_SELECT,
	BUTTON_START,
	DPAD_UP,
	DPAD_DOWN,
	DPAD_LEFT,
	DPAD_RIGHT,
	BUTTON_A,
	BUTTON_X,
	BUTTON_L,
	BUTTON_R,
};

GameControllers controllers;

GAMEPAD_CLASS gamepad;

void setup() {
	gamepad.begin();

	//initialize shared pins
	controllers.init(LATCH_PIN, CLOCK_PIN);

	//activate first controller ans set the type to SNES
	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		controllers.setController(c, GameControllers::NES, DATA_PIN[c]);
	}
}

/*
void controllerChangedDebug(const int c) {
	Serial.print("controllerChanged!!!!: ");
	Serial.println(c);
	Serial.print("c: ");
	Serial.print(c);
	for (uint8_t btn = 0; btn < 12; btn++) {
		Serial.print("; ");
		Serial.print(btn);
		Serial.print(", ");
		Serial.print(controllers.buttons[c][controllers.translate(c, static_cast<GameControllers::Button>(btn))]);
	}
	Serial.println("");
}
*/

void controllerChanged(const int c) {
	//controllerChangedDebug(c);

	gamepad.buttons(c, 0);
	// if start and select are held at the same time, send menu and only menu
	if (controllers.down(c, GameControllers::START) && controllers.down(c, GameControllers::SELECT)) {
		gamepad.press(c, BUTTON_MENU);
	} else {
		// actually send buttons held
		for (uint8_t btn = 0; btn < 12; btn++) {
			if (btn > 3 && btn < 8)
				continue;  // skip dpad
			if (controllers.down(c, static_cast<GameControllers::Button>(btn))) {
				gamepad.press(c, translateToHid[btn]);
			}
		}
	}
	if (controllers.down(c, GameControllers::DOWN)) {
		if (controllers.down(c, GameControllers::RIGHT)) {
			gamepad.setHatSync(c, DPAD_DOWN_RIGHT);
		} else if (controllers.down(c, GameControllers::LEFT)) {
			gamepad.setHatSync(c, DPAD_DOWN_LEFT);
		} else {
			gamepad.setHatSync(c, DPAD_DOWN);
		}
	} else if (controllers.down(c, GameControllers::UP)) {
		if (controllers.down(c, GameControllers::RIGHT)) {
			gamepad.setHatSync(c, DPAD_UP_RIGHT);
		} else if (controllers.down(c, GameControllers::LEFT)) {
			gamepad.setHatSync(c, DPAD_UP_LEFT);
		} else {
			gamepad.setHatSync(c, DPAD_UP);
		}
	} else if (controllers.down(c, GameControllers::RIGHT)) {
		gamepad.setHatSync(c, DPAD_RIGHT);
	} else if (controllers.down(c, GameControllers::LEFT)) {
		gamepad.setHatSync(c, DPAD_LEFT);
	} else {
		gamepad.setHatSync(c, DPAD_CENTERED);
	}
}

void loop() {
	if (gamepad.isConnected()) {
		controllers.poll(controllerChanged);  //read all controllers at once
	}
}
