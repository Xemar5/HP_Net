#define BOOST_ASIO_ENABLE_CANCELIO

#include <boost\bind.hpp>
#include "ServerTCP.h"
#include "Network.h"
#include "RPC.h"

using namespace boost::asio::ip;

int ccc = 0;
bool ServerTCP::_update()
{
	for (auto c : _clientSockets)
		if (!c->isReading() && c->connected()) _recv(c);

	NetworkTCP::_update();

	if (_peedingMessages.size()) _send();
	return false;

}
bool ServerTCP::_host()
{
	_acceptor = std::make_shared<tcp::acceptor>(*Network::_service);
	_connectable(true);
	std::cout << "Server hosted; awaiting connections...\n";
	return true;
}

bool ServerTCP::_send()
{
	return NetworkTCP::_send(std::bind(&ServerTCP::_asyncSendHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	//for (size_t i = 0; i < _peedingMessages.size(); ++i)
	//{
	//	std::string& s = _peedingMessages[i].second;
	//	while (s.size())
	//	{
	//		std::string buf(NetworkTCP::_packetLength - 1, '\0');

	//		if (s.size() > buf.size() - 1)
	//			buf[0] = 1;

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
	//					&ServerTCP::_asyncSendHandler,
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

bool ServerTCP::_recv(std::shared_ptr<NetworkSocket> receiveFrom)
{
	//if (receiveFrom->isReading()) return false;

	//receiveFrom->isReading(true);
	//std::shared_ptr<std::array<char, NetworkTCP::_packetLength>> buf = std::make_shared<std::array<char, NetworkTCP::_packetLength>>();
	//boost::system::error_code e;
	//auto timeout = std::make_shared<boost::asio::deadline_timer>(*Network::_service);
	//timeout->expires_from_now(boost::posix_time::seconds(3));
	//timeout->async_wait(boost::bind(&NetworkTCP::_asyncReadTimeoutHandler, this, boost::asio::placeholders::error, receiveFrom));

	//receiveFrom->socket.async_read_some(boost::asio::buffer(*buf, buf->size()),
	//	boost::bind(&NetworkTCP::_asyncReadHandler, this, boost::asio::placeholders::error, receiveFrom, buf, boost::asio::placeholders::bytes_transferred, timeout, std::bind(&ServerTCP::_asyncReadErrorHandler, Network::_serverTCP, std::placeholders::_1, receiveFrom))
	//);
	//return true;

	return NetworkTCP::_recv(receiveFrom, boost::bind(&ServerTCP::_asyncReadErrorHandler, this, boost::asio::placeholders::error, receiveFrom));
}

bool ServerTCP::_storeMessage(std::shared_ptr<NetworkSocket> sender, std::string str, RPC_Receiver receiver)
{
	std::vector<std::shared_ptr<NetworkSocket>> ns;

	switch (receiver)
	{
	case RPCr_Self:
		_peedingMessages.push_back({ { sender }, "0 " + str });
		break;

	case RPCr_All: 
		_peedingMessages.push_back({ _clientSockets, "0 " + str });
		Network::_receivingStreamQueue(str);
		break;

	case RPCr_Others:
		for (auto c : _clientSockets)
			if (c != sender) ns.push_back(c);
		_peedingMessages.push_back({ ns, "0 " + str });
		Network::_receivingStreamQueue(str);
		break;

	case RPCr_Server:
		Network::_receivingStreamQueue(str);
		break;

	default:
		break;
	}
	return false;
}

//bool ServerTCP::_recv(std::shared_ptr<NetworkSocket> receiveFrom)
//{
//	std::cout << "Reading TCP...\n";
//	std::shared_ptr<std::array<char, 128>> buf = std::make_shared<std::array<char, 128>>();
//	boost::system::error_code e;
//	auto timeout = std::make_shared<boost::asio::deadline_timer>(*Network::_service);
//	timeout->expires_from_now(boost::posix_time::seconds(3));
//	timeout->async_wait(boost::bind(&NetworkTCP::_asyncReadTimeoutHandler, this, boost::asio::placeholders::error, receiveFrom));
//
//	receiveFrom->socket.async_read_some(boost::asio::buffer(*buf, buf->size()),
//		boost::bind(&ServerTCP::_asyncReadHandler, this, boost::asio::placeholders::error, receiveFrom, buf, boost::asio::placeholders::bytes_transferred, timeout)
//	);
//	receiveFrom->isReading = true;
//	return false;
//}


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
			peedingClient->connected(true);
			Network::_serverTCP->_clientSockets.push_back(peedingClient);
		}
		peedingClient = std::make_shared<NetworkSocket>(*Network::_service);
		_acceptor->async_accept(peedingClient->socket, boost::bind(&ServerTCP::_asyncConnectHandler, this, boost::asio::placeholders::error, peedingClient));
	}
	else
	{
		std::cout << "Stopped responding for connection allowance\n";
	}
}
//void ServerTCP::_asyncReadHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket, std::shared_ptr<std::array<char, 128>> str, std::size_t len, std::shared_ptr<boost::asio::deadline_timer> timeout)
//{
//	auto it = _clientSockets.begin();
//	for (; it != _clientSockets.end() && *it != sendingSocket; ++it);
//	auto cls = [&it, &sendingSocket, this]()->void
//	{
//		sendingSocket->socket.shutdown(tcp::socket::shutdown_both);
//		sendingSocket->socket.close();
//		_clientSockets.erase(it);
//	};
//
//	if (error == boost::asio::error::eof)
//	{
//		std::cout << "Client has disconnected\n";
//		cls();
//	}
//	else if (error == boost::asio::error::timed_out)
//	{
//		std::cout << "Connection timeout in: " << sendingSocket->timeout << "\n";
//		--sendingSocket->timeout;
//		if (sendingSocket->timeout <= 0)
//		{
//			std::cout << "Connection timeout; Peer stopped responding\n";
//			cls();
//		}
//	}
//	else if (error == boost::asio::error::connection_reset)
//	{
//		std::cout << "Peer disconnected\n";
//		cls();
//	}
//	else if (error)
//	{
//		std::cout << "Other Read error: " << error << std::endl;
//	}
//	else if (!len)
//	{
//		std::cout << "Recieved message has length of 0\n";
//	}
//	else
//	{
//		sendingSocket->timeout = _responseTimeout;
//		std::cout << "Message received: " << std::string(str->data(), len) << std::endl;
//	}
//	timeout->cancel();
//	sendingSocket->timeout = false;
//}


