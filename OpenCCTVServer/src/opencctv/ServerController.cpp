/*
 * ServerController.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#include "ServerController.hpp"

namespace opencctv {

ServerController* ServerController::_pServerController = NULL;

ServerController* ServerController::getInstance() {
	if (!_pServerController)
	{
		try
		{
			_pServerController = new ServerController();
		}catch(opencctv::Exception& e)
		{
			throw e;
		}
	}
	return _pServerController;
}

ServerController::ServerController()
{
	// Loading Configuration file
	opencctv::util::Config* pConfig = NULL;
	try
	{
		pConfig = opencctv::util::Config::getInstance();
	}catch(opencctv::Exception& e)
	{
		throw e;
	}

	opencctv::util::log::Loggers::getDefaultLogger()->info("Loading Configuration file done.");

	//Read the starting port
	std::string sServerPort = pConfig->get(opencctv::util::PROPERTY_OPENCCTV_SERVER_PORT);
	if(sServerPort.empty())
	{
		throw opencctv::Exception("Failed to retrieve OpenCCTV server port number from Configuration file.");
	}

	opencctv::util::log::Loggers::getDefaultLogger()->info("Initializing values done.");

	// Creating OpenCCTV Server's request-reply MQ
	try
	{
		_pSocket = opencctv::mq::MqUtil::createNewMq(sServerPort, ZMQ_REP);
	}
	catch(std::runtime_error &e)
	{
		std::string sErrMsg = "Failed to create OpenCCTV Server's request-reply MQ. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	opencctv::util::log::Loggers::getDefaultLogger()->info("Creating OpenCCTV Server's request-reply MQ done.");
}

void ServerController::executeOperation()
{

}

ServerController::~ServerController()
{
}

} /* namespace opencctv */
