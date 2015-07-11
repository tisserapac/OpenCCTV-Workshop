/*
 * EventUtil.cpp
 *
 *  Created on: Jul 10, 2015
 *      Author: anjana
 */

#include "EventUtil.hpp"

namespace opencctv {
namespace event {

EventUtil::EventUtil()
{
}

bool EventUtil::loadVmsConnector(const opencctv::dto::Stream& stream)
{
	opencctv::util::Config* pConfig = opencctv::util::Config::getInstance();
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();

	std::string sVmsPluginDirPath = pConfig->get(opencctv::util::PROPERTY_VMS_CONNECTOR_DIR);
	if(*sVmsPluginDirPath.rbegin() != '/') // check last char
	{
		sVmsPluginDirPath.append("/");
	}
	sVmsPluginDirPath.append(stream.getVmsConnectorDirLocation());

	// Loading VMS Connector Plugin
	opencctv::api::VmsConnector* pVmsConnector = NULL;
	opencctv::PluginLoader<opencctv::api::VmsConnector>* pVmsPluginLoader = NULL;

	// if VMS Plugin Loader has been constructed, use it.
	if (pModel->containsVmsPluginLoader(stream.getVmsTypeId()))
	{
		pVmsPluginLoader = pModel->getVmsPluginLoaders()[stream.getVmsTypeId()];

	}else	// if VMS Plugin Loader has not been constructed, construct it.
	{
		try
		{
			pVmsPluginLoader = new opencctv::PluginLoader<opencctv::api::VmsConnector>();

			std::string sVmsPluginPath;
			opencctv::util::Util::findSharedLibOfPlugin(sVmsPluginDirPath, sVmsPluginPath);
			//std::cout << "sVmsPluginPath : " << sVmsPluginPath << std::endl;
			pVmsPluginLoader->loadPlugin(sVmsPluginPath);
			pModel->getVmsPluginLoaders()[stream.getVmsTypeId()] = pVmsPluginLoader;
		} catch (opencctv::Exception &e)
		{
			std::string sErrMsg = "Failed to load VMS Connector Plugin using VMS Connector Plugin Loader. ";
			sErrMsg.append(e.what());
			opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			return false;
		}
	}

	if(!pVmsPluginLoader)
	{
		return false;
	}

	// create VMS Connector using the VMS Plugin Loader
	try
	{
		pVmsConnector = pVmsPluginLoader->createPluginInstance();
		std::stringstream ssMsg;
		ssMsg << "VMS Connector plugin for Stream " << stream.getId();
		ssMsg << " constructed.";
		opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	} catch (opencctv::Exception &e)
	{
		std::string sErrMsg = "Failed to create VMS Connector plugin. ";
		sErrMsg.append(e.what());
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		return false;
	}

	if(!pVmsConnector)
	{
		return false;
	}

	// init VMS Connector
	opencctv::api::VmsConnectInfo connInfo = { stream.getVmsServerIp(), stream.getVmsServerPort(), stream.getVmsUsername(),
			stream.getVmsPassword(), stream.getCameraId(), stream.getWidth(), stream.getHeight(),
			stream.getKeepAspectRatio(), stream.getAllowUpSizing(), stream.getCompressionRate() };

	bool bVmsConnInitDone = false;

	try {
		bVmsConnInitDone = pVmsConnector->init(connInfo, sVmsPluginDirPath);
		if(bVmsConnInitDone)
		{
			//Add the VMS connector to the ApplicationModel
			std::pair <unsigned int,opencctv::api::VmsConnector*> vmsConn (stream.getVmsTypeId(),pVmsConnector);
			pModel->getVmsConnectors()[stream.getId()] = vmsConn;

		}else
		{
			pVmsPluginLoader->deletePluginInstance(pVmsConnector);
			std::string sErrMsg = "Failed to initialize VMS Connector plugin. ";
			opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		}
	} catch (opencctv::Exception &e) {
		std::string sErrMsg = "Failed to initialize VMS Connector plugin. ";
		sErrMsg.append(e.what());
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
	}

	return bVmsConnInitDone;
}

bool EventUtil::deleteVmsConnector(const unsigned int iStreamId)
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	opencctv::api::VmsConnector* pVmsConnector = NULL;
	unsigned int iVmsTypeId;
	PluginLoader<api::VmsConnector>* pPluginLoader = NULL;

