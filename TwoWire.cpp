/*
 * TwoWire.cpp
 *
 * Created: 10/23/2015 3:48:57 PM
 *  Author: ekt
 */ 

#include "TwoWire.h"
#include <sercom.h>
#include <asf.h>

bool ModuloHandleDataRequested(uint8_t address, uint8_t *data);
bool ModuloHandleDataReceived(uint8_t address, uint8_t *data);

#define DATA_LENGTH 32
uint8_t read_buffer[DATA_LENGTH] = {};
uint8_t write_buffer[DATA_LENGTH] = {};
volatile uint8_t addr;
static uint8_t _broadcastAddress = 9;

struct i2c_slave_module i2c_slave_instance;
enum i2c_slave_direction dir;
struct i2c_slave_packet packet = {
	.data_length = DATA_LENGTH,
	.data = write_buffer
};
uint8_t deviceAddress = 0xFF;

bool isReadValid = false;

void i2c_read_request_callback(struct i2c_slave_module *module)
{
	packet.data_length = 0;
	packet.data = write_buffer;

	// Get the address that we received the request on.
	// The address is in the high 7 bits of addr.
	SercomI2cs *const i2c_hw = &(module->hw->I2CS);
	addr = i2c_hw->DATA.reg >> 1;
	
	if ((addr == _broadcastAddress or addr == deviceAddress) and isReadValid) {
		packet.data_length = DATA_LENGTH;
	}
	
	i2c_slave_write_packet_job(module, &packet);
}

void i2c_read_complete_callback(struct i2c_slave_module *module) {
	// XXX: Hack to get the actual number of received bytes, since the ASF i2c driver
	// doesn't give it to us. Inspect the second byte in the packet for the number
	// of data bytes. The total number of bytes is that plus 3 (for command code,
	// length byte, and CRC).
	// It would be much better to change the ASF i2c driver to provide the real
	// packet length.
	int readLen = read_buffer[1]+3;
	
	int sendLen = TwoWireCallback(addr, read_buffer, readLen, DATA_LENGTH);
	for (int i=0; i < sendLen and i < DATA_LENGTH; i++) {
		write_buffer[i] = read_buffer[i];
	}
	isReadValid = (sendLen > 0);
}

void i2c_write_request_callback(struct i2c_slave_module *const module)
{
	packet.data_length = DATA_LENGTH;
	packet.data = read_buffer;
	
	SercomI2cs *const i2c_hw = &(module->hw->I2CS);
	addr = i2c_hw->DATA.reg >> 1;
	asm("nop");
	
	if (addr == _broadcastAddress or addr == deviceAddress) {
		packet.data_length = DATA_LENGTH;
	} else {
		packet.data_length = 0;
	}
	
	if (i2c_slave_read_packet_job(module, &packet) != STATUS_OK) {
		
	}
	
}

void i2c_write_complete_callback(struct i2c_slave_module *const module) {
}



void TwoWireInit(uint8_t broadcastAddress, bool useInterrupts) {
	_broadcastAddress = broadcastAddress;
	
	struct i2c_slave_config config_i2c_slave;
	i2c_slave_get_config_defaults(&config_i2c_slave);
	config_i2c_slave.address = broadcastAddress;
	config_i2c_slave.address_mask = 0xFF;
	config_i2c_slave.address_mode = I2C_SLAVE_ADDRESS_MODE_MASK;
	config_i2c_slave.pinmux_pad0 = PINMUX_PA22C_SERCOM3_PAD0;
	config_i2c_slave.pinmux_pad1 = PINMUX_PA23C_SERCOM3_PAD1;
	
	i2c_slave_init(&i2c_slave_instance, SERCOM3, &config_i2c_slave);
	i2c_slave_enable(&i2c_slave_instance);
	
	i2c_slave_register_callback(&i2c_slave_instance,
	i2c_read_request_callback,
	I2C_SLAVE_CALLBACK_READ_REQUEST);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_READ_REQUEST);

	i2c_slave_register_callback(&i2c_slave_instance,
	i2c_read_complete_callback,
	I2C_SLAVE_CALLBACK_READ_COMPLETE);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_READ_COMPLETE);
	
	i2c_slave_register_callback(&i2c_slave_instance,
	i2c_write_request_callback,
	I2C_SLAVE_CALLBACK_WRITE_REQUEST);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_WRITE_REQUEST);

	i2c_slave_register_callback(&i2c_slave_instance,
	i2c_write_complete_callback,
	I2C_SLAVE_CALLBACK_WRITE_COMPLETE);
	i2c_slave_enable_callback(&i2c_slave_instance,
	I2C_SLAVE_CALLBACK_WRITE_COMPLETE);
}

void TwoWireSetDeviceAddress(uint8_t address) {
	deviceAddress = address;
}

uint8_t TwoWireGetDeviceAddress() {
	return deviceAddress;
}