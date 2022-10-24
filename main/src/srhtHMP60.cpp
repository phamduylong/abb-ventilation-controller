#include "srhtHMP60.h"
#include "systick.h"

srhtHMP60::srhtHMP60(unsigned int retries, unsigned int wait) : node{241}, rh0{&node, 0x0000}, rh1{&node, 0x0001}, t0{&node, 0x0002}, t1{&node, 0x0003},
err_reg{&node, 0x0200}, retries(retries), wait(wait) {
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

srhtHMP60::~srhtHMP60() {}

/**
 * @brief Reads humidity and temperature values from relative humidity sensor.
 * 
 * @param temp value is set to the sensor reading on success. Not modified otherwise.
 * @param rhum value is set to the sensor reading on success. Not modified otherwise.
 * @param retry flag to retry reading the sensor value while waiting 'this->wait' ms between 'this->retries' readings.
 * @return true On success.
 * @return false On failure.
 */
bool srhtHMP60::read(float &temp, float &rhum, bool retry) {
	unsigned int htemp;
	unsigned int hrhum;
	int tpr;
	this->status = false;
	this->elapsed_time = 0;

	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		htemp = 0;
		hrhum = 0;
		tpr = 0;
		//Check status before reading.
		if(!this->check_status()) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		///////////////
		//TEMPERATURE//
		///////////////
		//Read second register value.
		tpr = this->t1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		htemp |= tpr;
		htemp <<= 16;
		//Read first register value.
		tpr = this->t0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		htemp |= tpr; //htemp is valid here, but it won't be passed if humidity reading will fail.
		
		////////////
		//HUMIDITY//
		////////////
		//Read second register value.
		tpr = this->rh1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		hrhum |= tpr;
		hrhum <<= 16;
		//Read first register value.
		tpr = this->rh0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Reading succeeded if we are here.
		hrhum |= tpr; //At this point both htemp and hrhum are valid.
		temp = binary32_to_float(htemp);
		rhum = binary32_to_float(hrhum);
		this->status = true;
	}

	return this->status;
}

/**
 * @brief Reads temperature value from the sensor.
 * 
 * @param data value is set to the sensor reading on success. Not modified otherwise.
 * @param retry flag to retry reading the sensor value while waiting 'this->wait' ms between 'this->retries' readings.
 * @return true On success. 
 * @return false On failure. 
 */
bool srhtHMP60::read_temp(float &data, bool retry) {
	unsigned int htemp;
	int tpr;
	this->status = false;
	this->elapsed_time = 0;
	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		htemp = 0;
		tpr = 0;

		//Check status before reading.
		if(!this->check_status()) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}

		//Read second register value.
		tpr = this->t1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		htemp |= tpr;
		htemp <<= 16;
		//Read first register value.
		tpr = this->t0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Reading succeeded if we are here.
		htemp |= tpr;
		data = binary32_to_float(htemp);
		this->status = true;
	}

	return this->status;
}

/**
 * @brief Reads humidity value from the sensor.
 * 
 * @param data value is set to the sensor reading on success. Not modified otherwise.
 * @param retry flag to retry reading the sensor value while waiting 'this->wait' ms between 'this->retries' readings.
 * @return true On success. 
 * @return false On failure. 
 */
bool srhtHMP60::read_rhum(float &data, bool retry) {
	unsigned int hrhum;
	int tpr;
	this->status = false;
	this->elapsed_time = 0;
	
	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		hrhum = 0;
		tpr = 0;

		//Check status before reading.
		if(!this->check_status()) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}

		//Read second register value.
		tpr = this->rh1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		hrhum |= tpr;
		hrhum <<= 16;
		//Read first register value.
		tpr = this->rh0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for this->wait (100ms by default) in case it was a minor failure.
			if(i != this->retries) {
				Sleep(this->wait);
				this->elapsed_time += this->wait;
			}
			continue;
		}
		//Reading succeeded if we are here.
		hrhum |= tpr;
		data = binary32_to_float(hrhum);
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
bool srhtHMP60::get_status() {
	return this->status;
}

/**
 * @brief Gets latest read() execution time
 * 
 * @return unsigned int Time in ms.
 */
unsigned int srhtHMP60::get_elapsed_time() {
	return this->elapsed_time;
}

/**
 * @brief Checks if sensor status register reports any errors.
 * 
 * @return true No errors
 * @return false One of the registers returned error.
 */
bool srhtHMP60::check_status() {
	int res = this->err_reg.read();
	if( res == -1 ) return false;
	if( res == 0 ) return false;
	return true;
}

/**
 * @brief Convertion of 32-bit binary representation of decimal to float via pointer.
 * 
 * @param bin32 32-bit binary representation of float.
 * @return float Floating point value of its binary representation.
 */
float srhtHMP60::binary32_to_float(const unsigned int bin32) {
	return *((float *)&bin32);
}
