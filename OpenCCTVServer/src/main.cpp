//#include "test/gateway/TestStreamGateway.hpp"
//#include "test/gateway/TestAnalyticInstanceStreamGateway.hpp"
#include "opencctv/util/Config.hpp"
#include "opencctv/ApplicationModel.hpp"
#include "opencctv/Exception.hpp"
#include "analytic/AnalyticInstanceManager.hpp"
#include "opencctv/util/log/Loggers.hpp"
#include <boost/thread/thread.hpp> // -lboost_thread -pthread
#include <boost/lexical_cast.hpp> // to cast values
#include <signal.h> // to handle terminate signal
#include "opencctv/db/AnalyticInstanceGateway.hpp"
#include "opencctv/ServerController.hpp"
#include "opencctv/event/EventType.hpp"
#include "opencctv/event/ServerEvent.hpp"

void terminateHandler(int signum); // Terminate signal handler

bool bEnabled = false;

int main()
{
	// Sending PID of OpenCCTV Server process to OpenCCTV Starter process through stdout
	fprintf(stdout, opencctv::util::Util::getPidMessage(getpid()).c_str());
	fflush (stdout);

	// Registering signal handlers
	signal(SIGTERM, terminateHandler); // for Terminate signal
	signal(SIGINT, terminateHandler); // for Ctrl + C keyboard interrupt

	//Listen to the events and respond
	opencctv::ServerController* pServerController = NULL;
	try
	{
		pServerController = opencctv::ServerController::getInstance();
		if(pServerController)
		{
			opencctv::event::ServerEvent::updateServerStatus(opencctv::event::SERVER_STATUS_STOPPED);
			bEnabled = true;
		}
	}catch(opencctv::Exception &e)
	{
		std::string sErrMsg = "Failed to initialize the OpenCCTV Server : ";
		sErrMsg.append(e.what());
		opencctv::util::log::Loggers::getDefaultLogger()->error(sErrMsg);
		return -1;
	}

	while(bEnabled)
	{
		opencctv::util::log::Loggers::getDefaultLogger()->info("Listening to the events....");
		pServerController->executeOperation();
	}

	return 0;
}

// Signal handler for SIGTERM (Terminate signal)
void terminateHandler(int signum) {
	std::map<unsigned int, analytic::AnalyticInstanceManager*> mAnalyticInstanceManagers = opencctv::ApplicationModel::getInstance()->getAnalyticInstanceManagers();
	std::map<unsigned int, analytic::AnalyticInstanceManager*>::iterator it;
	for(it = mAnalyticInstanceManagers.begin(); it != mAnalyticInstanceManagers.end(); /*it increment below*/)
	{
		analytic::AnalyticInstanceManager* pAnalyticInstanceManager = it->second;
		std::string sMessage;
		if(pAnalyticInstanceManager->killAllAnalyticInstances(sMessage))
		{
			if(pAnalyticInstanceManager)
			{
				delete pAnalyticInstanceManager;
				pAnalyticInstanceManager = NULL;
			}
			mAnalyticInstanceManagers.erase(it++); //remove analytic instance manager from the model
		}
		else
		{
			opencctv::util::log::Loggers::getDefaultLogger()->error(sMessage);
			++it;
		}
	}
	if(!mAnalyticInstanceManagers.empty())
	{
		opencctv::util::log::Loggers::getDefaultLogger()->error("Failed to reset all the Analytic Servers.");
	}
	else
	{
		opencctv::util::log::Loggers::getDefaultLogger()->info("Reset all the Analytic Servers.");
	}

	opencctv::event::ServerEvent::updateServerStatus(opencctv::event::SERVER_STATUS_STOPPED);

	bEnabled = false;
	exit(signum);
}

