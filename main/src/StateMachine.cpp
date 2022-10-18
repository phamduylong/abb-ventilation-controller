#include "StateMachine.h"

StateMachine::StateMachine(LpcUart *uart, LiquidCrystal *lcd) : uart(uart), lcd(lcd) {
	this->timeout = 5000;
	this->currentState = &StateMachine::sinit;
	(this->*currentState)(Event(Event::eEnter));
	this->mfanm = new IntegerUnitEdit(this->lcd, std::string("Fan Speed"),100,0,1,std::string("%"));
	this->menu.addItem(new MenuItem(this->mfanm));
}

StateMachine::~StateMachine() {}

void StateMachine::SetState(state_ptr newState) {
	(this->*currentState)(Event(Event::eExit));
	this->currentState = newState;
	(this->*currentState)(Event(Event::eEnter));
}

void StateMachine::HandleState(const Event& e) {
	(this->*currentState)(e);
}

//////////////////////////////////////////////////////////////////////////////
// States ////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void StateMachine::sinit(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		this->uart->write("Entered sinit.\r\n");
		this->timer = 0;
		break;
	case Event::eExit:
		this->uart->write("Exited sinit.\r\n");
		break;
	case Event::eSwitchAuto:
		
		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) SetState(&StateMachine::stemp);
		break;
	default:
		break;
	}
}

void StateMachine::stemp(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		this->uart->write("Entered stemp.\r\n");
		this->timer = 0;
		break;
	case Event::eExit:
		this->uart->write("Exited stemp.\r\n");
		break;
	case Event::eSwitchAuto:

		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) SetState(&StateMachine::sinit);
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Helpers ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
