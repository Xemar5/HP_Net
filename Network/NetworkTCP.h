#pragma once
#include <iostream>
#include <map>
#include <boost\asio.hpp>

class NetworkTCP
{
public:
protected:
	virtual bool _update();

	bool _isReading = false;

	void _asyncReadTimeoutHandler(const boost::system::error_code & error, std::shared_ptr<boost::asio::ip::tcp::socket> senderSocket);
	virtual void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<boost::asio::ip::tcp::socket> sendingSocket,
		std::shared_ptr<std::array<char, 128>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout) = 0;
	virtual void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<boost::asio::ip::tcp::socket> sendingSocket) = 0;

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

	void _asyncConnectHandler(const boost::system::error_code& error, std::shared_ptr<boost::asio::ip::tcp::socket> peedingClient);
	void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<boost::asio::ip::tcp::socket> sendingSocket) override;
	void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<boost::asio::ip::tcp::socket> sendingSocket,
		std::shared_ptr<std::array<char, 128>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout) override;

	std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor = nullptr;
	std::map<std::shared_ptr<boost::asio::ip::tcp::socket>, unsigned> _clientSockets;

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
	void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<boost::asio::ip::tcp::socket> sendingSocket) override;
	void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<boost::asio::ip::tcp::socket> sendingSocket,
		std::shared_ptr<std::array<char, 128>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout) override;

	std::shared_ptr<boost::asio::ip::tcp::socket> _serverSocket = nullptr;
	int _currentResponseTimeout = _responseTimeout;

	bool _disconnect() override;
	
	friend class Network;
};