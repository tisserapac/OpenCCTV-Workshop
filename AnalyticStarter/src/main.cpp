
#include "opencctv/mq/MqUtil.hpp"
#include "analytic/xml/AnalyticMessage.hpp"
#include "opencctv/Exception.hpp"
#include "opencctv/util/log/Loggers.hpp"
#include "analytic/AnalyticProcess.hpp"
#include "analytic/util/Config.hpp"
#include <string>
#include <sstream>
#include <map>
#include "analytic/ApplicationModel.hpp"
#include "analyticserver/AnalyticServerController.hpp"

using namespace opencctv;

int main()
{
	analyticserver::AnalyticServerController* pAnalyticServerController = NULL;
	bool bEnabled = false;
	try
	{
		pAnalyticServerController = analyticserver::AnalyticServerController::getInstance();
		if(pAnalyticServerController)
		{
			bEnabled = true;
		}
	}catch(opencctv::Exception &e)
	{
		std::string sErrMsg = "Failed to initialize the Analytic Server : ";
		sErrMsg.append(e.what());
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		return -1;
	}

	while(bEnabled)
	{
		pAnalyticServerController->executeOperation();
	}
	return 0;
}
