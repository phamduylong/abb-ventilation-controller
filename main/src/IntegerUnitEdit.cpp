/*
 * PressureEdit.cpp
 *
 *  Created on: 2022年10月17日
 *      Author: kendrick807
 */

#include "IntegerUnitEdit.h"
#include <cstdio>
#include "LpcUart.h"

IntegerUnitEdit::IntegerUnitEdit(LiquidCrystal *lcd_, std::string editTitle, int maximum, int minimum, int stepping, std::string unitInt):
lcd(lcd_), title(editTitle), min(minimum), max(maximum), step(stepping), unit(unitInt)
{
	value = 0;
	edit = 0;
	focus = false;
}

IntegerUnitEdit::~IntegerUnitEdit() {}

void IntegerUnitEdit::increment() {
	if(edit < max) edit += step;
}

void IntegerUnitEdit::decrement() {
	if(edit > min && edit != step) edit -= step;
	else edit = min;
}

void IntegerUnitEdit::accept() {
	save();
}

void IntegerUnitEdit::cancel() {
	edit = value;
}

void IntegerUnitEdit::setFocus(bool focus) {
	this->focus = focus;
}

bool IntegerUnitEdit::getFocus() {
	return this->focus;
}

void IntegerUnitEdit::display() {
	lcd->clear();
	lcd->setCursor(0,0);
	lcd->print(title);
	lcd->setCursor(0,1);
	char s[17];
	if(focus) {
		snprintf(s, 17, "     [%4d]   %s", edit, unit.c_str());
	}
	else {
		snprintf(s, 17, "      %4d    %s ", edit, unit.c_str());
	}
	lcd->print(s);
}
//void PressureEdit::print(){
//	char s[128];
//	snprintf(s,128, "\r\n%s      %4d      \r\n", title.c_str(), value);
//	lpc->write(s);
//}

void IntegerUnitEdit::save() {
	// set current value to be same as edit value
	value = edit;
	// todo: save current value for example to EEPROM for permanent storage
}

int IntegerUnitEdit::getValue() {
	return value;
}

void IntegerUnitEdit::setValue(int value) {
	edit = value;
	save();
}

void IntegerUnitEdit::setTitle(const char *new_title) {
	this->title = new_title;
}
