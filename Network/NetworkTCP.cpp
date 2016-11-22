#define BOOST_ASIO_ENABLE_CANCELIO
#include <SDL.h>
#include <boost\bind.hpp>
#include "NetworkTCP.h"
#include "Network.h"

using namespace boost::asio::ip;



bool NetworkTCP::_update()
{
	try
	{
		if (Network::_service && !Network::_service->stopped())
		{
			Network::_service->poll();
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		return false;
	}
	return true;
}

void NetworkTCP::_asyncReadTimeoutHandler(const boost::system::error_code & error, std::shared_ptr<tcp::socket> senderSocket)
{
	if (!error)
	{
		std::cout << "Timeout\n";
		senderSocket->cancel(boost::system::error_code(boost::asio::error::timed_out));
	}
}

bool NetworkTCP::_send()
{
	return false;
}

bool NetworkTCP::_recv()
{
	return false;
}















bool ServerTCP::_update()
{
	NetworkTCP::_update();
	//if (ccc < 0) ++ccc;
	//else
	//	for (size_t i = 0; i < _clientSockets.size(); ++i)
	//	{
	//		ccc = 0;
	//		char buf[] = "lolwtf is this ";
	//		boost::system::error_code e;
	//		_clientSockets[i]->async_send(boost::asio::buffer(buf, 128), boost::bind(_asyncSendHandler, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred, _clientSockets[i]));
	//	}
	return false;
}
bool ServerTCP::_host()
{
	_acceptor = std::make_shared<tcp::acceptor>(*Network::_service);
	_connectable(true);
	std::cout << "Server hosted; awaiting connections...\n";
	return true;
}


void ServerTCP::_asyncConnectHandler(const boost::system::error_code& error, std::shared_ptr<NetworkSocket> peedingClient)
{
	std::cout << "Resolving peeding connection...\n";
	if (!error)
	{
		if (_clientSockets.size() >= _maxPlayers)
		{
			std::cout << "Client Declined\n";
			peedingClient->socket.shutdown(tcp::socket::shutdown_both);
			peedingClient->socket.close();
		}
		else
		{
			std::cout << "Client Connected!\n";
			_clientSockets.push_back(peedingClient);
		}
		peedingClient = std::make_shared<NetworkSocket>(*Network::_service);
		_acceptor->async_accept(peedingClient->socket, boost::bind(&ServerTCP::_asyncConnectHandler, *this, boost::asio::placeholders::error, peedingClient));
	}
	else
	{
		std::cout << "Stopped responding for connection allowance\n";
	}
}
void ServerTCP::_asyncReadHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket, std::shared_ptr<std::array<char, 128>> str, std::size_t len, std::shared_ptr<boost::asio::deadline_timer> timeout)
{
	auto it = _clientSockets.begin();
	for (; it != _clientSockets.end() && *it != sendingSocket; ++it);
	auto cls = [&it, &sendingSocket, this]()->void 
	{
		sendingSocket->socket.shutdown(tcp::socket::shutdown_both);
		sendingSocket->socket.close();
		_clientSockets.erase(it);
	};

	if (error == boost::asio::error::eof)
	{
		std::cout << "Client has disconnected\n";
		cls();
	}
	else if (error == boost::asio::error::timed_out)
	{
		std::cout << "Connection timeout in: " << sendingSocket->timeout << "\n";
		--sendingSocket->timeout;
		if (sendingSocket->timeout <= 0)
		{
			std::cout << "Connection timeout; Peer stopped responding\n";
			cls();
		}
	}
	else if (error == boost::asio::error::connection_reset)
	{
		std::cout << "Peer disconnected\n";
		cls();
	}
	else if (error)
	{
		std::cout << "Other Read error: " << error << std::endl;
	}
	else if (!len)
	{
		std::cout << "Recieved message has length of 0\n";
	}
	else
	{
		sendingSocket->timeout = _responseTimeout;
		std::cout << "Message received: " << std::string(str->data(), len) << std::endl;
	}
	timeout->cancel();
	sendingSocket->timeout = false;
}


bool ServerTCP::_disconnect()
{
	std::cout << "Disconnecting...\n";
	for (auto c : _clientSockets)
	{
		c->socket.shutdown(tcp::socket::shutdown_both);
		c->socket.close();
	}
	_clientSockets.erase(_clientSockets.begin(), _clientSockets.end());
	return true;
}



void ServerTCP::_asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket)
{
	if (error)
	{
		if (error == boost::asio::error::connection_reset) std::cout << "Client has restarted connection\n";
		else std::cout << "An error has occured during paset sending: " << error << std::endl;

		for (auto s = _clientSockets.begin(); s != _clientSockets.end(); ++s)
		{
			if (*s == sendingSocket)
			{
				sendingSocket->socket.shutdown(tcp::socket::shutdown_both);
				sendingSocket->socket.close();
				_clientSockets.erase(s);
				Network::Connectable(true);
				break;
			}
		}
	}
	else
	{
		//std::cout << "Package sent!\n";
	}
}



