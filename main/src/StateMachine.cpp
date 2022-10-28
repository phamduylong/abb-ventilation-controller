#include "StateMachine.h"

std::string json_str_settings = "Lazy mqtt";

StateMachine::StateMachine(LiquidCrystal *lcd, bool fast):
lcd(lcd),
rhum_timeout(500), temp_timeout(500), co2_timeout(500), //Buttons are read every millisecond. Sensors should be checked every 0.5s.
fan_timeout(13), //Update fan speed every 13ms.
mqtt_timeout(1000), //Send mqtt messages every 1s (actual gap might be bigger)
spres{3, 50}, srht{3, 50}, sco2{3, 50}, fan{3, 50}, network{"SmartIotMQTT", "SmartIot", "192.168.1.254", 1883},
mqtt_rec_attempts(20), fast(fast) {
	this->rh = 0;
	this->temp = 0;
	this->co2 = 0;
	this->pres = 0;
	this->despres = 0;
	this->fan_speed = 0;
	this->desfan_speed = 0;
	this->operation_time = 0;
	this->mqtt_messagenum = 0;
	this->mqtt_attempts = 0;
	this->modeauto = true;
	this->busy = true;
	this->unreachable = false;
	this->mod = false;
	
	//Initialise all menu items.
	this->mrhum = new DecimalShow(this->lcd, std::string("Rel Humidity *DA"), std::string("%"));
	this->mirhum = new MenuItem(this->mrhum);
	this->mtemp = new DecimalShow(this->lcd, std::string("Temperature  *DA"), std::string("C"));
	this->mitemp = new MenuItem(this->mtemp);
	this->mco2 = new DecimalShow(this->lcd, std::string("CO2 Level    *DA"), std::string("ppm"));
	this->mico2 = new MenuItem(this->mco2);
	this->mfan = new IntegerEdit(this->lcd, std::string("Fan Speed    *DA"), 100, 0, 1, std::string("%"), false);
	this->mifan = new MenuItem(this->mfan);
	this->mpresm = new DecimalShow(this->lcd, std::string("Pressure     *DA"), std::string("Pa"));
	this->mipresm = new MenuItem(this->mpresm);
	this->mpres = new DecimalEdit(this->lcd, std::string("Set Pressure *DA"), 120.0, 0.0, 1.0, std::string("Pa"), true);
	this->mipres = new MenuItem(this->mpres);

	//Add all menu items to the menu, don't care for the mode just yet.
	this->menu.addItem(this->mirhum);
	this->menu.addItem(this->mitemp);
	this->menu.addItem(this->mico2);
	this->menu.addItem(this->mifan);
	this->menu.addItem(this->mipresm);
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
	delete this->mpresm;
	delete this->mipresm;
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
 * Set Pressure is modifiable. (But cannot be switched to)
 * @param e Event to handle.
 */
void StateMachine::sinit(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered sinit.\n");
		this->rhum_timer = 0;
		this->temp_timer = 0;
		this->co2_timer = 0;
		this->fan_timer = 0;
		this->mqtt_timer = 0;
		this->co2_readings = 0;
		this->pres_readings = 0;
		//Try to enstablish connection with every sensor. (Can take quite a while if sensors were connected incorrectly)
		this->check_sensors(true);
		printf("Sensors init. Time elapsed: %fms\n", (float) this->operation_time / 72000);
		this->set_fan(0); //Setting the fan, not checking it, since we always initialise with 0.
		printf("Fan init. Time elapsed: %fms\n", (float) this->operation_time / 72000);
		// DEBUG prints.
		printf("RHum/Temp sensor state: %s\n", this->sfrht_up ? "UP" : "DOWN");
		printf("Pressure  sensor state: %s\n", this->sfpres_up ? "UP" : "DOWN");
		printf("CO2       sensor state: %s\n", this->sfco2_up ? "UP" : "DOWN");
		printf("Fan     actuator state: %s\n", this->affan_up ? "UP" : "DOWN");
		// /DEBUG prints.
		this->modeauto ? SetState(&StateMachine::sauto) : SetState(&StateMachine::smanual);
		break;
	case Event::eExit:
		this->busy = false;
		//Set screens.
		this->screens_update();
		printf("Exited sinit.\n");
		break;
	//Do nothing on tick and key press, we are too busy.
	case Event::eKey:
		break;
	case Event::eTick:
		break;
	default:
		break;
	}
}

