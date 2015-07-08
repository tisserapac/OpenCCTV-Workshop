#include "ConsumerThread.hpp"

namespace opencctv {

ConsumerThread::ConsumerThread(unsigned int iStreamId, ImageMulticaster* pImageMulticaster) {
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
		_pImageMulticaster->start();

		delete _pImageMulticaster; _pImageMulticaster = NULL;
	}
	opencctv::util::log::Loggers::getDefaultLogger()->info("Consumer Thread stopped.");
}

} /* namespace opencctv */
