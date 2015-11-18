/*
 * SelfProgram.h
 *
 * Created: 11/12/2015 10:23:28 AM
 *  Author: ekt
 */ 


#ifndef SELFPROGRAM_H_
#define SELFPROGRAM_H_

#include <inttypes.h>

class SelfProgram {
public:
	SelfProgram();

	void setSafeMode(bool safeMode);

	uint32_t getSignature();

	void readEEPROM(uint8_t *data, void *address, int eeLen);

	void writeEEPROM(uint8_t *data, void *address, int len);

	void setLED(bool on);

	int getPageSize();

	void erasePage(uint32_t address);

	int readPage(uint32_t address, uint8_t *data, uint8_t len);

	void writePage(uint32_t address, uint8_t *data, uint8_t len);

	void startApplication();
	
	void checkBootMode();

	void jumpToApplication();

private:
	uint16_t _deviceID;
	bool _safeMode;	
};

#endif /* SELFPROGRAM_H_ */