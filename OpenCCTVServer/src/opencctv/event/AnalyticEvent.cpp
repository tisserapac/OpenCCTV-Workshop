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
				std::cout<< "i = " << i << std::endl;
				iStreamId = vStreamIds.at(i);
				std::cout<< "iStreamId = " << iStreamId << std::endl;

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
					std::cout << "Paused the consumer thread " << iStreamId << std::endl;

					//Remove this analytic from the multicast destinations of the stream
					destinations->removeDestination(iAnalyticId);
					std::cout << "Removed analytic instance " << iAnalyticId << " from the destinations of stream " << iStreamId << std::endl;

					//If no of Consumer’s multicast destinations = 0
					if(destinations->getNumberOfDestinations() == 0)
					{
						//Remove the MulticastDestinations from the ApplicationModel
						delete destinations; destinations = NULL;
						pModel->getMulticastDestinations().erase(iStreamId);

						//Remove the producer thread - unload the vms connector plugin
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
						std::cout << "Removed the producer thread " << iStreamId << std::endl;

						//Remove the concurrent queue
						if(pModel->containsInternalQueue(iStreamId))
						{
							pConcurrentQueue = pModel->getInternalQueues()[iStreamId];
							delete pConcurrentQueue; pConcurrentQueue = NULL;
							pModel->getInternalQueues().erase(iStreamId);
						}
						std::cout << "Removed the concurrent queue " << iStreamId << std::endl;
					}else
					{
						//Restart the consumer thread
						opencctv::ConsumerThread* pConsumer = new opencctv::ConsumerThread(iStreamId);
						boost::thread* pConsumerThread = new boost::thread(*pConsumer);
						pModel->getConsumerThreads()[iStreamId] = pConsumerThread;
						pModel->getConsumerThreadGroup()->add_thread(pConsumerThread);

						delete pConsumer;

						std::cout << "Resumed the consumer thread" << iStreamId << std::endl;
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
			std::cout << "Removed the results router thread " << iAnalyticId << std::endl;

			//Stop the analytic instance on the analytic server
			analytic::AnalyticInstanceManager* pAnalyticInstanceManager = pModel->getAnalyticInstanceManagers()[1];
			pAnalyticInstanceManager->stopAnalyticInstance(iAnalyticId, sErrorMessages);

			std::cout << "Results of stopping the analytic on the analytic server : " << sErrorMessages << std::endl;

			//Remove reference to Remote Input Queue
			if(pModel->containsImageInputQueueAddress(iAnalyticId))
			{
				pModel->getImageInputQueueAddresses().erase(iAnalyticId);
			}
			std::cout << "Removed reference to Remote Input Queue " << iAnalyticId << std::endl;

			//Remove reference to Remote Output Queue
			if(pModel->containsResultsOutputQueueAddress(iAnalyticId))
			{
				pModel->getResultsOutputQueueAddresses().erase(iAnalyticId);
			}
			std::cout << "Removed reference to Remote Output Queue " << iAnalyticId << std::endl;

			//Remove the flow controller of the analytic instance
			if(pModel->containsFlowController(iAnalyticId))
			{
				pFlowController = pModel->getFlowControllers()[iAnalyticId];
				delete pFlowController; pFlowController = NULL;
				pModel->getFlowControllers().erase(iAnalyticId);
			}
			std::cout << "Removed the flow controller of the analytic instance " << iAnalyticId << std::endl;

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
