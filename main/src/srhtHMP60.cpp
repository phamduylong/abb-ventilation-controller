/*
 * srhtHMP60.cpp
 *
 *  Created on: 17 Oct 2022
 *      Author: Danii
 */

#include "srhtHMP60.h"
#include "systick.h"

srhtHMP60::srhtHMP60(unsigned int retries) : node{241}, rh0{&node, 0x0000}, rh1{&node, 0x0001}, t0{&node, 0x0002}, t1{&node, 0x0003}, retries(retries) {
	//TODO: add 0x0200 reg -> error status. 0 - yes, 1 - no error.
	// add error code regs and error code handling.
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

srhtHMP60::~srhtHMP60() {}

//Note: data is not modified if reading failed. (TODO: Add status regs check.)
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
		///////////////
		//TEMPERATURE//
		///////////////
		//Read second register value.
		tpr = this->t1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
			}
			continue;
		}
		htemp |= tpr;
		htemp <<= 16;
		//Read first register value.
		tpr = this->t0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
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
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
			}
			continue;
		}
		hrhum |= tpr;
		hrhum <<= 16;
		//Read first register value.
		tpr = this->rh0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
			}
			continue;
		}
		hrhum |= tpr; //At this point both htemp and hrhum are valid.
		temp = binary32_to_float(htemp);
		rhum = binary32_to_float(hrhum);
		this->status = true;
	}

	return this->status;
}

//Note: data is not modified if reading failed. (TODO: Add status regs check.)
bool srhtHMP60::read_temp(float &data, bool retry) {
	unsigned int htemp;
	int tpr;
	this->status = false;
	this->elapsed_time = 0;
	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		htemp = 0;
		tpr = 0;
		//Read second register value.
		tpr = this->t1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
			}
			continue;
		}
		htemp |= tpr;
		htemp <<= 16;
		//Read first register value.
		tpr = this->t0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
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

//Note: data is not modified if reading failed. (TODO: Add status regs check.)
bool srhtHMP60::read_rhum(float &data, bool retry) {
	unsigned int hrhum;
	int tpr;
	this->status = false;
	this->elapsed_time = 0;
	
	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		hrhum = 0;
		tpr = 0;
		//Read second register value.
		tpr = this->rh1.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
			}
			continue;
		}
		hrhum |= tpr;
		hrhum <<= 16;
		//Read first register value.
		tpr = this->rh0.read();
		//Start over upon failure.
		if (tpr == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
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

bool srhtHMP60::get_status() {
	return this->status;
}

unsigned int srhtHMP60::get_elapsed_time() {
	return this->elapsed_time;
}

//Convertion of 32 binary representation of decimal to float via pointer.
float srhtHMP60::binary32_to_float(const unsigned int bin32) {
	return *((float *)&bin32);
}
//Redundant complex convertion of 32 binary representation of decimal to float.
/*
float binary32_to_float(const unsigned int bin32) {
	float res = 0;
	//Get and decode exponent part.
	int8_t exponent = ((bin32 & 0x7f800000) >> 23) - 127;
	//Get fraction part.
	unsigned int fraction = bin32 & 0x007fffff;
	//Add implicit 24 bit to fraction.
	fraction |= 0x00800000;
	//Sum fraction.
	float to_add = 1;
	unsigned int i = 0x00800000;
	do {
		if(fraction & i) res += to_add;
		to_add /= 2;
		i >>= 1;
	} while(i > 0x00000000);
	//Multiply the sum by the base 2 to the power of the exponent.
	res *= pow(2, exponent);
	//Get sign.
	if(bin32 >> 31) res *= -1;
	return res;
}
*/