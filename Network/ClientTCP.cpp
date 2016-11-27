#define BOOST_ASIO_ENABLE_CANCELIO

#include <boost\bind.hpp>
#include "ClientTCP.h"
#include "Network.h"


using namespace boost::asio::ip;

bool ClientTCP::_update()
{
	if (_serverSocket && _serverSocket->socket.is_open() && _serverSocket->connected())
		if (!_serverSocket->isReading()) ClientTCP::_recv();

	NetworkTCP::_update();

	if (_serverSocket && _serverSocket->socket.is_open() && _serverSocket->connected())
		if (_peedingMessages.size()) ClientTCP::_send();

	return false;
}


//void ClientTCP::_asyncReadHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket, std::shared_ptr<std::array<char, 128>> str, std::size_t len, std::shared_ptr<boost::asio::deadline_timer> timeout)
//{
//	if (error == boost::asio::error::eof)
//	{
//		std::cout << "Disconnected by the Server\n";
//		Network::Disconnect();
//	}
//	else if (error == boost::asio::error::timed_out || error == boost::asio::error::operation_aborted)
//	{
//		std::cout << "Connection timeout in: " << sendingSocket->timeout * 3 << "\n";
//		--sendingSocket->timeout;
//		if (sendingSocket->timeout < 0)
//		{
//			std::cout << "Connection timeout; Server stopped responding\n";
//			Network::Disconnect();
//		}
//	}
//	else if (error == boost::asio::error::connection_reset)
//	{
//		std::cout << "Disconnected by the server\n";
//		Network::Disconnect();
//	}
//	else if (error)
//	{
//		std::cout << "Other Read error: " << error << ": " << error.message() << std::endl;
//	}
//	else if (!len)
//	{
//		std::cout << "Recieved message has length of 0\n";
//	}
//	else
//	{
//		sendingSocket->timeout = _responseTimeout;
//		std::cout << "Message received: " << std::string(str->data(), len) << std::endl;
//		Network::_receivingStream << std::string(str->data(), len);
//	}
//	timeout->cancel();
//	sendingSocket->isReading = false;
//}

void ClientTCP::_asyncResolveHandler(const boost::system::error_code& error, tcp::resolver::iterator ep, std::shared_ptr<tcp::resolver> resolver)
{
	if (error)
	{
		std::cout << "An error has occured during client resolving: " << error << std::endl;
	}
	else
	{
		boost::asio::async_connect(_serverSocket->socket, ep, boost::bind(&ClientTCP::_asyncConnectHandler, this, boost::asio::placeholders::error, ep));
	}
}
void ClientTCP::_asyncConnectHandler(const boost::system::error_code& error, tcp::resolver::iterator ep)
{
	if (error)
	{
		std::cout << "Connection not allowed\n";
		//Network::Disconnect();
		//_serverSocket->socket.shutdown(tcp::socket::shutdown_both);
		//_serverSocket->socket.close();
		//_serverSocket = nullptr;
	}
	else
	{
		std::cout << "Connected!\n";
		_serverSocket->connected(true);
		//_connectionType = ConnectionType::CT_Client;
		//_currentResponseTimeout = _responseTimeout;
	}
}
bool ClientTCP::_join(std::string ip)
{
	std::shared_ptr<tcp::resolver> resolver = std::make_shared<tcp::resolver>(*Network::_service);
	_serverSocket = std::make_shared<NetworkSocket>(*Network::_service);
	std::cout << "Trying to connect to [" << ip << "]...\n";
	resolver->async_resolve(tcp::resolver::query(ip, std::to_string(Network::defaultPort)), boost::bind(&ClientTCP::_asyncResolveHandler, this, boost::asio::placeholders::error, boost::asio::placeholders::iterator, resolver));
	return true;
}

