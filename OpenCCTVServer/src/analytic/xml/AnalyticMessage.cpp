#include "AnalyticMessage.hpp"

namespace analytic {
namespace xml {


std::string AnalyticMessage::extractAnalyticRequestOperation(const std::string& sAnalyticRequest)
{
	std::string sRet;
	boost::property_tree::ptree pt;
	std::istringstream iss(sAnalyticRequest);
	try {
		read_xml(iss, pt);
		sRet = opencctv::util::Util::trim(pt.get<std::string>("analyticrequest.operation"));
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Analytic Start Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	boost::algorithm::trim (sRet);
	return sRet;
}

//===========Analytic Start====================
std::string AnalyticMessage::getAnalyticStartRequest(const unsigned int iAnalyticInstanceId, const std::string& sAnalyticPluginDirLocation, const std::string& sAnalyticPluginFilename)
{
	boost::property_tree::ptree pt;
	pt.put("analyticrequest.operation", OPERATION_START_ANALYTIC);
	pt.put("analyticrequest.analyticinstanceid", iAnalyticInstanceId);
	pt.put("analyticrequest.analyticdirlocation", sAnalyticPluginDirLocation);
	pt.put("analyticrequest.analyticfilename", sAnalyticPluginFilename);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to generate Analytic Start Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	return message;
}

void AnalyticMessage::extractAnalyticStartRequestData(const std::string& sAnalyticStartRequest, unsigned int& iAnalyticInstanceId, std::string& sAnalyticDirLocation, std::string& sAnalyticFilename)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sAnalyticStartRequest);
	try {
		read_xml(iss, pt);
		iAnalyticInstanceId = pt.get<unsigned int>("analyticrequest.analyticinstanceid");
		sAnalyticDirLocation = pt.get<std::string>("analyticrequest.analyticdirlocation");
		sAnalyticFilename = pt.get<std::string>("analyticrequest.analyticfilename");
		boost::algorithm::trim (sAnalyticDirLocation);
		boost::algorithm::trim (sAnalyticFilename);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Analytic Start Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

std::string AnalyticMessage::getAnalyticStartReply(const bool bDone, const std::string& sMessage, const std::string& sAnalyticQueueInAddress,const std::string& sAnalyticQueueOutAddress)
{
	boost::property_tree::ptree pt;
	pt.put("analyticreply.operation", OPERATION_START_ANALYTIC);
	pt.put("analyticreply.done", bDone);
	pt.put("analyticreply.message", sMessage);
	pt.put("analyticreply.analyticqueueinaddress", sAnalyticQueueInAddress);
	pt.put("analyticreply.analyticqueueoutaddress", sAnalyticQueueOutAddress);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to generate Analytic Start Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	return message;
}

void AnalyticMessage::extractAnalyticStartReplyData(const std::string& sAnalyticStartReply, bool& bDone, std::string& sMessage, std::string& sAnalyticQueueInAddress, std::string& sAnalyticQueueOutAddress)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sAnalyticStartReply);
	try {
		read_xml(iss, pt);
		bDone = pt.get<bool>("analyticreply.done");

		sMessage = pt.get<std::string>("analyticreply.message");
		sAnalyticQueueInAddress = pt.get<std::string>("analyticreply.analyticqueueinaddress");
		sAnalyticQueueOutAddress = pt.get<std::string>("analyticreply.analyticqueueoutaddress");
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Analytic Start Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	boost::algorithm::trim(sAnalyticQueueInAddress);
	boost::algorithm::trim(sAnalyticQueueOutAddress);
}

//===========Analytic Stop====================
std::string AnalyticMessage::getAnalyticStopRequest(const unsigned int iAnalyticInstanceId)
{
	boost::property_tree::ptree pt;
	pt.put("analyticrequest.operation", OPERATION_STOP_ANALYTIC);
	pt.put("analyticrequest.analyticinstanceid", iAnalyticInstanceId);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "AnalyticMessage : Failed to generate Analytic Instance Stop Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	return message;
}

void AnalyticMessage::extractAnalyticStopRequestData(const std::string& sAnalyticStartRequest, unsigned int& iAnalyticInstanceId)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sAnalyticStartRequest);
	try {
		read_xml(iss, pt);
		iAnalyticInstanceId = pt.get<unsigned int>("analyticrequest.analyticinstanceid");
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "AnalyticMessage: Failed to parse Analytic Stop Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}

}

std::string AnalyticMessage::getAnalyticStopReply(const bool bDone, const std::string& sMessage)
{
	boost::property_tree::ptree pt;
	pt.put("analyticreply.operation", OPERATION_STOP_ANALYTIC);
	pt.put("analyticreply.done", bDone);
	pt.put("analyticreply.message", sMessage);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "AnalyticMessage: Failed to generate Analytic Instance Stop Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	return message;
}

void AnalyticMessage::extractStopAnalyticProcessesReplyData(const std::string& sReply, bool& bDone, std::string& sMessage)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sReply);
	try {
		read_xml(iss, pt);
		bDone = pt.get<bool>("analyticreply.done");
		sMessage = opencctv::util::Util::trim(pt.get<std::string>("analyticreply.message"));
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "AnalyticMessage : Failed to Parse Stop Analytic Processes Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

//===========Kill All Analytics====================
std::string AnalyticMessage::getKillAllAnalyticProcessesRequest()
{
	boost::property_tree::ptree pt;
	pt.put("analyticrequest.operation", OPERATION_KILL_ALL_ANALYTICS);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to generate Kill All Analytic Processes Request. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	return message;
}


std::string AnalyticMessage::getKillAllAnalyticProcessesReply(bool bDone, const std::string& sMessage)
{
	boost::property_tree::ptree pt;
	pt.put("analyticreply.operation", OPERATION_KILL_ALL_ANALYTICS);
	pt.put("analyticreply.done", bDone);
	pt.put("analyticreply.message", sMessage);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to generate Kill All Analytic Processes Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	return message;
}

void AnalyticMessage::extractKillAllAnalyticProcessesReplyData(const std::string& sReply, bool& sDone, std::string& sMessage)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sReply);
	try {
		read_xml(iss, pt);
		sDone = pt.get<bool>("analyticreply.done");
		sMessage = pt.get<std::string>("analyticreply.message");
		boost::algorithm::trim (sMessage);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Kill All Analytic Processes Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

std::string AnalyticMessage::getErrorReply(const std::string& sOperation, const bool bDone, const std::string& sErrorMessage)
{
	std::stringstream ossReply;
	ossReply << "<?xml version=\"1.0\" encoding=\"utf-8\"?>";
	ossReply << "<analyticreply>";
	ossReply << "<operation>";
	ossReply <<	sOperation;
	ossReply << "</operation>";
	ossReply << "<done>";
	ossReply <<	bDone;
	ossReply << "</done>";
	ossReply << "<message>";
	ossReply <<	sErrorMessage;
	ossReply << "</message>";
	ossReply << "</analyticreply>";

	std::string sReplyMessage = ossReply.str();
	return sReplyMessage;
}

void AnalyticMessage::extractErrorReplyData(const std::string& sReply, std::string& sOperation, bool& bDone, std::string& sMessage)
{
	boost::property_tree::ptree pt;
	std::istringstream iss(sReply);
	try {
		read_xml(iss, pt);
		sOperation = pt.get<std::string>("analyticreply.operation");
		boost::algorithm::trim (sOperation);
		bDone = pt.get<bool>("analyticreply.done");
		sMessage = pt.get<std::string>("analyticreply.message");
		boost::algorithm::trim (sMessage);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse Kill All Analytic Processes Reply. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
}

//===================Analytic Process IDs====================

std::string AnalyticMessage::getPidMessage(pid_t pid)
{
	boost::property_tree::ptree pt;
	pt.put("pid", pid);
	std::ostringstream oss;
	try {
		write_xml(oss, pt);
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to generate PID message. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	std::string message = oss.str();
	boost::replace_all(message, "\n", "");
	message.append("\n");
	return message;
}

pid_t AnalyticMessage::getPid(const std::string& sPidMessage)
{
	pid_t pid = 0;
	boost::property_tree::ptree pt;
	std::istringstream iss(sPidMessage);
	try {
		read_xml(iss, pt);
		pid = pt.get<pid_t>("pid");
	} catch (boost::property_tree::xml_parser::xml_parser_error &e) {
		std::string sErrMsg = "Failed to parse PID message. ";
		sErrMsg.append(e.what());
		throw opencctv::Exception(sErrMsg);
	}
	return pid;
}

} /* namespace xml */
} /* namespace analytic */
