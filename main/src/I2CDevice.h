#ifndef I2CDEVICE_H_
#define I2CDEVICE_H_

#include "chip.h"
#include "board.h"
#include "I2C.h"

class I2CDevice {
public:
	/**
	 * @brief Construct a new I2CDevice::I2CDevice object
	 * 
	 * @param i2c_interface I2C interface to use for handling transmitions.
	 * @param dev_addr Slave device address.
	 */
	I2CDevice(I2C *i2c_interface, uint8_t dev_addr = 0x4d);
	I2CDevice(const I2CDevice &) = delete;
	virtual ~I2CDevice();

	/**
	 * @brief Writes data to specified register.
	 * 
	 * @param com Register to write data to. (command)
	 * @param trdata Pointer to data array to write into the register.
	 * @param size Size of an array.
	 * @return true (status returned 0, I2C transfer succeeded.)
	 * @return false (status returned non-zero, I2C transfer encountered an error.)
	 */
	bool write(uint8_t com, uint8_t *trdata, const uint16_t size);

	/**
 	* @brief Reads data from the specified register.
 	* 
 	* @param com Register to read from. (command)
 	* @param rdata Pointer to data array to put read value to.
 	* @param size Size of an array.
 	* @return true (status returned 0, I2C transfer succeeded.)
 	* @return false (status returned non-zero, I2C transfer encountered an error.)
 	*/
	bool read(uint8_t com, uint8_t *rdata, uint16_t size);
private:
	I2C *i2c_interface;
	uint8_t dev_addr = 0x4d;
};

#endif /* I2CDEVICE_H_ */
