
#ifndef ANALYTIC_ANALYTICPROCESS_HPP_
#define ANALYTIC_ANALYTICPROCESS_HPP_

#include "../opencctv/Process.hpp"

namespace analytic {

class AnalyticProcess : public opencctv::Process {
private:
	std::string _sPathToAnalyticRunnerExecutable;
	unsigned int _iAnalyticInstanceId;
	std::string _sAnalyticPluginDirLocation;
	//std::string _sAnalyticPluginFilename;
	std::string _sInputAnalyticQueueAddress;
	std::string _sOutputAnalyticQueueAddress;

public:
	AnalyticProcess(const std::string sPathToAnalyticRunnerExecutable, const unsigned int iAnalyticInstanceId, const std::string& sAnalyticPluginDirLocation, const std::string& sInputAnalyticQueueAddress, const std::string& sOutputAnalyticQueueAddress);
	bool startAnalytic();
	const std::string& getInputAnalyticQueueAddress() const;
	const std::string& getOutputAnalyticQueueAddress() const;
	virtual ~AnalyticProcess();
};

} /* namespace analytic */

#endif /* ANALYTIC_ANALYTICPROCESS_HPP_ */
