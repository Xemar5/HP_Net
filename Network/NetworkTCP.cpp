#define BOOST_ASIO_ENABLE_CANCELIO
#include <SDL.h>
#include <boost\bind.hpp>


#include "Network.h"
#include "ServerTCP.h"
#include "ClientTCP.h"
#include "NetworkTCP.h"
#include "RPC.h"

using namespace boost::asio::ip;



bool NetworkTCP::_update()
{
	while (Network::_receivingStreams.size())
	{
		std::string foo;
		Network::_receivingStreamMutex.lock();
		Network::_receivingStreams.front() >> foo;
		Network::_receivingStreamMutex.unlock();
		auto f = RPC::_Map.find(foo);
		if (f != RPC::_Map.end())
		{
			f->second.first(Network::_receivingStreams.front());
		}
		Network::_receivingStreams.erase(Network::_receivingStreams.begin());
	}
	return true;
}

bool NetworkTCP::_send(std::function<void(const boost::system::error_code&, std::size_t, std::shared_ptr<NetworkSocket>)> handler)
{
	for (auto it = _peedingMessages.begin();; ++it)
	{
		while (it != _peedingMessages.end() && !it->first.size()) it = _peedingMessages.erase(it);
		if (it == _peedingMessages.end()) return true;

		std::string& s = it->second;

		while (s.size())
		{
			
			std::string buf(NetworkTCP::_packetLength, '\0');
			std::size_t sz = buf.size() - 1;


			buf.replace
			(
				buf.begin() + 1,
				s.size() > sz ? buf.end() : buf.begin() + 1 + s.size(),
				s.begin(),
				s.size() > sz ? s.begin() + sz : s.end()
			);


			if (s.size() > sz)
			{
				s = s.substr(sz, s.size() - sz);
				buf[0] = 1;
			}
			else
			{
				s.clear();
				buf[0] = 0;
			}

			for (size_t j = 0; j < it->first.size(); ++j)
				it->first[j]->socket.async_send
				(
					boost::asio::buffer(buf.data(), buf.size()),
					boost::bind
					(
						handler,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred,
						it->first[j]
					)
				);

		}
	}
	_peedingMessages.clear();
	return true;
}

bool NetworkTCP::_recv(std::shared_ptr<NetworkSocket> receiveFrom, std::function<void(const boost::system::error_code&)> errorHandler)
{
	if (receiveFrom->isReading()) return false;

	receiveFrom->isReading(true);
	std::shared_ptr<std::array<char, NetworkTCP::_packetLength>> buf = std::make_shared<std::array<char, NetworkTCP::_packetLength>>();
	boost::system::error_code e;
	auto timeout = std::make_shared<boost::asio::deadline_timer>(*Network::_service);
	timeout->expires_from_now(boost::posix_time::seconds(3));
	timeout->async_wait(boost::bind(&NetworkTCP::_asyncReadTimeoutHandler, this, boost::asio::placeholders::error, receiveFrom));

	receiveFrom->socket.async_read_some(boost::asio::buffer(*buf, buf->size()),
		boost::bind(&NetworkTCP::_asyncReadHandler, this, boost::asio::placeholders::error, receiveFrom, buf, boost::asio::placeholders::bytes_transferred, timeout, errorHandler)
	);
	return true;
}

void NetworkTCP::_asyncReadTimeoutHandler(const boost::system::error_code & error, std::shared_ptr<NetworkSocket> senderSocket)
{
	if (!error)
	{
		std::cout << "Timeout\n";
		senderSocket->socket.cancel(boost::system::error_code(boost::asio::error::timed_out));
		//senderSocket->socket.close();
		//auto ep = senderSocket->socket.remote_endpoint();
		//boost::system::error_code ec;
		//senderSocket->socket.connect(ep,ec);
		//if (ec) std::cout << ec.message() << " " << ec << std::endl;
	}
}

void NetworkTCP::_asyncReadHandler
(
	const boost::system::error_code & error,
	std::shared_ptr<NetworkSocket> sendingSocket,
	std::shared_ptr<std::array<char, NetworkTCP::_packetLength>> str,
	std::size_t len,
	std::shared_ptr<boost::asio::deadline_timer> timeout,
	std::function<void(const boost::system::error_code&)> errorHandler
)
{
	if (error)
	{
		errorHandler(error);
	}
	else if (!len)
	{
		std::cout << "Recieved message has length of 0\n";
	}
	else
	{
		sendingSocket->timeout(_responseTimeout);
		std::string rcvd = std::string(str->data(), len);
		rcvd = rcvd.substr(1, rcvd.size() - 1);
		if (str->at(0))
		{
			sendingSocket->message(rcvd);
		}
		else
		{
			//Network::_receivingStreamMutex.lock();
			//Network::_receivingStreams.emplace_back(std::stringstream());
			//Network::_receivingStreams.back() << sendingSocket->message().str();
			//Network::_receivingStreams.back() << rcvd;
			//Network::_receivingStreamMutex.unlock();
			int receiver = -1;
			std::string msg;
			sendingSocket->message(rcvd);
			sendingSocket->message() >> receiver;
			std::getline(sendingSocket->message(), msg);
			sendingSocket->message().str(std::string());
			sendingSocket->message().clear();


			_storeMessage(sendingSocket, msg, (RPC_Receiver)receiver);
		}
	}
	timeout->cancel();
	sendingSocket->isReading(false);
}

