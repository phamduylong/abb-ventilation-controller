#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <cstdio>
#include <cmath>
#include "DigitalIoPin.h"
#include "LpcUart.h"
#include "systick.h"
#include "mqtt/esp8266_socket.h"
#include "retarget_uart.h"
#include "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h"
#include "LiquidCrystal.h"
#include "Event.h"
#include "EventQueue.h"
#include "StateMachine.h"

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

uint32_t get_ticks(void)
{
	return systicks;
}

#ifdef __cplusplus
}
#endif

// This function is required by the modbus library
uint32_t millis() {
	return systicks;
}

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
	//This call initialises debug uart for stdout redirection
	retarget_init();
	//Set up SWO to PIO1_2
	Chip_SWM_MovablePortPinAssign(SWM_SWO_O, 1, 2); //Needed for SWO printf
	//RIT
	Chip_RIT_Init(LPC_RITIMER);
	
	//SysTick
	uint32_t sysTickRate;
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / 1000);

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

	//Buttons init
	DigitalIoPin control(1, 8, true, true, true);
	DigitalIoPin up_ok(0, 5, true, true, true);
	DigitalIoPin down_back(0, 6, true, true, true);
	DigitalIoPin auto_fast(0, 7, true, true, true);

	//LCD
	LiquidCrystal *lcd = new LiquidCrystal(&rs, &en, &d4, &d5, &d6, &d7);
	lcd->begin(16, 2);
	lcd->setCursor(0, 0);
	lcd->clear();

	StateMachine base(lcd, true);
	EventQueue events;

	unsigned int back_timer = 0;
	unsigned int back_timout = 10000;
	unsigned int up_ok_held = 0;
	unsigned int down_back_held = 0;
	bool control_pressed = false; //control button flag.
	bool up_ok_pressed = false; //"up"/"ok" button flag.
	bool down_back_pressed = false; //"down"/"back" button flag.
	bool auto_fast_pressed = false; //"auto"/"fast" button flag.
	while(1) {
		Sleep(1);
		back_timer++;
		//Control button
		if(control.read()) {
			control_pressed = true;
			back_timer = 0;
		}
		else if(control_pressed){
			control_pressed = false;
		}
		//Up / (Control) Ok
		if(up_ok.read()) {
			up_ok_pressed = true;
			back_timer = 0;
			++up_ok_held;
			//Button held more than 1s, send rapid commands "up" if control is released.
			if(!control_pressed && up_ok_held >= 250) {
				events.publish(Event(Event::eKey, MenuItem::up)); //Up
				up_ok_held -= 11; //Avoid overflow.
			}
		}
		else if(up_ok_pressed) {
			if(control_pressed) events.publish(Event(Event::eKey, MenuItem::ok)); //Ok
			else events.publish(Event(Event::eKey, MenuItem::up)); //Up
			up_ok_pressed = false;
			up_ok_held = 0;
		}
		//Down / (Control) Back
		if(down_back.read()) {
			down_back_pressed = true;
			back_timer = 0;
			++down_back_held;
			//Button held more than 1s, send rapid commands "down" if control is released.
			if(!control_pressed && down_back_held >= 250) {
				events.publish(Event(Event::eKey, MenuItem::down)); //Down
				down_back_held -= 11; //Avoid overflow.
			}
		}
		else if(down_back_pressed) { 
			if(control_pressed) events.publish(Event(Event::eKey, MenuItem::back)); //Back
			else events.publish(Event(Event::eKey, MenuItem::down)); //Down
			down_back_pressed = false;
			down_back_held = 0;
		}
		//Auto / (Control) Fast (No rapid commands for you!)
		if(auto_fast.read()) {
			auto_fast_pressed = true;
			back_timer = 0;
		}
		else if(auto_fast_pressed) {
			if(control_pressed) events.publish(Event(Event::eKey, StateMachine::eFastToggle)); //Fast
			else events.publish(Event(Event::eKey, StateMachine::eAutoToggle)); //Auto
			auto_fast_pressed = false;
		}

		//If manual interface is left alone - dispatch "back" event around every 10s.
		//(*Needed for web functionality, since manual interface has a greater priority, locking any modifications from the web interface.)
		if(back_timer >= back_timout) {
			events.publish(Event(Event::eKey, MenuItem::back)); //Back
			back_timer = 0;
		}

		//Tick is given every 1 ms.
		events.publish(Event(Event::eTick, 0));

		while (events.pending()) {
			base.HandleState(events.consume());
		}
	}
	return 0;
}
