#pragma once
#include <iostream>
#include <map>
#include <boost\asio.hpp>

class NetworkSocket
{
public:
	NetworkSocket(boost::asio::io_service& service) : socket(service) {}

	boost::asio::ip::tcp::socket socket;
	bool isReading = false;
	unsigned timeout = 10;
};

class NetworkTCP
{
public:
protected:
	virtual bool _update();


	void _asyncReadTimeoutHandler(const boost::system::error_code & error, std::shared_ptr<boost::asio::ip::tcp::socket> senderSocket);
	virtual void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket) = 0;
	virtual void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<NetworkSocket> sendingSocket,
		std::shared_ptr<std::array<char, 128>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout) = 0;

	const int _responseTimeout = 10;

	bool _send();
	bool _recv();


	virtual bool _disconnect() = 0;

	friend class Network;
};

class ServerTCP : public NetworkTCP
{
public:

protected:
	bool _update() override;
	bool _host();

	void _asyncConnectHandler(const boost::system::error_code& error, std::shared_ptr<NetworkSocket> peedingClient);
	void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket) override;
	void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<NetworkSocket> sendingSocket,
		std::shared_ptr<std::array<char, 128>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout) override;

	std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor = nullptr;
	std::vector<std::shared_ptr<NetworkSocket>> _clientSockets;

	unsigned _maxPlayers = -1;

	bool _connectable(bool value);
	bool _disconnect() override;

	friend class Network;
};

class ClientTCP : public NetworkTCP
{
public:
protected:
	bool _update() override;
	bool _join(std::string ip);

	void _asyncResolveHandler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator ep, std::shared_ptr<boost::asio::ip::tcp::resolver> resolver);
	void _asyncConnectHandler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator ep);
	void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket) override;
	void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<NetworkSocket> sendingSocket,
		std::shared_ptr<std::array<char, 128>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout) override;

	std::shared_ptr<NetworkSocket> _serverSocket = nullptr;

	bool _disconnect() override;
	
	friend class Network;
};