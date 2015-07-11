/*
 * EventUtil.hpp
 *
 *  Created on: Jul 10, 2015
 *      Author: anjana
 */

#ifndef EVENTUTIL_HPP_
#define EVENTUTIL_HPP_

#include <boost/thread/thread.hpp>
#include <boost/chrono.hpp>
#include "../dto/Stream.hpp"
#include "../util/Config.hpp"
#include "../ApplicationModel.hpp"
#include "../ProducerThread.hpp"
#include "../ConsumerThread.hpp"
#include "../ResultRouterThread.hpp"
#include <iostream>

namespace opencctv {
namespace event {

const std::string PRODUCER_THREAD = "ProducerThread";
const std::string CONSUMER_THREAD = "ConsumerThread";
const std::string RESULTS_ROUTER_THREAD = "ResultsRouterThread";

class EventUtil {
public:
	EventUtil();
	virtual ~EventUtil();

	static bool loadVmsConnector(const opencctv::dto::Stream& stream);
	static bool deleteVmsConnector(const unsigned int iStreamId);
	static bool startThread(const std::string& sThreadType, const unsigned int iThreadId);
	static bool stopThread(const std::string& sThreadType, const unsigned int iThreadId);
	static bool startAnalyticInstance(const unsigned int& iAnalyticInstanceId, const std::string& sAnalyticPluginDir);
	static bool stopAnalyticInstance(const unsigned int iAnalyticInstanceId);
};

} /* namespace event */
} /* namespace opencctv */

#endif /* EVENTUTIL_HPP_ */
