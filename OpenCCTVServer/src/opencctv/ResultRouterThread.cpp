
#include "ResultRouterThread.hpp"

namespace opencctv {

ResultRouterThread::ResultRouterThread(unsigned int iAnalyticInstanceId) {
	ApplicationModel* pModel = ApplicationModel::getInstance();
	_pFlowController = NULL;
	if(pModel->containsFlowController(iAnalyticInstanceId))
	{
		_pFlowController = pModel->getFlowControllers()[iAnalyticInstanceId];
	}
	_pSerializer = util::serialization::Serializers::getInstanceOfDefaultSerializer();
	_iAnalyticInstanceId = iAnalyticInstanceId;
}

void ResultRouterThread::operator()()
{
	util::log::Loggers::getDefaultLogger()->info("Results Router started....");

	util::Config* pConfig = util::Config::getInstance();
	ApplicationModel* pModel = ApplicationModel::getInstance();
	std::stringstream ssMsg;

	if(pModel->containsResultsOutputQueueAddress(_iAnalyticInstanceId))
	{
		//Initialize the ZMQ connection to the analytic instance's output queue
		bool bConnected = false;
		mq::TcpMqReceiver receiver;
		try
		{
			receiver.connectToMq(pConfig->get(util::PROPERTY_ANALYTIC_SERVER_IP), pModel->getResultsOutputQueueAddresses()[_iAnalyticInstanceId]);
			bConnected = true;
		}
		catch(Exception &e)
		{
			std::string sErrMsg = "Failed to connect to Analytic Output Queue. ";
			sErrMsg.append(e.what());
			util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		}

		//Create the AnalyticResultGateway to the DB
		opencctv::db::AnalyticResultGateway* _pAnalyticResultGateway = NULL;
		try
		{
			_pAnalyticResultGateway = new opencctv::db::AnalyticResultGateway();

		}catch(opencctv::Exception &e)
		{
			util::log::Loggers::getDefaultLogger()->error(e.what());
		}

		//Start inserting the analytic instance's results to the results DB
		while(bConnected && _pFlowController && _pAnalyticResultGateway)
		{

			//Define the interrupt point of the results router threads
			try
			{
				boost::this_thread::interruption_point();
			}
			catch(const boost::thread_interrupted&)
			{
				// Thread interruption request received, break the loop
				ssMsg <<  "Results router thread of analytic instance : " << _iAnalyticInstanceId << " interrupted";
				opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				break;
			}


			std::string* pSSerializedResult;
			try
			{
				pSSerializedResult = receiver.receive();
			}catch(opencctv::Exception &e)
			{
				continue;
			}

			analytic::AnalyticResult result = _pSerializer->deserializeAnalyticResult(*pSSerializedResult);
			std::string sMsg = "\t\tReceived Result of ";
			sMsg.append(result.getTimestamp());
			util::log::Loggers::getDefaultLogger()->debug(sMsg);

			//Saving to DB
			if(result.getWriteToDatabase())
			{
				try
				{
					opencctv::db::AnalyticResultGateway analyticResultGateway;
					analyticResultGateway.insertResults(_iAnalyticInstanceId, result);
					//sMsg = "\t\t\tResult written to the database";
					util::log::Loggers::getDefaultLogger()->debug(sMsg);

				}catch(opencctv::Exception &e)
				{
					std::ostringstream sErrorMessage;
					sErrorMessage << "Failed to write results to the results database : analytic id - ";
					sErrorMessage << _iAnalyticInstanceId;
					sMsg = sErrorMessage.str();
					util::log::Loggers::getDefaultLogger()->error(sMsg);
				}
			}

			_pFlowController->received();
			if(pSSerializedResult) delete pSSerializedResult;

			//Define the interrupt point of the results router threads
			try
			{
				boost::this_thread::interruption_point();
			}
			catch(const boost::thread_interrupted&)
			{
				// Thread interruption request received, break the loop
				ssMsg <<  "Results router thread of analytic instance : " << _iAnalyticInstanceId << " interrupted";
				opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				break;
			}
		}
	}
	delete _pSerializer; _pSerializer = NULL;
	ssMsg.clear();
	ssMsg <<  "Results router thread of analytic instance : " << _iAnalyticInstanceId << " Stopped";
	opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
}

} /* namespace opencctv */
