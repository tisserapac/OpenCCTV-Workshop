/*
 * ServerEvent.hpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#ifndef SERVEREVENT_HPP_
#define SERVEREVENT_HPP_

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
#include "../MulticastDestination.hpp"
#include "../db/StreamGateway.hpp"
#include "../db/AnalyticInstanceStreamGateway.hpp"
#include "../db/AnalyticInstanceGateway.hpp"
#include "EventType.hpp"


namespace opencctv {
namespace event {

class ServerEvent {
public:
	ServerEvent();
	static bool startServer();
	static bool stopServer();
	static void updateServerStatus(std::string sStatus);
	virtual ~ServerEvent();
};

} /* namespace event */
} /* namespace opencctv */

#endif /* SERVEREVENT_HPP_ */
