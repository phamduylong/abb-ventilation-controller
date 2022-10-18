#include "dFanMIO12V.h"
#include "systick.h"

dFanMIO12V::dFanMIO12V(unsigned int retries) : node{1}, ao1{&node, 0x0000}, di1{&node, 0x0004}, speed(0), retries(retries) {
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

dFanMIO12V::~dFanMIO12V() {
	//Try to shut down the fan.
	//Add error checking and retries.
	ao1.write(0);
	//Pray that it was set up.
	this->speed = 0;
}

bool dFanMIO12V::set_speeed(int16_t v, bool retry) {
	//Add error checking
	ao1.write(v);
	//Pray that it was set up.
	this->speed = v;
	return true;
}

int16_t dFanMIO12V::get_speed() {
	return this->speed;
}

int dFanMIO12V::get_aspeed(bool retry) {
	return di1.read();
}

bool dFanMIO12V::get_status() {
	return this->status;
}

unsigned int dFanMIO12V::get_elapsed_time() {
	return this->elapsed_time;
}
