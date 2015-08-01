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

	}catch(opencctv::Exception &e)
	{
		sOperation = opencctv::event::EVENT_UNKNOWN;
		sMessage = e.what();
	}

	std::cout << "sRequest : " << sRequest << std::endl;

	//Execute the operation according to the operation type
	try
	{
		if(sOperation.compare(opencctv::event::SERVER_EVENT_START) == 0)
		{
			bool bResult = opencctv::event::ServerEvent::startServer();

			if(bResult)
			{
				sReply = opencctv::event::EventMessage::getStartMessageReply();
			}else
			{
				sReply = opencctv::event::EventMessage::getInvalidMessageReply("Failed to start the OpenCCTV Server");
			}
			sendReply(sReply);

		}else if(sOperation.compare(opencctv::event::SERVER_EVENT_STOP) == 0)
		{
			bool bResult = opencctv::event::ServerEvent::stopServer();

			if(bResult)
			{
				sReply = opencctv::event::EventMessage::getStopMessageReply();
			}else
			{
				sReply = opencctv::event::EventMessage::getInvalidMessageReply("Error occurred in stopping the OpenCCTV Server");
			}
			sendReply(sReply);

		}else if(sOperation.compare(opencctv::event::SERVER_EVENT_RESTART) == 0)
		{
			bool bResult = opencctv::event::ServerEvent::stopServer();

			if(bResult)
			{
				bResult = opencctv::event::ServerEvent::startServer();
			}

			if(bResult)
			{
				sReply = opencctv::event::EventMessage::getStatusMessageReply();
			}else
			{
				sReply = opencctv::event::EventMessage::getInvalidMessageReply("Error occurred in restarting the OpenCCTV Server");
			}
			sendReply(sReply);

		}else if(sOperation.compare(opencctv::event::SERVER_EVENT_STATUS) == 0)
		{
			sReply = opencctv::event::EventMessage::getStatusMessageReply();
			sendReply(sReply);

		}else if(sOperation.compare(opencctv::event::ANALYTIC_EVENT_START_INSTANCE) == 0)
		{
			sReply = opencctv::event::AnalyticEvent::startAnalytic(sRequest);
			sendReply(sReply);

		}else if(sOperation.compare(opencctv::event::ANALYTIC_EVENT_STOP_INSTANCE) == 0)
		{
			sReply = opencctv::event::AnalyticEvent::stopAnalytic(sRequest);
			sendReply(sReply);

		}else if(sOperation.compare(opencctv::event::ANALYTIC_EVENT_STOP_INSTANCES) == 0)
		{
			sReply = opencctv::event::AnalyticEvent::stopAnalytics(sRequest);
			sendReply(sReply);

		}else
		{
			std::string sErrMsg = "Request with an unknown operation.\nRequest: ";
			sErrMsg.append(sRequest);
			opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			sReply = opencctv::event::EventMessage::getInvalidMessageReply("Request with an unknown operation");
			sendReply(sReply);
		}

	}catch(opencctv::Exception &e)
	{
		std::string sErrMsg = "OpenCCTV server error - ";
		sErrMsg.append(e.what());
		sReply = opencctv::event::EventMessage::getInvalidMessageReply(sErrMsg);
		sendReply(sReply);
	}


}

void ServerController::sendReply(const std::string& sMessage)
{
	std::cout << "sReply : " << sMessage << std::endl;

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
