/*
 * MulticastDestination.cpp
 *
 *  Created on: Jul 1, 2015
 *      Author: anjana
 */

#include "MulticastDestination.hpp"

namespace opencctv {

MulticastDestination::MulticastDestination()
{
	//it = _mAISInfo.begin();
}

void MulticastDestination::addDestination(const unsigned int& iAnalyticInstanceId, const std::string& sInputName, const std::string& sInputQueueAddress)
{
	boost::mutex::scoped_lock lock(_mutex);

	mq::TcpMqSender* pSender = new mq::TcpMqSender();
	bool bConnectedToMq = false;
	try {
		bConnectedToMq = pSender->connectToMq(util::Config::getInstance()->get(util::PROPERTY_ANALYTIC_SERVER_IP),sInputQueueAddress);
		//pModel->getMulticastDestinations()[analyticInstance.getId()] = pSender;
	} catch (opencctv::Exception &e) {
		std::stringstream ssErrMsg;
		ssErrMsg << "Failed to connect to Input Image Queue of Analytic Instance ";
		ssErrMsg << iAnalyticInstanceId << ". ";
		ssErrMsg << e.what();
		throw opencctv::Exception(ssErrMsg.str());
	}
	if(bConnectedToMq)
	{
		AnalyticDestination e = {iAnalyticInstanceId, sInputName, pSender };
		_mAISInfo[iAnalyticInstanceId] = e;
		std::stringstream ssMsg;
		ssMsg << "Connection established to Input Image Queue of Analytic Instance ";
		ssMsg << iAnalyticInstanceId << ". ";
		util::log::Loggers::getDefaultLogger()->info(ssMsg.str());
	}
	else
	{
		if(pSender) delete pSender;
		std::stringstream ssErrMsg;
		ssErrMsg << "Failed to connect to Input Image Queue of Analytic Instance ";
		ssErrMsg << iAnalyticInstanceId << ". ";
		util::log::Loggers::getDefaultLogger()->error(ssErrMsg.str());
	}
}

bool MulticastDestination::getDestination(const unsigned int& iAnalyticInstanceId, std::string& sInputName, mq::TcpMqSender*& pSender)
{
	boost::mutex::scoped_lock lock(_mutex);

	std::map<unsigned int, AnalyticDestination> ::iterator it = _mAISInfo.find(iAnalyticInstanceId);
	if(it != _mAISInfo.end())
	{
		AnalyticDestination e = it->second;
		sInputName = e.sInputName;
		pSender = e.pMqSender;
		return true;
	}

	return false;
}

bool MulticastDestination::removeDestination(const unsigned int& iAnalyticInstanceId)
{
	boost::mutex::scoped_lock lock(_mutex);

	std::map<unsigned int, AnalyticDestination>::iterator it = _mAISInfo.find(iAnalyticInstanceId);
	if(it != _mAISInfo.end())
	{
		AnalyticDestination e = it->second;
		delete e.pMqSender; e.pMqSender = NULL;
		_mAISInfo.erase(it);
	}

	return false;
}

void MulticastDestination::removeAllDestinations()
{
	boost::mutex::scoped_lock lock(_mutex);

	//Element pFlowController = NULL;
	std::map<unsigned int, AnalyticDestination>::iterator itElement;
	for(itElement = _mAISInfo.begin(); itElement != _mAISInfo.end(); /*it increment below*/)
	{
		delete itElement->second.pMqSender; itElement->second.pMqSender = NULL;
		_mAISInfo.erase(itElement++);
	}
}

std::map<unsigned int, AnalyticDestination>& MulticastDestination::getAllDestinations()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _mAISInfo;
}

unsigned int MulticastDestination::getNumberOfDestinations()
{
	boost::mutex::scoped_lock lock(_mutex);
	return _mAISInfo.size();
}

MulticastDestination::~MulticastDestination()
{
}

} /* namespace opencctv */
