/*
 * EventMessage.hpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#ifndef EVENTMESSAGE_HPP_
#define EVENTMESSAGE_HPP_

#include <string>
#include <sys/types.h>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include "../../opencctv/Exception.hpp"
#include "../../opencctv/ApplicationModel.hpp"
#include "EventType.hpp"


namespace opencctv {
namespace event {

class EventMessage {
public:
	EventMessage();
	static std::string extractEventRequestOperation(const std::string& sEventRequest);

	//Analytic stop
	static void extractAnalyticStopRequest(const std::string& sEventRequest, unsigned int& iAnalyticInstanceId);
	static std::string getAnalyticStopReply(const unsigned int& iAnalyticInstanceId);

	//Analytic start
	static void extractAnalyticStartRequest(const std::string& sEventRequest, unsigned int& iAnalyticInstanceId, std::string& sAnalyticPluginDir, std::vector<std::pair<unsigned int,std::string> >& vInputStreams);
	static std::string getAnalyticStartReply(const unsigned int& iAnalyticInstanceId);

	//Analytics stop
	static void extractAnalyticsStopRequest(const std::string& sEventRequest, unsigned int& iAnalyticId, std::vector<unsigned int>& vAnalyticInstanceIds);
	static std::string getAnalyticsStopReply(const unsigned int& iAnalyticId);

	//Server start/stop/status
	static std::string getStartMessageReply();
	static std::string getStopMessageReply();
	static std::string getStatusMessageReply();

	//Error reply
	static std::string getInvalidMessageReply(const std::string& sContent);

	virtual ~EventMessage();
};

} /* namespace event */
} /* namespace opencctv */

#endif /* EVENTMESSAGE_HPP_ */
