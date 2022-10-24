#include "sco2GMP252.h"
#include "systick.h"

sco2GMP252::sco2GMP252(unsigned int retries, unsigned int wait) : node{240}, co2_reg{&node, 0x0000}, co2_reg2{&node, 0x0001},
voprcom_reg{&node, 0x208}, voprcom_reg2{&node, 0x209}, vohumcom_reg{&node, 0x20c}, vohumcom_reg2{&node, 0x20d},
mode_prescom_reg{&node, 0x304}, mode_tempcom_reg{&node, 0x305}, mode_humcom_reg{&node, 0x306},
status_device_reg{&node, 0x800}, status_co2_reg{&node, 0x801},
retries(retries), wait(wait), abspres_value(1015.0) {
	//TODO: add 0x0100 and 0x0101 regs handling. (overflow) (Check if it is really a case for the float value.)
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
	this->pres_value = 0;
	this->hum_value = 0;
	//Attempt to setup the sensor. (most probably will fail)
	if(!check_init_precise()) this->init_precise();
}

sco2GMP252::~sco2GMP252() {}

/**
 * @brief Reads sensor. Data might be imprecise.
 * This read() doesn't modify humidity and pressure values in volatile registers.
 * 
 * @param data value is set to the sensor reading on success. Not modified otherwise.
 * @param retry flag to retry reading the sensor value while waiting 'this->wait' ms between 'this->retries' readings.
 * @return true On success.
 * @return false On failure.
 */
bool sco2GMP252::read(float &data, bool retry) {
	//Try to be as precise as possible. (Don't care if we are not able to).
	if(!check_init_precise()) this->init_precise();
	unsigned int co2_hex;
	int temp;
	this->status = false;
	this->elapsed_time = 0;
	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		co2_hex = 0;
		temp = 0;
		//Fail the attempt on wrong status.
		if(!check_status()) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Read second register value.
		temp = co2_reg2.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		co2_hex |= temp;

		co2_hex <<= 16;
		//Read first register value.
		temp = co2_reg.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Reading succeeded if we are here.
		co2_hex |= temp;
		data = binary32_to_float(co2_hex);
		this->status = true;
	}
	
	return this->status;
}

/**
 * @brief Reads sensor. Data might be imprecise.
 * This read() tries to modify humidity and pressure values in volatile registers.
 * 
 * @param data value is set to the sensor reading on success. Not modified otherwise.
 * @param pres pressure value to set to volatile register.
 * @param rh relative humidity value to set to volatile register.
 * @param retry flag to retry reading the sensor value while waiting 'this->wait' ms between 'this->retries' readings.
 * @return true On success.
 * @return false On failure.
 */
bool sco2GMP252::read(float &data, float pres, float rh, bool retry) {
	unsigned int co2_hex;
	int temp;
	this->elapsed_time = 0;
	this->status = false;

	//Reading sensor.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		co2_hex = 0;
		temp = 0;

		//Fail the attempt on wrong status.
		if(!check_status()) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Try to be as precise as possible. (Don't care if we are not able to).
		if(!this->check_init_precise()) this->init_precise();
		this->set_precise(pres, rh);

		//Read second register value.
		temp = co2_reg2.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		co2_hex |= temp;

		co2_hex <<= 16;
		//Read first register value.
		temp = co2_reg.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Reading succeeded if we are here.
		co2_hex |= temp;
		data = binary32_to_float(co2_hex);
		this->status = true;
	}
	return this->status;
}


/**
 * @brief Reads sensor. Only precise data will pass.
 * This read() modifies humidity and pressure values in volatile registers.
 * 
 * @param data value is set to the sensor reading on success. Not modified otherwise.
 * @param pres pressure value to set to volatile register.
 * @param rh relative humidity value to set to volatile register.
 * @param retry flag to retry reading the sensor value while waiting 'this->wait' ms between 'this->retries' readings.
 * @return true On success.
 * @return false On failure.
 */
bool sco2GMP252::read_precise(float &data, float pres, float rh, bool retry) {
	unsigned int co2_hex;
	int temp;
	this->elapsed_time = 0;
	this->status = false;

	//Reading sensor.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		co2_hex = 0;
		temp = 0;
		//Fail the attempt on wrong status.
		if(!check_status()) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Must be precise.
		if(!check_init_precise()){
			this->init_precise();
			//Fail the attempt on failed initialisation.
			if(!check_init_precise()) {
				//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
				if(i != this->retries) {
					Sleep(this->wait);
					this->elapsed_time += this->wait;
				}
				continue;
			}
		}
		//Fail the attempt on unset new sensor data.
		if(!set_precise(pres, rh)) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}

		//Read second register value.
		temp = co2_reg2.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		co2_hex |= temp;

		co2_hex <<= 16;
		//Read first register value.
		temp = co2_reg.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Reading succeeded if we are here.
		co2_hex |= temp;
		data = binary32_to_float(co2_hex);
		this->status = true;
	}
	return this->status;
}

