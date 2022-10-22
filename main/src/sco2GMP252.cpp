#include "sco2GMP252.h"
#include "systick.h"

sco2GMP252::sco2GMP252(unsigned int retries) : node{240}, co2_reg{&node, 0x0000}, co2_reg2{&node, 0x0001},
voprcom_reg{&node, 0x208}, voprcom_reg2{&node, 0x209}, vohumcom_reg{&node, 0x20c}, vohumcom_reg2{&node, 0x20d},
mode_prescom_reg{&node, 0x304}, mode_tempcom_reg{&node, 0x305}, mode_humcom_reg{&node, 0x306}, retries(retries), abspres_value(1015.0) {
	//TODO: add 0x0100 and 0x0101 regs handling. (overflow)
	//Add 0x0800 and 0x0801 status regs handling. (status)
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
	this->pres_value = 0;
	this->hum_value = 0;
	//Attempt to setup the sensor. (most probably will fail)
	this->init_precise();
}

sco2GMP252::~sco2GMP252() {}

//Note: (imprecise) data is not modified if reading failed. (TODO: Add status regs check.)
bool sco2GMP252::read(float &data, bool retry) {
	if(!check_init_precise()) this->init_precise(); //Try to be as precise as possible. (Don't care if we are not able to).
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

//Note: data is not modified if reading failed. (TODO: Add status regs check. It is risky to wait 100ms in the loop without checking precision)
bool sco2GMP252::read(float &data, float pres, float rh, bool retry) {
	unsigned int co2_hex;
	int temp;
	this->elapsed_time = 0;
	this->status = false;
	//Must be precise.
	if(!check_init_precise()) this->init_precise();
	//Initialisation failed. Fail precise read immediately.
	if(!check_init_precise()) return this->status;
	//Must set new sensor data. Fail precise read otherwise.
	if(!set_precise(pres, rh)) return this->status;

	//Reading sensor.
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

bool sco2GMP252::get_status() {
	return this->status;
}

unsigned int sco2GMP252::get_elapsed_time() {
	return this->elapsed_time;
}

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

bool sco2GMP252::check_init_precise() {
	int check = 1;
	//If one of these registers returns -1 or 0 means, that sensor wasn't set up earlier.
	check *= this->mode_prescom_reg.read();
	check *= this->mode_tempcom_reg.read();
	check *= this->mode_humcom_reg.read();
	/*Don't care about volatile registers,
	as long as we are able to communicate with the sensor,
	we will set them up during the read().*/
	if (check <= 0) return false;
	else return true;
}

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

//Convertion of 32 binary representation of decimal to float via pointer.
float sco2GMP252::binary32_to_float(const unsigned int bin32) {
	return *((float *)&bin32);
}
//Convertion from float to 32 binary representation of decimal.
unsigned int sco2GMP252::float_to_binary32(float dec) {
	return *((unsigned int *)&dec);
}