bool ServerTCP::_connectable(bool allow)
{
	if (!_acceptor) return false;
	if (!allow)
	{
		_acceptor->cancel();
		_acceptor->close();
		std::cout << "Connections disallowed\n";
		return true;
	}
	else
	{
		tcp::endpoint ep(tcp::v4(), Network::defaultPort);
		_acceptor->open(ep.protocol());
		_acceptor->bind(ep);
		_acceptor->listen(1);
		std::shared_ptr<NetworkSocket> _serverSocket = std::make_shared<NetworkSocket>(*Network::_service);
		_acceptor->async_accept(_serverSocket->socket, boost::bind(&ServerTCP::_asyncConnectHandler, *this, boost::asio::placeholders::error, _serverSocket));
		std::cout << "Connections allowed\n";
		return _acceptor.get() ? true : false;
	}
	return false;
}























bool ClientTCP::_update()
{
	NetworkTCP::_update();
	//if (!_serverSocket->is_open())
	//{
	//	Network::Disconnect();
	//}
	//else if (!_isReading)
	//{
	//	std::cout << "Reading...\n";
	//	std::shared_ptr<std::array<char, 128>> buf = std::make_shared<std::array<char, 128>>();
	//	boost::system::error_code error;
	//	auto timeout = std::make_shared<boost::asio::deadline_timer>(*Network::_service);
	//	timeout->expires_from_now(boost::posix_time::seconds(3));
	//	timeout->async_wait(boost::bind(_asyncReadTimeoutHandler, this, boost::asio::placeholders::error));

	//	//_serverSocket->async_read_some(boost::asio::buffer(*buf, 128), boost::bind(_asyncReadHandler, boost::asio::placeholders::error, buf, boost::asio::placeholders::bytes_transferred, timeout));
	//	boost::asio::async_read(*_serverSocket, boost::asio::buffer(*buf, 128), boost::bind(_asyncReadHandler, this, _serverSocket, boost::asio::placeholders::error, buf, boost::asio::placeholders::bytes_transferred, timeout));
	//	_isReading = true;
	//}

	return false;
}


void ClientTCP::_asyncReadHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket, std::shared_ptr<std::array<char, 128>> str, std::size_t len, std::shared_ptr<boost::asio::deadline_timer> timeout)
{
	if (error == boost::asio::error::eof)
	{
		std::cout << "Disconnected by the Server\n";
		Network::Disconnect();
	}
	else if (error == boost::asio::error::timed_out)
	{
		std::cout << "Connection timeout in: " << sendingSocket->timeout << "\n";
		--sendingSocket->timeout;
		if (sendingSocket->timeout <= 0)
		{
			std::cout << "Connection timeout; Server stopped responding\n";
			Network::Disconnect();
		}
	}
	else if (error == boost::asio::error::connection_reset)
	{
		std::cout << "Server disconnected\n";
		Network::Disconnect();
	}
	else if (error)
	{
		std::cout << "Other Read error: " << error << std::endl;
	}
	else if (!len)
	{
		std::cout << "Recieved message has length of 0\n";
	}
	else
	{
		sendingSocket->timeout = _responseTimeout;
		std::cout << "Message received: " << std::string(str->data(), len) << std::endl;
	}
	timeout->cancel();
	sendingSocket->isReading = false;
}

void ClientTCP::_asyncResolveHandler(const boost::system::error_code& error, tcp::resolver::iterator ep, std::shared_ptr<tcp::resolver> resolver)
{
	if (error)
	{
		std::cout << "An error has occured during client resolving: " << error << std::endl;
	}
	else
	{
		boost::asio::async_connect(_serverSocket->socket, ep, boost::bind(&ClientTCP::_asyncConnectHandler, *this, boost::asio::placeholders::error, ep));
	}
}
void ClientTCP::_asyncConnectHandler(const boost::system::error_code& error, tcp::resolver::iterator ep)
{
	if (error)
	{
		std::cout << "Connection not allowed\n";
	}
	else
	{
		std::cout << "Connected!\n";
		//_connectionType = ConnectionType::CT_Client;
		//_currentResponseTimeout = _responseTimeout;
	}
}
void ClientTCP::_asyncSendHandler(const boost::system::error_code & error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket)
{
}
bool ClientTCP::_join(std::string ip)
{
	std::shared_ptr<tcp::resolver> resolver = std::make_shared<tcp::resolver>(*Network::_service);
	_serverSocket = std::make_shared<NetworkSocket>(*Network::_service);
	std::cout << "Trying to connect to [" << ip << "]...\n";
	resolver->async_resolve(tcp::resolver::query(ip, std::to_string(Network::defaultPort)), boost::bind(&ClientTCP::_asyncResolveHandler, *this, boost::asio::placeholders::error, boost::asio::placeholders::iterator, resolver));
	return true;
}


bool ClientTCP::_disconnect()
{
	std::cout << "Disconnecting...\n";
	_serverSocket->socket.shutdown(tcp::socket::shutdown_both);
	_serverSocket->socket.close();
	return true;
}
