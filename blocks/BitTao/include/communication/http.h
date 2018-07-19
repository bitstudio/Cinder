#pragma once

#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <functional>
#include "utilities/json.hpp"
#include <chrono>
#include <boost/system/system_error.hpp>
#include <asio/deadline_timer.hpp>
#include <cstdlib>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <cinder/app/App.h>

using json = nlohmann::json;
using boost::asio::deadline_timer;
using boost::asio::ip::tcp;
using boost::lambda::bind;
using boost::lambda::var;

enum HTTP_Verb
{
	GET, POST
};

struct HTTP_Params
{
	HTTP_Verb verb;
	std::string url;
	std::string port;
	std::string path;
	json body;
	std::string header;
	HTTP_Params(std::string url, std::string port, std::string path, json& body, std::string header) :url(url), port(port), path(path), body(body), header(header)
	{
		verb = HTTP_Verb::POST;
	}
	HTTP_Params(std::string url, std::string port, std::string path, std::string header) :url(url), port(port), path(path), header(header)
	{
		verb = HTTP_Verb::GET;
	}
};

// this class is an example how to use Configurable
class Http
{
public:

	void get(std::string url, std::string port, std::string path, std::string header = "")
	{
		if (num_active_calls_ > 20) return;
		mutex_.lock();
		num_active_calls_++;
		mutex_.unlock();
		auto thread = boost::thread(&Http::get_internal, this, HTTP_Params(url, port, path, header));
		thread.detach();
	}

	void post(std::string url, std::string port, std::string path, json& body, std::string header = "")
	{
		if (num_active_calls_ > 20) return;
		mutex_.lock();
		num_active_calls_++;
		mutex_.unlock();
		auto thread = boost::thread(&Http::post_internal, this, HTTP_Params(url, port, path, body, header));
		thread.detach();
	}

	int getQueueSize()
	{
		return buffer_.size();
	}

	bool pop(json& tree)
	{
		if (buffer_.size() <= 0) return false;
		mutex_.lock();
		tree = buffer_.front();
		buffer_.pop_front();
		mutex_.unlock();
		return true;
	}

	bool popUntilLast(json& tree)
	{
		bool temp = pop(tree);
		mutex_.lock();
		buffer_.clear();
		mutex_.unlock();
		return temp;
	}

	void reset()
	{
		last_update_ = 0;
	}

	float getLastUpdateTime()
	{
		float temp;
		mutex_.lock();
		temp = last_update_;
		mutex_.unlock();
		return temp;
	}

	Http()
	{
		num_active_calls_ = 0;
	}

	~Http()
	{
		reset();
	}

private:

	std::list<json> buffer_;
	boost::mutex mutex_;

	float last_update_;

	int num_active_calls_;

	void push_error(std::string path, int code = -1)
	{
		json tree;
		tree["request_path"] = path;
		tree["status_code"] = code;
		mutex_.lock();
		buffer_.push_back(tree);
		last_update_ = ci::app::getElapsedSeconds();
		mutex_.unlock();
	}

	void get_internal(HTTP_Params param)
	{
		std::string url = param.url;
		std::string port = param.port;
		std::string path = param.path;
		std::string header = param.header;
		try
		{
			boost::asio::io_service io_service;

			// Try each endpoint until we successfully establish a connection.
			tcp::socket socket(io_service);
			connect(io_service, socket, url, port, boost::posix_time::seconds(2));

			// Form the request. We specify the "Connection: close" header so that the
			// server will close the socket after transmitting the response. This will
			// allow us to treat all data up until the EOF as the content.
			boost::asio::streambuf request;
			std::ostream request_stream(&request);
			request_stream << "GET " << path << " HTTP/1.1\r\n";
			request_stream << "Host: " << url << "\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Cache-Control: no-cache\r\n";
			request_stream << header;
			request_stream << "Connection: close\r\n\r\n";

			// Send the request.
			boost::asio::write(socket, request);

			// Read the response status line. The response streambuf will automatically
			// grow to accommodate the entire line. The growth may be limited by passing
			// a maximum size to the streambuf constructor.
			boost::asio::streambuf response;
			read_until(io_service, socket, response, "\r\n", boost::posix_time::seconds(2));

			// Check that response is OK.
			std::istream response_stream(&response);
			std::string http_version;
			response_stream >> http_version;
			unsigned int status_code;
			response_stream >> status_code;
			std::string status_message;
			std::getline(response_stream, status_message);
			if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			{
				ci::app::console() << "Invalid response\n";
				push_error(path);
				return;
			}
			if (status_code != 200)
			{
				ci::app::console() << "Response returned with status code " << status_code << "\n";
				push_error(path, status_code);
				return;
			}

			// Read the response headers, which are terminated by a blank line.
			read_until(io_service, socket, response, "\r\n\r\n", boost::posix_time::seconds(2));

			// Process the response headers.
			std::string header;
			while (std::getline(response_stream, header) && header != "\r");

			// Read until EOF, writing data to output as we go.
			std::stringstream ss;
			ss << &response;
			boost::system::error_code error;
			while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
				ss << &response;

			if (error != boost::asio::error::eof)
				throw boost::system::system_error(error);

			// Write whatever content we already have to output.
			std::string temp = ss.str();
			if (temp.size() > 0)
			{
				json tree = json::parse(ss);
				tree["request_path"] = path;
				ci::app::console() << tree.dump(4) << std::endl;
				mutex_.lock();
				buffer_.push_back(tree);
				mutex_.unlock();
				last_update_ = ci::app::getElapsedSeconds();
			}

		}
		catch (std::exception& e)
		{
			ci::app::console() << "Exception: " << e.what() << "\n";
			//push_error(path);
		}

		mutex_.lock();
		num_active_calls_--;
		mutex_.unlock();
	}

