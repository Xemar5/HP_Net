#pragma once
#include <iostream>
#include <functional>
#include <map>
#include <boost\asio.hpp>


#include "NetworkSocket.h"

enum RPC_Receiver;

class NetworkTCP
{
public:
protected:
	virtual bool _update();
	virtual bool _disconnect() = 0;

	bool _send(std::function<void(const boost::system::error_code&, std::size_t, std::shared_ptr<NetworkSocket>)> handler);
	bool _recv(std::shared_ptr<NetworkSocket> reciveFrom, std::function<void(const boost::system::error_code&)> errorHandler);

	const int _responseTimeout = 10;
	static constexpr unsigned _packetLength = 128;
	std::vector<std::pair<std::vector<std::shared_ptr<NetworkSocket>>, std::string>> _peedingMessages;
	virtual bool _storeMessage(std::shared_ptr<NetworkSocket>sender, std::string str, RPC_Receiver reciever) = 0;
	//std::vector<std::string> _receivedMessages;

	virtual void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket) = 0;
	void _asyncReadTimeoutHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> senderSocket);
	virtual void _asyncReadErrorHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket) = 0;
	void _asyncReadHandler(
		const boost::system::error_code& error,
		std::shared_ptr<NetworkSocket> sendingSocket,
		std::shared_ptr<std::array<char, NetworkTCP::_packetLength>> str,
		std::size_t len,
		std::shared_ptr<boost::asio::deadline_timer> timeout,
		std::function<void(const boost::system::error_code&)> errorHandler);

	friend class Network;
	friend class ClientTCP;
	friend class ServerTCP;
};
