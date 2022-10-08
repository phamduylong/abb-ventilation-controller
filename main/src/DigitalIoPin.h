#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_

#include "chip.h"
#include "board.h"

class DigitalIoPin {
public:
	DigitalIoPin(int port, int pin, bool input = true, bool pullup = true, bool invert = false);
	DigitalIoPin(const DigitalIoPin &) = delete;
	virtual ~DigitalIoPin();
	bool read();
	void write(bool value);
private:
	int port;
	int pin;
	bool input;
	bool pullup;
	bool invert;
};

#endif /* DIGITALIOPIN_H_ */
