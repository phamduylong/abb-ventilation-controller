/*
 * mqtt.cpp
 *
 *  Created on: 19 Oct 2022
 *      Author: DBY & longph
 */

#include "mqtt.h"


void message_arrived(MessageData *data){

	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
			data->message->payloadlen, (char *)data->message->payload);
	//return message->payload;
}


mqtt::mqtt(char* ssid, char* pass, char* broker_ip, int broker_port ): SSID(ssid), PASSWORD(pass), BROKER_IP(broker_ip), BROKER_PORT(broker_port) {

	connectData = MQTTPacket_connectData_initializer;

	NetworkInit(&network, SSID, PASSWORD);

	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	rc = NetworkConnect(&network, (char*)BROKER_IP, BROKER_PORT);
	if(rc != 0){
		printf("Return code from network connect is %d\n", rc);
	}

	connectData.MQTTVersion = 3;

	connectData.clientID.cstring = (char *)"Ventilation_Project";

	if ((rc = MQTTConnect(&client, &connectData)) != 0)
		printf("Return code from MQTT connect is %d\n", rc);
	else
		printf("MQTT Connected\n");
}

mqtt::~mqtt() {
	// TODO Auto-generated destructor stub
}



void mqtt::mqtt_subscribe(const char* sub_topic){

	rc = MQTTSubscribe(&client, sub_topic, QOS2, message_arrived);

	if (rc != 0){
		printf("Return code from MQTT subscribe is %d\n", rc);
	} else {
	 	printf("Subscribed to %s\n", sub_topic);
	}
}


void mqtt::mqtt_unsubscribe(const char* unsub_topic){
	if((rc = MQTTUnsubscribe(&client, unsub_topic)) !=0){
		printf("Topic is Unsubscribed\n");
	}
}



void mqtt::mqtt_publish(const char* pub_topic, const std::string& data){


	MQTTMessage message;

	char payload[256];

	message.qos = QOS1;
	message.retained = 0;
	message.dup = false;
	sprintf(payload, "%s", data.c_str());
	message.payload = payload;

	message.payloadlen = strlen(payload);

	rc = MQTTPublish(&client, pub_topic, &message);

	if(rc != 0){
		printf("Return code from MQTT publish is %d\n", rc);
	}

}



void mqtt::mqtt_publish(const char* pub_topic, const char* msg) {


	MQTTMessage message;

	char payload[256];

	message.qos = QOS1;
	message.retained = 0;
	message.dup = false;
	sprintf(payload, "%s", msg);
	message.payload = payload;

	message.payloadlen = strlen(payload);

	rc = MQTTPublish(&client, pub_topic, &message);

	if(rc != 0){
		printf("Return code from MQTT publish is %d\n", rc);
	}

}




void mqtt::mqtt_disconnect(){

	NetworkDisconnect(&network);
}



int mqtt::mqtt_yield(MQTTClient* c, int timeout_ms){
	if ((rc = MQTTYield(&client, 100)) != 0){
		printf("Return code from yield is %d\n", rc);
	}
	return rc;
}

