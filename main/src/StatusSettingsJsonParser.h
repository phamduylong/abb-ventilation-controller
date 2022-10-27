#ifndef STATUSSETTINGSJSONPARSER_H_
#define STATUSSETTINGSJSONPARSER_H_

#include <stdio.h>
#include <string>
#include <cstdio>
#include <vector>
#include <typeinfo>

void tokenize(std::string const &str, const char* delim, std::vector<std::string> &out);

typedef struct settings_data {
	bool auto_mode;
	int16_t speed;
	float pressure;
} settings_data;

typedef struct status_data {
	unsigned int nr;
	int16_t speed;
	unsigned int setpoint;
	float pressure;
	bool auto_mode;
	bool error;
	float co2;
	float rh;
	float temp;
} status_data;

class StatusSettingsJsonParser {
public:
	StatusSettingsJsonParser();
	virtual ~StatusSettingsJsonParser();

	status_data getStatusObj();
	settings_data getSettingsObj();

	void setStatusObj();
	void setSettingsObj(bool auto_mode, float param);

	void parse_settings(const std::string &json_str);
	void parse_status(const std::string &json_str);
	std::string stringify(const status_data& json);
	std::string stringify(const settings_data& json);
private:
	static constexpr const char* STATUS_FORMAT = "{nr: %hu, speed: %hu, setpoint: %hu, pressure: %.2f, auto: %s, error: %s, co2: %.2f, rh: %.2f, temp: %.2f}";
	static constexpr const char* SETTINGS_FORMAT_AUTO = "{auto: True, pressure: %.2f}";
	static constexpr const char* SETTINGS_FORMAT_MAN = "{auto: False, speed: %hu}";
	std::string json_str;
	settings_data settings_json_obj;
	status_data status_json_obj;
};

#endif /* STATUSSETTINGSJSONPARSER_H_ */
