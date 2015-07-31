
#ifndef OPENCCTV_APPLICATIONMODEL_HPP_
#define OPENCCTV_APPLICATIONMODEL_HPP_

#include <map>
#include <list>
#include <algorithm>
#include <string>
#include <boost/thread/thread.hpp>
#include "mq/Mq.hpp"
#include "PluginLoader.hpp"
#include "api/VmsConnector.hpp"
#include "util/flow/FlowController.hpp"
#include "ConcurrentQueue.hpp"
#include "Image.hpp"
#include "../analytic/AnalyticInstanceManager.hpp"
#include "util/log/Loggers.hpp"
#include "MulticastDestination.hpp"

namespace opencctv {

class ApplicationModel {
private:
	static ApplicationModel* _pModel;
	ApplicationModel();

	std::string sServerStatus;

	boost::thread_group* _pProducerThreadGroup;
	boost::thread_group* _pConsumerThreadGroup;
	boost::thread_group* _pResultsRouterThreadGroup;

	std::map<unsigned int, std::list<unsigned int> > _mAnalyticInstances; // Analytic ID as key

	std::map<unsigned int, boost::thread*> _mProducerThreads; // Stream ID as key
	std::map<unsigned int, boost::thread*> _mConsumerThreads; // Stream ID as key
	std::map<unsigned int, boost::thread*> _mResultsRouterThreads; // Analytic ID as key

	std::map<unsigned int, std::string> _mImageInputQueueAddresses; // Analytic Instance ID as key
	std::map<unsigned int, std::string> _mResultsOutputQueueAddresses; // Analytic Instance ID as key
	std::map<unsigned int, util::flow::FlowController*> _mFlowControllers; // Analytic Instance ID as key
	//std::map<unsigned int, mq::Sender*> _mMulticastDestinations; // Analytic Instance Stream ID as key
	std::map<unsigned int, PluginLoader<api::VmsConnector>*> _mVmsPluginLoaders; //VMS Type ID as key
	std::map<unsigned int, ConcurrentQueue<Image>*> _mInternalQueues; // Stream ID as key
	std::map<unsigned int, analytic::AnalyticInstanceManager*> _mAnalyticInstanceManagers; // Analytic Server ID as key

	std::map<unsigned int, MulticastDestination*> _mMulticastDestinations; //Stream Id as the key

	std::map<unsigned int, std::pair <unsigned int,opencctv::api::VmsConnector*> > _mVmsConnectors; // Stream ID as key and VMS Type id in the pair

public:
	static ApplicationModel* getInstance();

	boost::thread_group*& getConsumerThreadGroup();
	void setConsumerThreadGroup(boost::thread_group*& consumerThreadGroup);
	boost::thread_group*& getProducerThreadGroup();
	void setProducerThreadGroup(boost::thread_group*& producerThreadGroup);
	boost::thread_group*& getResultsRouterThreadGroup();
	void setResultsRouterThreadGroup(boost::thread_group*& resultsRouterThreadGroup);

	bool containsAnalyticInstance(unsigned int iAnalyticInstanceId);

	bool containsProducerThread(unsigned int iStreamId);
	bool containsConsumerThread(unsigned int iStreamId);
	bool containsResultsRouterThread(unsigned int iAnalyticInstanceId);

	bool containsImageInputQueueAddress(unsigned int iAnalyticInstanceId);
	bool containsResultsOutputQueueAddress(unsigned int iAnalyticInstanceId);
	bool containsFlowController(unsigned int iAnalyticInstanceId);
	bool containsMulticastDestination(unsigned int iId);
	bool containsVmsPluginLoader(unsigned int iVmsTypeId);
	bool containsInternalQueue(unsigned int iStreamId);
	bool containsAnalyticInstanceManager(unsigned int iAnalyticServerId);

	bool containsVmsConnector(unsigned int iStreamId);

	std::map<unsigned int, std::list<unsigned int> >& getAnalyticInstances();

	std::map<unsigned int, boost::thread*>& getConsumerThreads();
	std::map<unsigned int, boost::thread*>& getProducerThreads();
	std::map<unsigned int, boost::thread*>& getResultsRouterThreads();

	std::map<unsigned int, std::string>& getImageInputQueueAddresses();
	std::map<unsigned int, std::string>& getResultsOutputQueueAddresses();
	std::map<unsigned int, util::flow::FlowController*>& getFlowControllers();
	//std::map<unsigned int, mq::Sender*>& getMulticastDestinations();
	std::map<unsigned int, MulticastDestination*>& getMulticastDestinations();
	std::map<unsigned int, PluginLoader<api::VmsConnector>*>& getVmsPluginLoaders();
	std::map<unsigned int, ConcurrentQueue<Image>*>& getInternalQueues();
	std::map<unsigned int, analytic::AnalyticInstanceManager*>& getAnalyticInstanceManagers();

	std::map<unsigned int, std::pair <unsigned int,opencctv::api::VmsConnector*> >& getVmsConnectors();

	virtual ~ApplicationModel();

	const std::string& getServerStatus() const;

	void setServerStatus(const std::string& serverStatus);

	void clear();
};

} /* namespace opencctv */

#endif /* OPENCCTV_APPLICATIONMODEL_HPP_ */
