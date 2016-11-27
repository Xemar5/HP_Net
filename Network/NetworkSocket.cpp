#include "NetworkSocket.h"
#include "Network.h"

unsigned NetworkSocket::_counter = 0;

NetworkSocket::NetworkSocket(boost::asio::io_service& service) : socket(service)
{
	if (Network::isServer())
		_index = _counter++;
}