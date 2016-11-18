#include "Network.h"

using namespace boost::asio::ip;

int Network::port = 48046;
boost::asio::io_service Network::service;
//tcp::acceptor Network::acceptor(service, tcp::endpoint(tcp::v4(), port));


bool Network::Init()
{
	return true;
}

bool Network::Update()
{
	return false;
}