	void post_internal(HTTP_Params param)
	{
		std::string url = param.url;
		std::string port = param.port;
		std::string path = param.path;
		json body = param.body;
		std::string header = param.header;
		try
		{
			boost::asio::io_service io_service;

			// Try each endpoint until we successfully establish a connection.
			tcp::socket socket(io_service);
			connect(io_service, socket, url, port, boost::posix_time::seconds(2));

			std::string payload = body.dump();

			// Form the request. We specify the "Connection: close" header so that the
			// server will close the socket after transmitting the response. This will
			// allow us to treat all data up until the EOF as the content.
			boost::asio::streambuf request;
			std::ostream request_stream(&request);
			request_stream << "POST " << path << " HTTP/1.1\r\n";
			request_stream << "Host: " << url << "\r\n";
			request_stream << "Accept: */*\r\n";
			request_stream << "Content-Type: application/json; charset=UTF-8" << "\r\n";
			request_stream << "Content-Length: " << payload.size() << "\r\n";
			request_stream << "Accept: application/json, text/javascript, */*; q=0.01" << "\r\n";
			request_stream << "Cache-Control: no-cache" << "\r\n";
			request_stream << header;
			request_stream << "Connection: close" << "\r\n\r\n";
			request_stream << payload;

			// Send the request.
			boost::asio::write(socket, request);

			// Read the response status line. The response streambuf will automatically
			// grow to accommodate the entire line. The growth may be limited by passing
			// a maximum size to the streambuf constructor.
			boost::asio::streambuf response;
			read_until(io_service, socket, response, "\r\n", boost::posix_time::seconds(2));

			// Check that response is OK.
			std::istream response_stream(&response);
			std::string http_version;
			response_stream >> http_version;
			unsigned int status_code;
			response_stream >> status_code;
			std::string status_message;
			std::getline(response_stream, status_message);
			if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			{
				ci::app::console() << "Invalid response\n";
				push_error(path);
				return;
			}
			if (status_code != 200)
			{
				ci::app::console() << "Response returned with status code " << status_code << "\n";
				push_error(path, status_code);
				return;
			}

			// Read the response headers, which are terminated by a blank line.
			read_until(io_service, socket, response, "\r\n\r\n", boost::posix_time::seconds(2));

			// Process the response headers.
			std::string header;
			while (std::getline(response_stream, header) && header != "\r");

			// Read until EOF, writing data to output as we go.
			std::stringstream ss;
			ss << &response;
			boost::system::error_code error;
			while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
				ss << &response;

			// Write whatever content we already have to output.
			std::string temp = ss.str();
			if (temp.size() > 0)
			{
				//ci::app::console() << temp << std::endl;
				json tree = json::parse(ss);
				tree["request_path"] = path;
				//ci::app::console() << tree.dump(4) << std::endl;
				mutex_.lock();
				buffer_.push_back(tree);
				mutex_.unlock();
				last_update_ = ci::app::getElapsedSeconds();
			}

			if (error != boost::asio::error::eof)
				throw boost::system::system_error(error);
		}
		catch (std::exception& e)
		{
			ci::app::console() << "Exception: " << e.what() << "\n";
			//push_error(path);
		}

		mutex_.lock();
		num_active_calls_--;
		mutex_.unlock();
	}

	void removeCharsFromString(std::string &str, char* charsToRemove)
	{
		for (unsigned int i = 0; i < strlen(charsToRemove); ++i)
		{
			str.erase(remove(str.begin(), str.end(), charsToRemove[i]), str.end());
		}
	}

	void connect(boost::asio::io_service& io_service, tcp::socket& socket, const std::string& host, const std::string& service,
		boost::posix_time::time_duration timeout)
	{
		// Resolve the host name and service to a list of endpoints.
		tcp::resolver::query query(host, service);
		tcp::resolver::iterator iter = tcp::resolver(io_service).resolve(query);
		
		deadline_timer deadline(io_service);
		deadline.expires_from_now(timeout);

		boost::system::error_code ec = boost::asio::error::would_block;

		boost::asio::async_connect(socket, iter, var(ec) = boost::lambda::_1);

		// Block until the asynchronous operation has completed.
		do io_service.run_one(); while (ec == boost::asio::error::would_block);

		if (ec || !socket.is_open())
			throw boost::system::system_error(ec ? ec : boost::asio::error::operation_aborted);
	}

	void read_until(boost::asio::io_service& io_service, tcp::socket& socket, boost::asio::streambuf& response, std::string pattern,
		boost::posix_time::time_duration timeout)
	{
		deadline_timer deadline(io_service);
		deadline.expires_from_now(timeout);
		boost::system::error_code ec = boost::asio::error::would_block;
		boost::asio::async_read_until(socket, response, pattern, var(ec) = boost::lambda::_1);

		// Block until the asynchronous operation has completed.
		do io_service.run_one(); while (ec == boost::asio::error::would_block);

		if (ec)
			throw boost::system::system_error(ec ? ec : boost::asio::error::operation_aborted);
	}

};
