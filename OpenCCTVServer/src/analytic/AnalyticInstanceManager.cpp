
#include "AnalyticInstanceManager.hpp"

namespace analytic {

AnalyticInstanceManager::AnalyticInstanceManager(const std::string& sAnalyticServerIp, const std::string& sAnalyticServerPort) {
	try
	{
		_pSocket = opencctv::mq::MqUtil::connectToMq(sAnalyticServerIp, sAnalyticServerPort, ZMQ_REQ);
	}
	catch(std::runtime_error &e)
	{
		std::string sErrMsg = "Failed to create ZMQ Socket to connect to Analytic Starter. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

bool AnalyticInstanceManager::startAnalyticInstance(unsigned int iAnalyticInstanceId, const std::string& sAnalyticPluginDirLocation, std::string& sAnalyticQueueInAddress, std::string& sAnalyticQueueOutAddress)
{
	bool bRet = false;
	if(_pSocket)
	{
		std::string sRequest, sReply;
		try
		{
			sRequest = xml::AnalyticMessage::getAnalyticStartRequest(iAnalyticInstanceId, sAnalyticPluginDirLocation);

			if (!opencctv::mq::MqUtil::writeToSocket(_pSocket, sRequest))
			{
				throw opencctv::Exception("Failed to send Analytic Start request.");
			}

			if(!opencctv::mq::MqUtil::readFromSocket(_pSocket, sReply))
			{
				throw opencctv::Exception("Failed to read Analytic Start reply.");
			}

			if (!sReply.empty() && sReply.compare("Error") != 0)
			{
				std::string sMessage;
				analytic::xml::AnalyticMessage::extractAnalyticStartReplyData(sReply, bRet, sMessage, sAnalyticQueueInAddress,sAnalyticQueueOutAddress);
			} else
			{
				throw opencctv::Exception("Error in extracting Analytic Start reply data.");
			}
		} catch (opencctv::Exception &e) {
			std::string sErrMsg = "AnalyticInstanceManager::startAnalyticInstance - ";
			sErrMsg.append(e.what());
			throw opencctv::Exception(sErrMsg);
		}
	}else
	{
		throw opencctv::Exception("AnalyticInstanceManager::startAnalyticInstance - Unable to communicate with analytic server.");
	}
	return bRet;
}

bool AnalyticInstanceManager::stopAnalyticInstance(const unsigned int iAnalyticInstanceId, std::string& sMessage)
{
	bool bDone = false;

	if(_pSocket)
	{
		std::string sRequest, sReply;
		try
		{
			sRequest = xml::AnalyticMessage::getAnalyticStopRequest(iAnalyticInstanceId);
			if(!opencctv::mq::MqUtil::writeToSocket(_pSocket, sRequest))
			{
				throw opencctv::Exception("Failed to send Analytic Instance Stop request to the Analytic Server.");
			}

			if(!opencctv::mq::MqUtil::readFromSocket(_pSocket, sReply))
			{
				throw opencctv::Exception("Failed to read Analytic Instance Stop reply from the Analytic Server.");
			}

			if (!sReply.empty() && sReply.compare("Error") != 0)
			{
				//std::string sMessage;
				analytic::xml::AnalyticMessage::extractStopAnalyticProcessesReplyData(sReply, bDone, sMessage);
			} else
			{
				throw opencctv::Exception("Error in extracting Analytic Stop reply data.");
			}

		} catch (opencctv::Exception &e) {
			std::string sErrMsg = "AnalyticInstanceManager::stopAnalyticInstance - ";
			sErrMsg.append(e.what());
			throw opencctv::Exception(sErrMsg);
		}

	}else
	{
		throw opencctv::Exception("AnalyticInstanceManager::stopAnalyticInstance : Unable to communicate with analytic server.");
	}

	return bDone;
}

bool AnalyticInstanceManager::killAllAnalyticInstances(std::string& sMessage)
{
	bool bDone = false;

	if(_pSocket)
	{
		std::string sRequest, sReply;
		try
		{
			sRequest = xml::AnalyticMessage::getKillAllAnalyticProcessesRequest();

			if(!opencctv::mq::MqUtil::writeToSocket(_pSocket, sRequest))
			{
				throw opencctv::Exception("Failed to send Kill All Analytics request.");
			}

			if(!opencctv::mq::MqUtil::readFromSocket(_pSocket, sReply))
			{
				throw opencctv::Exception("Failed to receive Kill All Analytics reply.");
			}

			if (!sReply.empty() && sReply.compare("Error") != 0)
			{
				std::string sMessage;
				xml::AnalyticMessage::extractKillAllAnalyticProcessesReplyData(sReply, bDone, sMessage);
			} else
			{
				throw opencctv::Exception("Error in extracting Kill All Analytics reply data.");
			}
		} catch (opencctv::Exception &e) {
			std::string sErrMsg = "AnalyticInstanceManager::killAllAnalyticInstances - ";
			sErrMsg.append(e.what());
			throw opencctv::Exception(sErrMsg);
		}

	}else
	{
		throw opencctv::Exception("AnalyticInstanceManager::killAllAnalyticInstances : Unable to communicate with analytic server.");
	}

	return bDone;
}

AnalyticInstanceManager::~AnalyticInstanceManager() {
	_pSocket->close();
	delete _pSocket; _pSocket = NULL;
}

} /* namespace analytic */
