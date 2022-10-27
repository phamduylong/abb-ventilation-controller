#include "aFanMIO12V.h"

#include "systick.h"

aFanMIO12V::aFanMIO12V(unsigned int retries, unsigned int wait) : node{1}, ao1{&node, 0x0000}, di1{&node, 0x0004}, speed(-1), retries(retries), wait(wait) {
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

aFanMIO12V::~aFanMIO12V() {}

bool aFanMIO12V::set_speed(int16_t v, bool retry) {
	unsigned int start_time = DWT->CYCCNT;
	if (v < 0) v = 0;
	else if (v > 1000) v = 1000;

	this->elapsed_time = 0;
	unsigned int i = (retry ? this->retries : 1);
	//Will be executed only once in case of success or !retry.
	do {
		Sleep(1);
		this->status = !(this->ao1.write(v));
		--i;
		//On failure wait this->wait (100ms by default) before next loop.
		if(i && !this->status) Sleep(this->wait);
	}while(!this->status && i);
	//Set speed to the desired one only if it was set up.
	if (this->status) this->speed = v;
	
	this->elapsed_time = DWT->CYCCNT;
	if (this->elapsed_time > start_time) this->elapsed_time -= start_time;
	else this->elapsed_time = 0xffffffff - start_time + 1 + this->elapsed_time;
	return this->status;
}

bool aFanMIO12V::get_speed(int16_t &v, bool retry) {
	unsigned int start_time = DWT->CYCCNT;
	int cur_sp = -1;
	this->elapsed_time = 0;
	this->status = false;
	unsigned int i = (retry ? this->retries : 1);
	//Will be executed only once in case of success or !retry.
	while(i && cur_sp == -1) {
		Sleep(1);
		cur_sp = this->ao1.read();
		
		if(cur_sp == -1) {
			//Sleep this->wait (100ms by default) only if we have to loop another time.
			if(--i) Sleep(this->wait);
			continue;
		}

		//If we are here - read succeeded.
		this->status = true;
		this->speed = cur_sp;
		v = this->speed;
	}

	this->elapsed_time = DWT->CYCCNT;
	if (this->elapsed_time > start_time) this->elapsed_time -= start_time;
	else this->elapsed_time = 0xffffffff - start_time + 1 + this->elapsed_time;
	return this->status;
}

int16_t aFanMIO12V::get_qspeed() {
	return this->speed;
}

bool aFanMIO12V::get_status() {
	return this->status;
}

unsigned int aFanMIO12V::get_elapsed_time() {
	return this->elapsed_time;
}
