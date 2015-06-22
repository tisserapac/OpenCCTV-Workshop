
#ifndef ANALYTIC_PRODUCERTHREAD_HPP_
#define ANALYTIC_PRODUCERTHREAD_HPP_

#include "../opencctv/mq/Mq.hpp"
#include "api/Analytic.hpp"
#include "../opencctv/Image.hpp"
#include "util/ImageUtil.hpp"
#include "../opencctv/util/log/Loggers.hpp"
#include "../opencctv/util/serialization/Serializable.hpp"
#include "ImageQueue.hpp"

namespace analytic {

class ProducerThread {
private:
	ImageQueue<api::Image_t>* _pImageInputQueue;
	opencctv::mq::Receiver* _pReceiver;
	opencctv::util::serialization::Serializable* _pSerializer;
public:
	ProducerThread(ImageQueue<api::Image_t>* pImageInputQueue, opencctv::mq::Receiver* pReceiver, opencctv::util::serialization::Serializable* pSerializer);
	void operator()();
};

} /* namespace analytic */

#endif /* ANALYTIC_PRODUCERTHREAD_HPP_ */
