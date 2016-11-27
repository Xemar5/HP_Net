#pragma once
#include <iostream>
#include <sstream>
#include <boost\asio.hpp>
#include <boost\thread\mutex.hpp>
#include "NetworkTCP.h"
#include "NetworkUDP.h"

class ClientTCP;
class ServerTCP;
class ClientUDP;
class ServerUDP;
enum RPC_Receiver;

#define RemoteFunc(RECEIVER, NAME, ...) FuncCall(RECEIVER, RPC::Set(#NAME, __VA_ARGS__))

enum ConnectionType
{
	CT_Server,
	CT_Client,
	CT_None
};

class Network
{
public:
	static bool Host(unsigned maxPlayers = 4, unsigned port = -1);
	static bool Join(std::string ip);
	static bool Disconnect();
	static bool Connectable(bool allow);

	static int defaultPort;

	static bool FuncCall(RPC_Receiver r, std::string str);

	static bool isConnected() { return _connectionType != ConnectionType::CT_None; }
	static bool isClient() { return _connectionType == ConnectionType::CT_Client; }
	static bool isServer() { return _connectionType == ConnectionType::CT_Server; }
	static bool isHost() { return false; }

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

	static void _receivingStreamQueue(std::string str);
	static std::vector<std::stringstream> _receivingStreams;
	static boost::mutex _receivingStreamMutex;

	friend class Network;
	friend class NetworkTCP;
	friend class NetworkUDP;
	friend class Client;
	friend class ClientTCP;
	friend class ClientUDP;
	friend class Server;
	friend class ServerTCP;
	friend class ServerUDP;
	friend class RPC;
};