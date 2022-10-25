#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "DigitalIoPin.h"
#include "LpcUart.h"
#include "systick.h"
#include "mqtt/esp8266_socket.h"
#include "retarget_uart.h"
#include "modbus/ModbusMaster.h"
#include "modbus/ModbusRegister.h"
#include "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h"
#include "LiquidCrystal.h"
#include "I2C.h"
#include "I2CDevice.h"
#include "sPressureSDP610.h"
#include "sco2GMP252.h"
#include "srhtHMP60.h"
#include "SimpleMenu.h"
#include "IntegerEdit.h"
#include "IntegerShow.h"
#include "IntegerUnitEdit.h"
#include "DecimalEdit.h"
#include "DecimalShow.h"
#include "aFanMIO12V.h"
#include "Event.h"
#include "EventQueue.h"
#include "StateMachine.h"

#define SSID	    "SmartIotMQTT"
#define PASSWORD    "SmartIot"
#define BROKER_IP   "192.168.1.254"
#define BROKER_PORT  1883

//DEBUG DEFINES //Leave only one ON, or none.
#define LPC_PROJ 1 //Use this to switch from home-lpc to proj-lpc
#define MODBUS_TEST 0
#define FAN_TEST 0
#define HUM_TEMP_TEST 0
#define CO2_TEST 0
#define PRES_TEST 0
#define FAN_PRES_TEST 0
#define MQTT_TEST 0
//sw1 - A2 - 1 8
//sw2 - A3 - 0 5
//sw3 - A4 - 0 6
//sw4 - A5 - 0 7
// /DEBUG DEFINES

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

#if MODBUS_TEST
void abbModbusTest();
#endif
#if MQTT_TEST
void socketTest();
void mqttTest();
#endif
#if FAN_TEST
void produalModbusTest();
#endif
#if HUM_TEMP_TEST
void humidity_test();
#endif
#if CO2_TEST
void co2_test();
#endif
#if PRES_TEST
void pressure_test();
#endif
#if FAN_PRES_TEST
void fan_pressure_test();
#endif

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

	/////////////////////////////////////////////////////////////////
	//Here go all initialisations, needed for still existing tests.//
	/////////////////////////////////////////////////////////////////

	#if FAN_TEST
	produalModbusTest();
	#endif
	#if HUM_TEMP_TEST
	humidity_test();
	#endif
	#if CO2_TEST
	co2_test();
	#endif
	#if PRES_TEST
	pressure_test();
	#endif
	#if FAN_PRES_TEST
	fan_pressure_test();
	#endif

	/////////////////////////////////////////////////////////
	//Main test. Works only in case if other tests are off.//
	/////////////////////////////////////////////////////////

	//LCD pins init.
#if LPC_PROJ
	DigitalIoPin rs(0, 29, false, true, false);
	DigitalIoPin en(0, 9, false, true, false);
	DigitalIoPin d4(0, 10, false, true, false);
	DigitalIoPin d5(0, 16, false, true, false);
	DigitalIoPin d6(1, 3, false, true, false);
	DigitalIoPin d7(0, 0, false, true, false);
#else
	DigitalIoPin rs(0, 8, false, false, false);
	DigitalIoPin en(1, 6, false, false, false);
	DigitalIoPin d4(1, 8, false, false, false);
	DigitalIoPin d5(0, 5, false, false, false);
	DigitalIoPin d6(0, 6, false, false, false);
	DigitalIoPin d7(0, 7, false, false, false);
#endif
	rs.write(false);
	en.write(false);
	d4.write(false);
	d5.write(false);
	d6.write(false);
	d7.write(false);

	//Buttons init
#if LPC_PROJ
	DigitalIoPin control(1, 8, true, true, true);
	DigitalIoPin up_ok(0, 5, true, true, true);
	DigitalIoPin down_back(0, 6, true, true, true);
	DigitalIoPin auto_fast(0, 7, true, true, true);
#else
	DigitalIoPin sw1(0, 17 ,true ,true, true);
	DigitalIoPin sw2(1, 11 ,true ,true, true);
	DigitalIoPin sw3(1, 9 ,true ,true, true);
#endif

	//LCD
	LiquidCrystal *lcd = new LiquidCrystal(&rs, &en, &d4, &d5, &d6, &d7);
	lcd->begin(16, 2);
	lcd->setCursor(0, 0);
	lcd->clear();

