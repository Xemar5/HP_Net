#include "NetworkUDP.h"


class ServerUDP : public NetworkUDP
{
public:
private:
	bool _update() override;
	bool _host();

	std::vector<std::shared_ptr<boost::asio::ip::udp::endpoint>> _clientEndpoints;

	bool _disconnect() override;
};

