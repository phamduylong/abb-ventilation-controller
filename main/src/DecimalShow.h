#ifndef DECIMALSHOW_H_
#define DECIMALSHOW_H_

#include "PropertyEdit.h"
#include "LiquidCrystal.h"
#include <string>
#include "LpcUart.h"

class DecimalShow: public PropertyEdit {
public:
	DecimalShow(LiquidCrystal *lcd_, std::string editTitle, std::string unit );
	virtual ~DecimalShow();
	void accept();
	void cancel();
	bool setFocus(bool focus);
	bool getFocus();
	void increment();
	void decrement();
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

	std::string unitValue;
	float value;
	float edit;
	bool focus;
};


#endif /* DECIMALSHOW_H_ */
