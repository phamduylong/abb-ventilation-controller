#include "DecimalShow.h"
#include <cstdio>
#include "LpcUart.h"

DecimalShow::DecimalShow(LiquidCrystal *lcd_, std::string editTitle, std::string unit): lcd(lcd_), title(editTitle), unitValue(unit) {
	value = 0;
	edit = 0;
	focus = 0;
}

DecimalShow::~DecimalShow() {}
void DecimalShow::increment() {}
void DecimalShow::decrement() {}

bool DecimalShow::getStatus(){
	return false;
}

void DecimalShow::setStatus(bool modify) {}

void DecimalShow::accept() {
	save();
}

void DecimalShow::cancel() {
	edit = value;
}

bool DecimalShow::setFocus(bool focus) {
	return false;
}

bool DecimalShow::getFocus() {
	return this->focus;
}

void DecimalShow::display() {
	lcd->clear();
	lcd->setCursor(0,0);
	lcd->print(title);
	lcd->setCursor(0,1);
	char s[17];
	if(focus) {
		snprintf(s, 17, "     [%2.1f]%4s  ", edit, unitValue.c_str());
	}
	else {
		snprintf(s, 17, "      %2.1f %4s   ", edit, unitValue.c_str());
	}
	lcd->print(s);
}

void DecimalShow::save() {
	// set current value to be same as edit value
	value = edit;
	// todo: save current value for example to EEPROM for permanent storage
}

int DecimalShow::getValue() {
	return value;
}

void DecimalShow::setValue(int value) {
	edit = value;
	save();
}

const char* DecimalShow::getTitle(){
	return this->title.c_str();
}

void DecimalShow::setTitle(const char *new_title) {
	this->title = new_title;
}
