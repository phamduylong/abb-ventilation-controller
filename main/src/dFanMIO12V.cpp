/*
 * dFanMIO12V.cpp
 *
 *  Created on: 18 Oct 2022
 *      Author: Danii
 */

#include "dFanMIO12V.h"

dFanMIO12V::dFanMIO12V(unsigned int retries) : node{1}, ai1{&node, 0x0000}, di1{&node, 0x0004}, speed(0), retries(retries) {
	this->node.begin(9600);
	this->status = false;
	this->elapsed_time = 0;
}

dFanMIO12V::~dFanMIO12V() {

}

