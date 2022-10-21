#include "StateMachine.h"

StateMachine::StateMachine(LpcUart *uart, LiquidCrystal *lcd, bool fast) : uart(uart), lcd(lcd), fast(fast) {
	this->timeout = 100; //Buttons are read every millisecond. Sensors should be checked every 100ms.
	this->rh = 0;
	this->temp = 0;
	this->co2 = 0;
	this->pres = 0;
	this->fan_speed = 0;
	this->modeauto = true;
	this->busy = true;
	
	//Initialise all possible menu items.
	this->mrhum = new DecimalShow(this->lcd, std::string("Rel Humidity*DA"), std::string("%"));
	this->mirhum = new MenuItem(this->mrhum);
	this->mtemp = new DecimalShow(this->lcd, std::string("Temperature *DA"), std::string("C"));
	this->mitemp = new MenuItem(this->mtemp);
	this->mco2 = new DecimalShow(this->lcd, std::string("CO2 Level   *DA"), std::string("ppm"));
	this->mico2 = new MenuItem(this->mco2);
	this->mfan = new IntegerEdit(this->lcd, std::string("Fan Speed   *DA"), 100, 0, 1, std::string("%"), false);
	this->mifan = new MenuItem(this->mfan);
	this->mpres = new DecimalEdit(this->lcd, std::string("Pressure    *DA"), 135.0, 0.0, 1.0, std::string("Pa"), false);
	this->mipres = new MenuItem(this->mpres);

	//Add all menu items to the menu, don't care for the mode just yet.
	this->menu.addItem(this->mirhum);
	this->menu.addItem(this->mitemp);
	this->menu.addItem(this->mico2);
	this->menu.addItem(this->mifan);
	this->menu.addItem(this->mipres);

	//Show everything on LCD
	this->menu.event(MenuItem::show);

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
	delete this->mfan;
	delete this->mifan;
	delete this->mpres;
	delete this->mipres;
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
//Upon initialisation and auto/manual switch (auto/man switch can be avoided with "fast" flag) the program runs a slow* connection enstablishment with all sensors.
//*Connection is slow only in case some sensor is not responding or responds with error.
//If all sensors are fine, it takes 5-6ms. Upon failure it waits for 100ms to retry the connection with sensor until runs out of retries.

/**
 * @brief Handles initialisation.
 * @paragraph State Init. Initialises UI and enstablishes connection with sensors.
 * Nothing is modifiable.
 * @param e 
 */
void StateMachine::sinit(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered sinit.\n");
		this->timer = 0;
		//Try to enstablish connection with every sensor. (Can take quite a while if sensors were connected incorrectly)
		this->check_sensors(true);
		// Debug prints.
		printf("Initialised sensors. Time elapsed: %d\n", this->operation_time);
		printf("RHum/Temp sensor state: %s\n", this->sfrht_up ? "UP" : "DOWN");
		printf("Pressure  sensor state: %s\n", this->sfpres_up ? "UP" : "DOWN");
		printf("CO2       sensor state: %s\n", this->sfco2_up ? "UP" : "DOWN");
		// /Debug prints.
		break;
	case Event::eExit:
		this->busy = false;
		//Set screens.
		this->screens_update();
		printf("Exited sinit.\n");
		break;
	case Event::eSwitchAuto:
		this->modeauto = !this->modeauto;
		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) this->modeauto ? SetState(&StateMachine::sauto) : SetState(&StateMachine::smanual);
		break;
	default:
		break;
	}
}

/**
 * @brief Handles automatic state.
 * @paragraph State Auto. Every tick reads sensors and handles the menu.
 * Allows to modify pressure value in order to set fan speed according to the desired pressure difference.
 * TODO: Add fan adjustment.
 * @param e Event to handle.
 */
void StateMachine::sauto(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered sauto.\n");
		this->timer = 0;
		//Unlock Pressure menu.
		this->screen_unlock(this->mpres);
		//Set all titles to A.
		this->screens_update();
		break;
	case Event::eExit:
		//Lock Pressure menu.
		this->screen_lock(this->mpres);
		printf("Exited sauto.\n");
		break;
	case Event::eSwitchAuto:
		this->modeauto = false;
		SetState(&StateMachine::ssensors);
		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) check_sensors();//Quickly read sensors //SetState(&StateMachine::ssensors);
		break;
	default:
		break;
	}
}

/**
 * @brief Handles manual state.
 * @paragraph State Manual. Every tick reads sensors and handles the menu.
 * Allows to modify fan speed value in order to set it up on the fan.
 * TODO: Add fan adjustment.
 * @param e Event to handle.
 */
