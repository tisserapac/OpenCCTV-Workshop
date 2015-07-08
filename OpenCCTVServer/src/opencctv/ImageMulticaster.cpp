
#include "ImageMulticaster.hpp"

namespace opencctv {

ImageMulticaster::ImageMulticaster(unsigned int iStreamId)
{
	_bEnable = false;
	_iStreamId = iStreamId;
	_pSerializer = util::serialization::Serializers::getInstanceOfDefaultSerializer();
	_pMulticastDestination = NULL;
}

void ImageMulticaster::start()
{
	ApplicationModel* pModel = ApplicationModel::getInstance();
	ConcurrentQueue<Image>* pQueue;
	std::stringstream ssMsg;

	if(pModel->containsInternalQueue(_iStreamId))
	{
		pQueue = pModel->getInternalQueues()[_iStreamId];
	}
	if(pModel->containsMulticastDestination(_iStreamId))
	{
		_pMulticastDestination = pModel->getMulticastDestinations()[_iStreamId];
	}
	if(pQueue && _pSerializer && _pMulticastDestination)
	{
		//_mAISInfo = _pMulticastDestination->getAllDestinations();
		_bEnable = true;
		ssMsg << "Image Multicaster " << _iStreamId << " started.";
		opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	}

	std::map<unsigned int, AnalyticDestination>::iterator it;
	unsigned int iAnalyticInstanceId;
	mq::Sender* pSender = NULL;
	//std::string sInputName;
	util::flow::FlowController* pFlowController = NULL;

	while (_bEnable)
	{
		Image* pImage = NULL;
		try
		{
			unsigned long long lProducedTime = pQueue->waitAndGetFrontElement(pImage);
			if(pImage)
			{
				// for each Analytic Input queue/Analytic Instance Stream
				_mAISInfo = _pMulticastDestination->getAllDestinations();
				for (it = _mAISInfo.begin(); it != _mAISInfo.end(); ++it)
				//if(_pMulticastDestination->getNextDestination(iAnalyticInstanceId, sInputName, pSender))
				{
					iAnalyticInstanceId = it->second.iAnalyticInstanceId;
					// if Flow Controller available
					if (pModel->containsFlowController(iAnalyticInstanceId))
					{
						pFlowController = pModel->getFlowControllers()[iAnalyticInstanceId];
						// if Flow Controller allows to send
						if (pFlowController->canSendImageGeneratedAt(lProducedTime))
						{
							pSender = it->second.pMqSender;
							if (pSender)
							{
								pImage->setStreamId(_iStreamId);
								pImage->setInputName(it->second.sInputName);

								// send to Analytic Input queue
								if (send(pSender, pImage))
								{
									pFlowController->sent(pImage, lProducedTime);
								}
							}
						}
					}
				}
			}
			pQueue->tryRemoveFrontElement();

			//Define the interrupt point of the consumer threads
			boost::this_thread::interruption_point();

		}catch(const boost::thread_interrupted&)
		{
			// Thread interruption request received, break the loop
			_bEnable = false;
			ssMsg.clear();
			//ssMsg <<  "Image Multicaster of Consumer thread : " << _iStreamId << "interrupted";
			opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
			break;
		}
	}

	ssMsg.clear();
	ssMsg <<  "Image Multicaster of Consumer thread : " << _iStreamId << " Stopped";
	opencctv::util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
}

bool ImageMulticaster::send(mq::Sender* pMqSender, Image* pImage) {
	bool bSent = false;
	if(_pSerializer)
	{
		std::string* pSSerializedImage = _pSerializer->serialize(pImage);
		try {
			if(pMqSender && pMqSender->send(pSSerializedImage))
			{
				bSent = true;
				std::stringstream ssMsg;
				ssMsg << "Image " << pImage->getTimestamp() << " sent. VM used = "
						<< util::Util::getCurrentVmUsageKb() << " Kb";
				opencctv::util::log::Loggers::getDefaultLogger()->debug(ssMsg.str());
			}
			else
			{
				std::string sErrMsg = "Image: ";
				sErrMsg.append(pImage->getTimestamp());
				sErrMsg.append(" sending failed.");
				util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			}
		} catch (std::exception &e) {
			std::string sErrMsg = "Failed to send serialized Image. ";
			sErrMsg.append(e.what());
			util::log::Loggers::getDefaultLogger()->error(sErrMsg);
			return false;
		}
		if(pSSerializedImage) delete pSSerializedImage;
	}
	return bSent;
}

/*void ImageMulticaster::addDestination(const dto::AnalyticInstanceStream& analyticInstance)
{
	ApplicationModel* pModel = ApplicationModel::getInstance();
	if(pModel->containsImageInputQueueAddress(analyticInstance.getAnalyticInstanceId()))
	{
		mq::TcpMqSender* pSender = new mq::TcpMqSender();
		bool bConnectedToMq = false;
		try {
			bConnectedToMq = pSender->connectToMq(util::Config::getInstance()->get(util::PROPERTY_ANALYTIC_SERVER_IP),
					pModel->getImageInputQueueAddresses()[analyticInstance.getAnalyticInstanceId()]);
			pModel->getMulticastDestinations()[analyticInstance.getId()] = pSender;
		} catch (Exception &e) {
			std::stringstream ssErrMsg;
			ssErrMsg << "Failed to connect to Input Image Queue of Analytic Instance ";
			ssErrMsg << analyticInstance.getAnalyticInstanceId() << ". ";
			ssErrMsg << e.what();
			throw Exception(ssErrMsg.str());
		}
		if(bConnectedToMq)
		{
			Element e = {analyticInstance.getAnalyticInstanceId(), analyticInstance.getInputName()};
			_mAISInfo[analyticInstance.getId()] = e;
			std::stringstream ssMsg;
			ssMsg << "Connection established to Input Image Queue of Analytic Instance ";
			ssMsg << analyticInstance.getAnalyticInstanceId() << ". ";
			util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
		}
		else
		{
			if(pSender) delete pSender;
			std::stringstream ssErrMsg;
			ssErrMsg << "Failed to connect to Input Image Queue of Analytic Instance ";
			ssErrMsg << analyticInstance.getAnalyticInstanceId() << ". ";
			util::log::Loggers::getDefaultLogger()->error(ssErrMsg.str());
		}
	}
}*/

size_t ImageMulticaster::getNumberOfDestinations()
{
	return _mAISInfo.size();
}

void ImageMulticaster::stop() {
	_bEnable = false;
}

ImageMulticaster::~ImageMulticaster() {
}

} /* namespace opencctv */
