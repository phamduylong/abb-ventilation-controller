/*
 * I2C.cpp
 *
 *  Created on: 29 Sep 2022
 *      Author: marin
 */

#include "I2C.h"

bool I2C::i2c_initialised = false;

/**
 * @brief Construct a new I2C::I2C object
 * Initialises I2C interface only once, on the first object creation.
 * Supports only LPC_I2C0
 * 
 * @param i2c_bitrate Speed of transfer Hz
 * @param i2c_clk_divider Divider for LPC frequency (72MHz).
 */
I2C::I2C(unsigned int i2c_bitrate, unsigned int i2c_clk_divider)
						: i2c_bitrate(i2c_bitrate), i2c_clk_divider(i2c_clk_divider)
{
	if(!i2c_initialised) {
		// Initialises pin muxing for I2C interface
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_DIGMODE_EN | 0);
		Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_DIGMODE_EN | 0);
		Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
		Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
		// Setup I2C handle and parameters
		Chip_I2C_Init(LPC_I2C0); // Enable I2C clock and reset I2C peripheral - the boot ROM does not do this
		Chip_I2C_SetClockDiv(LPC_I2C0, this->i2c_clk_divider); // Setup clock rate for I2C
		Chip_I2CM_SetBusSpeed(LPC_I2C0, this->i2c_bitrate); // Setup I2CM transfer rate
		Chip_I2CM_Enable(LPC_I2C0); // Enable Master Mode

		I2C::i2c_initialised = true;
	}
}

I2C::~I2C() {}

bool I2C::write(uint8_t devAddr, uint8_t *txBuffPtr, uint16_t txSize)
{
	return transaction(devAddr, txBuffPtr, txSize, nullptr, 0);
}

bool I2C::read(uint8_t devAddr, uint8_t *rxBuffPtr, uint16_t rxSize)
{
	return transaction(devAddr, nullptr, 0, rxBuffPtr, rxSize);
}


bool I2C::transaction(uint8_t devAddr, uint8_t *txBuffPtr, uint16_t txSize, uint8_t *rxBuffPtr, uint16_t rxSize) {
	I2CM_XFER_T  i2cmXferRec;

	// make sure that master is idle
	while(!Chip_I2CM_IsMasterPending(LPC_I2C0));

	/* Setup I2C transfer record */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	I2CM_XferBlocking(LPC_I2C0, &i2cmXferRec);
	// Chip_I2CM_XferBlocking returns before stop condition is fully completed
	// therefore we need to wait for master to be idle when doing back-to-back transactions (see beginning of the function)

	/* Test for valid operation */
	return !i2cmXferRec.status;
}

/* Transmit and Receive data in master mode */
/* This duplicates (and combines) the functionality of Chip_I2CM_Xfer and Chip_I2CM_XferBlocking with a modification
 * that allows us to do a zero length write (needed to use honeywell humidity/temp sensor)
 */
uint32_t I2C::I2CM_XferBlocking(LPC_I2C_T *pI2C, I2CM_XFER_T *xfer)
{
	uint32_t ret = 0;
	/* start transfer */
	/* set the transfer status as busy */
	xfer->status = I2CM_STATUS_BUSY;
	/* Clear controller state. */
	Chip_I2CM_ClearStatus(pI2C, I2C_STAT_MSTRARBLOSS | I2C_STAT_MSTSTSTPERR);
	/* Write Address and RW bit to data register */
	//Chip_I2CM_WriteByte(pI2C, (xfer->slaveAddr << 1) | (xfer->txSz == 0)); // original NXP version
	// krl : both read and write lenght is 0 --> write (for honeywell temp sensor)
	Chip_I2CM_WriteByte(pI2C, (xfer->slaveAddr << 1) | (xfer->txSz == 0 && xfer->rxSz != 0));
	/* Enter to Master Transmitter mode */
	Chip_I2CM_SendStart(pI2C);

	while (ret == 0) {
		/* wait for status change interrupt */
		while (!Chip_I2CM_IsMasterPending(pI2C)) {}
		/* call state change handler */
		ret = Chip_I2CM_XferHandler(pI2C, xfer);
	}
	return ret;
}