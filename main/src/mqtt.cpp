/*
 * mqtt.cpp
 *
 *  Created on: 19 Oct 2022
 *      Author: DBY
 */

#include "mqtt.h"


void message_arrived(MessageData *data){ //here should activate actuator to react

	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
			data->message->payloadlen, (char *)data->message->payload);
	//return message->payload;
}

mqtt::mqtt(char* ssid, char* pass, char* broker_ip, int broker_port ): SSID(ssid), PASSWORD(pass), BROKER_IP(broker_ip), BROKER_PORT(broker_port) {
	// TODO Auto-generated constructor stub
	connectData = MQTTPacket_connectData_initializer;
}


mqtt::~mqtt() {
	// TODO Auto-generated destructor stub
}

void mqtt::mqtt_init(){
	unsigned char sendbuf[256], readbuf[256];

	NetworkInit(&network,SSID,PASSWORD);

	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	char* address = (char*) BROKER_IP;
	if ((rc = NetworkConnect(&network, address, BROKER_PORT)) != 0)
		printf("Return code from network connect is %d\n", rc);

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = (char *)"Ventilation_Web";

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");
}


void mqtt::mqtt_subscribe(const char* topic){

	if ((rc = MQTTSubscribe(&client, topic, QOS2, message_arrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);
}


void mqtt::mqtt_unsubscribe(const char* topic){
	if((rc = MQTTUnsubscribe(&client, topic)) != 0){
		printf("Unsubscribed from topic %s", sub_topic);
	}
}



void mqtt::mqtt_publish(const char* pub_topic, const char *data){

	uint32_t sec = 0;

	if(get_ticks() / 1000 != sec) {
		MQTTMessage message;
		char payload[256];

		sec = get_ticks() / 1000;
		++count;

		message.qos = QOS1;
		message.retained = 0;
		message.payload = payload;
		sprintf(payload, "{\"nr\": %d, \"Speed\": %d, \"Setpoint\": %d, \"Pressure\": %d, \"auto\": %s, \"error\": %s, \"co2\": %d, \"rh\": %d, \"temp\": %d}",
				count, data->speed, data->setpoint, data->pressure, data->mode, data->error, data->co2, data->rh, data->temp);
		message.payloadlen = strlen(payload);

		if ((rc = MQTTPublish(&client, pub_topic, &message)) != 0)
			printf("Return code from MQTT publish is %d\n", rc);
	}

	// run MQTT for 100 ms
	if ((rc = MQTTYield(&client, 100)) != 0){
		printf("Return code from yield is %d\n", rc);
	}

printf("MQTT connection closed!\n");

}

void mqtt::mqtt_reconnect(){
	if(rc != 0) {
		NetworkDisconnect(&network);
		// we should re-establish connection!!
		mqtt_init();
	}
}



