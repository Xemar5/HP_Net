#include "Network.h"
#include "ClientTCP.h"
#include "ServerTCP.h"
#include "ClientUDP.h"
#include "ServerUDP.h"
#include "RPC.h"

int Network::defaultPort = 40856;
std::vector<std::stringstream> Network::_receivingStreams;
boost::mutex Network::_receivingStreamMutex;

ConnectionType Network::_connectionType = ConnectionType::CT_None;
std::shared_ptr<boost::asio::io_service> Network::_service = nullptr;
std::shared_ptr<boost::asio::io_service::work> Network::_work = nullptr;

std::shared_ptr<ServerUDP> Network::_serverUDP = nullptr;
std::shared_ptr<ServerTCP> Network::_serverTCP = nullptr;

std::shared_ptr<ClientUDP> Network::_clientUDP = nullptr;
std::shared_ptr<ClientTCP> Network::_clientTCP = nullptr;

bool Network::Host(unsigned maxPlayers, unsigned port)
{
	if (_connectionType != ConnectionType::CT_None) return false;
	if (!_init()) return false;
	_connectionType = ConnectionType::CT_Server;
	if(port != -1) defaultPort = port;
	_serverTCP = std::make_shared<ServerTCP>();
	_serverTCP->_maxPlayers = maxPlayers;
	_serverTCP->_host();
	return true;
}

bool Network::Join(std::string ip)
{
	if (_connectionType != ConnectionType::CT_None) return false;
	if (!_init()) return false;
	_connectionType = ConnectionType::CT_Client;
	_clientTCP = std::make_shared<ClientTCP>();
	_clientTCP->_join(ip);
	return true;
}

bool Network::Disconnect()
{
	if (_clientTCP) _clientTCP->_disconnect();
	if (_serverTCP) _serverTCP->_disconnect();
	//if (_clientUDP) _clientUDP->_disconnect();
	//if (_serverUDP) _serverUDP->_disconnect();
	return true;
}

bool Network::Connectable(bool allow)
{
	if (!_serverTCP) return false;
	return _serverTCP->_connectable(allow);
}

bool Network::Update()
{
	if (_serverTCP) _serverTCP->_update();
	if (_clientTCP) _clientTCP->_update();
	//if (_serverUDP) _serverUDP->_update();
	//if (_clientUDP) _clientUDP->_update();
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

bool Network::FuncCall(RPC_Receiver r, std::string str)
{
	if(_clientTCP) _clientTCP->_peedingMessages.push_back({ { _clientTCP->_serverSocket }, std::to_string(r) + " " + str });
	else if(_serverTCP)
	{
		switch (r)
		{
		case RPCr_Self: Network::_receivingStreamQueue(str); break;
		case RPCr_Server: Network::_receivingStreamQueue(str); break;
		case RPCr_Others: 
			_serverTCP->_peedingMessages.push_back({ _serverTCP->_clientSockets, "0 " + str }); break;
		case RPCr_All:
			_serverTCP->_peedingMessages.push_back({ _serverTCP->_clientSockets, "0 " + str });
			Network::_receivingStreamQueue(str); break;
		default: break;
		}
		
	}

	return true;
}

//bool Network::Send(std::vector<std::shared_ptr<NetworkSocket>> r, std::string str)
//{
//	if (_serverTCP)_serverTCP->_peedingMessages.push_back({ r, str });
//	if (_clientTCP)_clientTCP->_peedingMessages.push_back({ { _clientTCP->_serverSocket }, str });
//	return true;
//}

bool Network::_init()
{
	if (_service) return true;
	try
	{
		_service = std::make_shared<boost::asio::io_service>();
		_work = std::make_shared<boost::asio::io_service::work>(*_service);
	}
	catch (const std::exception&)
	{
		std::cout << "An error occured during io_service or work initializaion\n";
		_service = nullptr;
		_work = nullptr;
		return false;
	}
	return true;
}

void Network::_receivingStreamQueue(std::string str)
{
	Network::_receivingStreamMutex.lock();
	Network::_receivingStreams.emplace_back(std::stringstream());
	Network::_receivingStreams.back() << str;
	Network::_receivingStreamMutex.unlock();
}
