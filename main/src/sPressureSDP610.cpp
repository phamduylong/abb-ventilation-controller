#include "sPressureSDP610.h"
#include "systick.h"

uint8_t crc8(uint8_t *data, size_t size) {
	uint8_t crc = 0x00;
	uint8_t byteCtr;
	//calculates 8-Bit checksum with given polynomial
	for (byteCtr = 0; byteCtr < size; ++byteCtr) { 
		crc ^= (data[byteCtr]);
		for (uint8_t bit = 8; bit > 0; --bit) {
			if (crc & 0x80) crc = (crc << 1) ^ 0x131; //P(x)=x^8+x^5+x^4+1 = 0001 0011 0001 
			else crc = (crc << 1);
		}
	}
	return crc;
}

sPressureSDP610::sPressureSDP610(unsigned int retries) : i2c{}, sdp610{&i2c, 0x40}, retries(retries) {
	this->status = false;
	this->elapsed_time = 0;
}

sPressureSDP610::~sPressureSDP610() {}

/**
 * @brief Reads sensor pressure sensor reading.
 * Sets status and elapsed_time according to the operation result.
 * 
 * @param data Variable to put sensor reading into.
 * @param retry Determines whether to retry on unsuccessfull read.
 * @return true on success, false on fail.
 */
bool sPressureSDP610::read(float &data, bool retry) {
	int i = (retry ? this->retries : 0) + 1;
	this->status = false;
	uint8_t pres_raw[3] = {0};
	uint16_t pres_value = 0;
	this->elapsed_time = 0;
	while (i && !this->status) {
		Sleep(10); //Wait 10ms before every sensor reading. (Needed by I2C, can be lowered to 1ms, but it can be unsafe)
		this->elapsed_time += 10;
		this->status = this->sdp610.read(0xF1, pres_raw, 3);
		//Is not able to communicate with the sensor.
		if (!this->status) {
			--i;
			continue;
		}
		this->status = (pres_raw[2] == crc8(pres_raw, 2));
		//CRC error.
		if (!this->status) {
			--i;
			continue;
		}
		pres_value = 0;
		pres_value = pres_raw[0];
		pres_value <<= 8;
		pres_value |= pres_raw[1];
		int16_t diff_pres = *((int16_t *)&pres_value);
		data = (float)diff_pres / 240;
		--i;
	}
	return this->status;
}

bool sPressureSDP610::get_status() {
	return this->status;
}

unsigned int sPressureSDP610::get_elapsed_time() {
	return this->elapsed_time;
}
