#include "sco2GMP252.h"
#include "systick.h"

sco2GMP252::sco2GMP252(unsigned int retries) : node{240}, co2_reg{&node, 0x0000}, co2_reg2{&node, 0x0001}, retries(retries) {
	//TODO: add 0x0100 and 0x0101 regs handling.
	//Add 0x0800 and 0x0801 status regs handling.
	node.begin(9600);
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

sco2GMP252::~sco2GMP252() {}

//Note: data is not modified if reading failed. (TODO: Add status regs check.)
bool sco2GMP252::read(float &data, bool retry) {
	unsigned int co2_hex;
	int temp;
	this->status = false;
	this->elapsed_time = 0;
	//In case of success or !retry code is executed only once.
	for (unsigned int i = (retry ? 0 : this->retries); !this->status && i <= this->retries; ++i) {
		co2_hex = 0;
		temp = 0;
		//Read second register value.
		temp = co2_reg2.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
			}
			continue;
		}
		co2_hex |= temp;

		co2_hex <<= 16;
		//Read first register value.
		temp = co2_reg.read();
		//Start over upon failure.
		if (temp == -1) {
			//If it is not the last attempt - wait for 100ms in case it was a minor failure.
			if(i != this->retries) {
				Sleep(100);
				this->elapsed_time += 100;
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

bool read(float &data, float temp, float rh, float pres, bool retry) {
	return false;
}

bool sco2GMP252::get_status() {
	return this->status;
}

unsigned int sco2GMP252::get_elapsed_time() {
	return this->elapsed_time;
}

bool set_precise(bool mode, srhtHMP60 *srht, sPressureSDP610 *spres) {
	return false;
}

//Convertion of 32 binary representation of decimal to float via pointer.
float sco2GMP252::binary32_to_float(const unsigned int bin32) {
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

void sco2GMP252::read_sensors() {

}
