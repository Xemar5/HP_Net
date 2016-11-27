#pragma once
#include <iostream>
#include <boost\asio.hpp>


class NetworkUDP
{
public:
private:
	virtual bool _update() = 0;
	virtual bool _disconnect() = 0;
	std::shared_ptr<boost::asio::ip::udp::endpoint> _myEndpoint;
	bool _isReading;
	bool _send();
	bool _recv();
};


