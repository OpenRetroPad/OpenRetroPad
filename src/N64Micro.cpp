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

#define DATA_PIN 2

// how often to poll, 100? 14? polling must not occur faster than every 20 ms
#define POLL_DELAY 14

#include "Arduino.h"

#include "Nintendo.h"

//#define DEBUG

#define AXIS_CENTER_IN 0
#define AXIS_MAX_IN 100
#define AXIS_MIN_IN -100

#include "gamepad/Gamepad.h"
#include "util.cpp"

// Define a N64 Controller
CN64Controller N64Controller(DATA_PIN);

#define REPORT_SIZE 4
uint8_t oldReport[REPORT_SIZE];

GAMEPAD_CLASS gamepad;

#ifdef DEBUG
void print_n64_report(N64_Report_t &controller, N64_Status_t &n64_status) {
	Serial.print("buttons: ");
	Serial.print(controller.a ? "A" : "-");
	Serial.print(controller.b ? "B" : "-");
	Serial.print(controller.z ? "Z" : "-");
	Serial.print(controller.l ? "L" : "-");
	Serial.print(controller.r ? "R" : "-");
	Serial.print(controller.start ? "S" : "-");
	Serial.print(" DPAD: ");
	Serial.print(controller.dup ? "U" : "-");
	Serial.print(controller.ddown ? "D" : "-");
	Serial.print(controller.dleft ? "L" : "-");
	Serial.print(controller.dright ? "R" : "-");
	Serial.print(" C: ");
	Serial.print(controller.cup ? "U" : "-");
	Serial.print(controller.cdown ? "D" : "-");
	Serial.print(controller.cleft ? "L" : "-");
	Serial.print(controller.cright ? "R" : "-");
	Serial.print(" Y: ");
	Serial.print(controller.yAxis);
	Serial.print(" YT: ");
	Serial.print(translateAxis(-controller.yAxis));
	Serial.print(" X: ");
	Serial.print(controller.xAxis);
	Serial.print(" XT: ");
	Serial.print(translateAxis(controller.xAxis));
	Serial.println();
	/*
  // Print device information
  Serial.print(F("Device: "));
  switch (n64_status.device) {
    case NINTENDO_DEVICE_N64_NONE:
      Serial.println(F("No N64 Controller found!"));
      return;
      break;
    case NINTENDO_DEVICE_N64_WIRED:
      Serial.println(F("Original Nintendo N64 Controller"));
      break;

    default:
      Serial.print(F("Unknown "));
      Serial.println(n64_status.device, HEX);
      break;
  }
*/
}
#endif

void setup() {
#ifdef DEBUG
	Serial.begin(115200);
	if (N64Controller.begin()) {
		Serial.println(F("N64 begin() success."));
	} else {
		Serial.println(F("N64 begin() fail."));
	}
#else
	N64Controller.begin();
#endif

	gamepad.begin();
}

void loop() {
	delay(POLL_DELAY);
	// Try to read the controller data
	if (N64Controller.read()) {
		// Print Controller information
		auto controller = N64Controller.getReport();

		if (memcmp(oldReport, controller.raw8, REPORT_SIZE)) {
			memcpy(oldReport, controller.raw8, REPORT_SIZE);
		} else {
			// nothing changed
			return;
		}

#ifdef DEBUG
		auto status = N64Controller.getStatus();
		print_n64_report(controller, status);
#endif
		uint8_t c = 0;	// for now just do 1 pad
		gamepad.buttons(c, 0);
		if (controller.start) {
			if (controller.ddown) {
				// then only send menu, nothing else
				gamepad.press(c, BUTTON_MENU);
				gamepad.setHatSync(c, DPAD_CENTER);
				return;
			}
			gamepad.press(c, BUTTON_START);
		}
		if (controller.a) {
			gamepad.press(c, BUTTON_A);
		}
		if (controller.b) {
			gamepad.press(c, BUTTON_B);
		}
		if (controller.z) {
			gamepad.press(c, BUTTON_TR);
		}
		if (controller.l) {
			gamepad.press(c, BUTTON_L);
		}
		if (controller.r) {
			gamepad.press(c, BUTTON_R);
		}
		auto hat = calculateDpadDirection(controller.dup, controller.ddown, controller.dleft, controller.dright);
		auto cHat = dpadToAxis(calculateDpadDirection(controller.cup, controller.cdown, controller.cleft, controller.cright));
		gamepad.setAxis(c, translateAxis(controller.xAxis), translateAxis(-controller.yAxis), cHat.x, cHat.y, 0, 0, hat);
	} else {
		// Add debounce if reading failed
		delay(5000);
#ifdef DEBUG
		Serial.println(F("Error reading N64 controller."));
		if (N64Controller.begin()) {
			Serial.println(F("N64 begin() success."));
		} else {
			Serial.println(F("N64 begin() fail."));
		}
#else
		N64Controller.begin();
#endif
	}
}
