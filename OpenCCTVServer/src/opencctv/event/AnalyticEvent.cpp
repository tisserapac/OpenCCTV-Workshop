/*
 * AnalyticEvent.cpp
 *
 *  Created on: Jul 1, 2015
 *      Author: anjana
 */

#include "AnalyticEvent.hpp"

namespace opencctv {
namespace event {

AnalyticEvent::AnalyticEvent()
{
}

std::string AnalyticEvent::analyticStart(const std::string& sRequest)
{
	std::string sReply;
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();

	//If the server is not running send an error reply
	std::string sServerStatus = pModel->getServerStatus();
	if(sServerStatus.compare(opencctv::event::SERVER_STATUS_STOPPED) == 0)
	{
		sReply = EventMessage::getInvalidMessageReply("Invalid operation request. OpenCCTV server is currently not running");
		return sReply;
	}

	unsigned int iAnalyticInstanceId = 0;
	std::string sAnalyticPluginDir;
	std::vector<std::pair<unsigned int,std::string> > vInputStreams;
	std::string sErrorMessages;
	opencctv::util::Config* pConfig = opencctv::util::Config::getInstance();

	//Extract the details from the XML request
	try
	{
		EventMessage::extractAnalyticStartRequest(sRequest, iAnalyticInstanceId, sAnalyticPluginDir, vInputStreams);
	}catch(opencctv::Exception& e)
	{
		sErrorMessages = "Error occurred in starting the analytic instance - ";
		sErrorMessages.append(e.what());
		sReply = EventMessage::getInvalidMessageReply(sErrorMessages);
		return sReply;
	}


	if(iAnalyticInstanceId == 0 || sAnalyticPluginDir.empty() || vInputStreams.empty())
	{
		sReply = EventMessage::getInvalidMessageReply("Failed to retrieve analytic instance details");
		return sReply;
	}

	//Start the analytic instance at the analytic server
	bool bAIStarted = false;
	try
	{
		bAIStarted = EventUtil::startAnalyticInstance(iAnalyticInstanceId, sAnalyticPluginDir);
		if (!bAIStarted)
		{
			throw opencctv::Exception("");
		}
	}catch(opencctv::Exception& e)
	{
		sReply = EventMessage::getInvalidMessageReply("Error occurred in starting the analytic instance on the analytic server");
		return sReply;
	}

	//Initialize a flow controller for this analytic instance and add it to ApplicationModel
	size_t remoteQueueSize = boost::lexical_cast<size_t>(pConfig->get(opencctv::util::PROPERTY_REMOTE_QUEUE_SIZE));
	opencctv::util::flow::FlowController* pFlowController = new opencctv::util::flow::SimpleFlowController(remoteQueueSize);
	pModel->getFlowControllers()[iAnalyticInstanceId] = pFlowController;

	std::stringstream ssMsg;
	ssMsg << "Analytic Instance " << iAnalyticInstanceId << " started on the analytic server.";
	opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	ssMsg.str("");

	unsigned int iStreamId = 0;
	std::string sInputName;
	std::string sAnalyticQueueInAddress;
	opencctv::MulticastDestination* pMulticastDestination = NULL;
	opencctv::db::StreamGateway streamGateway;
	std::vector<unsigned int> vStreamIds;
	bool bResult = true;

	//Processing each input stream of the analytic
	for(unsigned int i=0; i<vInputStreams.size(); ++i)
	{
		iStreamId = vInputStreams.at(i).first;
		sInputName = vInputStreams.at(i).second;

		vStreamIds.push_back(iStreamId);

		//If the consumer thread already created for the input stream pause it
		if(pModel->containsConsumerThread(iStreamId))
		{
			ssMsg << "Pausing the consumer thread " << iStreamId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");
			EventUtil::stopThread(opencctv::event::CONSUMER_THREAD, iStreamId);
		}

		//If the consumer has a multicaster use it; otherwise create a new multicater and add it to ApplicationModel
		ssMsg << "Preparing the image multicaster for consumer " << iStreamId;
		opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
		ssMsg.str("");
		if(pModel->containsMulticastDestination(iStreamId))
		{
			pMulticastDestination = pModel->getMulticastDestinations()[iStreamId];
		}else
		{
			pMulticastDestination = new opencctv::MulticastDestination();
			pModel->getMulticastDestinations()[iStreamId] = pMulticastDestination;
		}

		//Add this analytic instance to the multicaster
		try
		{
			sAnalyticQueueInAddress = pModel->getImageInputQueueAddresses()[iAnalyticInstanceId];
			pMulticastDestination->addDestination(iAnalyticInstanceId, sInputName,sAnalyticQueueInAddress);
			//bResult = true;
			ssMsg << "Added the analytic instance " << iAnalyticInstanceId << " to the multicaster " << iStreamId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");

		}catch(opencctv::Exception& e)
		{
			bResult = false;
			ssMsg << "Failed to add " << iAnalyticInstanceId << "to the multicaster " << iStreamId << " ";
			ssMsg << e.what();
			opencctv::util::log::Loggers::getDefaultLogger()->error(ssMsg.str());
			break;
		}

		//Create a new ConcurrentQueue if it does not exist
		if(!pModel->containsInternalQueue(iStreamId))
		{
			ssMsg << "Creating the concurrent queue for the stream " << iStreamId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");
			opencctv::ConcurrentQueue<opencctv::Image>* pQueue = NULL;
			size_t internalQueueSize = boost::lexical_cast<size_t>(pConfig->get(opencctv::util::PROPERTY_INTERNAL_QUEUE_SIZE));
			pQueue = new opencctv::ConcurrentQueue<opencctv::Image>(internalQueueSize);
			pModel->getInternalQueues()[iStreamId] = pQueue;
		}

		//Load the Vms connector if it does not exist
		opencctv::dto::Stream stream;
		if(!pModel->containsVmsConnector(iStreamId))
		{
			try
			{
				streamGateway.findStream(iStreamId, stream);
				if(stream.getId() == 0)//Stream details are not properly retrieved
				{
					throw opencctv::Exception("");
				}
			}catch(opencctv::Exception& e)
			{
				bResult = false;
				ssMsg << "Failed to retrieve details of the stream " << iStreamId << " " << e.what();
				opencctv::util::log::Loggers::getDefaultLogger()->error(ssMsg.str());
				break;
			}

			if(!EventUtil::loadVmsConnector(stream))
			{
				bResult = false;
				ssMsg << "Failed to load the VMS connector for the stream  " << iStreamId;
				opencctv::util::log::Loggers::getDefaultLogger()->error(ssMsg.str());
				break;
			}

		}

		//Start the results router thread if it does not exist
		bool bStartThreadResult = false;
		if(!pModel->containsResultsRouterThread(iAnalyticInstanceId))
		{
			/*ssMsg << "Starting the results router thread for the analytic instance " << iAnalyticInstanceId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");*/

			if(pModel->containsResultsOutputQueueAddress(iAnalyticInstanceId))
			{
				bStartThreadResult = EventUtil::startThread(opencctv::event::RESULTS_ROUTER_THREAD, iAnalyticInstanceId);
				if(bStartThreadResult)
				{
					ssMsg << "Results router thread for the analytic instance " << iAnalyticInstanceId << " started";
					opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
					ssMsg.str("");
				}
			}

		}

		//Start the consumer thread
		/*ssMsg << "Starting the consumer thread " << iStreamId;
		opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
		ssMsg.str("");*/
		if(bStartThreadResult)
		{
			bStartThreadResult = EventUtil::startThread(opencctv::event::CONSUMER_THREAD, iStreamId);
			if(bStartThreadResult)
			{
				ssMsg << "Consumer thread for the stream " << iStreamId << " started";
				opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				ssMsg.str("");
			}
		}

		//start the producer thread if it does not exist
		if(bStartThreadResult && !pModel->containsProducerThread(iStreamId))
		{
			/*ssMsg << "Starting the producer thread " << iStreamId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");*/
			bStartThreadResult = EventUtil::startThread(opencctv::event::PRODUCER_THREAD, iStreamId);
			if(bStartThreadResult)
			{
				ssMsg << "Producer thread for the stream " << iStreamId << " started";
				opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				ssMsg.str("");
			}
		}

		if(!bStartThreadResult)
		{
			bResult = false;
			break;
		}

	}//End For Loop

	if(bResult)
	{
		ssMsg << "Successfully started the analytic " << iAnalyticInstanceId;
		opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
		ssMsg.str("");
		sReply = EventMessage::getAnalyticStartReply(iAnalyticInstanceId);
	}else //If Error remove the analytic instance and undo the streams
	{
		analyticStop(iAnalyticInstanceId, vStreamIds);
		std::stringstream ss;
		ss << "Failed to start the analytic " << iAnalyticInstanceId;
		opencctv::util::log::Loggers::getDefaultLogger()->error(ss.str());
		sReply =  EventMessage::getInvalidMessageReply(ss.str());
	}

	return sReply;
}

std::string AnalyticEvent::analyticStop(const std::string& sRequest)
{
	std::string sReply;
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();

	//If the server is not running send an error reply
	std::string sServerStatus = pModel->getServerStatus();
	if(sServerStatus.compare(opencctv::event::SERVER_STATUS_STOPPED) == 0)
	{
		sReply = EventMessage::getInvalidMessageReply("Invalid operation request. OpenCCTV server is currently not running");
		return sReply;
	}

	unsigned int iAnalyticId = 0;
	std::vector<unsigned int> vStreamIds;
	std::string sErrorMessages;
	std::stringstream ssMsg;

	try
	{
		EventMessage::extractAnalyticStopRequest(sRequest, iAnalyticId, vStreamIds);
		sReply = analyticStop(iAnalyticId,  vStreamIds);

	}catch(opencctv::Exception &e)
	{
		sErrorMessages = "Error occurred in stopping the analytic instance : ";
		sErrorMessages.append(e.what());
		sReply = EventMessage::getInvalidMessageReply(sErrorMessages);
	}

	return sReply;
}

std::string AnalyticEvent::analyticStop(const unsigned int iAnalyticId, std::vector<unsigned int>& vStreamIds)
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	std::string sErrorMessages;
	std::stringstream ssMsg;
	std::string sReply;

