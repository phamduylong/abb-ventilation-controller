/*
 * mqtt.cpp
 *
 *  Created on: 19 Oct 2022
 *      Author: DBY
 */

#include "mqtt.h"

void message_arrived(MessageData *data){

	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
			data->message->payloadlen, (char *)data->message->payload);

}


mqtt::mqtt(MQTTClient &client_, Network &network_):client(&client_),network(&network_) {
	// TODO Auto-generated constructor stub
	connectData = MQTTPacket_connectData_initializer;
}

mqtt::~mqtt() {
	// TODO Auto-generated destructor stub
}

void mqtt::mqtt_init(){

	unsigned char sendbuf[256], readbuf[2556];
	NetworkInit(network,SSID,PASSWORD);
	MQTTClientInit(client, network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));
}

void mqtt::mqtt_subscribe(){

	char* address = (char *)BROKER_IP;
	if ((rc = NetworkConnect(network, address, BROKER_PORT)) != 0)
		printf("Return code from network connect is %d\n", rc);

	connectData.MQTTVersion = 3;
	connectData.clientID.cstring = (char *)"esp_test";

	if ((rc = MQTTConnect(client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");

	if ((rc = MQTTSubscribe(client, "controller/settings/#", QOS2, message_arrived)) != 0)
		printf("Return code from MQTT subscribe is %d\n", rc);
}



void mqtt::mqtt_publish(){
	uint32_t sec = 0;
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

		if ((rc = MQTTPublish(client, "controller/status", &message)) != 0)
			printf("Return code from MQTT publish is %d\n", rc);
	}

	if(rc != 0) {
		NetworkDisconnect(network);
		// we should re-establish connection!!
	}// run MQTT for 100 ms
	else if ((rc = MQTTYield(client, 100)) != 0){
		printf("Return code from yield is %d\n", rc);
	}

printf("MQTT connection closed!\n");

}





