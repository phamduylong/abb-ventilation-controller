#include "aFanMIO12V.h"

#include "systick.h"

aFanMIO12V::aFanMIO12V(unsigned int retries) : node{1}, ao1{&node, 0x0000}, di1{&node, 0x0004}, speed(0), retries(retries) {
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

aFanMIO12V::~aFanMIO12V() {}

bool aFanMIO12V::set_speed(int16_t v, bool retry) {
	if (v < 0) v = 0;
	else if (v > 1000) v = 1000;
	//Add error checking
	ao1.write(v);
	//Pray that it was set up.
	this->speed = v;
	return true;
}

int16_t aFanMIO12V::get_speed() {
	return this->speed;
}

int aFanMIO12V::get_aspeed(bool retry) {
	return di1.read();
}

bool aFanMIO12V::get_status() {
	return this->status;
}

unsigned int aFanMIO12V::get_elapsed_time() {
	return this->elapsed_time;
}