	try
	{
		if(!iAnalyticId == 0)
		{
			unsigned int iStreamId;
			ConcurrentQueue<Image>* pConcurrentQueue = NULL;
			util::flow::FlowController* pFlowController = NULL;

			if(!vStreamIds.empty())
			{
				for(unsigned int i=0; i<vStreamIds.size(); ++i)
				{
					iStreamId = vStreamIds.at(i);
					std::cout<< "i = " << i << std::endl;
					std::cout<< "iStreamId = " << iStreamId << std::endl;

					if(pModel->containsMulticastDestination(iStreamId))
					{

						ssMsg << "Pausing the consumer thread " << iStreamId;
						opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
						ssMsg.str("");
						EventUtil::stopThread(opencctv::event::CONSUMER_THREAD, iStreamId);

						//Remove this analytic from the multicast destinations of the stream
						ssMsg << "Removing analytic instance " << iAnalyticId << " from the destinations of stream " << iStreamId;
						opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
						ssMsg.str("");
						MulticastDestination* destinations = pModel->getMulticastDestinations()[iStreamId];
						destinations->removeDestination(iAnalyticId);

						//If no of Consumer’s multicast destinations = 0
						if(destinations->getNumberOfDestinations() == 0)
						{
							//Remove the MulticastDestinations from the ApplicationModel
							delete destinations; destinations = NULL;
							pModel->getMulticastDestinations().erase(iStreamId);

							//Remove the producer thread
							ssMsg << "Removing the producer thread " << iStreamId;
							opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
							ssMsg.str("");
							EventUtil::stopThread(opencctv::event::PRODUCER_THREAD, iStreamId);

							//Remove the VMS connector instance used by the producer thread
							ssMsg << "Removing VMS plugin used by the producer thread " << iStreamId;
							opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
							ssMsg.str("");
							EventUtil::deleteVmsConnector(iStreamId);

							//Remove the concurrent queue
							ssMsg << "Removing the concurrent queue " << iStreamId;
							opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
							ssMsg.str("");
							if(pModel->containsInternalQueue(iStreamId))
							{
								pConcurrentQueue = pModel->getInternalQueues()[iStreamId];
								delete pConcurrentQueue; pConcurrentQueue = NULL;
								pModel->getInternalQueues().erase(iStreamId);
							}

						}else
						{
							//Restart the consumer thread
							ssMsg << "Resuming the consumer thread " << iStreamId;
							opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
							ssMsg.str("");
							EventUtil::startThread(opencctv::event::CONSUMER_THREAD,iStreamId);
						}
					}
				}

			}

			//Remove the results router thread
			ssMsg << "Removing the results router thread " << iAnalyticId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");
			EventUtil::stopThread(opencctv::event::RESULTS_ROUTER_THREAD,iAnalyticId);

			//Stop the analytic instance on the analytic server
			if(!EventUtil::stopAnalyticInstance(iAnalyticId))
			{
				ssMsg << "Error occurred in stopping the analytic instance " << iAnalyticId << "on Analytic Server";
				sErrorMessages = ssMsg.str();
				ssMsg.str("");
			}

			//Remove the flow controller of the analytic instance
			ssMsg << "Removing the flow controller of the analytic instance " << iAnalyticId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");
			if(pModel->containsFlowController(iAnalyticId))
			{
				pFlowController = pModel->getFlowControllers()[iAnalyticId];
				delete pFlowController; pFlowController = NULL;
				pModel->getFlowControllers().erase(iAnalyticId);
			}

			if(sErrorMessages.empty())
			{
				sReply = EventMessage::getAnalyticStopReply(iAnalyticId);
			}else
			{
				sReply = EventMessage::getInvalidMessageReply(sErrorMessages);
			}

		}else
		{
			sErrorMessages = "Error occurred in stopping the analytic instance : Invalid analytic instance details";
			sReply = EventMessage::getInvalidMessageReply(sErrorMessages);
		}

	}catch(opencctv::Exception &e)
	{
		sErrorMessages = "Error occurred in stopping the analytic instance : ";
		sErrorMessages.append(e.what());
		sReply = EventMessage::getInvalidMessageReply(sErrorMessages);
	}

	return sReply;
}

AnalyticEvent::~AnalyticEvent()
{
}

} /* namespace event */
} /* namespace opencctv */
