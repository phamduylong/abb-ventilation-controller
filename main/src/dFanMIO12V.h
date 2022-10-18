#ifndef DFANMIO12V_H_
#define DFANMIO12V_H_

#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"

class dFanMIO12V {
public:
	dFanMIO12V(unsigned int retries = 3);
	virtual ~dFanMIO12V();
	bool set_speed(int16_t v, bool retry = true);
	int16_t get_speed();
	int get_aspeed(bool retry = true);
	bool get_status();
	unsigned int get_elapsed_time();
private:
	ModbusMaster node;
	ModbusRegister ao1;
	ModbusRegister di1;
	int16_t speed;
	unsigned int retries;
	bool status;
	unsigned int elapsed_time;
};

#endif /* DFANMIO12V_H_ */
