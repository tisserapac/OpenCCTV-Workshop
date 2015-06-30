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
	std::string sRequest;
	std::string sReply;
	std::string sOperation;
	std::string sMessage;

	//Read the request
	try
	{
		opencctv::mq::MqUtil::readFromSocket(_pSocket, sRequest);
		sOperation = opencctv::event::EventMessage::extractEventRequestOperation(sRequest);
	}
	catch(opencctv::Exception &e)
	{
		sOperation = opencctv::event::EVENT_UNKNOWN;
		sMessage = e.what();
	}

	std::cout << "sOperation : " << sOperation << std::endl;

	//Execute the operation according to the operation type
	if(sOperation.compare(opencctv::event::SERVER_EVENT_START) == 0)
	{
		bool bResult = opencctv::event::ServerEvent::serverStart();

		if(bResult)
		{
			sReply = opencctv::event::EventMessage::getStartMessageReply();
		}else
		{
			sReply = "Error";
		}
		sendReply(sReply);
	}
	else if(sOperation.compare(opencctv::event::SERVER_EVENT_STOP) == 0)
	{
		bool bResult = opencctv::event::ServerEvent::serverStop();

		if(bResult)
		{
			sReply = opencctv::event::EventMessage::getStopMessageReply();
		}else
		{
			sReply = "Error";
		}
		sendReply(sReply);
	}
	else
	{
		std::string sErrMsg = "Request with an unknown Operation.\nRequest: ";
		sErrMsg.append(sRequest);
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		//sReply = analytic::xml::AnalyticMessage::getErrorReply(analytic::xml::OPERATION_UNKNOWN, false, sMessage);
		sReply = "Error";
		sendReply(sReply);
	}
}

void ServerController::sendReply(const std::string& sMessage)
{
	if(_pSocket)
	{
		try {
			opencctv::mq::MqUtil::writeToSocket(_pSocket, sMessage);
		} catch (opencctv::Exception &e) {
			opencctv::util::log::Loggers::getDefaultLogger()->error(e.what());
		}
	}else
	{
		opencctv::util::log::Loggers::getDefaultLogger()->error("OpenCCTV Server Request-Reply Queue is not Initialized");
	}
}

ServerController::~ServerController()
{
}

} /* namespace opencctv */
