#include "configuration/Config.h"
#include "cinder/app/App.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace Bit{

	// static member
	std::string Config::assetPath_ = "";

	Config::Config()
	{
		config_file_path_ = "config.json";
	}

	Config::~Config()
	{
		server_.stop();
		if(this->server_thread_.joinable())
			this->server_thread_.join();
	}

	void Config::readConfig()
	{
  		std::ifstream configFile;
		configFile.open(config_file_path_);
		if (configFile.is_open()) {
			configTree_ = json::parse(configFile);
			configFile.close();
		}
		else 
		{
			config_file_path_ = "../config/" + config_file_path_;
			configFile.open(config_file_path_);
			if (configFile.is_open()) {
				configTree_ = json::parse(configFile);
				configFile.close();
			}
		}
		assetPath_ = configTree_["assetPath"].get<std::string>();		// cache asset path
		if (assetPath_.compare(".") == 0){
			assetPath_ = ci::app::getAppPath().string();	// if asset path is ".", we use exe path instead
		}
	}

	void Config::write_back()
	{
		std::ofstream configFile;
		configFile.open(config_file_path_);
		if (configFile.is_open()) {
			configFile << this->getTreePtr().dump(4);
			configFile.close();
		}
	}


	void Config::populate_sender_script(std::stringstream& ss)
	{
		for (auto iter = unique_sender_scripts_.begin(); iter != unique_sender_scripts_.end(); ++iter)
		{
			ss << *iter;
		}
	}

	void Config::setup()
	{
		server_.config.port = configTree_["config_port"].get<int>();
		server_.resource["^/update$"]["POST"] = [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			try {
				//ci::app::console() << request->content.string() << std::endl;
				json content = json::parse(request->content);

				assign_variables("", content);

				std::string msg = "";
				*response << "HTTP/1.1 200 OK\r\n"
					<< "Content-Length: " << msg.length() << "\r\n\r\n"
					<< msg;
			}
			catch (const std::exception &e) {
				*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
			}
		};

		server_.resource["^/trigger$"]["POST"] = [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			try {
				//ci::app::console() << request->content.string() << std::endl;
				json content = json::parse(request->content);

				trigger_callbacks("", content);

				std::string msg = "";
				*response << "HTTP/1.1 200 OK\r\n"
					<< "Content-Length: " << msg.length() << "\r\n\r\n"
					<< msg;
			}
			catch (const std::exception &e) {
				*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
			}
		};

		server_.resource["^/save"]["POST"] = [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {
			try {
				json content = json::parse(request->content);

				this->write_back();

				std::string msg = "";
				*response << "HTTP/1.1 200 OK\r\n"
					<< "Content-Length: " << msg.length() << "\r\n\r\n"
					<< msg;
			}
			catch (const std::exception &e) {
				*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
			}
		};

		server_.resource["^/config$"]["GET"] = [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

			std::stringstream stream;

			std::list<std::string> list1;
			std::list<std::string> list2;

			for (auto iter = variables_.begin(); iter != variables_.end(); ++iter)
			{
				stream.str("");
				stream << iter->first << ":";
				stream << iter->second->get_tag(iter->first);
				list1.push_back(stream.str());
				list2.push_back(iter->second->get_event_handler(iter->first));
			}

			stream.str("");
			stream << "<html><head><title>Configuration</title></head><body><div>";
			stream << "<input type=\"button\" onclick=\"saveData()\" value=\"save param\"><br>";
			for (auto iter = callbacks_.begin(); iter != callbacks_.end(); ++iter)
				stream << "<input type=\"button\" id=\"" + iter->first + "\" value=\"" + iter->first + "\"><br>";
			for (auto iter = list1.begin(); iter != list1.end(); ++iter)
				stream << *iter;
			stream << "</div>" << std::endl;

			stream << "<script>window.onload=function(){";
			for (auto iter = callbacks_.begin(); iter != callbacks_.end(); ++iter)
				stream << "document.getElementById(\"" + iter->first + "\").addEventListener('click', trigger);";
			for (auto iter = list2.begin(); iter != list2.end(); ++iter)
				stream << *iter;
			stream << "};var XHR = new XMLHttpRequest();window.saveData = function(event) {XHR.open('POST', '/save');XHR.setRequestHeader('Content-Type', 'application/json');XHR.send('{}');};";
			this->populate_sender_script(stream);

			stream << "window.trigger = function(event) {";
			stream << "var update = {};update[event.target.id] = true;";
			stream << "XHR.open('POST', '/trigger');XHR.setRequestHeader('Content-Type', 'application/json');var msg = JSON.stringify(update);XHR.send(msg);};";

			stream << "</script></body></html>";

			response->write(stream);
		};

		server_.resource["^/info"]["GET"] = [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

			json msg;
			for (auto iter = variables_.begin(); iter != variables_.end(); ++iter)
			{
				json param;
				param["alias"] = iter->first;
				param["description"] = iter->second->to_json();
				msg["param_list"].push_back(param);
			}
			
			std::stringstream stream;
			stream << msg.dump();
			response->write(stream);
		};

		server_.default_resource["GET"] = [this](std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) {

			std::string path = this->getAssetPath() + request->path;
			std::ifstream file(path);
			if (file.is_open())
			{
				response->write(file);
			}
			else 
			{
				response->write(SimpleWeb::StatusCode::client_error_not_found);
			}
		};


		server_thread_ = std::thread([&]() {
			// Start server
			server_.start();
		});

		//server_thread_.detach();
	}

	void Config::readConfigurableConfig(Configurable& configurable, const std::string& configNodeName)
	{
		configurable.readConfig(configTree_[configNodeName], this);
	}

	std::string Config::getAssetPath()
	{
		return assetPath_;
	}

	Config::DisplayConfig Config::getDisplayConfig(std::string displayKey)
	{
		DisplayConfig displayConfig;
		//// create displayConfig
		// load display jsontree
		auto displayTree = configTree_[displayKey];
		// read application size
		displayConfig.windowSize.x = displayTree["width"].get<int>();
		displayConfig.windowSize.y = displayTree["height"].get<int>();
		// read application top left position
		displayConfig.windowPos.x = displayTree["x"].get<int>();
		displayConfig.windowPos.y = displayTree["y"].get<int>();
		// read window setup
		displayConfig.alwaysOnTop = displayTree["alwaysOnTop"].get<bool>();
		displayConfig.borderless = displayTree["borderLess"].get<bool>();
		displayConfig.hideCursor = displayTree["hideCursor"].get<bool>();
		return displayConfig;
	}

	json& Config::getTreePtr()
	{
		return configTree_;
	}

	void Config::assign_variables(std::string path, json node)
	{
		for (json::iterator it = node.begin(); it != node.end(); ++it) 
		{
			std::shared_ptr<ConfigurableVariable> tuple = variables_[path + it.key()];
			tuple->assign(it.value());
		}
	}

	void Config::trigger_callbacks(std::string path, json node)
	{
		for (json::iterator it = node.begin(); it != node.end(); ++it)
		{
			std::string key = path + it.key();
			auto tuple = callbacks_[key];
			std::get<0>(tuple)(key, std::get<1>(tuple));
		}
	}

	void Config::bind_interface(std::string alias, ConfigurableVariable* var)
	{
		auto p = std::shared_ptr<ConfigurableVariable>(var);
		variables_[alias] = p;
		unique_sender_scripts_.insert(p->get_sender_script());
	}

	void Config::bind_event(std::string alias, std::function<void(std::string, void*)> callback, void* data)
	{
		callbacks_[alias] = std::make_pair(callback, data);
	}

}
