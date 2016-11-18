#pragma once
#include <iostream>
#include <boost\asio.hpp>
#include <boost\array.hpp>


class Network
{
public:
	static bool Init();
	static bool Update();
	static int port;
private:
	static boost::asio::io_service service;
	//static boost::asio::ip::tcp::acceptor acceptor;
};