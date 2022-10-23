#ifndef SRHTHMP60_H_
#define SRHTHMP60_H_

#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"

class srhtHMP60 {
public:
	srhtHMP60(unsigned int retries = 3, unsigned int wait = 100);
	virtual ~srhtHMP60();
	bool read(float &temp, float &rhum, bool retry = true);
	bool read_temp(float &data, bool retry = true);
	bool read_rhum(float &data, bool retry = true);
	bool get_status();
	unsigned int get_elapsed_time();
private:
	ModbusMaster node;
	ModbusRegister rh0;
	ModbusRegister rh1;
	ModbusRegister t0;
	ModbusRegister t1;
	ModbusRegister err_reg;
	const unsigned int retries;
	const unsigned int wait;
	bool status;
	unsigned int elapsed_time;
	bool check_status();
	float binary32_to_float(const unsigned int bin32);
};

#endif /* SRHTHMP60_H_ */
