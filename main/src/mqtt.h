/*
 * mqtt.h
 *
 *  Created on: 19 Oct 2022
 *      Author: DBY & longph
 */

#include "paho.mqtt.embedded-c/MQTTClient-C/src/MQTTClient.h"
#include "paho.mqtt.embedded-c/MQTTPacket/src/MQTTPacket.h"
#include "mqtt/MQTT_lpc1549.h"
#include "paho.mqtt.embedded-c/MQTTPacket/src/MQTTConnect.h"

#include "systick.h"
#include <cstring>
#include <string>



#ifndef MQTT_H_
#define MQTT_H_

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
	/** @brief Constructor to initialize esp web socket and mqtt
	    @param ssid - SSID
	    @param pass - The exponent, number of times to multiply base by itself
	    @param broker_ip - Broker's IP address
	    @param broker_port - Broker's port
	*/
	mqtt(char* ssid, char* pass, char* broker_ip, int broker_port);


	/** @brief virtual destructor with an empty body
		*/
	virtual ~mqtt();


	/** @brief Subscribe to a topic on the MQTT server
		@param sub_topic - Topic in which the client will subscribe to
		@returns Nothing
	 */
	void mqtt_subscribe(const char* sub_topic = STATUS_TOPIC);


	/** @brief Publish a message to a particular topic
		@param pub_topic - Topic in which the client will publish to
		@param data - C++ String const reference, will be converted to C-style for transactions
	    @returns Nothing
			*/
	void mqtt_publish(const char* pub_topic = SETTINGS_TOPIC, const std::string &data);


	/** @brief Publish a message to a particular topic
		@param pub_topic - Topic in which the client will publish to
		@param data - C string containing message to be published
		@returns Nothing
	*/
	void mqtt_publish(const char* pub_topic = SETTINGS_TOPIC, const char* msg);


	/** @brief Disconnecting from current network connection
		@returns Nothing
				*/
	void mqtt_disconnect();


	/** @brief Unsubscribe from a topic
	 * @param ubsub_topic - Topic from which client unsubs from
	   @returns Nothing
	*/
	void mqtt_unsubscribe(const char* unsub_topic);


	/** @brief Letting MQTT run for an amount of time
		 * @param c - MQTTClient Object that is being allowed to run
		 * @param timeout_ms - The amount of time the client is allowed to run
		   @returns Nothing
		*/
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
	static constexpr const char* SETTINGS_TOPIC = "controller/settings";
	static constexpr const char* STATUS_TOPIC = "controller/status";
};

#endif /* MQTT_H_ */
