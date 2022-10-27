#include "StatusSettingsJsonParser.h"

void tokenize(std::string const &str, const char* delim, std::vector<std::string> &out) {
	size_t start;
	size_t end = 0;
	while ((start = str.find_first_not_of(delim, end)) != std::string::npos) {
		end = str.find(delim, start);
		out.push_back(str.substr(start, end - start));
	}
}

StatusSettingsJsonParser::StatusSettingsJsonParser() {}

StatusSettingsJsonParser::~StatusSettingsJsonParser() {}

status_data StatusSettingsJsonParser::getStatusObj() {
	return this->status_json_obj;
}

settings_data StatusSettingsJsonParser::getSettingsObj() {
	return this->settings_json_obj;
}

void StatusSettingsJsonParser::parse_status(const std::string& json_str) {
	size_t start_pos = json_str.find('{');
	size_t end_pos = json_str.find('}');
	if (start_pos != std::string::npos && end_pos != std::string::npos) {
		std::string props = json_str.substr(start_pos + 1, end_pos - start_pos - 1);
		std::vector<std::string> kp_vals;
		std::vector<std::string> tmp;

		tokenize(props, ", ", kp_vals);
		for (auto i : kp_vals) {
			tokenize(i, ": ", tmp);
		}

		for(size_t i = 0; i < tmp[9].length(); i++) {
			tmp[9][i] = tolower(tmp[9][i]);
		}
		for(size_t i = 0; i < tmp[11].length(); i++) {
			tmp[11][i] = tolower(tmp[11][i]);
		}

		this->status_json_obj.nr = std::stoul(tmp[1]);
		this->status_json_obj.speed = std::stoul(tmp[3]);
		this->status_json_obj.setpoint = std::stoi(tmp[5]);
		this->status_json_obj.pressure = std::stof(tmp[7]);
		this->status_json_obj.auto_mode = (tmp[9] == "true") ? "true" : "false";
		this->status_json_obj.error = (tmp[11] == "true") ? "true" : "false";
		this->status_json_obj.co2 = std::stof(tmp[13]);
		this->status_json_obj.rh = std::stof(tmp[15]);
		this->status_json_obj.temp = std::stof(tmp[17]);
	}
}


void StatusSettingsJsonParser::parse_settings(const std::string& json_str) {
	size_t start_pos = json_str.find('{');
	size_t end_pos = json_str.find('}');
	if (start_pos != std::string::npos && end_pos != std::string::npos) {
		std::string props = json_str.substr(start_pos + 1, end_pos - start_pos - 1);
		std::vector<std::string> kp_vals;
		std::vector<std::string> tmp;
		tokenize(props, ", ", kp_vals);
		for (auto i : kp_vals) {
			tokenize(i, ": ", tmp);
		}

		for(size_t i = 0; i < tmp[1].length(); i++) {
			tmp[1][i] = tolower(tmp[1][i]);
		}

		if(tmp[1] == "true") {
			bool auto_mode = true;
			float pressure = std::stof(tmp[3]);
			this->settings_json_obj.auto_mode = auto_mode;
			this->settings_json_obj.pressure = pressure;
		}
		else if(tmp[1] == "false") {
			bool auto_mode = false;
			unsigned long speed = std::stoul(tmp[3]);
			this->settings_json_obj.auto_mode = auto_mode;
			this->settings_json_obj.speed = (unsigned int) speed;
		}
	}
}

std::string StatusSettingsJsonParser::stringify(const status_data& json) {
	char json_str[256];
	sprintf(json_str, STATUS_FORMAT, json.nr, json.speed, json.setpoint, json.pressure,
	json.auto_mode ? "true" : "false", json.error ? "true" : "false", json.co2, json.rh, json.temp);
	return json_str;
}

std::string StatusSettingsJsonParser::stringify(const settings_data& json) {
	char json_str[256];
	if (json.auto_mode) sprintf(json_str, SETTINGS_FORMAT_AUTO, json.pressure); 
	else sprintf(json_str, SETTINGS_FORMAT_MAN, json.speed);
	return json_str;
}
