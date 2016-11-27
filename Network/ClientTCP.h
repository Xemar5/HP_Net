#include "NetworkTCP.h"


class ClientTCP : public NetworkTCP
{
public:
protected:
	bool _update() override;
	bool _join(std::string ip);
	bool _send();
	bool _recv();
	bool _storeMessage(std::shared_ptr<NetworkSocket>sender, std::string str, RPC_Receiver reciever) override;

	void _asyncResolveHandler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator ep, std::shared_ptr<boost::asio::ip::tcp::resolver> resolver);
	void _asyncConnectHandler(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator ep);
	void _asyncSendHandler(const boost::system::error_code& error, std::size_t len, std::shared_ptr<NetworkSocket> sendingSocket) override;
	void _asyncReadErrorHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> sendingSocket) override;
	//void _asyncReadHandler(
	//	const boost::system::error_code& error,
	//	std::shared_ptr<NetworkSocket> sendingSocket,
	//	std::shared_ptr<std::array<char, 128>> str,
	//	std::size_t len,
	//	std::shared_ptr<boost::asio::deadline_timer> timeout,
	//	std::function<void(const boost::system::error_code&)) override;

	std::shared_ptr<NetworkSocket> _serverSocket = nullptr;

	bool _disconnect() override;

	friend class Network;
	friend class NetworkTCP;
};