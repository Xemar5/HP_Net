#include "NetworkUDP.h"


class ClientUDP : public NetworkUDP
{
public:
private:
	bool _update() override;
	bool _join();

	std::shared_ptr<boost::asio::ip::udp::endpoint> _serverEndpoint;

	bool _disconnect() override;
};