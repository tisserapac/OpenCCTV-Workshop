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

	unsigned int iAnalyticId = 0;
	std::vector<unsigned int> vStreamIds;
	std::string sErrorMessages;
	opencctv::util::Config* pConfig = opencctv::util::Config::getInstance();

	//Start the analytic instance at the analytic server
	if(!pModel->containsAnalyticInstanceManager(1))
	{
		sReply = EventMessage::getInvalidMessageReply("Error occurred in starting the analytic instance : Unable to find analytic server details");
		return sReply;
	}

	analytic::AnalyticInstanceManager* pAnalyticInstanceManager = pModel->getAnalyticInstanceManagers()[1];

	/*bAIStarted = pAnalyticInstanceManager->startAnalyticInstance(
								analyticInstance.getAnalyticInstanceId(),
								analyticInstance.getAnalyticDirLocation(),
								analyticInstance.getAnalyticFilename(), sAnalyticQueueInAddress, sAnalyticQueueOutAddress);*/


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

		if(!(iAnalyticId == 0 || vStreamIds.empty()))
		{
			unsigned int iStreamId;
			boost::thread* pThread = NULL;
			ConcurrentQueue<Image>* pConcurrentQueue = NULL;
			util::flow::FlowController* pFlowController = NULL;

			for(unsigned int i=0; i<vStreamIds.size(); ++i)
			{
				iStreamId = vStreamIds.at(i);
				/*std::cout<< "i = " << i << std::endl;
				std::cout<< "iStreamId = " << iStreamId << std::endl;*/

				if(pModel->containsMulticastDestination(iStreamId))
				{
					MulticastDestination* destinations = pModel->getMulticastDestinations()[iStreamId];

					//Stop the consumer thread of the stream
					if(pModel->containsConsumerThread(iStreamId))
					{
						pThread = pModel->getConsumerThreads()[iStreamId];
						while (pThread->timed_join(boost::posix_time::seconds(1))==false)
						{
							pThread->interrupt();
						}

						if(pModel->getConsumerThreadGroup())
						{
							pModel->getConsumerThreadGroup()->remove_thread(pThread);
						}

						delete pThread; pThread = NULL;
						pModel->getConsumerThreads().erase(iStreamId);
					}
					ssMsg << "Paused the consumer thread " << iStreamId;
					opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
					ssMsg.str("");

					//Remove this analytic from the multicast destinations of the stream
					destinations->removeDestination(iAnalyticId);
					ssMsg << "Removed analytic instance " << iAnalyticId << " from the destinations of stream " << iStreamId;
					opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
					ssMsg.str("");

					//If no of Consumer’s multicast destinations = 0
					if(destinations->getNumberOfDestinations() == 0)
					{
						//Remove the MulticastDestinations from the ApplicationModel
						delete destinations; destinations = NULL;
						pModel->getMulticastDestinations().erase(iStreamId);

						//Remove the producer thread
						if(pModel->containsProducerThread(iStreamId))
						{
							pThread = pModel->getProducerThreads()[iStreamId];
							while (pThread->timed_join(boost::posix_time::millisec(100))==false)
							{
								pThread->interrupt();
							}

							if(pModel->getProducerThreadGroup())
							{
								pModel->getProducerThreadGroup()->remove_thread(pThread);
							}

							delete pThread; pThread = NULL;
							pModel->getProducerThreads().erase(iStreamId);
						}
						ssMsg << "Removed the producer thread " << iStreamId;
						opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
						ssMsg.str("");

						//Remove the VMS connector instance used by the producer thread
						opencctv::api::VmsConnector* pVmsConnector = NULL;
						unsigned int iVmsTypeId;
						PluginLoader<api::VmsConnector>* pPluginLoader = NULL;
						if(pModel->containsVmsConnector(iStreamId))
						{
							iVmsTypeId = pModel->getVmsConnectors()[iStreamId].first;
							pVmsConnector = pModel->getVmsConnectors()[iStreamId].second;

							if(pModel->containsVmsPluginLoader(iVmsTypeId))
							{
								pPluginLoader = pModel->getVmsPluginLoaders()[iVmsTypeId];
								try
								{
									pPluginLoader->deletePluginInstance(pVmsConnector);
									iVmsTypeId = pModel->getVmsConnectors().erase(iStreamId);
								}catch(opencctv::Exception& e)
								{
									std::string sErrorMsg = "ApplicationModel::clear - Error in deleting the VMS connector plugin - ";
									sErrorMsg.append(e.what());
									opencctv::util::log::Loggers::getDefaultLogger()->error(sErrorMsg);
								}
							}
						}
						ssMsg << "Removed VMS plugin used by the producer thread " << iStreamId;
						opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
						ssMsg.str("");

						//Remove the concurrent queue
						if(pModel->containsInternalQueue(iStreamId))
						{
							pConcurrentQueue = pModel->getInternalQueues()[iStreamId];
							delete pConcurrentQueue; pConcurrentQueue = NULL;
							pModel->getInternalQueues().erase(iStreamId);
						}
						ssMsg << "Removed the concurrent queue " << iStreamId;
						opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
						ssMsg.str("");
					}else
					{
						//Restart the consumer thread
						opencctv::ConsumerThread* pConsumer = new opencctv::ConsumerThread(iStreamId);
						boost::thread* pConsumerThread = new boost::thread(*pConsumer);
						pModel->getConsumerThreads()[iStreamId] = pConsumerThread;
						pModel->getConsumerThreadGroup()->add_thread(pConsumerThread);

						delete pConsumer;

						ssMsg << "Resumed the consumer thread " << iStreamId;
						opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
						ssMsg.str("");
					}
				}
			}

			//Remove the results router thread
			if(pModel->containsResultsRouterThread(iAnalyticId))
			{
				pThread = pModel->getResultsRouterThreads()[iAnalyticId];
				while (pThread->timed_join(boost::posix_time::millisec(100))==false)
				{
					pThread->interrupt();
				}

				if(pModel->getResultsRouterThreadGroup())
				{
					pModel->getResultsRouterThreadGroup()->remove_thread(pThread);
				}

				delete pThread; pThread = NULL;
				pModel->getResultsRouterThreads().erase(iAnalyticId);
			}
			ssMsg << "Removed the results router thread " << iAnalyticId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");

			//Stop the analytic instance on the analytic server
			analytic::AnalyticInstanceManager* pAnalyticInstanceManager = pModel->getAnalyticInstanceManagers()[1];
			pAnalyticInstanceManager->stopAnalyticInstance(iAnalyticId, sErrorMessages);

			ssMsg << "Results of stopping the analytic on the analytic server : " << sErrorMessages ;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");

			//Remove reference to Remote Input Queue
			if(pModel->containsImageInputQueueAddress(iAnalyticId))
			{
				pModel->getImageInputQueueAddresses().erase(iAnalyticId);
			}
			ssMsg << "Removed reference to Remote Input Queue " << iAnalyticId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");

			//Remove reference to Remote Output Queue
			if(pModel->containsResultsOutputQueueAddress(iAnalyticId))
			{
				pModel->getResultsOutputQueueAddresses().erase(iAnalyticId);
			}
			ssMsg << "Removed reference to Remote Output Queue " << iAnalyticId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");

			//Remove the flow controller of the analytic instance
			if(pModel->containsFlowController(iAnalyticId))
			{
				pFlowController = pModel->getFlowControllers()[iAnalyticId];
				delete pFlowController; pFlowController = NULL;
				pModel->getFlowControllers().erase(iAnalyticId);
			}
			ssMsg << "Removed the flow controller of the analytic instance " << iAnalyticId;
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			ssMsg.str("");

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