	bool bResult = true;

	if(pModel->containsVmsConnector(iStreamId))
	{
		iVmsTypeId = pModel->getVmsConnectors()[iStreamId].first;
		pVmsConnector = pModel->getVmsConnectors()[iStreamId].second;

		if (pModel->containsVmsPluginLoader(iVmsTypeId))
		{
			pPluginLoader = pModel->getVmsPluginLoaders()[iVmsTypeId];

			try {
				pPluginLoader->deletePluginInstance(pVmsConnector);
				iVmsTypeId = pModel->getVmsConnectors().erase(iStreamId);
			} catch (opencctv::Exception& e)
			{
				bResult = false;
				std::string sErrorMsg = "EventUtil::deleteVmsConnector - Error in deleting the VMS connector plugin - ";
				sErrorMsg.append(e.what());
				opencctv::util::log::Loggers::getDefaultLogger()->error(sErrorMsg);
			}
		}
	}

	return bResult;
}


bool EventUtil::startThread(const std::string& sThreadType, const unsigned int iThreadId)
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	boost::thread* pThread = NULL;

	bool bResult = false;

	if(sThreadType.compare(PRODUCER_THREAD) == 0)
	{
		opencctv::ProducerThread producer(iThreadId);
		pThread = new boost::thread(producer);
		if(pThread && pThread->joinable())
		{
			pModel->getProducerThreads()[iThreadId] = pThread;
			pModel->getProducerThreadGroup()->add_thread(pThread);
			bResult = true;
		}else
		{
			delete pThread; pThread = NULL;
		}
	}else if(sThreadType.compare(CONSUMER_THREAD) == 0)
	{
		opencctv::ConsumerThread consumer(iThreadId);
		pThread = new boost::thread(consumer);
		if(pThread && pThread->joinable())
		{
			pModel->getConsumerThreads()[iThreadId] = pThread;
			pModel->getConsumerThreadGroup()->add_thread(pThread);
			bResult = true;
		}else
		{
			delete pThread; pThread = NULL;
		}

	}else if(sThreadType.compare(RESULTS_ROUTER_THREAD) == 0)
	{
		opencctv::ResultRouterThread resultsRouter(iThreadId);
		pThread = new boost::thread(resultsRouter);
		if(pThread && pThread->joinable())
		{
			pModel->getResultsRouterThreads()[iThreadId] = pThread;
			pModel->getResultsRouterThreadGroup()->add_thread(pThread);
			bResult = true;

		}else
		{
			delete pThread; pThread = NULL;
		}
	}else
	{
		std::string sErrorMsg = "EventUtil::startThread - Invalid thread type";
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrorMsg);
		bResult = false;
	}

	if(!pThread)
	{
		bResult = false;
		std::stringstream ss;
		ss << "Failed to start the " << sThreadType << " " << iThreadId;
		opencctv::util::log::Loggers::getDefaultLogger()->error(ss.str());
	}

	return bResult;
}

