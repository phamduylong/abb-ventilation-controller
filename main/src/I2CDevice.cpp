#include "I2CDevice.h"

I2CDevice::I2CDevice(I2C *i2c_interface, uint8_t dev_addr) : i2c_interface(i2c_interface), dev_addr(dev_addr) {}

I2CDevice::~I2CDevice() {}

bool I2CDevice::write(uint8_t com, uint8_t *trdata, const uint16_t size) {
	uint8_t arr[size + 1];
	arr[0] = com;
	for (unsigned int i = 1; i < (unsigned int)size + 1; i++) {
		arr[i] = trdata[i - 1];
	}
	return i2c_interface->write(this->dev_addr, arr, size + 1);
}

bool I2CDevice::read(uint8_t com, uint8_t *rdata, uint16_t size) {
	return i2c_interface->transaction(this->dev_addr, &com, 1, rdata, size);
}