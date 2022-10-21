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
#include "DecimalEdit.h"
#include "IntegerUnitEdit.h"
#include "mqtt.h"


#define SSID	    "DBIN" //SmartIotMQTT  /* Use home localhost for test*/
#define PASSWORD    "WAASAdb81" //SmartIot /* Use home wifi password */
#define BROKER_IP   "10.0.1.3"  //192.168.1.254 /* Broker_IP is the home IP address */
#define BROKER_PORT  1883

//DEBUG DEFINES //Leave only one ON, or none.
#define LPC_PROJ 0 //Use this to switch from home-lpc to proj-lpc
#define MODBUS_TEST 0
#define FAN_TEST 0
#define HUM_TEMP_TEST 0
#define CO2_TEST 0
#define PRES_TEST 0
#define FAN_PRES_TEST 0
#define MQTT_TEST 1
#define SOCKET_TEST 0
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
	//Menu test. Works only in case if other tests are off.//
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
	DigitalIoPin sw1(1, 8, true, true, true);
	DigitalIoPin sw2(0, 5, true, true, true);
	DigitalIoPin sw3(0, 6, true, true, true);
#else
	DigitalIoPin sw1(0, 17 ,true ,true, true);
	DigitalIoPin sw2(1, 11 ,true ,true, true);
	DigitalIoPin sw3(1, 9 ,true ,true, true);
#endif
	//LCD
	LiquidCrystal *lcd = new LiquidCrystal(&rs, &en, &d4, &d5, &d6, &d7);
	// configure display geometry
	lcd->begin(16, 2);
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd->setCursor(0, 0);
	lcd->clear();
	SimpleMenu menu;
	IntegerUnitEdit *pressure= new IntegerUnitEdit(lcd, std::string("Pressure"),120,0,1,std::string("Pa"));
	IntegerUnitEdit *fan= new IntegerUnitEdit(lcd,std::string("Speed"),100,0,5,std::string("%"));

	menu.addItem(new MenuItem(pressure));
	menu.addItem(new MenuItem(fan));
	pressure->setValue(0);
	fan->setValue(5);
	int timer = 0;
	int delay = 0;


	//client and network

//	mqtt mqtt(SSID, PASSWORD, BROKER_IP, BROKER_PORT);
//	mqtt.mqtt_init();
	mqttTest();

	menu.event(MenuItem::show); // display first menu item
	while(1){
		//mqtt.mqtt_subscribe("test/sample");
		timer = millis();

		if(timer == 10000 || timer == delay){
			if(timer != 0 ){
				menu.event(MenuItem::back);
				delay = timer + 10000;
			}
		}
		if(sw1.read()){
			delay = timer + 10000;
			while(sw1.read());
			menu.event(MenuItem::up);
		}

		if(sw2.read()){
			delay = timer + 10000;
			while(sw2.read());
			menu.event(MenuItem::down);
		}

		if(sw3.read()){
			delay = timer + 10000;
			while(sw3.read());
			menu.event(MenuItem::ok);
		}
	}
	return 0 ;
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
	//I2C device. (Sensirion SDP610_125Pa pressure sensor)
	I2C i2c;
	const uint8_t addr = 0x40;
	I2CDevice i2c_sSDP610_pressure(&i2c, addr);
	uint8_t com = 0xF1;
	uint8_t pres_raw[3] = {0};
	uint16_t pres_value = 0;

	DigitalIoPin sw1(1, 8, true, true, true); //speed up fan
	DigitalIoPin sw2(0, 5, true, true, true); //slow down fan
	DigitalIoPin sw3(0, 6, true, true, true); //read sensor

	ModbusMaster node(1); // Create modbus object that connects to slave id 1
	node.begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here

	ModbusRegister AO1(&node, 0);
	ModbusRegister DI1(&node, 4, false);

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
			if(fa < 10) fa++;
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
			printf("Pressure\n-------------------\n");
			if (i2c_sSDP610_pressure.read(com, pres_raw, 3)) {
				printf("Pres: %02x%02x\n", pres_raw[0], pres_raw[1]);
				printf("CRC: %s\n", (pres_raw[2] == crc8(pres_raw, 2)) ? "OK" : "ERROR");
				pres_value = 0;
				pres_value = pres_raw[0];
				pres_value <<= 8;
				pres_value |= pres_raw[1];
				int16_t diff_pres = *((int16_t *)&pres_value);
				float diff = (float)diff_pres / 240;
				printf("Pres: %f Pa\n", diff);
			}
			else {
				printf("Invalid pressure read.\n");
			}
			printf("*******\n");
			printf("DI1=%4d\n", DI1.read());
			printf("-------------------\n");
        }

		if(prev_fa != fa) {
			prev_fa = fa;
			AO1.write(fa * 100);
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

#if SOCKET_TEST  // example of opening a plain socket
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
	unsigned char sendbuf[256], readbuf[256];
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
			char payload[256];

			sec = get_ticks() / 1000;
			++count;

//			snprintf(msg, 128, "{\r\n\t\"samplenr\": %d, \r\n\t\"timestamp\": %llu, \r\n\t\"temperature\" :%d\r\n}\r\n", sampleId, timeStamp, temp);



			message.qos = QOS1;
			message.retained = 0;
			message.payload = payload;
			sprintf(payload, "{\"nr\": %d, \"Speed\": %d, \"Setpoint\": %d, \"Pressure\": %d, \"auto\": %s, \"error\": %s, \"co2\": %d, \"rh\": %d, \"temp\": %d}", count, 23, 32, 10, "false", "false", 200, 37, 25);
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
