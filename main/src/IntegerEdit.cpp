#include "IntegerEdit.h"
#include <cstdio>
#include "LpcUart.h"

IntegerEdit::IntegerEdit(LiquidCrystal *lcd_, std::string editTitle, int maximum, int minimum, int step, std::string unit, bool modify):
lcd(lcd_), title(editTitle), max(maximum), min(minimum), incrementValue(step), unitValue(unit), modifiable(modify)
{
	value = 0;
	edit = 0;
	focus = false;
}

IntegerEdit::~IntegerEdit() {}

void IntegerEdit::increment() {
	if(this->modifiable) {
		if(edit + incrementValue <= max) edit += incrementValue;
		else edit = max;
	}
}

void IntegerEdit::decrement() {
	if(this->modifiable) {
		if(edit - incrementValue >= min) edit -= incrementValue;
		else edit = min;
	}
}

void IntegerEdit::accept() {
	save();
}

void IntegerEdit::cancel() {
	edit = value;
}

bool IntegerEdit::setFocus(bool focus) {
	if(this->modifiable){
		this->focus = focus;
		return true;
	}
	return false;
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
		snprintf(s, 17, "     [%2d]%4s  ", edit, unitValue.c_str());
	}
	else {
		snprintf(s, 17, "      %2d %4s   ", edit, unitValue.c_str());
	}
	lcd->print(s);
}

bool IntegerEdit::getStatus(){
	return this->modifiable;
}

void IntegerEdit::setStatus(bool modify) {
	this->modifiable = modify;
	if(this->focus) {
		if(!this->modifiable) this->focus = false;
		this->cancel();
		this->display();
	}

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

const char* IntegerEdit::getTitle(){
	return this->title.c_str();
}

void IntegerEdit::setTitle(const char *new_title) {
	this->title = new_title;
}
