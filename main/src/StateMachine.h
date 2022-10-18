#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include "Event.h"
#include "LpcUart.h"
#include "DigitalIoPin.h"
#include "sPressureSDP610.h"
#include "srhtHMP60.h"
#include "sco2GMP252.h"
#include "dFanMIO12V.h"
#include "SimpleMenu.h"
#include "DecimalEdit.h"
#include "IntegerEdit.h"
#include "IntegerUnitEdit.h"
#include "LiquidCrystal.h"

class StateMachine;
typedef void (StateMachine::*state_ptr)(const Event &);

class StateMachine {
public:
	StateMachine(LpcUart *uart, LiquidCrystal *lcd);
	virtual ~StateMachine();
	void HandleState(const Event& e);
private:
	//States.
	state_ptr currentState;
	void sinit(const Event& e);
	void stemp(const Event& e);
	
	//Functions.
	void SetState(state_ptr newState);

	//Variables.
	LpcUart *uart; //uart for debug prints.
	LiquidCrystal *lcd; //lcd display
	int timer; //Counts ticks for state.
	int timeout; //How many ticks will state remain.
	SimpleMenu menu;
	//DecimalEdit *mrhum;
	//DecimalEdit *mtemp;
	//DecimalEdit *mco2;
	IntegerUnitEdit *mfanm;
	//DecimalEdit *mpresm;
	//IntegerUnitEdit *mfana;
	//DecimalEdit *mpresa;

	sPressureSDP610 spres;
	srhtHMP60 srht;
	sco2GMP252 sco2;
	dFanMIO12V fan;
};

#endif /* STATEMACHINE_H_ */