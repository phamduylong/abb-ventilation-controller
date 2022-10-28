#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#define MAIN_DEBUG 1
#define SENSORS_DEBUG 0

#include <stdio.h>
#include <string>
#include <cmath>
#include "Event.h"
#include "retarget_uart.h"
#include "DigitalIoPin.h"
#include "sPressureSDP610.h"
#include "srhtHMP60.h"
#include "sco2GMP252.h"
#include "aFanMIO12V.h"
#include "SimpleMenu.h"
#include "DecimalEdit.h"
#include "DecimalShow.h"
#include "IntegerEdit.h"
#include "IntegerShow.h"
#include "LiquidCrystal.h"
#include "StatusSettingsJsonParser.h"
#include "Networking.h"

class StateMachine;
typedef void (StateMachine::*state_ptr)(const Event &);


class StateMachine {
public:
	StateMachine(LiquidCrystal *lcd, bool fast = false);
	virtual ~StateMachine();
	void HandleState(const Event& e);
	friend void mqtt_got_data(MessageData* data);
	enum smEvent {eAutoToggle = 4, eFastToggle = 5};
private:
	//States.
	state_ptr currentState;
	void sinit(const Event& e);
	void sauto(const Event& e);
	void smanual(const Event& e);
	void ssensors(const Event& e);

	//Helper functions.
	void SetState(state_ptr newState);
	void check_everything(bool retry = false);
	void check_sensors(bool retry = false);
	void set_fan(int speed);
	void check_fan(bool retry = false);
	void adjust_fan(float cur_pres, float des_pres);
	void readPres(bool retry = false);
	void readRhum(bool retry = false);
	void readTemp(bool retry = false);
	void readCo2(bool retry = false);
	void readCo2Long(bool retry = false);
	//Display functions.
	void screen_lock(PropertyEdit *pe);
	void screen_unlock(PropertyEdit *pe);
	void screens_update();
	void screens_pres_fan_update();

	//Lcd and timers.
	LiquidCrystal *lcd;              //lcd display
	unsigned int rhum_timer;         //Counts ticks for relative humidity reading.
	const unsigned int rhum_timeout; //How many ticks to wait before relative humidity reading.
	unsigned int temp_timer;         //Counts ticks for relative humidity reading.
	const unsigned int temp_timeout; //How many ticks to wait before relative humidity reading.
	unsigned int co2_timer;          //Counts ticks for co2 reading.
	const unsigned int co2_timeout;  //How many ticks to wait before co2 reading.
	uint8_t co2_readings;            //Number of co2 readings.
	unsigned int fan_timer;          //Counts ticks for fan adjustment.
	const unsigned int fan_timeout;  //How many ticks to wait before fan adjustment.
	uint8_t pres_readings;           //Number of pressure readings.
	unsigned int mqtt_timer;         //Counts ticks for mqtt send.
	const unsigned int mqtt_timeout; //How many ticks to wait before mqtt send.
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
	aFanMIO12V fan;

	//MQTT and JSON
	Networking network;
	StatusSettingsJsonParser mqtt_json_parser;
	unsigned int mqtt_messagenum;
	uint8_t mqtt_attempts;
	const uint8_t mqtt_rec_attempts;
	void mqtt_send_data();
	void mqtt_get_data();
	void mqtt_parse_data();
	void mqtt_reconnect();

	//Variables.
	float co2;                   //Last co2 reading
	float temp;                  //Last temperature reading
	float rh;                    //Last relative humidity reading
	float pres;                  //Last pressure reading
	float despres;               //Desired pressure value
	int16_t fan_speed;           //Current fan speed
	int16_t desfan_speed;        //Desired fan speed
	unsigned int operation_time; //Modified by various functions, containing how much clock cycles it took to execute.
	//Interface text
	const char titles[6][13] = {"Rel Humidity", "Temperature ", "CO2 Level   ", "Fan Speed   ", "Set Pressure", "Pressure    "};
	const char cunr = '!';
	const char cfea = ' ';
	const char cbusy = '*';
	const char cidle = ' ';
	const char cup = 'U';
	const char cdown = 'D';
	const char cauto = 'A';
	const char cman = 'M';
	//Flags.
	bool modeauto;    //Flag for auto mode.
	bool busy;        //Set true when input from screen can be unavailable for long period of time.
	bool fast;        //Set true to skip retries on manual/auto switch.
	bool unreachable; //Set true when unable to reach desired pressure using the fan in automatic mode.
	bool mod;         //State of modifying interface value flag.
	//Sensor/Devices flags.
	bool sfrht_up;
	bool sfco2_up;
	bool sfpres_up;
	bool affan_up;
};

#endif /* STATEMACHINE_H_ */