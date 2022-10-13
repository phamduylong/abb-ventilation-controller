/*
 * I2C.h
 *
 *  Created on: 29 Sep 2022
 *      Author: marin
 */

#ifndef I2C_H_
#define I2C_H_

#include "chip.h"
#include "board.h"

class I2C {
public:
	I2C(unsigned int i2c_bitrate = 55000, unsigned int i2c_clk_divider = 1309);
	I2C(const I2C &) = delete;
	virtual ~I2C();
	bool transaction(uint8_t devAddr, uint8_t *txBuffPtr, uint16_t txSize, uint8_t *rxBuffPtr, uint16_t rxSize);
	bool write(uint8_t devAddr, uint8_t *txBuffPtr, uint16_t txSize);
	bool read(uint8_t devAddr, uint8_t *rxBuffPtr, uint16_t rxSize);
private:
	static uint32_t I2CM_XferBlocking(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer);
	//Members
	unsigned int i2c_bitrate = 55000; // 55000Hz = 55kHz
	unsigned int i2c_clk_divider = 1309; // 72000kHz / 1309 ~= 55kHz
	static bool i2c_initialised; //flag to check if i2c was initialised by some other instance of this class.
};

#endif /* I2C_H_ */