void ClientTCP::_asyncSendHandler(const boost::system::error_code & error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket)
{
	if (error)
	{
		if (error == boost::asio::error::connection_reset) std::cout << "Server has restarted connection\n";
		else std::cout << "An error has occured during packet sending (" << error << "): " << error.message() << std::endl;

		Network::Disconnect();
	}
	else
	{
		//std::cout << "Package sent!\n";
	}

}
void ClientTCP::_asyncReadErrorHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket)
{
	if (error == boost::asio::error::eof)
	{
		std::cout << "Disconnected by the Server\n";
		Network::Disconnect();
	}
	else if (error == boost::asio::error::timed_out || error == boost::asio::error::operation_aborted)
	{
		std::cout << "Connection timeout in: " << sendingSocket->timeout() * 3 << "\n";
		sendingSocket->timeout(sendingSocket->timeout() - 1);
		if (sendingSocket->timeout() < 0)
		{
			std::cout << "Connection timeout; Server stopped responding\n";
			Network::Disconnect();
		}
	}
	else if (error == boost::asio::error::connection_reset)
	{
		std::cout << "Disconnected by the server\n";
		Network::Disconnect();
	}
	else if (error)
	{
		std::cout << "Other Read error: " << error << ": " << error.message() << std::endl;
		sendingSocket->timeout(sendingSocket->timeout() - 1);
		if (sendingSocket->timeout() < 0)
		{
			std::cout << "Connection timeout; Server stopped responding\n";
			Network::Disconnect();
		}
	}
	else
	{
		throw "No error wtf u doin' here\n";
	}
}
bool ClientTCP::_send()
{
	return NetworkTCP::_send(std::bind(&ClientTCP::_asyncSendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//for (size_t i = 0; i < _peedingMessages.size(); ++i)
	//{
	//	std::string& s = _peedingMessages[i].second;
	//	while (s.size())
	//	{
	//		std::string buf(NetworkTCP::_packetLength - 1, '\0');

	//		if (s.size() > buf.size() - 1)
	//			buf[0] = 1;
	//		
	//		buf.replace
	//		(
	//			buf.begin() + 1,
	//			s.size() > buf.size() - 1 ? buf.end() : buf.begin() + s.size(),
	//			s.begin(),
	//			s.end()
	//		);

	//		for (size_t j = 0; j < _peedingMessages[i].first.size(); ++j)
	//			_peedingMessages[i].first[j]->socket.async_send
	//			(
	//				boost::asio::buffer(buf.data(), buf.size()),
	//				boost::bind
	//				(
	//					&ClientTCP::_asyncSendHandler,
	//					this,
	//					boost::asio::placeholders::error,
	//					boost::asio::placeholders::bytes_transferred,
	//					_peedingMessages[i].first[j]
	//				)
	//			);

	//		if (s.size() > buf.size() - 1) s = s.substr(buf.size() - 1, s.size() - buf.size() + 1);
	//		else s.clear();
	//	}
	//}
	//_peedingMessages.clear();
	//return false;
}

bool ClientTCP::_recv()
{
	//if (_serverSocket->isReading()) return false;

	//_serverSocket->isReading(true);
	//std::shared_ptr<std::array<char, NetworkTCP::_packetLength>> buf = std::make_shared<std::array<char, NetworkTCP::_packetLength>>();
	//boost::system::error_code e;
	//auto timeout = std::make_shared<boost::asio::deadline_timer>(*Network::_service);
	//timeout->expires_from_now(boost::posix_time::seconds(3));
	//timeout->async_wait(boost::bind(&NetworkTCP::_asyncReadTimeoutHandler, this, boost::asio::placeholders::error, _serverSocket));

	//_serverSocket->socket.async_read_some(boost::asio::buffer(*buf, buf->size()),
	//	boost::bind(&NetworkTCP::_asyncReadHandler, this, boost::asio::placeholders::error, _serverSocket, buf, boost::asio::placeholders::bytes_transferred, timeout, std::bind(&ClientTCP::_asyncReadErrorHandler, Network::_clientTCP, std::placeholders::_1, _serverSocket))
	//);
	//return true;

	return NetworkTCP::_recv(_serverSocket, std::bind(&ClientTCP::_asyncReadErrorHandler, Network::_clientTCP, std::placeholders::_1, _serverSocket));
}

bool ClientTCP::_storeMessage(std::shared_ptr<NetworkSocket> sender, std::string str, RPC_Receiver reciever)
{
	Network::_receivingStreamQueue(str);
	return true;
}

//bool ClientTCP::_recv(std::shared_ptr<NetworkSocket> receiveFrom)
//{
//	std::cout << "Reading TCP...\n";
//	std::shared_ptr<std::array<char, 128>> buf = std::make_shared<std::array<char, 128>>();
//	boost::system::error_code e;
//	auto timeout = std::make_shared<boost::asio::deadline_timer>(*Network::_service);
//	timeout->expires_from_now(boost::posix_time::seconds(3));
//	timeout->async_wait(boost::bind(&NetworkTCP::_asyncReadTimeoutHandler, this, boost::asio::placeholders::error, receiveFrom));
//
//	receiveFrom->socket.async_read_some(boost::asio::buffer(*buf, buf->size()),
//		boost::bind(&NetworkTCP::_asyncReadHandler, this, boost::asio::placeholders::error, receiveFrom, buf, boost::asio::placeholders::bytes_transferred, timeout)
//	);
//	receiveFrom->isReading = true;
//	return false;
//}


bool ClientTCP::_disconnect()
{
	if (!_serverSocket || !_serverSocket->connected() || !_serverSocket->socket.is_open()) return false;
	_serverSocket->socket.shutdown(tcp::socket::shutdown_both);
	_serverSocket->socket.close();
	_serverSocket->connected(false);
	_serverSocket->isReading(true);
	return true;
}
