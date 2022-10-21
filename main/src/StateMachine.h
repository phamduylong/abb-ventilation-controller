#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <stdio.h>
#include <string>
#include "Event.h"
#include "retarget_uart.h"
#include "DigitalIoPin.h"
#include "sPressureSDP610.h"
#include "srhtHMP60.h"
#include "sco2GMP252.h"
#include "dFanMIO12V.h"
#include "SimpleMenu.h"
#include "DecimalEdit.h"
#include "DecimalShow.h"
#include "IntegerEdit.h"
#include "IntegerShow.h"
#include "LiquidCrystal.h"

class StateMachine;
typedef void (StateMachine::*state_ptr)(const Event &);

class StateMachine {
public:
	StateMachine(LiquidCrystal *lcd, bool fast = false);
	virtual ~StateMachine();
	void HandleState(const Event& e);
	enum smEvent {eAutoToggle = 4, eFastToggle = 5};
private:
	//States.
	state_ptr currentState;
	void sinit(const Event& e);
	void sauto(const Event& e);
	void smanual(const Event& e);
	void ssensors(const Event& e);

	//Functions.
	void SetState(state_ptr newState);
	void check_sensors(bool retry = false);
	void set_fan(int speed);
	void adjust_fan();
	void readPres(bool retry = false);
	//Display functions.
	void screen_lock(PropertyEdit *pe);
	void screen_unlock(PropertyEdit *pe);
	void screens_update();

	//Uart, Lcd and timer.
	LpcUart *uart; //uart for debug prints.
	LiquidCrystal *lcd; //lcd display
	unsigned int timer; //Counts ticks for state.
	unsigned int timeout; //How many ticks will state remain.
	unsigned int fan_timer;
	unsigned int fan_timeout;
	//Menu.
	SimpleMenu menu;
	DecimalShow *mrhum;
	DecimalShow *mtemp;
	DecimalShow *mco2;
	IntegerEdit *mfan;
	DecimalEdit *mpres;
	DecimalShow *mpresm;
	MenuItem *mirhum;
	MenuItem *mitemp;
	MenuItem *mico2;
	MenuItem *mifan;
	MenuItem *mipres;
	MenuItem *mipresm;

	//Sensors and devices.
	sPressureSDP610 spres;
	srhtHMP60 srht;
	sco2GMP252 sco2;
	dFanMIO12V fan;

	//Variables.
	float co2;
	float temp;
	float rh;
	float pres;
	float despres;
	int16_t fan_speed;
	int16_t desfan_speed;
	unsigned int operation_time;
	const char titles[6][13] = {"Rel Humidity", "Temperature ", "CO2 Level   ", "Fan Speed   ", "Set Pressure", "Pressure    "};
	const char cbusy = '*';
	const char cidle = ' ';
	const char cup = 'U';
	const char cdown = 'D';
	const char cauto = 'A';
	const char cman = 'M';
	//Flags.
	bool modeauto; //Flag for auto mode.
	bool busy; //Set true when input from screen can be unavailable for long period of time.
	bool fast; //Set true to skip retries on manual/auto switch.
	//Sensor/Devices flags.
	bool sfrht_up;
	bool sfco2_up;
	bool sfpres_up;
	bool dffan_up;
};

#endif /* STATEMACHINE_H_ */