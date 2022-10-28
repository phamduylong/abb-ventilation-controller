#include "Networking.h"

void MessageArrived(MessageData* data)
{
	printf("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data, 
	                                                data->message->payloadlen, (char *)data->message->payload);
}

Networking::Networking(char* ssid, char* password, char* broker_ip, int broker_port): SSID(ssid), PASSWORD(password), BROKER_IP(broker_ip), BROKER_PORT (broker_port){

	NetworkInit(&network, SSID, PASSWORD);

	NetworkConnect(&network, (char*)BROKER_IP, BROKER_PORT);

	connectData = MQTTPacket_connectData_initializer;

	MQTTClientInit(&client, &network, 30000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

	connectData.MQTTVersion = 3;

	connectData.clientID.cstring = (char *)"Ventilation_Project";

	rc = MQTTConnect(&client, &connectData);

	if(rc != 0) printf("Return code from MQTT connect is %d\n", rc);
	else printf("MQTT Connected\n");
}

Networking::~Networking() {}

bool Networking::MQTT_subscribe(const char* topic){

	rc = MQTTSubscribe(&client, topic, QOS2, MessageArrived);

	if(rc != 0) printf("Return code from MQTT subscribe is %d\n", rc);
	else printf("Subscribe to %s\n", topic);
	return rc;
}

bool Networking::MQTT_publish(const char* pub_topic, const std::string& data){

	MQTTMessage message;

	char payload[256];

	message.qos = QOS1;
	message.retained = 0;
	message.dup = false;
	sprintf(payload, "%s", data.c_str());
	message.payload = payload;

	message.payloadlen = strlen(payload);

	rc = MQTTPublish(&client, pub_topic, &message);

	if(rc != 0) printf("Return code from MQTT publish is %d\n", rc);
	return rc;
}

bool Networking::MQTT_publish(const char* pub_topic, const char* msg){

	MQTTMessage message;
	char payload[256];

	message.qos = QOS1;
	message.retained = 0;
	message.dup = false;
	sprintf(payload, "%s", msg);
	message.payload = payload;
	message.payloadlen = strlen(payload);

	rc = MQTTPublish(&client, pub_topic, &message);
	if(rc != 0) printf("Return code from MQTT publish is %d\n", rc);
	return rc;
}

bool Networking::MQTT_yield(int duration){
	rc = MQTTYield(&client, duration);
	if(rc != 0) printf("Return code from yield is %d\n", rc);
	return rc;
}

bool Networking::Network_reconnect(){
	if ((rc = NetworkConnect(&network, BROKER_IP, BROKER_PORT)) != 0){

		NetworkInit(&network, SSID, PASSWORD);

		rc = NetworkConnect(&network, (char*)BROKER_IP, BROKER_PORT);
	}
	return rc;
}

bool Networking::MQTT_reconnect(){
	if ((rc = MQTTConnect(&client, &connectData)) != 0){

		connectData = MQTTPacket_connectData_initializer;

		MQTTClientInit(&client, &network, 6000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));

		connectData.MQTTVersion = 3;

		connectData.clientID.cstring = (char *)"Ventilation_Project";

		rc = MQTTConnect(&client, &connectData);
		if(rc != 0) printf("MQTT reconnected.\n");
	}
	return rc;
}

bool Networking::check_rc() {
	return this->rc;
}
