#pragma once
#define BOOST_ASIO_ENABLE_CANCELIO

#include <boost\asio.hpp>
#include <mutex>
#include <sstream>

class NetworkSocket
{
public:
	NetworkSocket(boost::asio::io_service& service);
	NetworkSocket(const NetworkSocket& ns) = delete;
	NetworkSocket& operator=(const NetworkSocket& ns) = delete;

	boost::asio::ip::tcp::socket socket;
	void isReading(bool flag) { _mutex.lock(); _isReading = flag; _mutex.unlock(); }
	bool isReading() { return _isReading; }
	void connected(bool flag) { _mutex.lock(); _connected = flag; _mutex.unlock(); }
	bool connected() { return _connected; }
	void timeout(unsigned val) { _mutex.lock(); _timeout = val; _mutex.unlock(); }
	unsigned timeout() { return _timeout; }
	void message(std::string str) { _mutex.lock(); _messageHolder << str; _mutex.unlock(); }
	std::stringstream& message() { return _messageHolder; }

	~NetworkSocket() { socket.close(); }

private:
	bool _isReading = false;
	bool _connected = false;

	unsigned _timeout = 10;
	unsigned _index = -1;
	static unsigned _counter;

	std::stringstream _messageHolder;
	std::mutex _mutex;
};
