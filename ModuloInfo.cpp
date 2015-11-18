/*
 * DeviceID.cpp
 *
 * Created: 10/26/2015 10:17:13 AM
 *  Author: ekt
 */ 

#include "ModuloInfo.h"
#include "asf.h"

// Reserve space for the Modulo info in nvm space. The application will overwrite it
// so that when the bootloader runs it can determine the type, device ID, and version used
// by the application
ModuloInfo moduloInfo __attribute__((section(".moduloInfo"))) = {.id=0xFFFF, .version=0xFFFF, ""};
		
// A copy of _nvmModuloInfo in ordinary ram.
static ModuloInfo _localModuloInfo = {0xFFFF, 0xFFFF, ""};
	
// The current device ID. If it was 0xFFFF or 0 in the moduloInfo, then this will be a generated
// number from the 
static uint16_t _deviceID = 0xFFFF;

static uint16_t _generateDeviceID() {
	// Extract the serial number from the specific addresses according to the data sheet
	uint32_t serialNum[] = {
		*(uint32_t*)(0x0080A00C),
		*(uint32_t*)(0x0080A040),
		*(uint32_t*)(0x0080A044),
	    *(uint32_t*)(0x0080A048)};
	
	// Hash the 128 bit unique ID into a 16 bit unique-ish ID
	uint16_t deviceID = 0;
	for (int i=0; i < 4; i++) {
		deviceID ^= (serialNum[i] & 0xFFFF);
		deviceID ^= (serialNum[i] >> 16);
	}
	
	// Ensure that the generated device ID is never invalid
	if (deviceID == 0 or deviceID == 0xFFFF) {
		deviceID = 1;
	}
	
	return deviceID;
}

void LoadModuloInfo() {
	_localModuloInfo = moduloInfo;

	if (_localModuloInfo.id == 0xFFFF or _localModuloInfo.id == 0) {
		// Valid id not found in the persistent info. Generate it.
		_deviceID = _generateDeviceID();
	} else {
		_deviceID = _localModuloInfo.id;
	}
}

uint16_t GetDeviceID() {
	return _deviceID;
}

void SetDeviceID(uint16_t deviceID) {
	_deviceID = deviceID;
	_localModuloInfo.id = deviceID;
	
	nvm_erase_row((uint32_t)&moduloInfo);
	nvm_write_buffer((uint32_t)&moduloInfo, (uint8_t*)&_localModuloInfo, sizeof(ModuloInfo));
}

uint8_t GetModuloType(uint8_t i) {
	if (i < MODULO_TYPE_SIZE) {
		return _localModuloInfo.type[i];
	}
	return 0;
}

uint16_t GetModuloVersion() {
	return _localModuloInfo.version;
}
