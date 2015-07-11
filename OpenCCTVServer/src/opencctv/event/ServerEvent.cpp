/*
 * ServerEvent.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#include "ServerEvent.hpp"

namespace opencctv {
namespace event {

ServerEvent::ServerEvent()
{
}

bool ServerEvent::startServer()
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();

	//If the server is already running just return TRUE
	std::string sServerStatus = pModel->getServerStatus();
	if(sServerStatus.compare(opencctv::event::SERVER_STATUS_STARTED) == 0)
	{
		return true;
	}

	// Initializing variables
	//test::gateway::TestStreamGateway* pStreamGateway;
	opencctv::db::StreamGateway* pStreamGateway = NULL;
	try
	{
		pStreamGateway = new opencctv::db::StreamGateway();
		//pStreamGateway = new test::gateway::TestStreamGateway();
	}
	catch(opencctv::Exception &e)
	{
		opencctv::util::log::Loggers::getDefaultLogger()->error(e.what());
		return false;
	}

	//test::gateway::TestAnalyticInstanceStreamGateway* pAnalyticInstanceGateway;
	opencctv::db::AnalyticInstanceStreamGateway* pAnalyticInstanceGateway = NULL;
	try
	{
		pAnalyticInstanceGateway = new opencctv::db::AnalyticInstanceStreamGateway();
		//pAnalyticInstanceGateway = new test::gateway::TestAnalyticInstanceStreamGateway();
	}
	catch(opencctv::Exception &e)
	{
		opencctv::util::log::Loggers::getDefaultLogger()->error(e.what());
		return false;
	}

	opencctv::util::Config* pConfig = opencctv::util::Config::getInstance();

	//Create and add the thread groups to the ApplicationModel
	boost::thread_group* _pPproducerThreadGroup = new boost::thread_group();
	boost::thread_group* _pConsumerThreadGroup = new boost::thread_group();
	boost::thread_group* _pResultsRouterThreadGroup = new boost::thread_group();
	pModel->setProducerThreadGroup(_pPproducerThreadGroup);
	pModel->setConsumerThreadGroup(_pConsumerThreadGroup);
	pModel->setResultsRouterThreadGroup(_pResultsRouterThreadGroup);

	size_t internalQueueSize = boost::lexical_cast<size_t>(pConfig->get(opencctv::util::PROPERTY_INTERNAL_QUEUE_SIZE));
	size_t remoteQueueSize = boost::lexical_cast<size_t>(pConfig->get(opencctv::util::PROPERTY_REMOTE_QUEUE_SIZE));
	analytic::AnalyticInstanceManager* pAnalyticInstanceManager = new analytic::AnalyticInstanceManager(pConfig->get(opencctv::util::PROPERTY_ANALYTIC_SERVER_IP), pConfig->get(opencctv::util::PROPERTY_ANALYTIC_SERVER_PORT));
	pModel->getAnalyticInstanceManagers()[1] = pAnalyticInstanceManager;
	opencctv::util::log::Loggers::getDefaultLogger()->info("Initializing variables done.");

	std::vector<opencctv::dto::Stream> vStreams;
	try
	{
		pStreamGateway->findAll(vStreams);
		opencctv::util::log::Loggers::getDefaultLogger()->info("Streams loaded.");
	}
	catch(opencctv::Exception &e)
	{
		std::string sErrMsg = "Failed to find all Streams. ";
		sErrMsg.append(e.what());
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		return false;
	}

	for(size_t i = 0; i < vStreams.size(); ++i)
	{
		opencctv::dto::Stream stream = vStreams[i];
		opencctv::MulticastDestination* pMulticastDestination = new opencctv::MulticastDestination();
		pModel->getMulticastDestinations()[stream.getId()] = pMulticastDestination;
		//opencctv::ImageMulticaster* pMulticaster = new opencctv::ImageMulticaster(stream.getId());
		std::vector<opencctv::dto::AnalyticInstanceStream> vAnalyticInstances;
		try
		{
			pAnalyticInstanceGateway->findAllForStream(stream.getId(), vAnalyticInstances);
			opencctv::util::log::Loggers::getDefaultLogger()->info("Analytic Instances Streams loaded.");
		}
		catch(opencctv::Exception &e)
		{
			std::string sErrMsg = "Failed to find all AnalyticInstanceStream. ";
			sErrMsg.append(e.what());
			opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			return false;
		}
		// Starting Analytic Instances
		std::string sAnalyticQueueInAddress, sAnalyticQueueOutAddress;
		for(size_t j = 0; j < vAnalyticInstances.size(); ++j)
		{
			opencctv::dto::AnalyticInstanceStream analyticInstance = vAnalyticInstances[j];
			// if the Analytic Instance has not been started yet
			if(!pModel->containsImageInputQueueAddress(analyticInstance.getAnalyticInstanceId()))
			{
				bool bAIStarted = false;
				try {
					// start Analytic Instance, store Analytic Input, Output queue addresses into the Application Model.
					bAIStarted = pAnalyticInstanceManager->startAnalyticInstance(
									analyticInstance.getAnalyticInstanceId(),
									analyticInstance.getAnalyticDirLocation(),
									sAnalyticQueueInAddress,
									sAnalyticQueueOutAddress);
				} catch (opencctv::Exception &e) {
					std::stringstream ssErrMsg;
					ssErrMsg << "Failed to start Analytic Instance ";
					ssErrMsg << analyticInstance.getAnalyticInstanceId() << ". ";
					ssErrMsg << e.what();
					opencctv::util::log::Loggers::getDefaultLogger()->error(ssErrMsg.str());
				}
				if (bAIStarted) {
					// store analytic input queue address and output queue address in model
					pModel->getImageInputQueueAddresses()[analyticInstance.getAnalyticInstanceId()] = sAnalyticQueueInAddress;
					pModel->getResultsOutputQueueAddresses()[analyticInstance.getAnalyticInstanceId()] = sAnalyticQueueOutAddress;
					// if Analytic Instance started, construct Flow Controller for Input Analytic queue and store it on Application Model.
					opencctv::util::flow::FlowController* pFlowController = new opencctv::util::flow::SimpleFlowController(remoteQueueSize);
					//opencctv::util::flow::FlowController* pFlowController = new opencctv::util::flow::BasicFlowController();
					pModel->getFlowControllers()[analyticInstance.getAnalyticInstanceId()] = pFlowController;
					std::stringstream ssMsg;
					ssMsg << "Analytic Instance " << analyticInstance.getAnalyticInstanceId();
					ssMsg << " started.";
					opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				}
				else {
					std::stringstream ssErrMsg;
					ssErrMsg << "Starting Analytic Instance " << analyticInstance.getAnalyticInstanceId();
					ssErrMsg << " failed.";
					opencctv::util::log::Loggers::getDefaultLogger()->error(ssErrMsg.str());
				}
			}
			// Adding Input Image Queue destinations to Image Multicaster
			try {
				if(pModel->containsImageInputQueueAddress(analyticInstance.getAnalyticInstanceId()))
				{
					sAnalyticQueueInAddress = pModel->getImageInputQueueAddresses()[analyticInstance.getAnalyticInstanceId()];
					pMulticastDestination->addDestination(analyticInstance.getAnalyticInstanceId(), analyticInstance.getInputName(),sAnalyticQueueInAddress);
					//pMulticaster->addDestination(analyticInstance);
				}
			} catch (opencctv::Exception &e) {
				std::string sErrMsg = "Failed to add destination to Image Multicaster. ";
				sErrMsg.append(e.what());
				opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			}
		}

		opencctv::util::log::Loggers::getDefaultLogger()->info("Starting Analytic Instances done.");

		// Creating threads and internal queues
		opencctv::ConcurrentQueue<opencctv::Image>* pQueue = NULL;
		opencctv::ConsumerThread* pConsumer = NULL;
		opencctv::ProducerThread* pProducer = NULL;
		if(pMulticastDestination->getNumberOfDestinations() > 0)
		{
			pQueue = new opencctv::ConcurrentQueue<opencctv::Image>(internalQueueSize);
			pModel->getInternalQueues()[stream.getId()] = pQueue;
			//pConsumer = new opencctv::ConsumerThread(stream.getId(), pMulticaster);
			pConsumer = new opencctv::ConsumerThread(stream.getId());

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
			if (pModel->containsVmsPluginLoader(stream.getVmsTypeId())) {
				pVmsPluginLoader = pModel->getVmsPluginLoaders()[stream.getVmsTypeId()];
			}
			// if VMS Plugin Loader has not been constructed, construct it.
			else {
				try {
					pVmsPluginLoader = new opencctv::PluginLoader<opencctv::api::VmsConnector>();

					std::string sVmsPluginPath;
					opencctv::util::Util::findSharedLibOfPlugin(sVmsPluginDirPath, sVmsPluginPath);
					//std::cout << "sVmsPluginPath : " << sVmsPluginPath << std::endl;
					pVmsPluginLoader->loadPlugin(sVmsPluginPath);
					pModel->getVmsPluginLoaders()[stream.getVmsTypeId()] = pVmsPluginLoader;
				} catch (opencctv::Exception &e) {
					std::string sErrMsg = "Failed to load VMS Connector Plugin using VMS Connector Plugin Loader. ";
					sErrMsg.append(e.what());
					opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
				}
			}
			// create VMS Connector using the VMS Plugin Loader
			if(pVmsPluginLoader)
			{
				try {
					pVmsConnector = pVmsPluginLoader->createPluginInstance();
					std::stringstream ssMsg;
					ssMsg << "VMS Connector plugin for Stream " << stream.getId();
					ssMsg << " constructed.";
					opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
				} catch (opencctv::Exception &e) {
					std::string sErrMsg = "Failed to create VMS Connector plugin. ";
					sErrMsg.append(e.what());
					opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
				}
			}
			// init VMS Connector
			opencctv::api::VmsConnectInfo connInfo = { stream.getVmsServerIp(),
					stream.getVmsServerPort(), stream.getVmsUsername(),
					stream.getVmsPassword(), stream.getCameraId(),
					stream.getWidth(), stream.getHeight(),
					stream.getKeepAspectRatio(), stream.getAllowUpSizing(),
					stream.getCompressionRate() };

			bool bVmsConnInitDone = false;
			if(pVmsConnector)
			{
				try {
					bVmsConnInitDone = pVmsConnector->init(connInfo, sVmsPluginDirPath);
				} catch (std::exception &e) {
					std::string sErrMsg = "Failed to initialize VMS Connector plugin. ";
					sErrMsg.append(e.what());
					opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
				}
			}

			if(bVmsConnInitDone)
			{
				std::stringstream ssMsg;
				ssMsg << "VMS Connector plugin " << stream.getId();
				ssMsg << " init done.";
				opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());

				//Add the VMS connector to the ApplicationModel
				std::pair <unsigned int,opencctv::api::VmsConnector*> vmsConn (stream.getVmsTypeId(),pVmsConnector);
				pModel->getVmsConnectors()[stream.getId()] = vmsConn;

				//pProducer = new opencctv::ProducerThread(stream.getId(), stream.getVmsTypeId(), pVmsConnector);
				pProducer = new opencctv::ProducerThread(stream.getId());
			}
		}
		if (pQueue && pConsumer && pProducer)
		{
			// Create and start Results Router threads
			for(size_t j = 0; j < vAnalyticInstances.size(); ++j)
			{
				opencctv::dto::AnalyticInstanceStream analyticInstance = vAnalyticInstances[j];

				if(!pModel->containsResultsRouterThread(analyticInstance.getAnalyticInstanceId()))
				{
					if(pModel->containsResultsOutputQueueAddress(analyticInstance.getAnalyticInstanceId()))
					{
						opencctv::ResultRouterThread* pResultsRouter = new opencctv::ResultRouterThread(analyticInstance.getAnalyticInstanceId());
						boost::thread* pResultsRouterThread = new boost::thread(*pResultsRouter);
						pModel->getResultsRouterThreads()[analyticInstance.getAnalyticInstanceId()] = pResultsRouterThread;
						if(pResultsRouterThread->joinable())
						{
							_pResultsRouterThreadGroup->add_thread(pResultsRouterThread);
						}

						delete pResultsRouter;
					}

				}
			}
			// Start Consumer and Producer threads
			boost::thread* pConsumerThread = new boost::thread(*pConsumer);
			pModel->getConsumerThreads()[stream.getId()] = pConsumerThread;
			if (pConsumerThread->joinable()) {
				boost::thread* pProducerThread = new boost::thread(*pProducer);
				pModel->getProducerThreads()[stream.getId()] = pProducerThread;
				if (pProducerThread->joinable()) {
					_pConsumerThreadGroup->add_thread(pConsumerThread);
					_pPproducerThreadGroup->add_thread(pProducerThread);
				}
			}
		}

		delete pConsumer;
		delete pProducer;
	}

	delete pStreamGateway;
	delete pAnalyticInstanceGateway;

	pModel->setServerStatus(opencctv::event::SERVER_STATUS_STARTED);
	return true;
}

bool ServerEvent::stopServer()
{
	opencctv::ApplicationModel* pModel = opencctv::ApplicationModel::getInstance();
	pModel->clear();
	pModel->setServerStatus(opencctv::event::SERVER_STATUS_STOPPED);
	return true;
}

ServerEvent::~ServerEvent()
{
}

} /* namespace event */
} /* namespace opencctv */