/**
 * @brief Handles automatic state.
 * @paragraph State Auto. Every tick reads sensors and handles the menu.
 * Allows to modify Set Pressure value in order to set fan speed according to the desired pressure difference.
 * @param e Event to handle.
 */
void StateMachine::sauto(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered sauto.\n");
		this->rhum_timer = 200;
		this->temp_timer = 100;
		this->co2_timer = 0;
		this->fan_timer = 0;
		this->mqtt_timer = 0;
		this->co2_readings = 0;
		this->pres_readings = 0;
		//If user is in modification - discard it.
		if(this->mod) {
			this->mod = false;
			this->menu.event(MenuItem::back);
		}
		//Set all titles to A.
		this->screens_update();
		break;
	case Event::eExit:
		this->unreachable = false;
		printf("Exited sauto.\n");
		break;
	case Event::eKey:
		switch (e.value)
		{
		case MenuItem::up:
			this->menu.event(MenuItem::up);
			break;
		case MenuItem::down:
			this->menu.event(MenuItem::down);
			break;
		case MenuItem::ok:
			if (this->menu.event(MenuItem::ok)) this->mod = !this->mod;
			break;
		case MenuItem::back:
			this->mod = false;
			this->menu.event(MenuItem::back);
			break;
		case eAutoToggle:
			this->modeauto = false;
			SetState(&StateMachine::ssensors);
			break;
		case eFastToggle:
			this->fast = !this->fast;
			//this->mqtt_get_data();
			break;
		default:
			break;
		}
		break;
	case Event::eTick:
		this->rhum_timer++;
		this->temp_timer++;
		this->co2_timer++;
		this->fan_timer++;
		this->mqtt_timer++;
		//Every 0.5s by default. Starts after 300ms timeout.
		if (this->rhum_timer >= this->rhum_timeout) {
			this->readRhum(); //Quickly read relative humidity and temperature
			if(SENSORS_DEBUG) printf("Rel Hum reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			this->rhum_timer = 0;
		}
		//Every 0.5s by default. Starts after 400ms timeout.
		if (this->temp_timer >= this->temp_timeout) {
			this->readTemp(); //Quickly read temperature
			if(SENSORS_DEBUG) printf("Temperature reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			this->temp_timer = 0;
		}
		//Every 0.5s by default.
		if (this->co2_timer >= this->co2_timeout) {
			this->co2_readings++;
			if (this->co2_readings >= 20) {
				this->busy = true;
				if(!this->mod) this->screens_update();
				this->co2_readings = 0;
				this->readCo2Long(); //Slowly read co2 after 20 reads. Should work around every 10s
				this->busy = false;
			}
			else {
				this->readCo2(); //Quickly read co2
			}
			if(SENSORS_DEBUG) printf("CO2 reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			this->co2_timer = 0;
			if(!this->mod) this->screens_update();
		}
		//Every 13ms by default.
		if (this->fan_timer >= this->fan_timeout) {
			this->pres_readings++;
			this->fan_timer = 0;
			this->despres = this->mpres->getValue();
			this->readPres(); //Takes 3ms.
			if(SENSORS_DEBUG) printf("Pressure reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			this->adjust_fan(this->pres, this->despres);
			if(SENSORS_DEBUG) printf("Fan setting. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			if(!this->mod && this->pres_readings > 5) {
				this->pres_readings = 0;
				this->screens_pres_fan_update();
			}
		}
		//Every 1s by default.
		if (this->mqtt_timer >= this->mqtt_timeout) {
			this->mqtt_timer = 0;
			this->mqtt_attempts++;
			if(!this->network.check_rc() && this->mqtt_attempts >= this->mqtt_rec_attempts) {
				this->busy = true;
				this->screens_update();
				this->mqtt_reconnect();
				this->busy = false;
				mqtt_attempts = 0;
			}
			this->mqtt_send_data();
			this->mqtt_get_data();
		}
		
		break;
	default:
		break;
	}
}

/**
 * @brief Handles manual state.
 * @paragraph State Manual. Every tick reads sensors and handles the menu.
 * Allows to modify fan speed value in order to set it up on the fan.
 * @param e Event to handle.
 */
void StateMachine::smanual(const Event& e) {
	switch (e.type) 
	{
	case Event::eEnter:
		printf("Entered smanual.\n");
		this->rhum_timer = 200;
		this->temp_timer = 100;
		this->co2_timer = 0;
		this->fan_timer = 0;
		this->mqtt_timer = 0;
		this->co2_readings = 0;
		this->pres_readings = 0;
		//Unlock Fan menu.
		this->screen_unlock(this->mfan);
		//If user is in modification - discard it.
		if(this->mod) {
			this->mod = false;
			this->menu.event(MenuItem::back);
		}
		//Set all titles to M.
		this->screens_update();
		break;
	case Event::eExit:
		//Lock Fan menu.
		this->screen_lock(this->mfan);
		printf("Exited smanual.\n");
		break;
	case Event::eKey:
		switch (e.value)
		{
		case MenuItem::up:
			this->menu.event(MenuItem::up);
			break;
		case MenuItem::down:
			this->menu.event(MenuItem::down);
			break;
		case MenuItem::ok:
			if (this->menu.event(MenuItem::ok)) this->mod = !this->mod;
			break;
		case MenuItem::back:
			this->mod = false;
			this->menu.event(MenuItem::back);
			break;
		case eAutoToggle:
			this->modeauto = true;
			SetState(&StateMachine::ssensors);
			break;
		case eFastToggle:
			this->fast = !this->fast;
			//this->mqtt_get_data();
			break;
		default:
			break;
		}
		break;
	case Event::eTick:
		this->rhum_timer++;
		this->temp_timer++;
		this->co2_timer++;
		this->fan_timer++;
		this->mqtt_timer++;
		//Every 0.5s by default. Starts after 300ms timeout.
		if (this->rhum_timer >= this->rhum_timeout) {
			this->readRhum(); //Quickly read relative humidity
			this->rhum_timer = 0;
			if(SENSORS_DEBUG) printf("Rel Hum reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
		}
		//Every 0.5s by default. Starts after 400ms timeout.
		if (this->temp_timer >= this->temp_timeout) {
			this->readTemp(); //Quickly read temperature
			this->temp_timer = 0;
			if(SENSORS_DEBUG) printf("Temperature reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
		}
		//Every 0.5s by default.
		if (this->co2_timer >= this->co2_timeout) {
			this->co2_readings++;
			if (this->co2_readings >= 20) {
				this->busy = true;
				if(!this->mod) this->screens_update();
				this->co2_readings = 0;
				this->readCo2Long(); //Slowly read co2 after 20 reads. Should work around every 10s
				this->busy = false;
			}
			else {
				this->readCo2(); //Quickly read co2
			}
			if(SENSORS_DEBUG) printf("CO2 reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			this->co2_timer = 0;
			if(!this->mod) this->screens_update();
		}
		//Every 13ms by default.
		if (this->fan_timer >= this->fan_timeout) {
			this->pres_readings++;
			this->fan_timer = 0;
			this->despres = this->mpres->getValue();
			this->readPres(); //Takes 3ms.
			if(SENSORS_DEBUG) printf("Pressure reading. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			this->desfan_speed = this->mfan->getValue() * 10;
			this->set_fan(this->desfan_speed);
			if(SENSORS_DEBUG) printf("Fan setting. Time elapsed: %fms\n", (float) this->operation_time / 72000);
			if(!this->mod && this->pres_readings > 5) {
				this->pres_readings = 0;
				this->screens_pres_fan_update();
			}
		}
		//Every 1s by default.
		if (this->mqtt_timer >= this->mqtt_timeout) {
			this->mqtt_timer = 0;
			this->mqtt_attempts++;
			if(!this->network.check_rc() && this->mqtt_attempts >= this->mqtt_rec_attempts) {
				this->busy = true;
				this->screens_update();
				this->mqtt_reconnect();
				this->busy = false;
				mqtt_attempts = 0;
			}
			this->mqtt_send_data();
			this->mqtt_get_data();
		}
		
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
		this->rhum_timer = 0;
		this->temp_timer = 0;
		this->co2_timer = 0;
		this->fan_timer = 0;
		this->mqtt_timer = 0;
		this->co2_readings = 0;
		this->pres_readings = 0;
		this->busy = true;
		this->screens_update(); //Might cause some screen flickering if sensors are ok. (Currently fastest exec time is 5-6ms)
		this->check_everything(!fast);
		// DEBUG prints.
		printf("State switch handled. Time elapsed: %fms\n", (float) this->operation_time / 72000);
		printf("RHum/Temp sensor state: %s\n", this->sfrht_up ? "UP" : "DOWN");
		printf("Pressure  sensor state: %s\n", this->sfpres_up ? "UP" : "DOWN");
		printf("CO2       sensor state: %s\n", this->sfco2_up ? "UP" : "DOWN");
		printf("Fan     actuator state: %s\n", this->affan_up ? "UP" : "DOWN");
		// /DEBUG prints.

		this->modeauto ? SetState(&StateMachine::sauto) : SetState(&StateMachine::smanual);
		break;
	case Event::eExit:
		this->busy = false;
		printf("Exited ssensors.\n");
		break;
	//Do nothing on tick and key press, we are too busy.
	case Event::eKey:
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

/**
 * @brief Checks all sensors and actuators and changes according flags and variables.
 * @param retry Whether to retry communication.
 */
void StateMachine::check_everything(bool retry) {
	unsigned int time;
	this->check_sensors(retry); //operation_time is zeroed out in here.
	time = this->operation_time;
	this->check_fan(retry);
	this->operation_time += time;
}

/**
 * @brief Checks every sensor and changes according flags and variables.
 * @param retry Whether to retry communication.
 */
void StateMachine::check_sensors(bool retry) {
	this->operation_time = 0;
	//We should be able to communicate with all sensors.
	sfrht_up = this->srht.read(this->temp, this->rh, retry);
	this->operation_time += this->srht.get_elapsed_time();
	sfpres_up = this->spres.read(this->pres, retry);
	this->operation_time += this->spres.get_elapsed_time();
	//CO2 sensor requires other sensors readings. (Don't care about precision so much, thus simple read())
	sfco2_up = this->sco2.read(this->co2, this->pres, this->rh, retry);
	this->operation_time += this->sco2.get_elapsed_time();
}

/**
 * @brief Reads pressure value from pressure sensor.
 * Sets sfrpes_up flag to true/false as communication result.
 * Changes pres variable to current if communication succeeded.
 * @param retry Whether to retry communication.
 */
void StateMachine::readPres(bool retry) {
	this->operation_time = 0;
	this->sfpres_up = this->spres.read(this->pres, retry);
	this->operation_time += this->spres.get_elapsed_time();
}

/**
 * @brief Reads relative humidity value from relative humidity sensor.
 * Sets sfrht_up flag to true/false as communication result.
 * Changes relative humidity variable to current if communication succeeded.
 * @param retry Whether to retry communication.
 */
void StateMachine::readRhum(bool retry) {
	this->operation_time = 0;
	sfrht_up = this->srht.read_rhum(this->rh, retry);
	this->operation_time += this->srht.get_elapsed_time();
}

/**
 * @brief Reads temperature value from relative humidity sensor.
 * Sets sfrht_up flag to true/false as communication result.
 * Changes temperature variable to current if communication succeeded.
 * @param retry Whether to retry communication.
 */
void StateMachine::readTemp(bool retry) {
	this->operation_time = 0;
	sfrht_up = this->srht.read_temp(this->temp, retry);
	this->operation_time += this->srht.get_elapsed_time();
}

/**
 * @brief Reads co2 value from co2 sensor.
 * Sets sfco2_up flag to true/false as communication result.
 * Changes co2 variable to current if communication succeeded.
 * @param retry Whether to retry communication.
 */
void StateMachine::readCo2(bool retry) {
	this->operation_time = 0;
	sfco2_up = this->sco2.read_rapid(this->co2, retry);
	this->operation_time += this->sco2.get_elapsed_time();
}

/**
 * @brief Reads co2 value from co2 sensor. (Takes up to 350ms)
 * Sets sfco2_up flag to true/false as communication result.
 * Changes co2 variable to current if communication succeeded.
 * @param retry Whether to retry communication.
 */
void StateMachine::readCo2Long(bool retry) {
	this->operation_time = 0;
	//CO2 sensor requires other sensors readings. (Don't care about precision so much, thus simple read())
	sfco2_up = this->sco2.read(this->co2, this->pres, this->rh, retry);
	this->operation_time += this->sco2.get_elapsed_time();
}

/**
 * @brief Sets fan speed to provided.
 * Sets affan_up flag to true/false as communication result.
 * Changes fan_speed variable to provided if communication succeeded.
 * Always quick.
 * @param speed to set up on the fan.
 */
void StateMachine::set_fan(int speed) {
	this->check_fan(false); //operation_time zeroed out here. 
	if(speed != this->fan_speed) {
		this->affan_up = this->fan.set_speed(speed, false);
		this->operation_time += this->fan.get_elapsed_time();
		if (this->affan_up) this->fan_speed = speed;
	}
}

/**
 * @brief Checks if fan is up and gets current speed if it is.
 * Sets affan_up flag to true/false as communication result.
 * Changes fan_speed variable to current one if communication succeeded.
 * 
 * @param retry Whether to retry communication.
 */
void StateMachine::check_fan(bool retry) {
	this->operation_time = 0;
	this->affan_up = this->fan.get_speed(this->fan_speed, retry);
	this->operation_time += this->fan.get_elapsed_time();
}

/**
 * @brief Adjusts fan speed using simple increase or decrease of fan speed.
 * TODO: Make it adaptable to various fans.
 * @param cur_pres Current pressure level.
 * @param des_pres Desired pressure level.
 */
void StateMachine::adjust_fan(float cur_pres, float des_pres) {
	this->unreachable = false;
	//Check fan right away.
	this->check_fan();
	if(!this->affan_up) return; //return if it's unavailable.

	//Here should be some kind of complex calculation for fan adjustment, but it will do for now.
	//(It's ok with fast fan update rate.)
	if((round(cur_pres) > round(des_pres)) && this->fan_speed > 89) {
		this->set_fan((cur_pres - des_pres > 30.0) ? this->fan_speed - 10 : this->fan_speed - 1);
	}
	else if((round(cur_pres) < round(des_pres)) && this->fan_speed < 1000) {
		this->set_fan((des_pres - cur_pres > 30.0) ? this->fan_speed + 10 : this->fan_speed + 1);
	}
	else if((round(cur_pres) < round(des_pres)) && this->fan_speed == 1000) {
		this->unreachable = true;
	}
	else if((round(cur_pres) > round(des_pres)) && this->fan_speed == 0) {
		this->unreachable = true;
	}
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
	unsigned int start_time = DWT->CYCCNT;
	//Feed current sensor readings.
	this->mrhum->setValue(this->rh);
	this->mtemp->setValue(this->temp);
	this->mco2->setValue(this->co2);
	this->mfan->setValue(this->fan_speed / 10);
	this->mpres->setValue(this->despres);
	this->mpresm->setValue(this->pres);

	//Setting titles according to flags.
	char buf[18];
	//Relative Humidity screen.
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[0],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->sfrht_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mrhum->setTitle(buf);
	//Temperature screen.
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[1],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->sfrht_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mtemp->setTitle(buf);
	//CO2 screen.
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[2],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->sfco2_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mco2->setTitle(buf);
	//Fan screen.
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[3],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->affan_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mfan->setTitle(buf);
	//Pressure screen. (Setting desired pressure)
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[4],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->sfpres_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mpres->setTitle(buf);
	//Pressure Manual screen (With the sensor reading)
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[5],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->sfpres_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mpresm->setTitle(buf);
	//Show everything on LCD
	this->menu.event(MenuItem::show);

	this->operation_time = DWT->CYCCNT;
	if (this->operation_time > start_time) this->operation_time -= start_time;
	else this->operation_time = 0xffffffff - start_time + 1 + this->operation_time;
	if(MAIN_DEBUG) printf("Screen update. Time elapsed: %fms\n", (float) this->operation_time / 72000);
}

/**
 * @brief Updates pressure and fan screens title and value.
 */
void StateMachine::screens_pres_fan_update() {
	this->mfan->setValue(this->fan_speed / 10);
	this->mpresm->setValue(this->pres);

	//Setting titles according to flags.
	char buf[18];
	//Fan screen.
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[3],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->affan_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mfan->setTitle(buf);
	//Pressure Manual screen (With the sensor reading)
	snprintf(buf, 18, "%s%c%c%c%c", this->titles[5],
	this->unreachable ? this->cunr : this->cfea,
	this->busy ? this->cbusy : this->cidle,
	this->sfpres_up ? this->cup : this->cdown,
	this->modeauto ? this->cauto : this->cman);
	this->mpresm->setTitle(buf);
	//Show everything on LCD
	this->menu.event(MenuItem::show);
}

/**
 * @brief TODO:
 * 
 */
void StateMachine::mqtt_send_data() {
	mqtt_messagenum++;
	status_data sd = {this->mqtt_messagenum, (int16_t)(this->fan_speed / 10),
	 this->modeauto ? (unsigned int)this->despres : (unsigned int)this->desfan_speed / 10,
	 this->pres, this->modeauto, this->unreachable, this->co2, this->rh, this->temp};

	std::string str = mqtt_json_parser.stringify(sd);
	if(MAIN_DEBUG) printf("MQTT MESSAGE!\n%s\n", str.c_str());

	//Actual MQTT send.
	if(!this->network.check_rc()){
		printf("Unable to connect to mqtt.\n");
		return;
	}
	this->network.MQTT_publish("controller/status", str.c_str());
}

void mqtt_got_data(MessageData* data) {
	json_str_settings = (char *)data->message->payload;
}

/**
 * @brief TODO:
 * 
 */
void StateMachine::mqtt_get_data() {
	if(!this->network.check_rc()){
		printf("Unable to connect to mqtt.\n");
		return;
	}
	network.MQTT_subscribe("controller/setting", mqtt_got_data);
	network.MQTT_yield(25);
	if(!this->network.check_rc() || !strcmp(json_str_settings.c_str(), "Lazy mqtt")){
		printf("Unable to connect to mqtt.\n");
		return;
	}
	this->mqtt_parse_data();
}

/**
 * @brief TODO:
 * 
 */
void StateMachine::mqtt_reconnect() {
	this->network.Network_reconnect();
	this->network.MQTT_reconnect();
}

void StateMachine::mqtt_parse_data() {
	mqtt_json_parser.parse_settings(json_str_settings);
	settings_data sd = mqtt_json_parser.getSettingsObj();
	//Pay attention to mqtt only if physical interface is not being modified.
	if(!this->mod) {
		//Setting mode and members according to the provided settings.
		if(sd.auto_mode != this->modeauto) {
			this->modeauto = !this->modeauto;
			if(this->modeauto) {
				if(sd.pressure > 120) this->despres = 120;
				else if(sd.pressure < 0) this->despres = 0;
				else this->despres = sd.pressure;
			}
			else this->desfan_speed = sd.speed * 10;
			SetState(&StateMachine::ssensors);
		}
		else {
			if(this->modeauto) {
				if(sd.pressure > 120) this->despres = 120;
				else if(sd.pressure < 0) this->despres = 0;
				else this->despres = sd.pressure;
			}
			else this->desfan_speed = sd.speed * 10;
			this->screens_update();
		}
	}
}
