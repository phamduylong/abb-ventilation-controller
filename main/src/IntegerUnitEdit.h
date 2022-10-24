/*
 * PressureEdit.h
 *
 *  Created on: 2022年10月17日
 *      Author: kendrick807
 */

#ifndef INTEGERUNITEDIT_H_
#define INTEGERUNITEDIT_H_

#include "PropertyEdit.h"
#include "LiquidCrystal.h"
#include <string>
#include "LpcUart.h"

class IntegerUnitEdit: public PropertyEdit {
public:
	IntegerUnitEdit(LiquidCrystal *lcd_, std::string editTitle, int max, int min, int step, std::string unit);
	virtual ~IntegerUnitEdit();
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void display();
	int getValue();
	void setValue(int value);
	void setTitle(const char *new_title);
private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	std::string title;
	int value;
	int edit;
	bool focus;
	int min;
	int max;
	int step;
	std::string unit;
};



#endif /* INTEGERUNITEDIT_H_ */
