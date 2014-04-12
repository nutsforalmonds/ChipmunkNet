// boost_client1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>
#include <time.h>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

using boost::asio::ip::udp;


class udp_client
{

public:
	udp_client(boost::asio::io_service& io_service, const std::string& host, const std::string& port)
		: io_service_(io_service), socket_(io_service, udp::endpoint(udp::v4(), 0)) {
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), host, port);
		udp::resolver::iterator itr = resolver.resolve(query);

		remote_endpoint_ = *itr;
		start_send();
	}

private:
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];

	boost::array<char, 1> send_buf_ = { { 0 } };
	boost::array<char, 128> recv_buf_;

	udp::endpoint remote_endpoint_;

	void start_send()
	{
		std::cout << "start_send" << std::endl;

		boost::shared_ptr<std::string> message(
			new std::string("this is a string"));

		socket_.async_send_to(
			boost::asio::buffer(send_buf_), remote_endpoint_,
			boost::bind(&udp_client::handle_send, this, message,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(const boost::system::error_code& error,
		std::size_t len)
	{
		std::cout << "handle_receive" << std::endl;
		std::cout.write(recv_buf_.data(), len);
	}

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/)
	{
		std::cout << "handle_send" << std::endl;
		std::cout << error << std::endl;
		if (!error || error == boost::asio::error::message_size)
		{
			std::cout << "recv_from" << std::endl;
			socket_.async_receive_from(boost::asio::buffer(recv_buf_), remote_endpoint_,
				boost::bind(&udp_client::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));

			Sleep(100);
			start_send();
		}
	}
};

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service;
		udp_client server(io_service, "127.0.0.10", "13");
		io_service.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

