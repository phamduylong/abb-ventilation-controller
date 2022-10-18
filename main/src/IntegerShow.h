/*
 * IntegerShow.h
 *
 *  Created on: 2022年10月18日
 *      Author: kendrick807
 */

#ifndef INTEGERSHOW_H_
#define INTEGERSHOW_H_

#include "PropertyEdit.h"
#include "LiquidCrystal.h"
#include <string>
#include "LpcUart.h"

class IntegerShow: public PropertyEdit {
public:
	IntegerShow(LiquidCrystal *lcd_, std::string editTitle,std::string unit );
	virtual ~IntegerShow();
	void accept();
	void cancel();
	void setFocus(bool focus);
	bool getFocus();
	void increment();
	void decrement();
	void display();
	int getValue();
	void setValue(int value);
	bool getStatus();
	const char* getTitle();
//	void deleteItem();

private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	std::string title;
	std::string unitValue;
	int value;
	int edit;
	bool focus;


};




#endif /* INTEGERSHOW_H_ */
