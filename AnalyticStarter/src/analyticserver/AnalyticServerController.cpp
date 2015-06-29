/*
 * AnalyticServerController.cpp
 *
 *  Created on: Jun 29, 2015
 *      Author: anjana
 */

#include "AnalyticServerController.hpp"

namespace analyticserver {

AnalyticServerController* AnalyticServerController::_pAnalyticServerController = NULL;

AnalyticServerController* AnalyticServerController::getInstance() {
	if (!_pAnalyticServerController)
	{
		try
		{
			_pAnalyticServerController = new AnalyticServerController();
		}catch(opencctv::Exception& e)
		{
			throw e;
		}
	}
	return _pAnalyticServerController;
}

AnalyticServerController::AnalyticServerController()
{
	// Loading Configuration file
	analytic::util::Config* pConfig = NULL;
	try
	{
		pConfig = analytic::util::Config::getInstance();
	}catch(opencctv::Exception& e)
	{
		throw e;
	}

	opencctv::util::log::Loggers::getDefaultLogger()->info("Loading Configuration file done.");

	// Initializing values
	//Read the starting port
	std::string sStartingPort = pConfig->get(analytic::util::PROPERTY_STARTER_PORT);
	if(sStartingPort.empty())
	{
		//opencctv::util::log::Loggers::getDefaultLogger()->error("Failed to retrieve Analytic Starter port from Configuration file.");
		throw opencctv::Exception("Failed to retrieve Analytic Starter port from Configuration file.");
	}
	STARTING_PORT = boost::lexical_cast<unsigned int>(sStartingPort);

	//Read the number of analytics
	std::string sNoAnalytics = pConfig->get(analytic::util::PROPERTY_NO_ANALYTICS);
	if(sNoAnalytics.empty())
	{
		throw opencctv::Exception("Failed to retrieve Number of Analytics from Configuration file.");
	}
	NO_ANALYTICS = boost::lexical_cast<unsigned int>(sNoAnalytics);

	fillIOPorts();

	opencctv::util::log::Loggers::getDefaultLogger()->info("Initializing values done.");

	// Creating Analytic Server's request-reply MQ
	try
	{
		_pSocket = opencctv::mq::MqUtil::createNewMq(sStartingPort, ZMQ_REP);
	}
	catch(std::runtime_error &e)
	{
		std::string sErrMsg = "Failed to create Analytic Server's request-reply MQ. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	opencctv::util::log::Loggers::getDefaultLogger()->info("Creating Analytic Server's request-reply MQ done.");
}

void AnalyticServerController::executeOperation()
{

	std::string sRequest;
	std::string sOperation;
	std::string sMessage;

	//Read the request
	try
	{
		opencctv::mq::MqUtil::readFromSocket(_pSocket, sRequest);
		sOperation = analytic::xml::AnalyticMessage::extractAnalyticRequestOperation(sRequest);
	}
	catch(opencctv::Exception &e)
	{
		sOperation = analytic::xml::OPERATION_UNKNOWN;
		sMessage = e.what();
	}

	//Execute the operation according to the operation type
	std::string sReply;
	if(sOperation.compare(analytic::xml::OPERATION_START_ANALYTIC) == 0)
	{
		sReply = startAnalytic(sRequest);
		sendReply(sReply);
	}
	else if(sOperation.compare(analytic::xml::OPERATION_STOP_ANALYTIC) == 0)
	{
		sReply = stopAnalytic(sRequest);
		sendReply(sReply);

	}
	else if(sOperation.compare(analytic::xml::OPERATION_KILL_ALL_ANALYTICS) == 0)
	{
		sReply = killAllAnalytics(sRequest);
		sendReply(sReply);
	}
	else
	{
		std::string sErrMsg = "Request with an unknown Operation.\nRequest: ";
		sErrMsg.append(sRequest);
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		sReply = analytic::xml::AnalyticMessage::getErrorReply(analytic::xml::OPERATION_UNKNOWN, false, sMessage);
		sendReply(sReply);
	}
}

std::string AnalyticServerController::startAnalytic(const std::string& sRequest)
{
	// Request data
	unsigned int iAnalyticInstanceId;
	std::string sAnalyticDirPath;
	std::string sAnalyticFilename;

	//Reply data
	std::string sReply;
	bool bAIStarted = false;
	std::string sErrMsg;
	std::string sAnalyticInputQueueAddress;
	std::string sAnalyticOutputQueueAddress;
	try
	{
		analytic::xml::AnalyticMessage::extractAnalyticStartRequestData(sRequest, iAnalyticInstanceId, sAnalyticDirPath, sAnalyticFilename);

		if(iAnalyticInstanceId > 0 && !sAnalyticDirPath.empty() && !sAnalyticFilename.empty())
		{
			std::string sAnalyticInputQueueAddress;
			std::string sAnalyticOutputQueueAddress;
			if(!analytic::ApplicationModel::getInstance()->getAnalyticPorts().empty())
			{
				sAnalyticInputQueueAddress = boost::lexical_cast<std::string>(analytic::ApplicationModel::getInstance()->getAnalyticPorts().back());
				analytic::ApplicationModel::getInstance()->getAnalyticPorts().pop_back();
				sAnalyticOutputQueueAddress = boost::lexical_cast<std::string>(analytic::ApplicationModel::getInstance()->getAnalyticPorts().back());
				analytic::ApplicationModel::getInstance()->getAnalyticPorts().pop_back();

			}else
			{
				sErrMsg = "The analytic server has reached the maximum number of analytic instances it can support";
				sReply = analytic::xml::AnalyticMessage::getAnalyticStartReply(bAIStarted, sErrMsg, sAnalyticInputQueueAddress, sAnalyticInputQueueAddress);
				return sReply;
			}

			analytic::util::Config* pConfig = analytic::util::Config::getInstance();
			std::string sAnalyticRunnerPath;
			sAnalyticRunnerPath.append(pConfig->get(analytic::util::PROPERTY_ANALYTIC_RUNNER_DIR));
			sAnalyticRunnerPath.append("/");
			sAnalyticRunnerPath.append(pConfig->get(analytic::util::PROPERTY_ANALYTIC_RUNNER_FILENAME));

			analytic::AnalyticProcess* pAnalyticProcess = new analytic::AnalyticProcess(sAnalyticRunnerPath, iAnalyticInstanceId, sAnalyticDirPath, sAnalyticFilename, sAnalyticInputQueueAddress, sAnalyticOutputQueueAddress);
			bAIStarted = pAnalyticProcess->startAnalytic();

			if(bAIStarted)
			{
				analytic::ApplicationModel::getInstance()->getAnalyticProcesses()[iAnalyticInstanceId] = pAnalyticProcess;
				std::stringstream ssMsg;
				ssMsg << "Analytic Instance " << iAnalyticInstanceId << " started. ";
				ssMsg << "Input queue at: " << sAnalyticInputQueueAddress << ", ";
				ssMsg << "Output queue at: " << sAnalyticOutputQueueAddress;
				opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				sErrMsg = "Analytic instance start successful";
				sReply = analytic::xml::AnalyticMessage::getAnalyticStartReply(bAIStarted, sErrMsg, sAnalyticInputQueueAddress, sAnalyticOutputQueueAddress);
			}
			else
			{
				//Reclaim the IO queue addresses
				unsigned int iPort;
				iPort = boost::lexical_cast<unsigned int>(pAnalyticProcess->getInputAnalyticQueueAddress());
				analytic::ApplicationModel::getInstance()->getAnalyticPorts().push_back(iPort);
				iPort = boost::lexical_cast<unsigned int>(pAnalyticProcess->getOutputAnalyticQueueAddress());
				analytic::ApplicationModel::getInstance()->getAnalyticPorts().push_back(iPort);
				sAnalyticInputQueueAddress = "";
				sAnalyticOutputQueueAddress = "";

				std::stringstream ssErrMsg;
				ssErrMsg << "Failed to start Analytic Instance " << iAnalyticInstanceId << ". ";
				sErrMsg = ssErrMsg.str();
				opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
				sReply = analytic::xml::AnalyticMessage::getAnalyticStartReply(bAIStarted, sErrMsg, sAnalyticInputQueueAddress, sAnalyticOutputQueueAddress);
				return sReply;
			}
		}
		else
		{
			std::string sErrMsg = "Request with invalid data.\nRequest: ";
			sErrMsg.append(sRequest);
			opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			sErrMsg = "Request with invalid data";
			sReply = analytic::xml::AnalyticMessage::getAnalyticStartReply(bAIStarted, sErrMsg, sAnalyticInputQueueAddress, sAnalyticOutputQueueAddress);
			return sReply;
		}
	}
	catch(opencctv::Exception &e)
	{
		std::stringstream ossReply;
		ossReply << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
		ossReply << "<analyticreply><operation>" << analytic::xml::OPERATION_START_ANALYTIC << "</operation>";
		ossReply << "<done>" << bAIStarted << "</done>";
		ossReply << "<message>" << e.what() << "</message>";
		ossReply << "<analyticqueueinaddress>" << sAnalyticInputQueueAddress << "</analyticqueueinaddress>";
		ossReply << "<analyticqueueoutaddress>" << sAnalyticOutputQueueAddress << "</analyticqueueoutaddress>";
		ossReply << "</analyticreply>";
		sReply = ossReply.str();
	}

	return sReply;

}

std::string AnalyticServerController::stopAnalytic(const std::string& sRequest)
{
	// Request data
	unsigned int iAnalyticInstanceId;

	//Reply data
	std::string sReply;
	bool bDone = false;
	try
	{
		analytic::xml::AnalyticMessage::extractAnalyticStopRequestData(sRequest, iAnalyticInstanceId);
		if(iAnalyticInstanceId > 0)
		{
			std::ostringstream msg;

			if(analytic::ApplicationModel::getInstance()->containsAnalyticProcess(iAnalyticInstanceId))
			{
				analytic::AnalyticProcess* pAnalyticProcess = analytic::ApplicationModel::getInstance()->getAnalyticProcesses()[iAnalyticInstanceId];
				bDone = pAnalyticProcess->stop();

				//Reclaim the ports issued to the stopped analytic
				unsigned int iPort;
				iPort = boost::lexical_cast<unsigned int>(pAnalyticProcess->getInputAnalyticQueueAddress());
				analytic::ApplicationModel::getInstance()->getAnalyticPorts().push_back(iPort);
				iPort = boost::lexical_cast<unsigned int>(pAnalyticProcess->getOutputAnalyticQueueAddress());
				analytic::ApplicationModel::getInstance()->getAnalyticPorts().push_back(iPort);

				//Remove the analytic from the ApplicationModel
				analytic::ApplicationModel::getInstance()->getAnalyticProcesses().erase(iAnalyticInstanceId);
				delete pAnalyticProcess; pAnalyticProcess = NULL;

				if(bDone)
				{
					msg << " Successfully stopped ";
				}else
				{
					msg << " Analytic server failed to stop ";
				}

				msg << "the analytic instance : " << iAnalyticInstanceId;

				sReply = analytic::xml::AnalyticMessage::getAnalyticStopReply(bDone,msg.str());

			}else
			{
				msg << "Cannot find the analytic instance with the id : ";
				msg << iAnalyticInstanceId;
				sReply = analytic::xml::AnalyticMessage::getAnalyticStopReply(bDone,msg.str());
				return sReply;
			}
		}else
		{
			sReply = analytic::xml::AnalyticMessage::getAnalyticStopReply(bDone,"Failed to extract analytic instance Id");
			return sReply;
		}
	}
	catch(opencctv::Exception &e)
	{
		opencctv::util::log::Loggers::getDefaultLogger()->error(e.what());
		sReply = analytic::xml::AnalyticMessage::getErrorReply(analytic::xml::OPERATION_STOP_ANALYTIC, bDone, e.what());
	}

	return sReply;

}

std::string AnalyticServerController::killAllAnalytics(const std::string& sRequest)
{
	bool bDone = false;
	std::map<unsigned int, analytic::AnalyticProcess*> mAnalyticProcesses = analytic::ApplicationModel::getInstance()->getAnalyticProcesses();
	std::map<unsigned int, analytic::AnalyticProcess*>::iterator it;
	for(it = mAnalyticProcesses.begin(); it != mAnalyticProcesses.end(); /*it increment below*/) {
		analytic::AnalyticProcess* pAnalyticProcess = it->second;
		pAnalyticProcess->stop();
		delete pAnalyticProcess; pAnalyticProcess = NULL;
		mAnalyticProcesses.erase(it++); //remove analytic process from the model
	}
	if(mAnalyticProcesses.empty())
	{
		bDone = true; // killed all analytics
	}

	std::string sReply, sMessage;
	if(bDone)
	{
		fillIOPorts();
		sMessage = "Successfully kill all the analytic instances";
		opencctv::util::log::Loggers::getDefaultLogger()->info(sMessage);
	}else
	{
		sMessage = "Failed to kill all the analytic instances";
		opencctv::util::log::Loggers::getDefaultLogger()->error(sMessage);
	}

	try {
		sReply = analytic::xml::AnalyticMessage::getKillAllAnalyticProcessesReply(bDone,sMessage);
	} catch(opencctv::Exception &e)
	{
		opencctv::util::log::Loggers::getDefaultLogger()->error(e.what());
		sReply = analytic::xml::AnalyticMessage::getErrorReply(analytic::xml::OPERATION_KILL_ALL_ANALYTICS, bDone, e.what());
	}

	return sReply;
}

void AnalyticServerController::sendReply(const std::string& sMessage)
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
		opencctv::util::log::Loggers::getDefaultLogger()->error("Analytic Server Request-Reply Queue is not Initialized");
	}
}

void AnalyticServerController::fillIOPorts()
{
	unsigned int iPort = STARTING_PORT;
	for(unsigned int i = 0; i < NO_ANALYTICS; i++)
	{
		analytic::ApplicationModel::getInstance()->getAnalyticPorts().push_back(++iPort);//for input queue address
		analytic::ApplicationModel::getInstance()->getAnalyticPorts().push_back(++iPort);//for output queue address
	}
	std::cout << "Size Analytic ports = " << analytic::ApplicationModel::getInstance()->getAnalyticPorts().size();
}

AnalyticServerController::~AnalyticServerController()
{
}

} /* namespace analytic */
