#include "TcpMqReceiver.hpp"

namespace opencctv {
namespace mq {

TcpMqReceiver::TcpMqReceiver() {
	_pSocket = NULL;
}

bool TcpMqReceiver::createMq(const std::string& serverPortStr) {
	try {
		_pSocket = MqUtil::createNewMq(serverPortStr, ZMQ_PULL);
	} catch(opencctv::Exception &e)
	{
		throw e;
	}
	if (_pSocket) {
		return true;
	}
	return false;
}

bool TcpMqReceiver::connectToMq(const std::string& serverName,
		const std::string& serverPortStr) {
	try {
		_pSocket = MqUtil::connectToMq(serverName, serverPortStr, ZMQ_PULL);
	} catch(opencctv::Exception &e)
	{
		throw e;
	}
	if (_pSocket) {
		return true;
	}
	return false;
}

/**
 * Alert! Please remember to delete the returned std::string after using.
 * Because it has been created on the heap using new operator.
 * Use this function when receiving very large serialized images from the MQ.
 */
std::string* TcpMqReceiver::receive() {
	std::string* pSMessage = NULL;
	try
	{
		pSMessage = MqUtil::readFromSocket(_pSocket);
	}
	catch(opencctv::Exception &e)
	{
		throw e;
	}
	return pSMessage;
}

TcpMqReceiver::~TcpMqReceiver() {
	if(_pSocket)
	{
		_pSocket->close();
		delete _pSocket;
	}
}

} /* namespace mq */
} /* namespace opencctv */
