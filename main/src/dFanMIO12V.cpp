#include "dFanMIO12V.h"
#include "systick.h"

dFanMIO12V::dFanMIO12V(unsigned int retries) : node{1}, ao1{&node, 0x0000}, di1{&node, 0x0004}, speed(0), retries(retries) {
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

dFanMIO12V::~dFanMIO12V() {}

bool dFanMIO12V::set_speed(int16_t v, bool retry) {
	//Only values from 0 to 1000 allowed.
	if (v < 0) v = 0;
	else if (v > 1000) v = 1000;

	this->elapsed_time = 0;
	unsigned int i = (retry ? this->retries : 1);
	//Will be executed only once in case of success or !retry.
	do {
		this->status = !(this->ao1.write(v));
		--i;
		//On failure wait 100ms before next loop.
		if(i && !this->status) {
			Sleep(100);
			this->elapsed_time += 100;
		}
	}while(!this->status && i);
	//Set speed to the desired one only if it was set up.
	if (this->status) this->speed = v;
	
	return this->status;
}

int16_t dFanMIO12V::get_speed() {
	return this->speed;
}

int dFanMIO12V::get_aspeed(bool retry) {
	return this->di1.read();
}

bool dFanMIO12V::get_status() {
	return this->status;
}

unsigned int dFanMIO12V::get_elapsed_time() {
	return this->elapsed_time;
}
