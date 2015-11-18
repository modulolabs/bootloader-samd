/*
 * SelfProgram.cpp
 *
 * Created: 11/13/2015 10:22:52 AM
 *  Author: ekt
 */ 

#include "SelfProgram.h"
#include "ModuloInfo.h"
#include "asf.h"

#define APP_START_ADDRESS          0x00002000
#define LED_PIN 15

SelfProgram::SelfProgram() : _deviceID(0xFFFF), _safeMode(true)  {
}

void SelfProgram::setSafeMode(bool safeMode) {
	_safeMode = safeMode;
}

uint32_t SelfProgram::getSignature() {
	return system_get_device_id();
}

void SelfProgram::readEEPROM(uint8_t *data, void *address, int eeLen) {

}

void SelfProgram::writeEEPROM(uint8_t *data, void *address, int len) {

}

void SelfProgram::setLED(bool on) {
	struct port_config config_port_pin;
	port_get_config_defaults(&config_port_pin);
	config_port_pin.direction = PORT_PIN_DIR_OUTPUT;
	port_pin_set_config(LED_PIN, &config_port_pin);
	port_pin_set_output_level(LED_PIN, true);
	
	port_pin_set_output_level(LED_PIN, on);
}

int SelfProgram::getPageSize() {
	return NVMCTRL_PAGE_SIZE;
}

void SelfProgram::erasePage(uint32_t address) {
}

uint8_t pageBuffer[64];
uint32_t pageAddress;

int SelfProgram::readPage(uint32_t address, uint8_t *data, uint8_t len) {
	for (int i=0; i < len; i++) {
		data[i] = ((uint8_t*)address)[i];
	}
	
	return len;
}



void SelfProgram::writePage(uint32_t address, uint8_t *buffer, uint8_t len) {
	if (_safeMode and (address < APP_START_ADDRESS or address+len >= 256*1024)) {
		return;
	}
	
	// If the lower 6 bits are 0, this is the start of a page
	if ((address % 64) == 0) {
		pageAddress = address;
	}
	
	// The address must be in the same page that we're currently writing
	if ((address & ~0x3F) != pageAddress) {
		return;
	}
	
	// If the lower 8 bits are 0, this is the start of a row. Erase the row.
	if ((address % 256) == 0) {
		/* Erase row */
		nvm_erase_row(address);
	}

	// Store the bytes in the page buffer
	for (int i=0; i < len; i++) {
		pageBuffer[(address+i)%64] = buffer[i];
	}
	
	// If this is the end of the page, write it
	if (((address+len) % 64) == 0) {
		// Write page
		nvm_write_buffer(address & ~0x3F, pageBuffer, 64);
	}	
}

void SelfProgram::checkBootMode() {
	/* Check if WDT is locked */
#if (SAMD) || (SAMR21)
	if (!(WDT->CTRL.reg & WDT_CTRL_ALWAYSON)) {
		/* Disable the Watchdog module */
		WDT->CTRL.reg &= ~WDT_CTRL_ENABLE;
	}
#else
	if (!(WDT->CTRLA.reg & WDT_CTRLA_ALWAYSON)) {
		/* Disable the Watchdog module */
		WDT->CTRLA.reg &= ~WDT_CTRLA_ENABLE;
	}
#endif

	/* Check the BOOT pin or the reset cause is Watchdog */
#if (SAMD) || (SAMR21)
	if (PM->RCAUSE.reg & PM_RCAUSE_WDT) {
#else
	if (RSTC->RCAUSE.reg & RSTC_RCAUSE_WDT) {
#endif
		jumpToApplication();		
	}

}

void SelfProgram::jumpToApplication() {
	setLED(false);
	
	uint32_t app_check_address = APP_START_ADDRESS;
	uint32_t *app_check_address_ptr = (uint32_t *) app_check_address;

	/*
		* Read the first location of application section
		* which contains the address of stack pointer.
		* If it is 0xFFFFFFFF then the application section is empty.
		*/
	if (*app_check_address_ptr == 0xFFFFFFFF) {
		while (1) {
		}
	}
	/* Pointer to the Application Section */
	void (*application_code_entry)(void);

	/* Rebase the Stack Pointer */
	__set_MSP(*(uint32_t *) APP_START_ADDRESS);

	/* Rebase the vector table base address */
	SCB->VTOR = ((uint32_t) APP_START_ADDRESS & SCB_VTOR_TBLOFF_Msk);

	/* Load the Reset Handler address of the application */
	application_code_entry = (void (*)(void))(unsigned *)(*(unsigned *)
			(APP_START_ADDRESS + 4));

	/* Jump to user Reset Handler in the application */
	application_code_entry();
}

void SelfProgram::startApplication() {
	struct wdt_conf wdt_config;

	/* Get WDT default configuration */
	wdt_get_config_defaults(&wdt_config);

	/* Set the required clock source and timeout period */
#if (SAMD) || (SAMR21)
	wdt_config.clock_source   = GCLK_GENERATOR_0;
#endif
	wdt_config.timeout_period = WDT_PERIOD_256CLK;

	/* Initialize and enable the Watchdog with the user settings */
	wdt_set_config(&wdt_config);

	while (1) {
		/* Wait for watchdog reset */
	}
}