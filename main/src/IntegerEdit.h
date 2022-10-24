/*
 * IntegerEdit.h
 *
 *  Created on: 2.2.2016
 *      Author: krl
 */

#ifndef INTEGEREDIT_H_
#define INTEGEREDIT_H_


#include "PropertyEdit.h"
#include "LiquidCrystal.h"
#include <string>
#include "LpcUart.h"

class IntegerEdit: public PropertyEdit {
public:
	IntegerEdit(LiquidCrystal *lcd_, std::string editTitle, int maximum, int minimum, int step, std::string unit, bool modify = true);
	virtual ~IntegerEdit();
	void increment();
	void decrement();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void display();
	int getValue();
	void setValue(int value);
	bool getStatus();
	void setStatus(bool modify);
	const char* getTitle();
	void setTitle(const char *new_title);
private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	std::string title;
	int max;
	int min;
	int incrementValue;
	std::string unitValue;
	bool modifiable;
	int value;
	int edit;
	bool focus;
};


#endif /* INTEGEREDIT_H_ */
