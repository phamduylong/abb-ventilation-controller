#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include "DigitalIoPin.h"
#include "LpcUart.h"
#include "LiquidCrystal.h"
#include "I2C.h"
#include "I2CDevice.h"

static volatile int counter = 0;
static volatile unsigned int systicks = 0;
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	systicks++;
	if(counter > 0) counter--;
}

unsigned int get_ticks(void)
{
	return systicks;
}

#ifdef __cplusplus
}
#endif

void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
#endif
#endif
	//RIT
    Chip_RIT_Init(LPC_RITIMER);
	
	//SysTick
	uint32_t sysTickRate;
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / 1000);


	//UART setup
	LpcPinMap none = {-1, -1}; // unused pin has negative values in it
	LpcPinMap txpin = { 0, 18 }; // transmit pin that goes to debugger's UART->USB converter
	LpcPinMap rxpin = { 0, 13 }; // receive pin that goes to debugger's UART->USB converter
	LpcUartConfig cfg = { LPC_USART0, 115200, UART_CFG_DATALEN_8 | UART_CFG_PARITY_NONE | UART_CFG_STOPLEN_1, false, txpin, rxpin, none, none };
	LpcUart uart(cfg);

	//LCD pins init.
    DigitalIoPin rs(0, 29, false, true, false);
    DigitalIoPin en(0, 9, false, true, false);
    DigitalIoPin d4(0, 10, false, true, false);
    DigitalIoPin d5(0, 16, false, true, false);
    DigitalIoPin d6(1, 3, false, true, false);
    DigitalIoPin d7(0, 0, false, true, false);
    rs.write(false);
    en.write(false);
    d4.write(false);
    d5.write(false);
    d6.write(false);
    d7.write(false);

    //LCD
    LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7, false);
    lcd.begin(16,2);
    lcd.setCursor(0,0);
	lcd.print("Septentrinoalis");

	//I2C device. (Sensirion SDP610_125kPa pressure sensor)
    I2C i2c;
    I2CDevice i2c_sSDP610_pressure(&i2c, (uint8_t)0x40);

	while(1) {
		Sleep(100);
	}
	return 0 ;
}
