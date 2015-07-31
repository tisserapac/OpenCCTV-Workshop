/*
 * EventType.hpp
 *
 *  Created on: Jun 30, 2015
 *      Author: anjana
 */

#ifndef EVENTTYPE_HPP_
#define EVENTTYPE_HPP_

namespace opencctv {
namespace event {

const std::string SERVER_STATUS_STARTED = "Started";
const std::string SERVER_STATUS_STOPPED = "Stopped";

const std::string ANALYTIC_STATUS_STARTED = "Started";
const std::string ANALYTIC_STATUS_STOPPED = "Stopped";

const std::string SERVER_EVENT_START = "StartServer";
const std::string SERVER_EVENT_STOP = "StopServer";
const std::string SERVER_EVENT_RESTART = "RestartServer";
const std::string SERVER_EVENT_STATUS = "ServerStatus";

const std::string ANALYTIC_EVENT_STOP = "StopAnalyticInstance";
const std::string ANALYTIC_EVENT_START = "StartAnalyticInstance";

const std::string EVENT_UNKNOWN = "UnknownEvent";


} /* namespace event */
} /* namespace opencctv */




#endif /* EVENTTYPE_HPP_ */