#if LPC_PROJ
	StateMachine base(lcd, true);
	EventQueue events;

	unsigned int up_ok_held = 0;
	unsigned int down_back_held = 0;
	bool control_pressed = false; //control button flag.
	bool up_ok_pressed = false; //"up"/"ok" button flag.
	bool down_back_pressed = false; //"down"/"back" button flag.
	bool auto_fast_pressed = false; //"auto"/"fast" button flag.
	while(1) {
		Sleep(1);
		//Control button
		if(control.read()) {
			control_pressed = true;
		}
		else if(control_pressed){
			control_pressed = false;
		}
		//Up / Ok
		if(up_ok.read()) {
			up_ok_pressed = true;
			++up_ok_held;
			//Button held more than 1s, send rapid commands "up" if control is released.
			if(!control_pressed && up_ok_held >= 1000) {
				events.publish(Event(Event::eKey, MenuItem::up)); //Up
				up_ok_held -= 15; //Avoid overflow.
			}
		}
		else if(up_ok_pressed) {
			if(control_pressed) events.publish(Event(Event::eKey, MenuItem::ok)); //Ok
			else events.publish(Event(Event::eKey, MenuItem::up)); //Up
			up_ok_pressed = false;
			up_ok_held = 0;
		}
		//Down / Back
		if(down_back.read()) {
			down_back_pressed = true;
			++down_back_held;
			//Button held more than 1s, send rapid commands "down" if control is released.
			if(!control_pressed && down_back_held >= 1000) {
				events.publish(Event(Event::eKey, MenuItem::down)); //Down
				down_back_held -= 15; //Avoid overflow.
			}
		}
		else if(down_back_pressed) { 
			if(control_pressed) events.publish(Event(Event::eKey, MenuItem::back)); //Back
			else events.publish(Event(Event::eKey, MenuItem::down)); //Down
			down_back_pressed = false;
			down_back_held = 0;
		}
		//Auto / Fast (No rapid commands for you!)
		if(auto_fast.read()) {
			auto_fast_pressed = true;
		}
		else if(auto_fast_pressed) {
			if(control_pressed) events.publish(Event(Event::eKey, StateMachine::eFastToggle)); //Fast
			else events.publish(Event(Event::eKey, StateMachine::eAutoToggle)); //Auto
			auto_fast_pressed = false;
		}

		//Tick is given every 1 ms.
		events.publish(Event(Event::eTick, 0));

		while (events.pending()) {
			base.HandleState(events.consume());
		}
	}
#else	
	SimpleMenu menu;
	DecimalEdit *pressure = new DecimalEdit(lcd, std::string("Pressure"),125,0,0.5,std::string("Pa"),true);
	IntegerEdit *fan = new IntegerEdit(lcd, std::string("Fan Speed"),100,0,10,std::string("%"),true);
	DecimalShow *test = new DecimalShow(lcd, std::string("Light Intensity"),std::string("%"));
	DecimalShow *test2 = new DecimalShow(lcd, std::string("Light Intensity"),std::string("%"));

	menu.addItem(new MenuItem(pressure));
	menu.addItem(new MenuItem(fan));
	menu.addItem(new MenuItem(fan));
	menu.addItem(new MenuItem(test));
	menu.addItem(new MenuItem(test2));
	pressure->setValue(0);
	fan->setValue(5);
	test->setValue(95.5);
	int timer = 0;
	int delay = 0;
	bool sw1_pressed = false; //"ok" button flag.
	bool sw2_pressed = false; //"down" button flag.
	bool sw3_pressed = false; //"up" button flag.
	bool deleted = false;

	menu.event(MenuItem::show); // display first menu item
	while(1){
		timer = millis();

		if(timer == 10000 || timer == delay){
			if(timer != 0 ){
				menu.event(MenuItem::back);
				delay = timer + 10000;
			}
		}

		if(sw1.read()){
			sw1_pressed = true;
		}
		else if(sw1_pressed){
			sw1_pressed = false;
		}

		if(sw2.read()){
			sw2_pressed = true;
		}
		else if(sw1_pressed && sw2_pressed) {
			delay = timer + 10000;
			sw1_pressed = false;
			sw2_pressed = false;
			menu.event(MenuItem::ok);
		}
		else if(sw2_pressed){

			delay = timer + 10000;
			sw2_pressed = false;
			menu.event(MenuItem::up);
		}

		if(sw3.read()){
			sw3_pressed = true;
		}
		else if (sw1_pressed && sw3_pressed) {
			delay = timer + 10000;
			sw3_pressed = false;
			sw1_pressed = false;
			pressure->setStatus(!(pressure->getStatus()));

		}
		else if(sw3_pressed){
			delay = timer + 10000;
			sw3_pressed = false;
			menu.event(MenuItem::down);
		}

		if(deleted == false){
			menu.deleteItem(new MenuItem(test));
			deleted = true;
		}
//		const char* testing = pressure->getTitle();
//		lcd->print(testing);
	}
