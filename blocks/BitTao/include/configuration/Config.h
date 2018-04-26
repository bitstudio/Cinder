#pragma once

#include <set>
#include <string>
#include "utilities/json.hpp"
#include "cinder/Cinder.h"
#include "cinder/Vector.h"
#include <thread>
#include <functional>
#include "communication/server_http.hpp"

using json = nlohmann::json;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

namespace Bit {

	class ConfigurableVariable;
	class Config;

	class Configurable
	{
	public:
		virtual void readConfig(json& tree, Config* config) = 0;
	};

	struct script_compare {
		bool operator() (const std::string& lhs, const std::string& rhs) const {
			if (lhs.size() == rhs.size()) {
				return lhs.compare(rhs) < 0;
			}
			return lhs.size() < rhs.size();
		}
	};

	class Config {
	public:
		typedef struct DisplayConfig {
			ci::ivec2 windowSize;
			ci::ivec2 windowPos;
			bool alwaysOnTop;
			bool hideCursor;
			bool borderless;
		} DisplayConfig;

		Config();
		~Config();
		void readConfig();		// called in prepareSettings() from mainApp
		void setup();			// called in setup() to init Bit::Param (before this, we don't have gl context)
		void readConfigurableConfig(Configurable& configurable, const std::string& configNodeName);		// call configurable.readConfig() giving the specified child node as argument

		void bind_interface(std::string alias, ConfigurableVariable* var);
		void bind_event(std::string alias, std::function<void(std::string, void*)> callback, void* data);

		static std::string getAssetPath();	// return cached assetPath_
		DisplayConfig getDisplayConfig(std::string displayKey = "display");

		void write_back();

		json& getTreePtr();

		void populate_sender_script(std::stringstream& ss);

	private:
		json configTree_;
		std::string config_file_path_;
		static std::string assetPath_;

		HttpServer server_;
		std::thread server_thread_;

		std::map<std::string, std::shared_ptr<ConfigurableVariable> > variables_;
		std::map<std::string, std::pair<std::function<void(std::string, void*)>, void*> > callbacks_;
		std::set<std::string, script_compare> unique_sender_scripts_;

		void assign_variables(std::string path, json node);
		void trigger_callbacks(std::string path, json node);
	};

	class ConfigEndScopeSetup {
	public:
		ConfigEndScopeSetup(Config* config) : config_(config) {}
		~ConfigEndScopeSetup()
		{
			config_->setup();
		}
	private:
		Config * config_;
	};

	class ConfigurableVariable {
	public:
		ConfigurableVariable() : storage(nullptr), ptr(nullptr) {}
		ConfigurableVariable(json* storage, void* ptr) : storage(storage), ptr(ptr) {}
		virtual json to_json() = 0;
		virtual void assign(json::value_type value) = 0;

		virtual std::string get_tag(std::string id) = 0;
		virtual std::string get_event_handler(std::string id)
		{
			return "document.getElementById(\"" + id + "\").addEventListener('change', sendData);";
		}
		virtual std::string get_sender_script()
		{
			std::stringstream stream;
			stream << "window.sendData = function(event) {console.log(event.target.id, event.target.type);console.log(event.target.value);console.log(event.target.checked);";
			stream << "var update = {};switch(event.target.type){case \"text\": update[event.target.id] = event.target.value;break;case \"checkbox\": update[event.target.id] = event.target.checked;break;";
			stream << "case \"number\":update[event.target.id] = event.target.valueAsNumber;break;case \"range\":update[event.target.id] = event.target.valueAsNumber;break;case \"radio\":update[event.target.name] = parseInt(event.target.value);break;}";
			stream << "XHR.open('POST', '/update');XHR.setRequestHeader('Content-Type', 'application/json');var msg = JSON.stringify(update);XHR.send(msg);};";
			return stream.str();
		}

	protected:
		json * storage;
		void* ptr;
	};

	class StringVariable : public ConfigurableVariable {
	public:
		StringVariable(json* storage, std::string* ptr): ConfigurableVariable(storage, ptr)
		{
		}

		json to_json()
		{
			json obj;
			obj["value"] = *((std::string*)ptr);
			return obj;
		}

		void assign(json::value_type value)
		{
			std::string v = value.get<std::string>();
			if (storage) *(storage) = v;
			if (ptr) *((std::string*)ptr) = v;
		}

		std::string get_value()
		{
			return *((std::string*)ptr);
		}

		std::string get_tag(std::string id)
		{
			std::stringstream stream;
			stream << "<input type=" << "\"text\" id=\"" << id << "\" value=\"" << get_value() << "\"><br>";
			return stream.str();
		}

	};

	class IntVariable : public ConfigurableVariable {
	public:
		IntVariable(json* storage, int* ptr, int min, int max) :max(max), min(min), ConfigurableVariable(storage, ptr)
		{
		}

		json to_json()
		{
			json obj;
			obj["value"] = *((int*)ptr);
			obj["min"] = min;
			obj["max"] = max;
			return obj;
		}

		void assign(json::value_type value)
		{
			int v = value.get<int>();
			if (storage) *(storage) = v;
			if (ptr) *((int*)ptr) = v;
		}

		int get_value()
		{
			return *((int*)ptr);
		}

		std::string get_tag(std::string id)
		{
			std::stringstream stream;
			stream << "<input type=" << "\"number\" id=\"" << id << "\" value=\"" << get_value() << "\"><br>";
			return stream.str();
		}

	private:
		int max;
		int min;
	};

