/*
 * ServerController.hpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#ifndef SERVERCONTROLLER_HPP_
#define SERVERCONTROLLER_HPP_

#include "mq/MqUtil.hpp"
#include "Exception.hpp"
#include "util/log/Loggers.hpp"
#include "util/Config.hpp"
#include "ApplicationModel.hpp"
//#include "../analytic/xml/AnalyticMessage.hpp"
#include "event/EventMessage.hpp"
#include "event/ServerEvent.hpp"
#include "event/AnalyticEvent.hpp"
#include "event/EventType.hpp"

namespace opencctv {

class ServerController {
private:
	static ServerController* _pServerController;
	zmq::socket_t* _pSocket;

	ServerController();
	ServerController(ServerController const& source);
	ServerController& operator=(ServerController const&);

	void sendReply(const std::string& sMessage);

public:
	static ServerController* getInstance();
	void executeOperation();
	virtual ~ServerController();
};

} /* namespace opencctv */

#endif /* SERVERCONTROLLER_HPP_ */
