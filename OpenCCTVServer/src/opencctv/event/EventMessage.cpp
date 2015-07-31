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

//===========Server Start=====================
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

//==============Server Stop==================
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

//==============Server Status==================
std::string EventMessage::getStatusMessageReply()
{
	std::string sReplyMessage = "";
	std::string sContent = "";

	boost::property_tree::ptree pt;
	opencctv::ApplicationModel* _pModel = opencctv::ApplicationModel::getInstance();
	std::string sServerStatus = _pModel->getServerStatus();
	pid_t iServerProcessId = getpid();

	if(sServerStatus.compare(opencctv::event::SERVER_STATUS_STARTED) == 0)
	{
		sContent = "OpenCCTV Server is Running";
	}else
	{
		sContent = "OpenCCTV Server Stopped";
	}

	pt.put("opencctvmsg.type", "StatusReply");
	pt.put("opencctvmsg.content", sContent);
	pt.put("opencctvmsg.serverstatus", sServerStatus);
	pt.put("opencctvmsg.serverpid", iServerProcessId);

	std::ostringstream oss;
	try
	{
		write_xml(oss, pt);
		sReplyMessage = oss.str();
	}catch (boost::property_tree::xml_parser::xml_parser_error &e)
	{
		std::string sMessage = "EventMessage::getStatusReply: XML parsing error ";
		throw opencctv::Exception(sMessage.append(e.what()));
	}

	return sReplyMessage;
}

//==========Analytic Stop===================
//sRequest = "<?xml version=\"1.0\" encoding=\"utf-8\"?><opencctvmsg><type>StopAnalyticInstance</type><analyticinstanceid>1</analyticinstanceid><inputstreams><streamid>1</streamid></inputstreams></opencctvmsg>";
void EventMessage::extractAnalyticStopRequest(const std::string& sEventRequest, unsigned int& iAnalyticInstanceId)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sEventRequest);
	try {
		read_xml(iss, pt);
		//sRet = pt.get<std::string>("opencctvmsg.type");

		iAnalyticInstanceId = pt.get<unsigned int>("opencctvmsg.analyticinstanceid");

	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Analytic Stop Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}catch (boost::property_tree::ptree_error &e)
	{
		std::string sErrMsg = "Failed to parse Analytic Stop Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

std::string EventMessage::getAnalyticStopReply(const unsigned int& iAnalyticInstanceId)
{
	std::string sReplyMessage = "";

	std::stringstream ss;
	ss << "Analytic instance " << iAnalyticInstanceId << " stopped";

	std::string sContent = ss.str();

	boost::property_tree::ptree pt;
	pt.put("opencctvmsg.type", "AnalyticStopReply");
	pt.put("opencctvmsg.content", sContent);

	std::ostringstream oss;
	try
	{
		write_xml(oss, pt);
		sReplyMessage = oss.str();
	}
	catch (boost::property_tree::xml_parser::xml_parser_error &e)
	{
		std::string sMessage = "EventMessage::getAnalyticStopReply: XML parsing error ";
		throw opencctv::Exception(sMessage.append(e.what()));
	}

	return sReplyMessage;
}

//==========Analytic Start===================
//<?xml version=\"1.0\" encoding=\"utf-8\"?><opencctvmsg><type>StartAnalyticInstance</type><analyticinstanceid>1</analyticinstanceid><analyticplugindir>MockAnalytic</analyticplugindir><inputstreams><inputstream><streamid>1</streamid><name>default</name></inputstream><inputstream><streamid>2</streamid><name>low res</name></inputstream></inputstreams></opencctvmsg>
void EventMessage::extractAnalyticStartRequest(const std::string& sEventRequest, unsigned int& iAnalyticInstanceId, std::string& sAnalyticPluginDir, std::vector<std::pair<unsigned int,std::string> >& vInputStreams)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sEventRequest);
	try {
		read_xml(iss, pt);
		iAnalyticInstanceId = pt.get<unsigned int>("opencctvmsg.analyticinstanceid");
		sAnalyticPluginDir = pt.get<std::string>("opencctvmsg.analyticplugindir");

		std::pair <unsigned int, std::string> pInputStram;

		BOOST_FOREACH( boost::property_tree::ptree::value_type const& v, pt.get_child("opencctvmsg.inputstreams") )
		{
			if (v.first == "inputstream")
			{
				pInputStram.first = v.second.get<unsigned int>("streamid");
				pInputStram.second = v.second.get<std::string>("name");
				vInputStreams.push_back(pInputStram);
			}
		}

	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Analytic Start Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}catch (boost::property_tree::ptree_error &e)
	{
		std::string sErrMsg = "Failed to parse Analytic Start Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

std::string EventMessage::getAnalyticStartReply(const unsigned int& iAnalyticInstanceId)
{
	std::string sReplyMessage = "";

	std::stringstream ss;
	ss << "Analytic instance " << iAnalyticInstanceId << " started";

	std::string sContent = ss.str();

	boost::property_tree::ptree pt;
	pt.put("opencctvmsg.type", "AnalyticStartReply");
	pt.put("opencctvmsg.content", sContent);

	std::ostringstream oss;
	try
	{
		write_xml(oss, pt);
		sReplyMessage = oss.str();
	}
	catch (boost::property_tree::xml_parser::xml_parser_error &e)
	{
		std::string sMessage = "EventMessage::getAnalyticStartReply: XML parsing error ";
		throw opencctv::Exception(sMessage.append(e.what()));
	}

	return sReplyMessage;
}

//==========Invalid Reply===================
std::string EventMessage::getInvalidMessageReply(const std::string& sContent)
{
	std::string sReplyMessage = "";

	boost::property_tree::ptree pt;
	pt.put("opencctvmsg.type", "Error");
	pt.put("opencctvmsg.content", sContent);

	opencctv::ApplicationModel* _pModel = opencctv::ApplicationModel::getInstance();
	std::string sServerStatus = _pModel->getServerStatus();
	pid_t iServerProcessId = getpid();
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
		std::ostringstream ossReply;
		ossReply << "<?xml version=\"1.0\" encoding=\"utf-8\"?> <opencctvmsg><type>Error</type><content>";
		ossReply << sContent;
		ossReply << "</content><serverstatus>";
		ossReply << sServerStatus;
		ossReply << "</serverstatus><serverpid>";
		ossReply << iServerProcessId;
		ossReply << "</serverpid></opencctvmsg>";

		sReplyMessage = ossReply.str();
	}

	return sReplyMessage;
}

EventMessage::~EventMessage()
{
}

} /* namespace event */
} /* namespace opencctv */
