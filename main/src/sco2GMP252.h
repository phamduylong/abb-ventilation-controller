#ifndef SCO2GMP252_H_
#define SCO2GMP252_H_

#include "chip.h"
#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"
#include "srhtHMP60.h"
#include "sPressureSDP610.h"

class sco2GMP252 {
public:
	sco2GMP252(unsigned int retries = 3, unsigned int wait = 100);
	virtual ~sco2GMP252();
	bool read_rapid(float &data, bool retry = true);
	bool read(float &data, float pres, float rh, bool retry = true);
	bool read_precise(float &data, float pres, float rh, bool retry = true);
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
	ModbusRegister status_device_reg;
	ModbusRegister status_co2_reg;
	const unsigned int retries;
	const unsigned int wait;
	//Flags
	bool status;
	//Variables
	unsigned int elapsed_time;
	const float abspres_value;
	float pres_value;
	float hum_value;
	//Functions
	bool init_precise();
	int16_t check_device_status();
	int16_t check_co2_status();
	bool check_status();
	bool check_init_precise();
	bool set_precise(float pres, float rh = 0);
	float binary32_to_float(const unsigned int bin32);
	unsigned int float_to_binary32(float dec);
};

#endif /* SCO2GMP252_H_ */
