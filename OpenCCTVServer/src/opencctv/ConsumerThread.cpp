#include "ConsumerThread.hpp"

namespace opencctv {

ConsumerThread::ConsumerThread(unsigned int iStreamId, ImageMulticaster* pImageMulticaster)
{
	_iStreamId = iStreamId;

	ApplicationModel* pModel = ApplicationModel::getInstance();
	_pQueue = NULL;
	if(pModel->containsInternalQueue(iStreamId))
	{
		_pQueue = pModel->getInternalQueues()[iStreamId];
	}
	_pImageMulticaster = pImageMulticaster;
}

ConsumerThread::ConsumerThread(unsigned int iStreamId)
{
	_iStreamId = iStreamId;

	ApplicationModel* pModel = ApplicationModel::getInstance();
	_pQueue = NULL;
	if(pModel->containsInternalQueue(iStreamId))
	{
		_pQueue = pModel->getInternalQueues()[iStreamId];
	}
	_pImageMulticaster = new ImageMulticaster(iStreamId);
}

void ConsumerThread::operator ()()
{
	if(_pImageMulticaster)
	{
		opencctv::util::log::Loggers::getDefaultLogger()->info("Consumer Thread started.");
		std::stringstream ssMsg;
		ssMsg << "Starting the Image Multicaster for the Consumer Thread " << _iStreamId;
		opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
		_pImageMulticaster->start();

		delete _pImageMulticaster; _pImageMulticaster = NULL;
	}
	opencctv::util::log::Loggers::getDefaultLogger()->info("Consumer Thread stopped.");
}

} /* namespace opencctv */
