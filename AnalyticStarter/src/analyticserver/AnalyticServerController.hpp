/*
 * AnalyticServerController.hpp
 *
 *  Created on: Jun 29, 2015
 *      Author: anjana
 */

#ifndef ANALYTICSERVERCONTROLLER_HPP_
#define ANALYTICSERVERCONTROLLER_HPP_

#include <string>
#include <sstream>
#include <map>
#include "../opencctv/mq/MqUtil.hpp"
#include "../opencctv/Exception.hpp"
#include "../opencctv/util/log/Loggers.hpp"
#include "../analytic/xml/AnalyticMessage.hpp"
#include "../analytic/AnalyticProcess.hpp"
#include "../analytic/util/Config.hpp"
#include "../analytic/ApplicationModel.hpp"

namespace analyticserver {

class AnalyticServerController {
private:
	static AnalyticServerController* _pAnalyticServerController;
	unsigned int STARTING_PORT;
	unsigned int NO_ANALYTICS;
	zmq::socket_t* _pSocket;

	AnalyticServerController();
	AnalyticServerController(AnalyticServerController const& source);
	AnalyticServerController& operator=(AnalyticServerController const&);

	void fillIOPorts();
	std::string startAnalytic(const std::string& sRequest);
	std::string stopAnalytic(const std::string& sRequest);
	std::string killAllAnalytics(const std::string& sRequest);
	void sendReply(const std::string& sMessage);

public:
	static AnalyticServerController* getInstance();
	void executeOperation();
	virtual ~AnalyticServerController();
};

} /* namespace analytic */

#endif /* ANALYTICSERVERCONTROLLER_HPP_ */
