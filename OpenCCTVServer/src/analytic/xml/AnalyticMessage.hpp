
#ifndef ANALYTIC_ANALYTICMESSAGE_HPP_
#define ANALYTIC_ANALYTICMESSAGE_HPP_

#include <string>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include "../../opencctv/Exception.hpp"

namespace analytic {
namespace xml {

const std::string OPERATION_START_ANALYTIC = "startanalytic";
const std::string OPERATION_STOP_ANALYTIC = "stopanalytic";
const std::string OPERATION_KILL_ALL_ANALYTICS = "killallanalytics";
const std::string OPERATION_UNKNOWN = "unknown";

class AnalyticMessage {
public:
	//Request Extract
	static std::string extractAnalyticRequestOperation(const std::string& sAnalyticRequest);
	static void extractAnalyticStartRequestData(const std::string& sAnalyticStartRequest, unsigned int& iAnalyticInstanceId, std::string& sAnalyticDirLocation, std::string& sAnalyticFilename);
	static void extractAnalyticStopRequestData(const std::string& sAnalyticStartRequest, unsigned int& iAnalyticInstanceId);

	//Reply Extract
	static void extractAnalyticStartReplyData(const std::string& sAnalyticStartReply, bool bDone, std::string& sMessage, std::string& sAnalyticQueueInAddress, std::string& sAnalyticQueueOutAddress);
	static void extractKillAllAnalyticProcessesReplyData(const std::string& sReply, bool& sDone, std::string& sMessage);
	static void extractStopAnalyticProcessesReplyData(const std::string& sReply, bool& bDone, std::string& sMessage);
	void extractErrorReplyData(const std::string& sReply, std::string& sOperation, bool bDone, std::string& sMessage);

	//Request XML Messages
	static std::string getAnalyticStartRequest(const unsigned int iAnalyticInstanceId, const std::string& sAnalyticPluginDirLocation, const std::string& sAnalyticPluginFilename);
	static std::string getAnalyticStopRequest(const unsigned int iAnalyticInstanceId);
	static std::string getKillAllAnalyticProcessesRequest();

	//Reply XML Messages
	static std::string getAnalyticStartReply(const bool bDone, const std::string& sMessage, const std::string& sAnalyticQueueInAddress,const std::string& sAnalyticQueueOutAddress);
	static std::string getAnalyticStopReply(const bool bDone, const std::string& sMessage);
	static std::string getKillAllAnalyticProcessesReply(const bool bDone, const std::string& sMessage);
	static std::string getErrorReply(const std::string& sOperation, const bool bDone, const std::string& sErrorMessage);

	static std::string getPidMessage(pid_t pid);
	static pid_t getPid(const std::string& sPidMessage);
};

} /* namespace xml */
} /* namespace analytic */

#endif /* ANALYTIC_ANALYTICMESSAGE_HPP_ */
