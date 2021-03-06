
#ifndef OPENCCTV_RESULTROUTERTHREAD_HPP_
#define OPENCCTV_RESULTROUTERTHREAD_HPP_

#include <sstream>
#include <boost/thread/thread.hpp>
#include "ApplicationModel.hpp"
#include "mq/TcpMqReceiver.hpp"
#include "util/Config.hpp"
#include "Exception.hpp"
#include "util/log/Loggers.hpp"
#include "util/serialization/Serializers.hpp"
#include "db/AnalyticResultGateway.hpp"


namespace opencctv {

class ResultRouterThread {
private:
	util::flow::FlowController* _pFlowController;
	unsigned int _iAnalyticInstanceId;
	util::serialization::Serializable* _pSerializer;
public:
	ResultRouterThread(unsigned int iAnalyticInstanceId);
	void operator()();
};

} /* namespace opencctv */

#endif /* OPENCCTV_RESULTROUTERTHREAD_HPP_ */
