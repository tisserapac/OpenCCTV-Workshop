/*
 * EventMessage.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#include "EventMessage.hpp"

namespace opencctv {
namespace event {

EventMessage::EventMessage()
{
}

std::string EventMessage::extractEventRequestOperation(const std::string& sEventRequest)
{
	std::string sRet;
	boost::property_tree::ptree pt;
	std::istringstream iss(sEventRequest);
	try {
		read_xml(iss, pt);
		sRet = pt.get<std::string>("opencctvmsg.type");
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Analytic Start Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	boost::algorithm::trim (sRet);
	return sRet;
}

EventMessage::~EventMessage()
{
}

} /* namespace event */
} /* namespace opencctv */
