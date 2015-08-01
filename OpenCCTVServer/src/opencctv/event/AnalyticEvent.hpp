/*
 * AnalyticEvent.hpp
 *
 *  Created on: Jul 1, 2015
 *      Author: anjana
 */

#ifndef ANALYTICEVENT_HPP_
#define ANALYTICEVENT_HPP_

#include <vector>
#include <boost/thread/thread.hpp> // -lboost_thread -pthread
#include <boost/lockfree/queue.hpp>
#include <boost/lexical_cast.hpp> // to cast values
#include <signal.h> // to handle terminate signal
#include "../../test/gateway/TestStreamGateway.hpp"
#include "../../test/gateway/TestAnalyticInstanceStreamGateway.hpp"
#include "../../analytic/AnalyticInstanceManager.hpp"
#include "../util/Config.hpp"
#include "../Exception.hpp"
#include "../util/log/Loggers.hpp"
#include "../util/flow/SimpleFlowController.hpp"
#include "../ImageMulticaster.hpp"
#include "../ConsumerThread.hpp"
#include "../ProducerThread.hpp"
#include "../ResultRouterThread.hpp"
#include "../db/StreamGateway.hpp"
#include "../db/AnalyticInstanceGateway.hpp"
#include "EventType.hpp"
#include "EventMessage.hpp"
#include "EventUtil.hpp"

namespace opencctv {
namespace event {

class AnalyticEvent {
private:
	//static std::string analyticStop(const unsigned int iAnalyticInstanceId, std::vector<unsigned int>& vStreamIds);
	//static std::string analyticStart(const unsigned int iAnalyticInstanceId, std::string& sAnalyticPluginDir, std::vector<std::pair<unsigned int,std::string> >& vInputStreams);

public:
	AnalyticEvent();
	static std::string startAnalytic(const std::string& sRequest);
	static std::string startAnalytic(const unsigned int iAnalyticInstanceId, std::string& sAnalyticPluginDir, std::vector<std::pair<unsigned int,std::string> >& vInputStreams);
	static std::string stopAnalytic(const std::string& sRequest);
	static std::string stopAnalytic(const unsigned int iAnalyticInstanceId);

	static std::string stopAnalytics(const std::string& sRequest);

	virtual ~AnalyticEvent();
};

} /* namespace event */
} /* namespace opencctv */

#endif /* ANALYTICEVENT_HPP_ */
