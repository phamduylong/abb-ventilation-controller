/*
 * DecimalEdit.h
 *
 *  Created on: 2022年10月13日
 *      Author: kendrick807
 */

#ifndef DECIMALEDIT_H_
#define DECIMALEDIT_H_

#include "PropertyEdit.h"
#include "LiquidCrystal.h"
#include <string>
#include "LpcUart.h"

class DecimalEdit: public PropertyEdit {
public:
	DecimalEdit(LiquidCrystal *lcd_,std::string editTitle,float max,float min,float step);
	virtual ~DecimalEdit();
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void display();
	float getValue();
	void setValue(float value);
private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	std::string title;
	float value;
	float edit;
	bool focus;
	float min;
	float max;
	float step;
};

#endif /* DECIMALEDIT_H_ */



