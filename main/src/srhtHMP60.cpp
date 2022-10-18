/*
 * srhtHMP60.cpp
 *
 *  Created on: 17 Oct 2022
 *      Author: Danii
 */

#include "srhtHMP60.h"

srhtHMP60::srhtHMP60(unsigned int retries) : node{241}, rh0{&node, 0x0000}, rh1{&node, 0x0001}, t0{&node, 0x0002}, t1{&node, 0x0003}, retries(retries) {
	//TODO: add 0x0200 reg -> error status. 0 - yes, 1 - no error.
	// add error code regs and error code handling.
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

srhtHMP60::~srhtHMP60() {}

bool srhtHMP60::read(float &temp, float &rhum, bool retry) {
	//TODO: Add check for valid data.
	unsigned int htemp = 0;
	htemp |= this->t1.read();
	htemp <<= 16;
	htemp |= this->t0.read();
	temp = binary32_to_float(htemp);

	unsigned int hrhum = 0;
	hrhum |= this->rh1.read();
	hrhum <<= 16;
	hrhum |= this->rh0.read();
	rhum = binary32_to_float(hrhum);

	this->elapsed_time = 0;
	this->status = true;
	return this->status;
}

bool srhtHMP60::read_temp(float &data, bool retry) {
	unsigned int htemp = 0;
	htemp |= this->t1.read();
	htemp <<= 16;
	htemp |= this->t0.read();
	data = binary32_to_float(htemp);
	this->elapsed_time = 0;
	this->status = true;
	return this->status;
}

bool srhtHMP60::read_rhum(float &data, bool retry) {
	unsigned int hrhum = 0;
	hrhum |= this->rh1.read();
	hrhum <<= 16;
	hrhum |= this->rh0.read();
	data = binary32_to_float(hrhum);
	this->elapsed_time = 0;
	this->status = true;
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