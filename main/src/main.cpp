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

#define SSID	    "SmartIotMQTT"
#define PASSWORD    "SmartIot"
#define BROKER_IP   "192.168.1.254"
#define BROKER_PORT  1883

//DEBUG DEFINES //Leave only one ON, or none.
#define MODBUS_TEST 0
#define FAN_TEST 0
#define HUM_TEMP_TEST 0
#define CO2_TEST 1
#define PRES_TEST 0
#define MQTT_TEST 0
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

float binary32_to_float(const unsigned int bin32);

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

	//LCD
	LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7, false);
	lcd.begin(16,2);
	lcd.setCursor(0,0);
	lcd.print("Septentrinoalis");

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

	while(1) {
		Sleep(100);
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
	ModbusMaster node(241);
	node.begin(9600);

	ModbusRegister rh0 (&node, 0x0000);
	ModbusRegister rh1 (&node, 0x0001);
	ModbusRegister t0 (&node, 0x0002);
	ModbusRegister t1 (&node, 0x0003);
	//TODO: add 0x0200 reg -> error status. 0 - yes, 1 - no error.
	// add error code regs and error code handling. (already in separate class)

	unsigned int temp = 0;
	unsigned int humidity = 0;
	printf("Temperature\n");
	while(1) {
		temp = 0;
		humidity = 0;
		Sleep(5000);
		//Temperature
		temp = temp | t1.read();
		temp = temp << 16;
		temp = temp | t0.read();
		printf("Temp = %08X\n", temp);
		float t = binary32_to_float(temp);
		printf("Decoded temp: %f C\n", t);
		//Humidity
		humidity |= rh1.read();
		humidity <<= 16;
		humidity |= rh0.read();
		printf("R Humidity = %08x\n", humidity);
		float hum = binary32_to_float(humidity);
		printf("Decoded hum: %f %%\n", hum);
	}
}
#endif
//Convertion of 32 binary representation of decimal to float via pointer.
float binary32_to_float(const unsigned int bin32) {
	return *((float *)&bin32);
}
//Redundant complex convertion of 32 binary representation of decimal to float.
/*
float binary32_to_float(const unsigned int bin32) {
	float res = 0;
	//Get and decode exponent part.
	int8_t exponent = ((bin32 & 0x7f800000) >> 23) - 127;
	//Get fraction part.
	unsigned int fraction = bin32 & 0x007fffff;
	//Add implicit 24 bit to fraction.
	fraction |= 0x00800000;
	//Sum fraction.
	float to_add = 1;
	unsigned int i = 0x00800000;
	do {
		if(fraction & i) res += to_add;
		to_add /= 2;
		i >>= 1;
	} while(i > 0x00000000);
	//Multiply the sum by the base 2 to the power of the exponent.
	res *= pow(2, exponent);
	//Get sign.
	if(bin32 >> 31) res *= -1;
	return res;
}
*/

// Vaisala GMP252 CO2 probe
#if CO2_TEST
void co2_test() {
	ModbusMaster node(240);
	node.begin(9600);
	//TODO: add 0x0100 and 0x0101 regs handling.
	//Add 0x0800 and 0x0801 status regs handling.

	ModbusRegister co2_reg(&node, 0x0000);
	ModbusRegister co2_reg2(&node, 0x0001);
	unsigned int co2_hex = 0;
	printf("CO2\n");
	while(1) {
		Sleep(5000);
		//CO2
		co2_hex = 0;
		co2_hex |= co2_reg2.read();
		co2_hex <<= 16;
		co2_hex |= co2_reg.read();
		printf("CO2 = %08X\n", co2_hex);
		float co2 = binary32_to_float(co2_hex);
		printf("Decoded co2: %f ppm\n", co2);
	}
}
#endif

// Sensirion SDP610 – 120Pa pressure sensor
#if PRES_TEST
void pressure_test() {
	//I2C device. (Sensirion SDP610_125kPa pressure sensor)
	I2C i2c;
	I2CDevice i2c_sSDP610_pressure(&i2c, (uint8_t)0x40);
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
