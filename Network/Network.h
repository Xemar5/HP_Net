#pragma once
#include <iostream>
#include <boost\asio.hpp>
#include "NetworkTCP.h"
#include "NetworkUDP.h"

enum ConnectionType
{
	CT_Server,
	CT_Client,
	CT_None
};

class Network
{
public:
	static bool Host(unsigned maxPlayers);
	static bool Join(std::string ip);
	static ConnectionType connectionType() { return _connectionType; }
	static bool Disconnect();
	static bool Connectable(bool allow);
	static int defaultPort;
	static bool Update();
private:
	static bool _init();
	static ConnectionType _connectionType;
	static std::shared_ptr<boost::asio::io_service> _service;
	static std::shared_ptr<boost::asio::io_service::work> _work;

	static std::shared_ptr<ServerUDP> _serverUDP;
	static std::shared_ptr<ServerTCP> _serverTCP;

	static std::shared_ptr<ClientUDP> _clientUDP;
	static std::shared_ptr<ClientTCP> _clientTCP;

	friend class Network;
	friend class NetworkTCP;
	friend class NetworkUDP;
	friend class Client;
	friend class ClientTCP;
	friend class ClientUDP;
	friend class Server;
	friend class ServerTCP;
	friend class ServerUDP;
};