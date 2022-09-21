#include "DigitalIoPin.h"
#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

//constructor
DigitalIoPin::DigitalIoPin(int _port, int _pin, bool input, bool pullup, bool _invert)
							: port(_port), pin(_pin), invert(_invert)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
	if (input == true) {
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	} else {
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
	}
}

//read pin value
bool DigitalIoPin::read() {
	return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}

//write value to pin
void DigitalIoPin::write(bool value) {
	//Board_LED_Set()
	if (invert) Chip_GPIO_SetPinState(LPC_GPIO, port, pin, !value);
	else Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
}

//dummy destructor so that complier wont complain
DigitalIoPin::~DigitalIoPin() {

}
