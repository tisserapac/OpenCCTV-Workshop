
#ifndef OPENCCTV_CONSUMERTHREAD_HPP_
#define OPENCCTV_CONSUMERTHREAD_HPP_

#include <vector>
#include "ApplicationModel.hpp"
#include "mq/TcpMqSender.hpp"
#include "ImageMulticaster.hpp"


namespace opencctv {

class ConsumerThread {
private:
	ImageMulticaster* _pImageMulticaster;
	ConcurrentQueue<Image>* _pQueue;
public:
	ConsumerThread(unsigned int iStreamId, ImageMulticaster* pImageMulticaster);
	ConsumerThread(unsigned int iStreamId);
	void operator()();
};

} /* namespace opencctv */

#endif /* OPENCCTV_CONSUMERTHREAD_HPP_ */
