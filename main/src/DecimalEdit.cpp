#include "DecimalEdit.h"
#include <cstdio>
#include "LpcUart.h"

DecimalEdit::DecimalEdit(LiquidCrystal *lcd_, std::string editTitle, float maximum, float minimum, float step, std::string unit, bool modify): 
lcd(lcd_), title(editTitle), max(maximum), min(minimum), incrementValue(step), unitValue(unit), modifiable(modify)
{
	value = 0;
	edit = 0;
	focus = false;
}

DecimalEdit::~DecimalEdit() {}

void DecimalEdit::increment() {
	if(this->modifiable) {
		if(edit + incrementValue <= max) edit += incrementValue;
		else edit = max;
	}
}

void DecimalEdit::decrement() {
	if(this->modifiable) {
		if(edit - incrementValue >= min) edit -= incrementValue;
		else edit = min;
	}
}

void DecimalEdit::accept() {
	save();
}

void DecimalEdit::cancel() {
	edit = value;
}

bool DecimalEdit::setFocus(bool focus) {
	if(this->modifiable){
		this->focus = focus;
		return true;
	}
	return false;
}

bool DecimalEdit::getFocus() {
	return this->focus;
}

bool DecimalEdit::getStatus(){
	return this->modifiable;
}

void DecimalEdit::setStatus(bool modify) {
	this->modifiable = modify;
	if(this->focus) {
		if(!this->modifiable) this->focus = false;
		this->cancel();
		this->display();
	}
}

void DecimalEdit::display() {
	lcd->clear();
	lcd->setCursor(0,0);
	lcd->print(title);
	lcd->setCursor(0,1);
	char s[17];
	if(focus) {
		snprintf(s, 17, "     [%2.1f]%4s", edit, unitValue.c_str());
	}
	else {
		snprintf(s, 17, "      %2.1f %4s", edit, unitValue.c_str());
	}
	lcd->print(s);
}
//void DecimalEdit::print(){
//	char s[128];
//	snprintf(s,128, "\r\n%s      %4.1f      \r\n", title.c_str(), value);
//	lpc->write(s);
//}

void DecimalEdit::save() {
	// set current value to be same as edit value
	value = edit;
	// todo: save current value for example to EEPROM for permanent storage
}

float DecimalEdit::getValue() {
	return value;
}

void DecimalEdit::setValue(float value) {
	edit = value;
	save();
}

const char* DecimalEdit::getTitle() {
	return this->title.c_str();
}

void DecimalEdit::setTitle(const char *new_title) {
	this->title = new_title;
}
