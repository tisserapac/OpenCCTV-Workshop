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

std::string EventMessage::getStartMessageReply()
{
	std::string sReplyMessage = "";

	std::string sContent = "OpenCCTV Server Started";

	opencctv::ApplicationModel* _pModel = opencctv::ApplicationModel::getInstance();
	std::string sServerStatus = _pModel->getServerStatus();
	pid_t iServerProcessId = getpid();

	boost::property_tree::ptree pt;
	pt.put("opencctvmsg.type", "StartReply");
	pt.put("opencctvmsg.content", sContent);
	pt.put("opencctvmsg.serverstatus", sServerStatus);
	pt.put("opencctvmsg.serverpid", iServerProcessId);

	std::ostringstream oss;
	try
	{
		write_xml(oss, pt);
		sReplyMessage = oss.str();
	}
	catch (boost::property_tree::xml_parser::xml_parser_error &e)
	{
		std::string sMessage = "EventMessage::getStartMessageReply: XML parsing error ";
		throw opencctv::Exception(sMessage.append(e.what()));
	}

	return sReplyMessage;

}

std::string EventMessage::getStopMessageReply()
{
	std::string sReplyMessage = "";

	std::string sContent = "OpenCCTV Server Stopped";

	opencctv::ApplicationModel* _pModel = opencctv::ApplicationModel::getInstance();
	std::string sServerStatus = _pModel->getServerStatus();
	pid_t iServerProcessId = getpid();

	boost::property_tree::ptree pt;
	pt.put("opencctvmsg.type", "StopReply");
	pt.put("opencctvmsg.content", sContent);
	pt.put("opencctvmsg.serverstatus", sServerStatus);
	pt.put("opencctvmsg.serverpid", iServerProcessId);

	std::ostringstream oss;
	try
	{
		write_xml(oss, pt);
		sReplyMessage = oss.str();
	}
	catch (boost::property_tree::xml_parser::xml_parser_error &e)
	{
		std::string sMessage = "EventMessage::getStartMessageReply: XML parsing error ";
		throw opencctv::Exception(sMessage.append(e.what()));
	}

	return sReplyMessage;

}

EventMessage::~EventMessage()
{
}

} /* namespace event */
} /* namespace opencctv */
