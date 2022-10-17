/*
 * srhtHMP60.h
 *
 *  Created on: 17 Oct 2022
 *      Author: Danii
 */

#ifndef SRHTHMP60_H_
#define SRHTHMP60_H_

#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"

class srhtHMP60 {
public:
	srhtHMP60(unsigned int retries = 3);
	virtual ~srhtHMP60();
	bool read(float &temp, float &rhum, bool retry = true);
	bool read_temp(float &data, bool retry = true);
	bool read_rhum(float &data, bool retry = true);
	bool get_status();
	unsigned int get_elapsed_time();
ModbusMaster node;
	ModbusRegister rh0;
	ModbusRegister rh1;
	ModbusRegister t0;
	ModbusRegister t1;
	unsigned int retries;
	bool status;
	unsigned int elapsed_time;
	float binary32_to_float(const unsigned int bin32);
};

#endif /* SRHTHMP60_H_ */
