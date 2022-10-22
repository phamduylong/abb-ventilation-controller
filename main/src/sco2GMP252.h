#ifndef SCO2GMP252_H_
#define SCO2GMP252_H_

#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"
#include "srhtHMP60.h"
#include "sPressureSDP610.h"

class sco2GMP252 {
public:
	sco2GMP252(unsigned int retries = 3);
	virtual ~sco2GMP252();
	bool read(float &data, bool retry = true);
	bool read(float &data, float pres, float rh, bool retry = true);
	bool get_status();
	unsigned int get_elapsed_time();
private:
	ModbusMaster node;
	ModbusRegister co2_reg;
	ModbusRegister co2_reg2;
	ModbusRegister voprcom_reg;
	ModbusRegister voprcom_reg2;
	ModbusRegister vohumcom_reg;
	ModbusRegister vohumcom_reg2;
	ModbusRegister mode_prescom_reg;
	ModbusRegister mode_tempcom_reg;
	ModbusRegister mode_humcom_reg;
	unsigned int retries;
	//Flags
	bool status;
	//Variables
	unsigned int elapsed_time;
	const float abspres_value;
	float pres_value;
	float hum_value;
	//Functions
	bool init_precise();
	bool check_init_precise();
	bool set_precise(float pres, float rh = 0);
	float binary32_to_float(const unsigned int bin32);
	unsigned int float_to_binary32(float dec);
};

#endif /* SCO2GMP252_H_ */