#endif
	return 0;
}




// Produal MIO 12-V (Fan)
#if FAN_TEST
void produalModbusTest()
{
	ModbusMaster node(1); // Create modbus object that connects to slave id 1
	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	ModbusRegister AO1(&node, 0);
	ModbusRegister DI1(&node, 4, false);

	const uint16_t fa[3] = { 0, 0, 0 };

	while (1) {

		for(int i = 0; i < 3; ++i) {
			printf("DI1=%4d\n", DI1.read());
			AO1.write(fa[i]*100);
			// just print the value without checking if we got a -1
			printf("AO1=%4d\n", (int) fa[i]*100);

			Sleep(5000);
			printf("DI1=%4d\n", DI1.read());
			Sleep(5000);
		}
	}
}
#endif

// Vaisala HMP60 relative humidity and temperature sensor
#if HUM_TEMP_TEST
void humidity_test() {
	srhtHMP60 sensor;
	float t = 0;
	float rhum = 0;
	printf("Temperature\n");
	while(1) {
		Sleep(5000);
		//Temperature
		sensor.read_temp(t);
		printf("Decoded temp: %f C\n", t);
		//Humidity
		sensor.read_rhum(rhum);
		printf("Decoded hum: %f %%\n", rhum);
	}
}
#endif

// Vaisala GMP252 CO2 probe
#if CO2_TEST
void co2_test() {
	sco2GMP252 co2;
	float data = 0;
	printf("CO2\n");
	while(1) {
		Sleep(5000);
		printf("-------------------\n");
		co2.read(data);
		printf("Decoded co2: %f ppm\n", data);
		printf("-------------------\n");
	}
}
#endif

// Sensirion SDP610 – 125Pa pressure sensor
#if PRES_TEST
void pressure_test() {
	sPressureSDP610 spres;
	float pres = 0;
	//Attempt with I2CDevice.
	printf("Pressure\n");
	while(1) {
		Sleep(5000);
		printf("-------------------\n");
		if (spres.read(pres)) {
			printf("Pres: %f Pa\n", pres);
			printf("Time: %d\n", spres.get_elapsed_time());
		}
		else {
			printf("Invalid pressure read.\n");
			printf("Time: %d\n", spres.get_elapsed_time());
		}
		printf("-------------------\n");
	}
}
#endif

#if FAN_PRES_TEST
void fan_pressure_test() {
	sPressureSDP610 spres;
	aFanMIO12V fan;
	float pres = 0;

	DigitalIoPin sw1(1, 8, true, true, true); //speed up fan
	DigitalIoPin sw2(0, 5, true, true, true); //slow down fan
	DigitalIoPin sw3(0, 6, true, true, true); //read sensor

	uint16_t fa = 0;
	uint16_t prev_fa = 0;
	bool sw1_pressed = false;
    bool sw2_pressed = false;
    bool sw3_pressed = false;
	while(1) {
		Sleep(1);
        if(sw1.read()) {
            sw1_pressed = true;
        }
        else if(sw1_pressed){
            sw1_pressed = false;
			if(fa < 200) fa++;
        }
        if(sw2.read()) {
            sw2_pressed = true;
        }
        else if(sw2_pressed){
            sw2_pressed = false;
			if(fa > 0) fa--;
        }
		if(sw3.read()) {
            sw3_pressed = true;
        }
        else if(sw3_pressed){
            sw3_pressed = false;
			printf("-------------------\n");
			if (spres.read(pres)) {
				printf("Pres: %f Pa\n", pres);
				printf("Time: %d\n", spres.get_elapsed_time());
			}
			else {
				printf("Invalid pressure read.\n");
				printf("Time: %d\n", spres.get_elapsed_time());
			}
			printf("*******\n");
			printf("DI1=%4d\n", fan.get_aspeed());
			printf("AO1=%4d\n", fan.get_speed());
			printf("-------------------\n");
        }

		if(prev_fa != fa) {
			prev_fa = fa;
			fan.set_speed(fa * 10);
		}
	}
}
#endif

#if MODBUS_TEST   // example that uses modbus library directly
void printRegister(ModbusMaster& node, uint16_t reg)
{
	uint8_t result;
	// slave: read 16-bit registers starting at reg to RX buffer
	result = node.readHoldingRegisters(reg, 1);

	// do something with data if read is successful
	if (result == node.ku8MBSuccess)
	{
		printf("R%d=%04X\n", reg, node.getResponseBuffer(0));
	}
	else {
		printf("R%d=???\n", reg);
	}
}

