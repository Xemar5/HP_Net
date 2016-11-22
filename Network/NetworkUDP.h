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

class ServerUDP : public NetworkUDP
{
public:
private:
	bool _update() override;
	bool _host();

	std::vector<std::shared_ptr<boost::asio::ip::udp::endpoint>> _clientEndpoints;

	bool _disconnect() override;
};


class ClientUDP : public NetworkUDP
{
public:
private:
	bool _update() override;
	bool _join();

	std::shared_ptr<boost::asio::ip::udp::endpoint> _serverEndpoint;

	bool _disconnect() override;
};