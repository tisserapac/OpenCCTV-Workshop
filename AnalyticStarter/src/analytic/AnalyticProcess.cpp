
#include "AnalyticProcess.hpp"

namespace analytic {

AnalyticProcess::AnalyticProcess(const std::string sPathToAnalyticRunnerExecutable, const unsigned int iAnalyticInstanceId, const std::string& sAnalyticPluginDirLocation, const std::string& sInputAnalyticQueueAddress, const std::string& sOutputAnalyticQueueAddress) : opencctv::Process()
{
	_sPathToAnalyticRunnerExecutable = sPathToAnalyticRunnerExecutable;
	_iAnalyticInstanceId = iAnalyticInstanceId;
	_sAnalyticPluginDirLocation = sAnalyticPluginDirLocation;
	//_sAnalyticPluginFilename = sAnalyticPluginFilename;
	_sInputAnalyticQueueAddress = sInputAnalyticQueueAddress;
	_sOutputAnalyticQueueAddress = sOutputAnalyticQueueAddress;

}

bool AnalyticProcess::startAnalytic()
{
	bool bRet = false;
	std::stringstream ssArgs;
	ssArgs << _iAnalyticInstanceId << " ";
	ssArgs << _sAnalyticPluginDirLocation << " ";
	//ssArgs << _sAnalyticPluginFilename << " ";
	ssArgs << _sInputAnalyticQueueAddress << " ";
	ssArgs << _sOutputAnalyticQueueAddress;

	try
	{
		bRet =  start(_sPathToAnalyticRunnerExecutable, ssArgs.str());
	}catch(opencctv::Exception &e)
	{
		throw e;
	}

	return bRet;
}

const std::string& AnalyticProcess::getInputAnalyticQueueAddress() const {
	return _sInputAnalyticQueueAddress;
}

const std::string& AnalyticProcess::getOutputAnalyticQueueAddress() const {
	return _sOutputAnalyticQueueAddress;
}

AnalyticProcess::~AnalyticProcess() {
}

} /* namespace analytic */
