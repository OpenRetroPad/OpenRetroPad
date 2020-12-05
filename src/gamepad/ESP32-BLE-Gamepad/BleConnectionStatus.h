#ifndef ESP32_BLE_CONNECTION_STATUS_H
#define ESP32_BLE_CONNECTION_STATUS_H
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLE2902.h"
#include "BLECharacteristic.h"
#include <BLEServer.h>

class BleConnectionStatus : public BLEServerCallbacks {
   public:
	BleConnectionStatus(void);
	bool connected = false;
	void onConnect(BLEServer *pServer);
	void onDisconnect(BLEServer *pServer);
	BLECharacteristic *inputGamepad[GAMEPAD_COUNT];
};

#endif	// CONFIG_BT_ENABLED
#endif	// ESP32_BLE_CONNECTION_STATUS_H
