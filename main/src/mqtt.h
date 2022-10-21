/*
 * mqtt.h
 *
 *  Created on: 19 Oct 2022
 *      Author: DBY
 */

#include "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h"
#include "paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.h"
#include "mqtt/MQTT_lpc1549.h"
#include "paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnect.h"
#include "systick.h"
#include "string.h"

//#define SSID	    "DBIN" //SmartIotMQTT
//#define PASSWORD    "WAASAdb81" //SmartIot
//#define BROKER_IP   "10.0.1.3"  //192.168.1.254
//#define BROKER_PORT  1883

#ifndef MQTT_MQTT_H_
#define MQTT_MQTT_H_

class mqtt {
public:

	mqtt(char* ssid, char* pass, char* broker_ip, int broker_port);
	virtual ~mqtt();
	void mqtt_init();
	void mqtt_subscribe(const char* sub_topic);
	void mqtt_publish(const char* pub_topic);
	void mqtt_reconnect();


private:
	MQTTClient client;
	Network network;
	MQTTPacket_connectData connectData;
	int rc;
	int count;
	char* SSID;
	char* PASSWORD;
	char* BROKER_IP;
	int BROKER_PORT;


};

#endif /* MQTT_MQTT_H_ */