bool ServerTCP::_disconnect()
{
	std::cout << "Disconnecting...\n";
	auto client = _clientSockets.begin();
	while (client != _clientSockets.end())
		client = _disconnectClient(client);
	return true;
}

bool ServerTCP::_disconnectClient(std::shared_ptr<NetworkSocket> client)
{
	for (auto b = _clientSockets.begin(); b != _clientSockets.end(); ++b)
		if (*b == client) { _disconnectClient(b); return true; }
	return false;
}

decltype(ServerTCP::_clientSockets.end()) ServerTCP::_disconnectClient(decltype(_clientSockets.end()) client)
{
	if (client == _clientSockets.end() || !*client || !(*client)->connected()) return _clientSockets.end();
	(*client)->socket.shutdown(tcp::socket::shutdown_both);
	(*client)->socket.close();
	(*client)->connected(false);
	(*client)->isReading(true);
	return _clientSockets.erase(client);
}



void ServerTCP::_asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket)
{
	if (error)
	{
		if (error == boost::asio::error::connection_reset) std::cout << "Send unavailable: Client has restarted connection\n";
		else std::cout << "An error has occured during packet sending (" << error << "): " << error.message() << std::endl;

		_disconnectClient(sendingSocket);
	}
	else
	{
		//std::cout << "Package sent!\n";
	}
}

void ServerTCP::_asyncReadErrorHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket)
{
	auto it = _clientSockets.begin();
	for (; it != _clientSockets.end() && *it != sendingSocket; ++it);
	//auto cls = [&it, &sendingSocket, this]()->void
	//{
	//	sendingSocket->socket.shutdown(tcp::socket::shutdown_both);
	//	sendingSocket->socket.close();
	//	sendingSocket->connected(false);
	//	_clientSockets.erase(it);
	//};

	if (error == boost::asio::error::eof)
	{
		std::cout << "Client has disconnected\n";
		_disconnectClient(it);
	}
	else if (error == boost::asio::error::timed_out || error == boost::asio::error::operation_aborted)
	{
		std::cout << "Connection timeout in: " << sendingSocket->timeout() * 3 << "\n";
		sendingSocket->timeout(sendingSocket->timeout() - 1);
		if (sendingSocket->timeout() <= 0)
		{
			std::cout << "Connection timeout; Peer stopped responding\n";
			_disconnectClient(it);
		}
	}
	else if (error == boost::asio::error::connection_reset)
	{
		std::cout << "Receive unavailable: Client has restarted connection\n";
		_disconnectClient(it);
	}
	else if (error)
	{
		std::cout << "Other Read error: " << error << std::endl;
		sendingSocket->timeout(sendingSocket->timeout() - 1);
		if (sendingSocket->timeout() <= 0)
		{
			std::cout << "Connection timeout; Peer stopped responding\n";
			_disconnectClient(it);
		}
	}
	else
	{
		throw "No error wtf u doin' here\n";
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
		_acceptor->listen(_maxPlayers);
		std::shared_ptr<NetworkSocket> peedingSocket = std::make_shared<NetworkSocket>(*Network::_service);
		_acceptor->async_accept(peedingSocket->socket, boost::bind(&ServerTCP::_asyncConnectHandler, this, boost::asio::placeholders::error, peedingSocket));
		std::cout << "Connections allowed\n";
		return _acceptor.get() ? true : false;
	}
	return false;
}
