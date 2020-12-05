#include "BleConnectionStatus.h"

BleConnectionStatus::BleConnectionStatus(void) {
}

void BleConnectionStatus::onConnect(BLEServer *pServer) {
	this->connected = true;
	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		BLE2902 *desc = (BLE2902 *)this->inputGamepad[c]->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
		desc->setNotifications(true);
	}
}

void BleConnectionStatus::onDisconnect(BLEServer *pServer) {
	this->connected = false;
	for (int c = 0; c < GAMEPAD_COUNT; c++) {
		BLE2902 *desc = (BLE2902 *)this->inputGamepad[c]->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
		desc->setNotifications(false);
	}
}
