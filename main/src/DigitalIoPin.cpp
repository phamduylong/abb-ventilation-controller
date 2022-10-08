#include "DigitalIoPin.h"

/**
 * @brief Construct a new Digital_Io_Pin object
 * 
 * @param port Pin port
 * @param pin Pin address
 * @param input TRUE - input, FALSE - output
 * @param pullup TRUE - pullup resistor, FALSE - pulldown resistor
 * @param invert TRUE - 1 is LOW, 0 is HIGH, FALSE - 1 is HIGH, 0 is LOW
 */
DigitalIoPin::DigitalIoPin(int port, int pin, bool input, bool pullup, bool invert) : port(port), pin(pin), input(input), pullup(pullup), invert(invert) {
	if(this->input) {
		unsigned int pin_modes = IOCON_DIGMODE_EN;
		if(invert) pin_modes |= IOCON_INV_EN;
		if(pullup) pin_modes |= IOCON_MODE_PULLUP;
		else pin_modes |= IOCON_MODE_PULLDOWN;

		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, pin_modes);
    	Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	}
	else {
		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
    	Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
	}
}
/**
 * @brief Destroy the Digital_Io_Pin object
 */
DigitalIoPin::~DigitalIoPin() {}

/**
 * @brief Reads pin value.
 * If invert is TRUE, FALSE is HIGH and TRUE is LOW.
 * Else, TRUE is HIGH and FALSE is LOW.
 * @return true
 * @return false
 */
bool DigitalIoPin::read() {
	if (this->input) return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
	else if (this->invert) return !Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
	else return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}

/**
 * @brief Sets pin to HIGH or LOW, depending on the value and invert flag.
 * If invert is TRUE, HIGH is set by FALSE and LOW by TRUE.
 * Else, HIGH is set by TRUE and LOW by FALSE.
 * @param value true/false
 */
void DigitalIoPin::write(bool value) {
	if(this->input) return;
	if(!this->invert) return Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
	else return Chip_GPIO_SetPinState(LPC_GPIO, port, pin, !value);
}
