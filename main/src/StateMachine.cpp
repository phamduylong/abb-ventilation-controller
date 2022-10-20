#include "StateMachine.h"

StateMachine::StateMachine(LpcUart *uart, LiquidCrystal *lcd, bool fast) : uart(uart), lcd(lcd), fast(fast) {
	this->timeout = 5000;
	this->rh = 0;
	this->temp = 0;
	this->co2 = 0;
	this->pres = 0;

	this->check_sensors();
	
	//Initialise all possible menu items.
	this->mrhum = new DecimalShow(this->lcd, std::string("Rel Humidity"), std::string("%"));
	this->mirhum = new MenuItem(this->mrhum);
	this->mtemp = new DecimalShow(this->lcd, std::string("Temperature"), std::string("C"));
	this->mitemp = new MenuItem(this->mtemp);
	this->mco2 = new DecimalShow(this->lcd, std::string("CO2 Level"), std::string("%"));
	this->mico2 = new MenuItem(this->mco2);
	this->mfana = new IntegerShow(this->lcd, std::string("Fan Speed"), std::string("%")); //Should be redundant.
	this->mifana = new MenuItem(this->mfana); //Should be redundant.
	this->mfanm = new IntegerEdit(this->lcd, std::string("Fan Speed"), 100, 0, 10, std::string("%"), true);
	this->mifanm = new MenuItem(this->mfanm);
	this->mpresa = new DecimalEdit(this->lcd, std::string("Pressure"), 100, 0, 10, std::string("%"), true);
	this->mipresa = new MenuItem(this->mpresa);
	this->mpresm = new DecimalShow(this->lcd, std::string("Pressure"), std::string("%")); //Should be redundant.
	this->mipresm = new MenuItem(this->mpresm); //Should be redundant.

	//Add all menu items to the menu, don't care for the mode just yet.
	this->menu.addItem(this->mirhum);
	this->menu.addItem(this->mitemp);
	this->menu.addItem(this->mico2);
	this->menu.addItem(this->mifana); //Should be redundant.
	this->menu.addItem(this->mifanm);
	this->menu.addItem(this->mipresa);
	this->menu.addItem(this->mipresm); //Should be redundant.

	//Go into init state
	this->currentState = &StateMachine::sinit;
	(this->*currentState)(Event(Event::eEnter));
}

//State machine should not be deleted at any point, but in case it happens - delete all allocated memory.
StateMachine::~StateMachine() {
	delete this->mrhum;
	delete this->mirhum;
	delete this->mtemp;
	delete this->mitemp;
	delete this->mco2;
	delete this->mico2;
	delete this->mfana; //Should be redundant.
	delete this->mifana; //Should be redundant.
	delete this->mfanm;
	delete this->mifanm;
	delete this->mpresa;
	delete this->mipresa;
	delete this->mpresm; //Should be redundant.
	delete this->mipresm; //Should be redundant.
}

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
		if (this->timer >= this->timeout) SetState(&StateMachine::sauto);
		break;
	default:
		break;
	}
}

void StateMachine::sauto(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		this->uart->write("Entered sauto.\r\n");
		this->timer = 0;
		break;
	case Event::eExit:
		this->uart->write("Exited sauto.\r\n");
		break;
	case Event::eSwitchAuto:

		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) SetState(&StateMachine::ssensors);
		break;
	default:
		break;
	}
}

void StateMachine::smanual(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		this->uart->write("Entered smanual.\r\n");
		this->timer = 0;
		break;
	case Event::eExit:
		this->uart->write("Exited smanual.\r\n");
		break;
	case Event::eSwitchAuto:

		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) SetState(&StateMachine::ssensors);
		break;
	default:
		break;
	}
}

void StateMachine::ssensors(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		this->uart->write("Entered ssensors.\r\n");
		this->timer = 0;
		break;
	case Event::eExit:
		this->uart->write("Exited ssensors.\r\n");
		break;
	case Event::eSwitchAuto:

		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) SetState(&StateMachine::sauto);
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Helpers ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void StateMachine::check_sensors() {
	//We should be able to communicate with all sensors.
	sfrht_up = this->srht.read(this->temp, this->rh, !fast);
	sfpres_up = this->spres.read(this->pres, !fast);
	//CO2 sensor requires other sensors readings. (TODO: implement that functionality)
	sfco2_up = this->sco2.read(this->co2, !fast);
}

void StateMachine::readRelHum() {
	this->sfrht_up = this->srht.read_rhum(this->rh, !fast);
	this->operation_time += this->srht.get_elapsed_time();
}

void StateMachine::readTemp() {
	this->sfrht_up = this->srht.read_temp(this->temp, !fast);
	this->operation_time += this->srht.get_elapsed_time();
}

void StateMachine::readCo2() {
	this->sfco2_up = this->sco2.read(this->co2, !fast);
	this->operation_time += this->sco2.get_elapsed_time();
}

void StateMachine::readPres() {
	this->sfpres_up = this->spres.read(this->pres, !fast);
	this->operation_time += this->spres.get_elapsed_time();
}

void StateMachine::set_fan(int16_t speed) {
	this->dffan_up = this->fan.set_speed(speed, !fast);
	this->operation_time += this->fan.get_elapsed_time();
}
