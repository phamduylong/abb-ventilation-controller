#include "I2CDevice.h"

/**
 * @brief Construct a new I2CDevice::I2CDevice object
 * 
 * @param i2c_interface I2C interface to use for handling transmitions.
 * @param dev_addr Slave device address.
 */
I2CDevice::I2CDevice(I2C *i2c_interface, uint8_t dev_addr) : i2c_interface(i2c_interface), dev_addr(dev_addr) {}

I2CDevice::~I2CDevice() {}

/**
 * @brief Writes data to specified register.
 * 
 * @param com Register to write data to. (command)
 * @param trdata Pointer to data array to write into the register.
 * @param size Size of an array.
 * @return true (status returned 0, I2C transfer succeeded.)
 * @return false (status returned non-zero, I2C transfer encountered an error.)
 */
bool I2CDevice::write(uint8_t com, uint8_t *trdata, const uint16_t size) {
	uint8_t arr[size + 1];
	arr[0] = com;
	for (unsigned int i = 1; i < (unsigned int)size + 1; i++) {
		arr[i] = trdata[i - 1];
	}
	return i2c_interface->write(this->dev_addr, arr, size + 1);
}

/**
 * @brief Reads data from the specified register.
 * 
 * @param com Register to read from. (command)
 * @param rdata Pointer to data array to put read value to.
 * @param size Size of an array.
 * @return true (status returned 0, I2C transfer succeeded.)
 * @return false (status returned non-zero, I2C transfer encountered an error.)
 */
bool I2CDevice::read(uint8_t com, uint8_t *rdata, uint16_t size) {
	return i2c_interface->transaction(this->dev_addr, &com, 1, rdata, size);
}