bool EventUtil::stopThread(const std::string& sThreadType, const unsigned int iThreadId)
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	boost::thread* pThread = NULL;
	boost::thread_group* pThreadGroup = NULL;

	//std::cout << "sThreadType : " << sThreadType << " iThreadId : " << iThreadId << std::endl;

	std::map<unsigned int, boost::thread*>::iterator itThread;

	if(sThreadType.compare(PRODUCER_THREAD) == 0 )
	{
		if(pModel->containsProducerThread(iThreadId))
		{
			pThread = pModel->getProducerThreads()[iThreadId];
			pThreadGroup = pModel->getProducerThreadGroup();
		}
	}else if(sThreadType.compare(CONSUMER_THREAD) == 0 )
	{
		if(pModel->containsConsumerThread(iThreadId))
		{
			pThread = pModel->getConsumerThreads()[iThreadId];
			pThreadGroup = pModel->getConsumerThreadGroup();
		}

	}else if(sThreadType.compare(RESULTS_ROUTER_THREAD) == 0 )
	{
		if(pModel->containsResultsRouterThread(iThreadId))
		{
			pThread = pModel->getResultsRouterThreads()[iThreadId];
			pThreadGroup = pModel->getResultsRouterThreadGroup();
		}

	}else
	{
		std::string sErrorMsg = "EventUtil::stopThread - Invalid thread type";
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrorMsg);
		return false;
	}

	//TODO : Will this logic work if the callable object's operate()() function has returned?
	if(pThread && pThreadGroup)
	{
		while (pThread->try_join_for(boost::chrono::milliseconds(100)) == false)
		{
			pThread->interrupt();
		}
		pThreadGroup->remove_thread(pThread);
		delete pThread; pThread = NULL;

		if(sThreadType.compare(PRODUCER_THREAD) == 0)
		{
			pModel->getProducerThreads().erase(iThreadId);

		}else if(sThreadType.compare(CONSUMER_THREAD) == 0)
		{
			pModel->getConsumerThreads().erase(iThreadId);

		}else if(sThreadType.compare(RESULTS_ROUTER_THREAD) == 0)
		{
			pModel->getResultsRouterThreads().erase(iThreadId);
		}
	}

	return true;
}

bool EventUtil::startAnalyticInstance(const unsigned int& iAnalyticInstanceId, const std::string& sAnalyticPluginDir)
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	if(!pModel->containsAnalyticInstanceManager(1))
	{
		throw opencctv::Exception("EventUtil::startAnalyticInstance - Unable to find analytic server details");
	}

	analytic::AnalyticInstanceManager* pAnalyticInstanceManager = pModel->getAnalyticInstanceManagers()[1];
	std::string sAnalyticQueueInAddress;
	std::string sAnalyticQueueOutAddress;
	bool bAIStarted = false;

	try
	{
		bAIStarted = pAnalyticInstanceManager->startAnalyticInstance(iAnalyticInstanceId, sAnalyticPluginDir,sAnalyticQueueInAddress,sAnalyticQueueOutAddress);

	}catch(opencctv::Exception& e)
	{
		throw e;
	}

	if (bAIStarted && !sAnalyticQueueInAddress.empty() && !sAnalyticQueueOutAddress.empty())
	{
		//Store analytic input queue address and output queue address in ApplicationModel
		pModel->getImageInputQueueAddresses()[iAnalyticInstanceId] = sAnalyticQueueInAddress;
		pModel->getResultsOutputQueueAddresses()[iAnalyticInstanceId] = sAnalyticQueueOutAddress;
	}

	return bAIStarted;
}

bool EventUtil::stopAnalyticInstance(const unsigned int iAnalyticInstanceId)
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	std::string sMessages;
	std::stringstream ssMsg;

	analytic::AnalyticInstanceManager* pAnalyticInstanceManager = pModel->getAnalyticInstanceManagers()[1];
	try
	{
		pAnalyticInstanceManager->stopAnalyticInstance(iAnalyticInstanceId, sMessages);
	}catch (opencctv::Exception& e)
	{
		sMessages = "EventUtil::stopAnalyticInstance - ";
		sMessages.append(e.what());
		opencctv::util::log::Loggers::getDefaultLogger()->error(sMessages);
		return false;
	}

	ssMsg << "Results of stopping the analytic on the analytic server : " << sMessages ;
	opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	ssMsg.str("");

	//Remove reference to Remote Input Queue
	if(pModel->containsImageInputQueueAddress(iAnalyticInstanceId))
	{
		pModel->getImageInputQueueAddresses().erase(iAnalyticInstanceId);
	}
	ssMsg << "Removed reference to Remote Input Queue " << iAnalyticInstanceId;
	opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	ssMsg.str("");

	//Remove reference to Remote Output Queue
	if(pModel->containsResultsOutputQueueAddress(iAnalyticInstanceId))
	{
		pModel->getResultsOutputQueueAddresses().erase(iAnalyticInstanceId);
	}
	ssMsg << "Removed reference to Remote Output Queue " << iAnalyticInstanceId;
	opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	ssMsg.str("");

	return true;
}

EventUtil::~EventUtil()
{
}

} /* namespace event */
} /* namespace opencctv */
