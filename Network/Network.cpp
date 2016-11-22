#include "Network.h"

int Network::defaultPort = 40856;
ConnectionType Network::_connectionType = ConnectionType::CT_None;
std::shared_ptr<boost::asio::io_service> Network::_service = nullptr;
std::shared_ptr<boost::asio::io_service::work> Network::_work = nullptr;

std::shared_ptr<ServerUDP> Network::_serverUDP = nullptr;
std::shared_ptr<ServerTCP> Network::_serverTCP = nullptr;

std::shared_ptr<ClientUDP> Network::_clientUDP = nullptr;
std::shared_ptr<ClientTCP> Network::_clientTCP = nullptr;

bool Network::Host(unsigned maxPlayers)
{
	if (_connectionType != ConnectionType::CT_None) return false;
	if (!_init()) return false;
	_connectionType = ConnectionType::CT_Server;
	_serverTCP = std::make_shared<ServerTCP>();
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
	return true;
}

bool Network::_init()
{
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
