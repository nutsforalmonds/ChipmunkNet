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

		// Possibly send some kind of init message to server on this initial send
		start_send();
	}

	~udp_client()
	{
		socket_.close();
	}

	void send_keyState(int keyState)
	{
		send_buf_[0] = keyState;
		start_send();
	}
	int get_keyState()
	{
		return recv_buf_[0];
	}


private:
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];

	boost::array<int, 1> send_buf_ = { { 0 } };
	boost::array<int, 1> recv_buf_;
	boost::array<char, 128> recv_hb_;

	udp::endpoint remote_endpoint_;

	void start_send()
	{
		//std::cout << "Sending: " << send_buf_.data() << std::endl;

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
		//std::cout << "Receiving: " << recv_buf_[0] << std::endl;
	}

	void handle_send(boost::shared_ptr<std::string> /*message*/,
		const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/)
	{
		if (!error || error == boost::asio::error::message_size)
		{
			socket_.async_receive_from(boost::asio::buffer(recv_buf_), remote_endpoint_,
				boost::bind(&udp_client::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}
};

int main(int argc, char* argv[])
{
	try
	{
		boost::asio::io_service io_service;
		udp_client cli(io_service, "127.0.0.10", "13");

		int keyState = 0;
		int recvState = 0;

		while (1)
		{
			//encoding
			if (GetAsyncKeyState(VK_UP))
			{
				keyState = keyState | 1;
			}
			if (GetAsyncKeyState(VK_DOWN))
			{
				keyState = keyState | 1 << 1;
			}
			if (GetAsyncKeyState(VK_LEFT))
			{
				keyState = keyState | 1 << 2;
			}
			if (GetAsyncKeyState(VK_RIGHT))
			{
				keyState = keyState | 1 << 3;
			}

			cli.send_keyState(keyState);

			// Honestly not sure what difference the position of this poll() makes
			// adding it after the receive seems to work the same, but not having it
			// results in nothing ever being received.
			io_service.poll();
			
			recvState = cli.get_keyState();

			//decoding
			if (recvState & 1)
			{
				std::cout << "Up" << std::endl;
			}
			if (recvState & 1 << 1)
			{
				std::cout << "Down" << std::endl;
			}
			if (recvState & 1 << 2)
			{
				std::cout << "Left" << std::endl;
			}
			if (recvState & 1 << 3)
			{
				std::cout << "Right" << std::endl;
			}

			keyState = 0;
			recvState = 0;
			Sleep(100);
		}

	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}