bool setFrequency(ModbusMaster& node, uint16_t freq)
{
	uint8_t result;
	int ctr;
	bool atSetpoint;
	const int delay = 500;

	node.writeSingleRegister(1, freq); // set motor frequency

	printf("Set freq = %d\n", freq/40); // for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;
	do {
		Sleep(delay);
		// read status word
		result = node.readHoldingRegisters(3, 1);
		// check if we are at setpoint
		if (result == node.ku8MBSuccess) {
			if(node.getResponseBuffer(0) & 0x0100) atSetpoint = true;
		}
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	printf("Elapsed: %d\n", ctr * delay); // for debugging

	return atSetpoint;
}


void abbModbusTest()
{
	ModbusMaster node(2); // Create modbus object that connects to slave id 2
	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	printRegister(node, 3); // for debugging

	node.writeSingleRegister(0, 0x0406); // prepare for starting

	printRegister(node, 3); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printRegister(node, 3); // for debugging

	node.writeSingleRegister(0, 0x047F); // set drive to start mode

	printRegister(node, 3); // for debugging

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	printRegister(node, 3); // for debugging

	int i = 0;
	int j = 0;
	const uint16_t fa[20] = { 1000, 2000, 3000, 3500, 4000, 5000, 7000, 8000, 10000, 15000, 20000, 9000, 8000, 7000, 6000, 5000, 4000, 3000, 2000, 1000 };

	while (1) {
		uint8_t result;

		// slave: read (2) 16-bit registers starting at register 102 to RX buffer
		j = 0;
		do {
			result = node.readHoldingRegisters(102, 2);
			j++;
		} while(j < 3 && result != node.ku8MBSuccess);
		// note: sometimes we don't succeed on first read so we try up to threee times
		// if read is successful print frequency and current (scaled values)
		if (result == node.ku8MBSuccess) {
			printf("F=%4d, I=%4d  (ctr=%d)\n", node.getResponseBuffer(0), node.getResponseBuffer(1),j);
		}
		else {
			printf("ctr=%d\n",j);
		}

		Sleep(3000);
		i++;
		if(i >= 20) {
			i=0;
		}
		// frequency is scaled:
		// 20000 = 50 Hz, 0 = 0 Hz, linear scale 400 units/Hz
		setFrequency(node, fa[i]);
	}
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// WEB STUFF ///////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if MQTT_TEST  // example of opening a plain socket
void socketTest()
{

	esp_socket(SSID, PASSWORD);

	const char *http_request = "GET / HTTP/1.0\r\n\r\n"; // HTTP requires cr-lf to end a line

	for(int i = 0; i < 2; ++i) {
		printf("\nopen socket\n");
		esp_connect(1,  "www.metropolia.fi", 80);
		printf("\nsend request\n");
		esp_write(1, http_request, strlen(http_request));

		uint32_t now = get_ticks();
		printf("\nreply:\n");

		while(get_ticks() - now < 3000) {
			char buffer[64];
			memset(buffer, 0, 64);
			if(esp_read(1, buffer, 63) > 0) {
				fputs(buffer,stdout);
			}
		}
		esp_close(1);

		printf("\nsocket closed\n");
	}

}
#endif

#if MQTT_TEST

void messageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
			data->message->payloadlen, (char *)data->message->payload);
}

void mqttTest()
{
	/* connect to mqtt broker, subscribe to a topic, send and receive messages regularly every 1 sec */
	MQTTClient client;
	Network network;
	unsigned char sendbuf[256], readbuf[2556];
	int rc = 0, count = 0;
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

	NetworkInit(&network,SSID,PASSWORD);
	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char* address = (char *)BROKER_IP;
	if ((rc = NetworkConnect(&network, address, BROKER_PORT)) != 0)
		printf("Return code from network connect is %d\n", rc);


	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = (char *)"esp_test";

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");

	if ((rc = MQTTSubscribe(&client, "test/sample/#", QOS2, messageArrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);

	uint32_t sec = 0;
	while (true)
	{
		// send one message per second
		if(get_ticks() / 1000 != sec) {
			MQTTMessage message;
			char payload[30];

			sec = get_ticks() / 1000;
			++count;

			message.qos = QOS1;
			message.retained = 0;
			message.payload = payload;
			sprintf(payload, "message number %d", count);
			message.payloadlen = strlen(payload);

			if ((rc = MQTTPublish(&client, "test/sample/a", &message)) != 0)
				printf("Return code from MQTT publish is %d\n", rc);
		}

		if(rc != 0) {
			NetworkDisconnect(&network);
			// we should re-establish connection!!
			break;
		}

		// run MQTT for 100 ms
		if ((rc = MQTTYield(&client, 100)) != 0)
			printf("Return code from yield is %d\n", rc);
	}

	printf("MQTT connection closed!\n");

}
#endif
