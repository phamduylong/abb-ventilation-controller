/*
 * sco2GMP252.h
 *
 *  Created on: 17 Oct 2022
 *      Author: Danii
 */

#ifndef SCO2GMP252_H_
#define SCO2GMP252_H_

#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"

class sco2GMP252 {
public:
	sco2GMP252(unsigned int retries = 3);
	virtual ~sco2GMP252();
	bool read(float &data, bool retry = true);
	bool get_status();
	unsigned int get_elapsed_time();
private:
	ModbusMaster node;
	ModbusRegister co2_reg;
	ModbusRegister co2_reg2;
	unsigned int retries;
	bool status;
	unsigned int elapsed_time;
	float binary32_to_float(const unsigned int bin32);
};

#endif /* SCO2GMP252_H_ */
