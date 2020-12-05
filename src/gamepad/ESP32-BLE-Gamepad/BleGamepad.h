#ifndef ESP32_BLE_GAMEPAD_H
#define ESP32_BLE_GAMEPAD_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "../common.h"

#include "BLE2902.h"
#include "BLEHIDDevice.h"
#include "HIDKeyboardTypes.h"
#include "HIDTypes.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <driver/adc.h>

#include "BLECharacteristic.h"
#include "BLEHIDDevice.h"
#include "BleConnectionStatus.h"

#if defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#define LOG_TAG ""
#else
#include "esp_log.h"
static const char *LOG_TAG = "BLEDevice";
#endif

class BleGamepad : public AbstractGamepad {
   private:
	BleConnectionStatus *connectionStatus;
	BLEHIDDevice *hid;
	BLECharacteristic *inputGamepad[GAMEPAD_COUNT];
	void rawAction(uint8_t msg[], char msgSize);
	static void taskServer(void *pvParameter) {
		BleGamepad *BleGamepadInstance = (BleGamepad *)pvParameter;	 // static_cast<BleGamepad *>(pvParameter);
		BLEDevice::init(BleGamepadInstance->deviceName);
		BLEServer *pServer = BLEDevice::createServer();
		pServer->setCallbacks(BleGamepadInstance->connectionStatus);

		BleGamepadInstance->hid = new BLEHIDDevice(pServer);

		for (int c = 0; c < GAMEPAD_COUNT; c++) {
			BleGamepadInstance->inputGamepad[c] = BleGamepadInstance->hid->inputReport(c);	// <-- input REPORTID from report map
			BleGamepadInstance->connectionStatus->inputGamepad[c] = BleGamepadInstance->inputGamepad[c];
		}

		BleGamepadInstance->hid->manufacturer()->setValue(BleGamepadInstance->deviceManufacturer);

		// todo: is this change required for 2 inputs???
		// BleGamepadInstance->hid->pnp(0x01,0x02e5,0xabcc,0x0110);
		BleGamepadInstance->hid->pnp(0x02, 0x8282, 0x0132, 0x0106);

		BleGamepadInstance->hid->hidInfo(0x00, 0x01);

		BLESecurity *pSecurity = new BLESecurity();

		pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

		BleGamepadInstance->hid->reportMap((uint8_t *)_hidReportDescriptor, sizeof(_hidReportDescriptor));
		BleGamepadInstance->hid->startServices();

		BleGamepadInstance->onStarted(pServer);

		BLEAdvertising *pAdvertising = pServer->getAdvertising();
		pAdvertising->setAppearance(HID_GAMEPAD);
		pAdvertising->addServiceUUID(BleGamepadInstance->hid->hidService()->getUUID());
		pAdvertising->start();
		BleGamepadInstance->hid->setBatteryLevel(BleGamepadInstance->batteryLevel);

		ESP_LOGD(LOG_TAG, "Advertising started!");
		vTaskDelay(portMAX_DELAY);	// delay(portMAX_DELAY);
	}

   public:
	uint8_t batteryLevel;
	std::string deviceManufacturer;
	std::string deviceName;

	BleGamepad(std::string deviceName = "8BitDo SN30 Pro+", std::string deviceManufacturer = "8Bitdo SF30 Pro", uint8_t batteryLevel = 100) : AbstractGamepad(), hid(0) {
		this->deviceName = deviceName;
		this->deviceManufacturer = deviceManufacturer;
		this->batteryLevel = batteryLevel;
		this->connectionStatus = new BleConnectionStatus();
	}

	void begin(void) {
		xTaskCreate(this->taskServer, "server", 20000, (void *)this, 5, NULL);
	}

	void sync(const uint8_t cIdx) {
		if (this->isConnected()) {
			this->inputGamepad[cIdx]->setValue(gamepadReport, GAMEPAD_REPORT_LEN);
			this->inputGamepad[cIdx]->notify();
		}
	}

	bool isConnected(void) {
		return this->connectionStatus->connected;
	}

	void setBatteryLevel(uint8_t level) {
		this->batteryLevel = level;
		if (hid != 0)
			this->hid->setBatteryLevel(this->batteryLevel);
	}

   protected:
	virtual void onStarted(BLEServer *pServer){};
};

typedef BleGamepad Gamepad;

#endif	// CONFIG_BT_ENABLED
#endif	// ESP32_BLE_GAMEPAD_H
