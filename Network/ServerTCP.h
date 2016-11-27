#include "NetworkTCP.h"

enum RPC_Receiver;

class ServerTCP : public NetworkTCP
{
public:

protected:
	bool _update() override;
	bool _host();
	bool _send();
	bool _recv(std::shared_ptr<NetworkSocket> reciveFrom);
	bool _storeMessage(std::shared_ptr<NetworkSocket>sender, std::string str, RPC_Receiver reciever) override;

	void _asyncConnectHandler(const boost::system::error_code& error, std::shared_ptr<NetworkSocket> peedingClient);
	void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket) override;
	void _asyncReadErrorHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket) override;
	//void _asyncReadHandler(
	//	const boost::system::error_code& error,
	//	std::shared_ptr<NetworkSocket> sendingSocket,
	//	std::shared_ptr<std::array<char, 128>> str,
	//	std::size_t len,
	//	std::shared_ptr<boost::asio::deadline_timer> timeout) override;

	std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor = nullptr;
	std::vector<std::shared_ptr<NetworkSocket>> _clientSockets;

	unsigned _maxPlayers = -1;

	bool _connectable(bool value);
	bool _disconnect() override;
	bool _disconnectClient(std::shared_ptr<NetworkSocket> client);
	decltype(_clientSockets.end()) _disconnectClient(decltype(_clientSockets.end()) client);

	friend class Network;
};
