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
#include <cstring>
#include <string>



#ifndef MQTT_MQTT_H_
#define MQTT_MQTT_H_

typedef struct {
	uint8_t speed;
	uint8_t setpoint;
	uint8_t pressure;
	unsigned char* mode;
	unsigned char* error;
	uint8_t co2;
	uint8_t rh;
	uint8_t temp;
}SENSOR_DATA;


class mqtt {
public:

	mqtt(char* ssid, char* pass, char* broker_ip, int broker_port);
	virtual ~mqtt();
	void mqtt_subscribe(const char* sub_topic);
	void mqtt_publish(const char* pub_topic, const std::string &data);
	void mqtt_disconnect();
	void mqtt_unsubscribe(const char* sub_topic);
	int mqtt_yield(MQTTClient* c, int timeout_ms);

private:
	MQTTClient client;
	Network network;
	MQTTPacket_connectData connectData;
	unsigned char sendbuf[256];
	unsigned char readbuf[256];
	int rc;
	int count;
	char* SSID;
	char* PASSWORD;
	char* BROKER_IP;
	int BROKER_PORT;


};

#endif /* MQTT_MQTT_H_ */
