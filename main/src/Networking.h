#ifndef NETWORKING_H_
#define NETWORKING_H_

#include "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h"
#include "paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.h"
#include "mqtt/MQTT_lpc1549.h"
#include "paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnect.h"
#include "systick.h"
#include <cstring>
#include <string>
#include <array>


class Networking {
public:
	Networking(char* ssid, char* password, char* broker_ip, int broker_port);
	bool MQTT_subscribe(const char* topic);
	bool MQTT_publish(const char* pub_topic, const std::string &data);
	bool MQTT_publish(const char* pub_topic, const char* msg);
	bool MQTT_yield(int duration);
	bool MQTT_reconnect();
	bool Network_reconnect();
	virtual ~Networking();
	bool check_rc();


private:
	MQTTClient client;
	Network network;
	MQTTPacket_connectData connectData;
	unsigned char sendbuf[256];
	unsigned char readbuf[256];
	int rc;
	char* SSID;
	char* PASSWORD;
	char* BROKER_IP;
	int BROKER_PORT;
//	static constexpr const char* SETTING_TOPIC = "controller/setting";
//	static constexpr const char* STATUS_TOPIC = "controller/status";
};

#endif /* NETWORKING_H_ */
