#include "ApplicationModel.hpp"

namespace opencctv {

ApplicationModel* ApplicationModel::_pModel = NULL;

ApplicationModel* ApplicationModel::getInstance()
{
	if (!_pModel) {
		_pModel = new ApplicationModel();
	}
	return _pModel;
}

ApplicationModel::ApplicationModel()
{
	_pProducerThreadGroup = NULL;
	_pConsumerThreadGroup =NULL;
	_pResultsRouterThreadGroup = NULL;
}

bool ApplicationModel::containsProducerThread(unsigned int iStreamId)
{
	std::map<unsigned int, boost::thread*>::iterator it = _mProducerThreads.find(iStreamId);
	if (it != _mProducerThreads.end()) {
		return true;
	}
	return false;
}

bool ApplicationModel::containsConsumerThread(unsigned int iStreamId)
{
	std::map<unsigned int, boost::thread*>::iterator it = _mConsumerThreads.find(iStreamId);
	if (it != _mConsumerThreads.end()) {
		return true;
	}
	return false;
}

bool ApplicationModel::containsResultsRouterThread(unsigned int iAnalyticInstanceId)
{
	std::map<unsigned int, boost::thread*>::iterator it = _mResultsRouterThreads.find(iAnalyticInstanceId);
	if (it != _mResultsRouterThreads.end()) {
		return true;
	}
	return false;
}

bool ApplicationModel::containsImageInputQueueAddress(unsigned int iAnalyticInstanceId)
{
	std::map<unsigned int, std::string>::iterator it = _mImageInputQueueAddresses.find(iAnalyticInstanceId);
	if (it != _mImageInputQueueAddresses.end()) {
		return true;
	}
	return false;
}

bool ApplicationModel::containsResultsOutputQueueAddress(unsigned int iAnalyticInstanceId)
{
	std::map<unsigned int, std::string>::iterator it = _mResultsOutputQueueAddresses.find(iAnalyticInstanceId);
	if (it != _mResultsOutputQueueAddresses.end()) {
		return true;
	}
	return false;
}

bool ApplicationModel::containsFlowController(unsigned int iAnalyticInstanceId)
{
	std::map<unsigned int, util::flow::FlowController*>::iterator it = _mFlowControllers.find(iAnalyticInstanceId);
	if(it != _mFlowControllers.end()) {
		return true;
	}
	return false;
}

bool ApplicationModel::containsMulticastDestination(unsigned int iId)
{
	std::map<unsigned int, mq::Sender*>::iterator it = _mMulticastDestinations.find(iId);
	if(it != _mMulticastDestinations.end())
	{
		return true;
	}
	return false;
}

bool ApplicationModel::containsVmsPluginLoader(unsigned int iVmsTypeId)
{
	std::map<unsigned int, PluginLoader<api::VmsConnector>*>::iterator it = _mVmsPluginLoaders.find(iVmsTypeId);
	if(it != _mVmsPluginLoaders.end())
	{
		return true;
	}
	return false;
}

bool ApplicationModel::containsInternalQueue(unsigned int iStreamId)
{
	std::map<unsigned int, ConcurrentQueue<Image>*>::iterator it = _mInternalQueues.find(iStreamId);
	if(it != _mInternalQueues.end())
	{
		return true;
	}
	return false;
}

bool ApplicationModel::containsAnalyticInstanceManager(unsigned int iAnalyticServerId)
{
	std::map<unsigned int, analytic::AnalyticInstanceManager*>::iterator it = _mAnalyticInstanceManagers.find(iAnalyticServerId);
	if (it != _mAnalyticInstanceManagers.end()) {
		return true;
	}
	return false;
}

boost::thread_group*& ApplicationModel::getConsumerThreadGroup()
{
	return _pConsumerThreadGroup;
}

void ApplicationModel::setConsumerThreadGroup(boost::thread_group*& consumerThreadGroup)
{
	_pConsumerThreadGroup = consumerThreadGroup;
}

boost::thread_group*& ApplicationModel::getProducerThreadGroup()
{
	return _pProducerThreadGroup;
}

void ApplicationModel::setProducerThreadGroup(boost::thread_group*& producerThreadGroup)
{
	_pProducerThreadGroup = producerThreadGroup;
}

boost::thread_group*& ApplicationModel::getResultsRouterThreadGroup()
{
	return _pResultsRouterThreadGroup;
}

void ApplicationModel::setResultsRouterThreadGroup(boost::thread_group*& resultsRouterThreadGroup)
{
	_pResultsRouterThreadGroup = resultsRouterThreadGroup;
}

std::map<unsigned int, boost::thread*>& ApplicationModel::getConsumerThreads(){
	return _mConsumerThreads;
}

std::map<unsigned int, boost::thread*>& ApplicationModel::getProducerThreads(){
	return _mProducerThreads;
}

std::map<unsigned int, boost::thread*>& ApplicationModel::getResultsRouterThreads(){
	return _mResultsRouterThreads;
}

std::map<unsigned int, std::string>& ApplicationModel::getImageInputQueueAddresses()
{
	return _mImageInputQueueAddresses;
}

std::map<unsigned int, std::string>& ApplicationModel::getResultsOutputQueueAddresses()
{
	return _mResultsOutputQueueAddresses;
}

std::map<unsigned int, util::flow::FlowController*>& ApplicationModel::getFlowControllers()
{
	return _mFlowControllers;
}

std::map<unsigned int, mq::Sender*>& ApplicationModel::getMulticastDestinations()
{
	return _mMulticastDestinations;
}

std::map<unsigned int, PluginLoader<api::VmsConnector>*>& ApplicationModel::getVmsPluginLoaders()
{
	return _mVmsPluginLoaders;
}

std::map<unsigned int, ConcurrentQueue<Image>*>& ApplicationModel::getInternalQueues()
{
	return _mInternalQueues;
}

std::map<unsigned int, analytic::AnalyticInstanceManager*>& ApplicationModel::getAnalyticInstanceManagers()
{
	return _mAnalyticInstanceManagers;
}

const std::string& ApplicationModel::getServerStatus() const {
	return sServerStatus;
}

void ApplicationModel::setServerStatus(const std::string& serverStatus) {
	sServerStatus = serverStatus;
}

void ApplicationModel::clear()
{
	boost::thread* pThread = NULL;
	std::map<unsigned int, boost::thread*>::iterator itThread;

	//Remove all producer threads
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all producer threads");
	for(itThread = _mProducerThreads.begin(); itThread != _mProducerThreads.end(); /*it increment below*/)
	{
		pThread = itThread->second;

		while (pThread->timed_join(boost::posix_time::seconds(1))==false)
		{
			// Interupt the thread
			//std::cout<<"Producer Thread not stopped, interrupt it now."<<std::endl;
			pThread->interrupt();
			//std::cout<<"Producer Thread interrupt request sent. ";
			//std::cout<<"Wait to finish for 1 seconds again."<<std::endl;
		}
		//std::cout<<"Producer Thread stopped"<<std::endl;
		_pProducerThreadGroup->remove_thread(pThread);
		delete pThread; pThread = NULL;
		_mProducerThreads.erase(itThread++);
	}

	//Remove all consumer threads
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all consumer threads");
	for(itThread = _mConsumerThreads.begin(); itThread != _mConsumerThreads.end(); /*it increment below*/)
	{
		pThread = itThread->second;

		while (pThread->timed_join(boost::posix_time::seconds(1))==false)
		{
			pThread->interrupt();
		}
		_pConsumerThreadGroup->remove_thread(pThread);
		delete pThread; pThread = NULL;
		_mConsumerThreads.erase(itThread++);
	}

	//Remove all results router threads
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all results router threads");
	for(itThread = _mResultsRouterThreads.begin(); itThread != _mResultsRouterThreads.end(); /*it increment below*/)
	{
		pThread = itThread->second;

		while (pThread->timed_join(boost::posix_time::seconds(1))==false)
		{
			pThread->interrupt();
		}
		_pResultsRouterThreadGroup->remove_thread(pThread);
		delete pThread; pThread = NULL;
		_mResultsRouterThreads.erase(itThread++);
	}

	//Remove the thread groups
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all thread groups");
	delete _pProducerThreadGroup; _pProducerThreadGroup = NULL;
	delete _pConsumerThreadGroup; _pConsumerThreadGroup = NULL;
	delete _pResultsRouterThreadGroup; _pResultsRouterThreadGroup = NULL;

	//Remove all AnalyticInstanceManagers
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all AnalyticInstanceManagers");
	analytic::AnalyticInstanceManager* pAnalyticInstanceManager = NULL;
	std::map<unsigned int, analytic::AnalyticInstanceManager*>::iterator itAnalyticInstanceManager;
	for(itAnalyticInstanceManager = _mAnalyticInstanceManagers.begin(); itAnalyticInstanceManager != _mAnalyticInstanceManagers.end(); /*it increment below*/)
	{
		pAnalyticInstanceManager = itAnalyticInstanceManager->second;
		std::string sMessage;
		if(pAnalyticInstanceManager->killAllAnalyticInstances(sMessage))
		{
			if(pAnalyticInstanceManager)
			{
				delete pAnalyticInstanceManager;
				pAnalyticInstanceManager = NULL;
			}
			_mAnalyticInstanceManagers.erase(itAnalyticInstanceManager++); //remove analytic instance manager from the model
		}
		else
		{
			opencctv::util::log::Loggers::getDefaultLogger()->error(sMessage);
			++itAnalyticInstanceManager;
		}
	}

	//Remove the concurrent queues
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all queues");
	ConcurrentQueue<Image>* pConcurrentQueue = NULL;
	std::map<unsigned int, ConcurrentQueue<Image>*>::iterator itQueue;
	for(itQueue = _mInternalQueues.begin(); itQueue != _mInternalQueues.end(); /*it increment below*/)
	{
		pConcurrentQueue = itQueue->second;
		delete pConcurrentQueue; pConcurrentQueue = NULL;
		_mInternalQueues.erase(itQueue++);
	}

	//Remove analytic input queues
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all input queues");
	std::map<unsigned int, std::string>::iterator itInQueue = _mImageInputQueueAddresses.begin();
	_mImageInputQueueAddresses.erase(itInQueue, _mImageInputQueueAddresses.end());

	//Remove analytic output queues
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all output queues");
	std::map<unsigned int, std::string>::iterator itOutQueue = _mResultsOutputQueueAddresses.begin();
	_mResultsOutputQueueAddresses.erase(itOutQueue, _mResultsOutputQueueAddresses.end());

	//Remove all flow controllers
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all flow controllers");
	util::flow::FlowController* pFlowController = NULL;
	std::map<unsigned int, util::flow::FlowController*>::iterator itFlowController;
	for(itFlowController = _mFlowControllers.begin(); itFlowController != _mFlowControllers.end(); /*it increment below*/)
	{
		pFlowController = itFlowController->second;
		delete pFlowController; pFlowController = NULL;
		_mFlowControllers.erase(itFlowController++);
	}

	//Remove all multicast destinations
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all multicast destinations");
	mq::Sender* pSender = NULL;
	std::map<unsigned int, mq::Sender*>::iterator itSender;
	for(itSender = _mMulticastDestinations.begin(); itSender != _mMulticastDestinations.end(); /*it increment below*/)
	{
		pSender = itSender->second;
		delete pSender; pSender = NULL;
		_mMulticastDestinations.erase(itSender++);
	}

	//Remove all VmsPluginLoaders
	opencctv::util::log::Loggers::getDefaultLogger()->info("Removing all VmsPluginLoaders");
	PluginLoader<api::VmsConnector>* pVmsConnector = NULL;
	std::map<unsigned int, PluginLoader<api::VmsConnector>*>::iterator itVmsConnector;
	for(itVmsConnector = _mVmsPluginLoaders.begin(); itVmsConnector != _mVmsPluginLoaders.end(); /*it increment below*/)
	{
		pVmsConnector = itVmsConnector->second;
		delete pVmsConnector; pVmsConnector = NULL;
		_mVmsPluginLoaders.erase(itVmsConnector++);
	}
}

ApplicationModel::~ApplicationModel() {
	if (_pModel) {
		delete _pModel;
	}
}

} /* namespace opencctv */
