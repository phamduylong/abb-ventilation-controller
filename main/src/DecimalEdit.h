#ifndef DECIMALEDIT_H_
#define DECIMALEDIT_H_

#include "PropertyEdit.h"
#include "LiquidCrystal.h"
#include <string>
#include "LpcUart.h"

class DecimalEdit: public PropertyEdit {
public:
	DecimalEdit(LiquidCrystal *lcd_, std::string editTitle, float maximum, float minimum, float step, std::string unit, bool modify = true);
	virtual ~DecimalEdit();
	void increment();
	void decrement();
	void accept();
	void cancel();
	bool setFocus(bool focus);
	bool getFocus();
	void display();
	float getValue();
	void setValue(float value);
	bool getStatus();
	void setStatus(bool modify);
	const char* getTitle();
	void setTitle(const char *new_title);
private:
	void save();
	void displayEditValue();
	LiquidCrystal *lcd;
	std::string title;
	float max;
	float min;
	float incrementValue;
	std::string unitValue;
	bool modifiable;
	float value;
	float edit;
	bool focus;

};

#endif /* DECIMALEDIT_H_ */
