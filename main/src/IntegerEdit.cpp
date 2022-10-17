/*
 * IntegerEdit.cpp
 *
 *  Created on: 2.2.2016
 *      Author: krl
 */

#include "IntegerEdit.h"
#include <cstdio>
#include "LpcUart.h"

IntegerEdit::IntegerEdit(LiquidCrystal *lcd_, LpcUart *lpc_ ,std::string editTitle,int maximum,int minimum): lcd(lcd_), lpc(lpc_), title(editTitle),max(maximum),min(minimum) {
	value = 0;
	edit = 0;
	focus = false;
}

IntegerEdit::~IntegerEdit() {
}

void IntegerEdit::increment() {
	if(edit < max)edit++;

}

void IntegerEdit::decrement() {
	if(edit > min)edit--;

}

void IntegerEdit::accept() {
	save();
}

void IntegerEdit::cancel() {
	edit = value;
}


void IntegerEdit::setFocus(bool focus) {
	this->focus = focus;
}

bool IntegerEdit::getFocus() {
	return this->focus;
}

void IntegerEdit::display() {
	lcd->clear();
	lcd->setCursor(0,0);
	lcd->print(title);
	lcd->setCursor(0,1);
	char s[17];
	if(focus) {
		snprintf(s, 17, "     [%4d]     ", edit);
	}
	else {
		snprintf(s, 17, "      %4d      ", edit);
	}
	lcd->print(s);
}
void IntegerEdit::print(){
	char s[128];
	snprintf(s,128, "\r\n%s      %4d      \r\n", title.c_str(), value);
	lpc->write(s);
}


void IntegerEdit::save() {
	// set current value to be same as edit value
	value = edit;
	// todo: save current value for example to EEPROM for permanent storage
}


int IntegerEdit::getValue() {
	return value;
}
void IntegerEdit::setValue(int value) {
	edit = value;
	save();
}