	template<class T>
	class NumericVariable : public ConfigurableVariable {
	public:
		NumericVariable(json* storage, T* ptr, T min, T max, T step) :max(max), min(min), step(step), ConfigurableVariable(storage, ptr)
		{
		}

		json to_json()
		{
			json obj;
			obj["value"] = *((T*)ptr);
			obj["min"] = min;
			obj["max"] = max;
			obj["step"] = step;
			return obj;
		}

		void assign(json::value_type value)
		{
			T v = value.get<T>();
			if (storage) *(storage) = v;
			if (ptr) *((T*)ptr) = v;
		}

		T get_value()
		{
			return *((T*)ptr);
		}

		std::string get_tag(std::string id)
		{
			std::stringstream stream;
			stream << "<input type=" << "\"range\" id=\"" << id << "\" min=\"" << min << "\" max=\"" << max << "\" step=\"" << step << "\" value=\"" << get_value() << "\"><br>";
			return stream.str();
		}

	private:
		T max;
		T min;
		T step;
	};

	class BoolVariable : public ConfigurableVariable {
	public:
		BoolVariable(json* storage, bool* ptr): ConfigurableVariable(storage, ptr)
		{
		}

		json to_json()
		{
			json obj;
			obj["value"] = *((bool*)ptr);
			return obj;
		}

		void assign(json::value_type value)
		{
			bool v = value.get<bool>();
			if (storage) *(storage) = v;
			if (ptr) *((bool*)ptr) = v;
		}

		bool get_value()
		{
			return *((bool*)ptr);
		}

		std::string get_tag(std::string id)
		{
			std::stringstream stream;
			if(get_value()) stream << "<input type=" << "\"checkbox\" id=\"" << id << "\" checked><br>";
			else stream << "<input type=" << "\"checkbox\" id=\"" << id << "\"><br>";
			return stream.str();
		}

	};

	class Vec2dVariable : public ConfigurableVariable {
	public:
		Vec2dVariable(json* storage, ci::vec2* ptr) : ConfigurableVariable(storage, ptr)
		{
		}

		json to_json()
		{
			json obj;
			obj["x"] = ((ci::vec2*)ptr)->x;
			obj["y"] = ((ci::vec2*)ptr)->y;
			return obj;
		}

		void assign(json::value_type value)
		{
			if (storage)
			{
				*(storage) = value;
			}
			if (ptr)
			{
				ci::vec2 v = ci::vec2(value["x"].get<float>(), value["y"].get<float>());
				*((ci::vec2*)ptr) = v;
			}
		}

		ci::vec2 get_value()
		{
			return *((ci::vec2*)ptr);
		}

		std::string get_tag(std::string id)
		{
			std::stringstream stream;
			ci::vec2 v = get_value();
			stream << "<input type=" << "\"number\" id=\"" << id << ".x\" value=\"" << v.x << "\">";
			stream << "<input type=" << "\"number\" id=\"" << id << ".y\" value=\"" << v.y << "\"><br>";
			return stream.str();
		}

		std::string get_event_handler(std::string id)
		{
			std::stringstream stream;
			stream << "document.getElementById(\"" + id + ".x\").addEventListener('change', sendVectorData);";
			stream << "document.getElementById(\"" + id + ".y\").addEventListener('change', sendVectorData);";
			return stream.str();
		}

		std::string get_sender_script()
		{
			std::stringstream stream;
			stream << "window.sendVectorData = function(event) {";
			stream << "var update = {};var id = event.target.id; id = id.substring(0, id.length - 2);";
			stream << "update[id] = {'x':document.getElementById(id + '.x').valueAsNumber, 'y':document.getElementById(id + '.y').valueAsNumber};";
			stream << "XHR.open('POST', '/update');XHR.setRequestHeader('Content-Type', 'application/json');var msg = JSON.stringify(update);XHR.send(msg);};";
			return stream.str();
		}

	};

	class ChoiceVariable : public ConfigurableVariable {
	private:
		std::vector<int> choices_;
	public:
		ChoiceVariable(json* storage, int* ptr, std::vector<int> &choices) : ConfigurableVariable(storage, ptr)
		{
			choices_ = choices;
		}

		json to_json()
		{
			json obj;
			obj["value"] = *((int*)ptr);
			return obj;
		}

		void assign(json::value_type value)
		{
			int v = value.get<int>();
			if (storage) *(storage) = v;
			if (ptr) *((int*)ptr) = v;
		}

		int get_value()
		{
			return *((int*)ptr);
		}

		std::string get_tag(std::string id)
		{
			std::stringstream stream;

			int i = 0;
			for (auto iter = choices_.begin(); iter != choices_.end(); ++iter, ++i)
			{
				if (get_value() == *iter)
				{
					stream << "<input name=\"" + id + "\" " << "type=" << "\"radio\" id=\"" << id << i << "\" value=\"" << *iter << "\" checked>";
				}
				else
				{
					stream << "<input name=\"" + id + "\" " << "type=" << "\"radio\" id=\"" << id << i << "\" value=\"" << *iter << "\">";
				}
				stream << "<label for=\"" << id << i << "\">" << *iter << "</label>";
			}
			stream << "<br>";
			return stream.str();
		}

		std::string get_event_handler(std::string id)
		{
			std::stringstream stream;

			int i = 0;
			for (auto iter = choices_.begin(); iter != choices_.end(); ++iter, ++i)
			{
				stream << "document.getElementById(\"" << id << i << "\").addEventListener('change', sendData);";
			}
			return stream.str();
		}

	};

}
