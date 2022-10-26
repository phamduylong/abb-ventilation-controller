#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <cstdio>
#include <cmath>
#include "DigitalIoPin.h"
#include "LpcUart.h"
#include "systick.h"
#include "mqtt/esp8266_socket.h"
#include "retarget_uart.h"
#include "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h"
#include "LiquidCrystal.h"
#include "Event.h"
#include "EventQueue.h"
#include "StateMachine.h"

#define SSID	    "SmartIotMQTT"
#define PASSWORD    "SmartIot"
#define BROKER_IP   "192.168.1.254"
#define BROKER_PORT  1883

//DEBUG DEFINES //Leave only one ON, or none.
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

#if MQTT_TEST
void socketTest();
void mqttTest();
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

	//Buttons init
	DigitalIoPin control(1, 8, true, true, true);
	DigitalIoPin up_ok(0, 5, true, true, true);
	DigitalIoPin down_back(0, 6, true, true, true);
	DigitalIoPin auto_fast(0, 7, true, true, true);

	//LCD
	LiquidCrystal *lcd = new LiquidCrystal(&rs, &en, &d4, &d5, &d6, &d7);
	lcd->begin(16, 2);
	lcd->setCursor(0, 0);
	lcd->clear();

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
			if(!control_pressed && up_ok_held >= 250) {
				events.publish(Event(Event::eKey, MenuItem::up)); //Up
				up_ok_held -= 11; //Avoid overflow.
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
			if(!control_pressed && down_back_held >= 250) {
				events.publish(Event(Event::eKey, MenuItem::down)); //Down
				down_back_held -= 11; //Avoid overflow.
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
	return 0;
}

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
