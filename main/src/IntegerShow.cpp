/*
 * IntegerShow.cpp
 *
 *  Created on: 2022年10月18日
 *      Author: kendrick807
 */



#include "IntegerShow.h"
#include <cstdio>
#include "LpcUart.h"

IntegerShow::IntegerShow(LiquidCrystal *lcd_, std::string editTitle, std::string unit): lcd(lcd_), title(editTitle), unitValue(unit) {
	value = 0;
	edit = 0;

}

IntegerShow::~IntegerShow() {}
void IntegerShow::increment() {}
void IntegerShow::decrement() {}

bool IntegerShow::getStatus(){
	return false;
}

void IntegerShow::setStatus(bool modify) {}

void IntegerShow::accept() {
	save();
}

void IntegerShow::cancel() {
	edit = value;
}

void IntegerShow::setFocus(bool focus) {
	this->focus = focus;
}

bool IntegerShow::getFocus() {
	return this->focus;
}

void IntegerShow::display() {
	lcd->clear();
	lcd->setCursor(0,0);
	lcd->print(title);
	lcd->setCursor(0,1);
	char s[17];
	if(focus) {
		snprintf(s, 17, "     [%2d]%4s  ", edit,unitValue.c_str());
	}
	else {
		snprintf(s, 17, "      %2d %4s   ", edit,unitValue.c_str());
	}
	lcd->print(s);
}

void IntegerShow::save() {
	// set current value to be same as edit value
	value = edit;
	// todo: save current value for example to EEPROM for permanent storage
}

int IntegerShow::getValue() {
	return value;
}

void IntegerShow::setValue(int value) {
	edit = value;
	save();
}

const char* IntegerShow::getTitle() {
	return (this->title.c_str());
}

void IntegerShow::setTitle(const char *new_title) {
	this->title = new_title;
}
