#ifndef SPRESSURESDP610_H_
#define SPRESSURESDP610_H_

#include "chip.h"
#include "I2C.h"
#include "I2CDevice.h"
#include <stdio.h>

uint8_t crc8(uint8_t *data, size_t size);

class sPressureSDP610 {
public:
	sPressureSDP610(unsigned int retries = 3, unsigned int wait = 100);
	virtual ~sPressureSDP610();
	bool read(float &data, bool retry = true);
	bool get_status();
	unsigned int get_elapsed_time();
private:
	I2C i2c;
	I2CDevice sdp610;
	const unsigned int retries;
	const unsigned int wait;
	bool status;
	unsigned int elapsed_time;
};

#endif /* SPRESSURESDP610_H_ */