/**
 * @brief Gets latest sensor status.
 * 
 * @return true Sensor was UP.
 * @return false Sensor was DOWN.
 */
bool sco2GMP252::get_status() {
	return this->status;
}

/**
 * @brief Gets latest read() execution time
 * 
 * @return unsigned int Time in ms.
 */
unsigned int sco2GMP252::get_elapsed_time() {
	return this->elapsed_time;
}

/**
 * @brief Tries to setup precise sensor readings, with pres, temp and hum registers.
 * 
 * @return true Setup successful.
 * @return false Setup failed.
 */
bool sco2GMP252::init_precise() {
	bool res = true;
	int pres_hex = 0;
	res = res && !this->mode_prescom_reg.write(1);
	res = res && !this->mode_tempcom_reg.write(2);
	res = res && !this->mode_humcom_reg.write(1);
	//Write volatile pressure register with absolute pressure.
	pres_hex = this->float_to_binary32(this->abspres_value);
	pres_hex >>= 16;
	res = res && !this->voprcom_reg2.write(pres_hex);
	pres_hex = this->float_to_binary32(this->abspres_value);
	res = res && !this->voprcom_reg.write(pres_hex);
	return res;
}

/**
 * @brief Checks sensor's device status register.
 * 
 * @return int16_t Device status register value or -1 on failure.
 */
int16_t sco2GMP252::check_device_status() {
	return this->status_device_reg.read();
}

/**
 * @brief Checks sensor's co2 status register.
 * 
 * @return int16_t CO2 status register value or -1 on failure.
 */
int16_t sco2GMP252::check_co2_status() {
	return this->status_co2_reg.read();
}

/**
 * @brief Checks if sensor status registers report any errors.
 * 
 * @return true No errors
 * @return false One of the registers returned error.
 */
bool sco2GMP252::check_status() {
	int16_t device_st = this->check_device_status();
	int16_t co2_st = this->check_co2_status();
	if(co2_st == 2) {
		printf("Co2 reading unreliable.\n");
		co2_st = 0;
	}
	return !(device_st | co2_st);
}

/**
 * @brief Checks if precise reading was initialised.
 * 
 * @return true Precise reading was set up.
 * @return false Precise reading is not set up.
 */
bool sco2GMP252::check_init_precise() {
	int check = 1;
	//If one of these registers returns -1 or 0 means, that sensor wasn't set up earlier.
	check *= this->mode_prescom_reg.read(); //expect 1
	check *= this->mode_tempcom_reg.read(); //expect 2
	check *= this->mode_humcom_reg.read(); //expect 1
	/*Don't care about volatile registers,
	as long as we are able to communicate with the sensor,
	we will set them up during the read().*/
	if (check != 2) return false;
	else return true;
}

/**
 * @brief Sets sensor's voaltile registers to be used for more precise reading.
 * 
 * @param pres Absolute pressure value to use.
 * @param rh Relative humidity value to use.
 * @return true Values were set.
 * @return false Failed to set values.
 */
bool sco2GMP252::set_precise(float pres, float rh) {
	bool res = true;
	int hex = 0;
	pres = (pres / 10) + abspres_value;
	//Write volatile pressure register with absolute pressure + relative pressure.
	hex = this->float_to_binary32(pres);
	hex >>= 16;
	res = res && !this->voprcom_reg2.write(hex);
	hex = this->float_to_binary32(pres);
	res = res && !this->voprcom_reg.write(hex);
	//Write volatile relative humidity register with relative humidity.
	hex = this->float_to_binary32(rh);
	hex >>= 16;
	res = res && !this->vohumcom_reg2.write(hex);
	hex = this->float_to_binary32(rh);
	res = res && !this->vohumcom_reg.write(hex);
	if(res) {
		this->pres_value = pres;
		this->hum_value = rh;
	}
	return res;
}

/**
 * @brief Convertion of 32-bit binary representation of decimal to float via pointer.
 * 
 * @param bin32 32-bit binary representation of float.
 * @return float Floating point value of its binary representation.
 */
float sco2GMP252::binary32_to_float(const unsigned int bin32) {
	return *((float *)&bin32);
}

/**
 * @brief Convertion of float to 32-bit binary representation of decimal via pointer.
 * 
 * @param dec Floating point value to represent in binary format.
 * @return unsigned int 32-bit binary representation of float.
 */
unsigned int sco2GMP252::float_to_binary32(float dec) {
	return *((unsigned int *)&dec);
}