void StateMachine::smanual(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered smanual.\n");
		this->timer = 0;
		//Unlock Fan menu.
		this->screen_unlock(this->mfan);
		//Set all titles to M.
		this->screens_update();
		break;
	case Event::eExit:
		//Lock Fan menu.
		this->screen_lock(this->mfan);
		printf("Exited smanual.\n");
		break;
	case Event::eSwitchAuto:
		this->modeauto = true;
		SetState(&StateMachine::ssensors);
		break;
	case Event::eTick:
		this->timer++;
		if (this->timer >= this->timeout) check_sensors();//Quickly read sensors //SetState(&StateMachine::ssensors);
		break;
	default:
		break;
	}
}

/**
 * @brief Handles sensors state.
 * @paragraph State sensors. Executed only on auto/manual state switch in order to attempt reinstablishing connection with sensors.
 * It is done in order to allow retries, which won't be done during the auto/manual state handling to save time.
 * 
 * @param e Event to handle.
 */
void StateMachine::ssensors(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered ssensors.\n");
		this->timer = 0;
		this->busy = true;
		this->screens_update(); //Might cause some screen flickering if sensors are ok. (Currently fastest exec time is 5-6ms)
		this->check_sensors(!fast);
		// Debug prints.
		printf("State switch handled. Time elapsed: %d\n", this->operation_time);
		printf("RHum/Temp sensor state: %s\n", this->sfrht_up ? "UP" : "DOWN");
		printf("Pressure  sensor state: %s\n", this->sfpres_up ? "UP" : "DOWN");
		printf("CO2       sensor state: %s\n", this->sfco2_up ? "UP" : "DOWN");
		// /Debug prints.
		this->modeauto ? SetState(&StateMachine::sauto) : SetState(&StateMachine::smanual);
		break;
	case Event::eExit:
		this->busy = false;
		printf("Exited ssensors.\n");
		break;
	//Do nothing on tick and auto/manual switch, we are too busy.
	case Event::eSwitchAuto:
		break;
	case Event::eTick:
		break;
	default:
		break;
	}
}

//////////////////////////////////////////////////////////////////////////////
// Helpers ///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void StateMachine::check_sensors(bool retry) {
	this->operation_time = 0;
	//We should be able to communicate with all sensors.
	sfrht_up = this->srht.read(this->temp, this->rh, retry);
	this->operation_time += this->srht.get_elapsed_time();
	sfpres_up = this->spres.read(this->pres, retry);
	this->operation_time += this->spres.get_elapsed_time();
	//CO2 sensor requires other sensors readings. (TODO: implement that functionality)
	sfco2_up = this->sco2.read(this->co2, retry);
	this->operation_time += this->sco2.get_elapsed_time();
}
/*
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
*/
void StateMachine::set_fan(int16_t speed) {
	this->dffan_up = this->fan.set_speed(speed, !fast);
	this->operation_time += this->fan.get_elapsed_time();
}

/**
 * @brief Prohibits screen modification if it is allowed.
 * 
 * @param pe Property, which allows to change modifiable flag.
 */
void StateMachine::screen_lock(PropertyEdit *pe) {
	pe->setStatus(false);
}

/**
 * @brief Allows screen modification if it is allowed.
 * 
 * @param pe Property, which allows to change modifiable flag.
 */
void StateMachine::screen_unlock(PropertyEdit *pe) {
	pe->setStatus(true);
}

/**
 * @brief Updates every screen title and value.
 */
void StateMachine::screens_update() {
	//Feed current sensor readings.
	this->mrhum->setValue(this->rh);
	this->mtemp->setValue(this->temp);
	this->mco2->setValue(this->co2);
	this->mpres->setValue(this->pres);
	this->mfan->setValue(this->fan_speed);

	//Setting titles according to flags.
	char buf[17];
	//Relative Humidity screen.
	snprintf(buf, 17, "%s%c%c%c", this->titles[0],
	this->busy ? this->cbusy : this->cidle,
	this->sfrht_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mrhum->setTitle(buf);
	//Temperature screen.
	snprintf(buf, 17, "%s%c%c%c", this->titles[1],
	this->busy ? this->cbusy : this->cidle,
	this->sfrht_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mtemp->setTitle(buf);
	//CO2 screen.
	snprintf(buf, 17, "%s%c%c%c", this->titles[2],
	this->busy ? this->cbusy : this->cidle,
	this->sfco2_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mco2->setTitle(buf);
	//Pressure screen.
	snprintf(buf, 17, "%s%c%c%c", this->titles[3],
	this->busy ? this->cbusy : this->cidle,
	this->sfpres_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mpres->setTitle(buf);
	//Fan screen.
	snprintf(buf, 17, "%s%c%c%c", this->titles[4],
	this->busy ? this->cbusy : this->cidle,
	this->dffan_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mfan->setTitle(buf);
}