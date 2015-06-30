/*
 * EventMessage.hpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#ifndef EVENTMESSAGE_HPP_
#define EVENTMESSAGE_HPP_

#include <string>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include "../../opencctv/Exception.hpp"
#include "../../opencctv/ApplicationModel.hpp"

namespace opencctv {
namespace event {

class EventMessage {
public:
	EventMessage();
	static std::string extractEventRequestOperation(const std::string& sEventRequest);
	static std::string getStartMessageReply();
	static std::string getStopMessageReply();
	virtual ~EventMessage();
};

} /* namespace event */
} /* namespace opencctv */

#endif /* EVENTMESSAGE_HPP_ */
