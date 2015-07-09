/*
 * MulticastDestination.hpp
 *
 *  Created on: Jul 1, 2015
 *      Author: anjana
 */

#ifndef MULTICASTDESTINATION_HPP_
#define MULTICASTDESTINATION_HPP_

#include <map>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include "mq/TcpMqSender.hpp"
#include "util/Config.hpp"
#include "Exception.hpp"
#include "util/log/Loggers.hpp"

namespace opencctv {

typedef struct {
	unsigned int iAnalyticInstanceId;
	std::string sInputName;
	mq::TcpMqSender* pMqSender;
} AnalyticDestination;

class MulticastDestination {
private:
	/*typedef struct {
		unsigned int iAnalyticInstanceId;
		std::string sInputName;
		mq::TcpMqSender* pMqSender;
	} Element;*/

	boost::mutex _mutex;
	//std::map<unsigned int, AnalyticDestination> ::iterator it;
	std::map<unsigned int, AnalyticDestination> _mAISInfo; // Analytic Instance id as the key
public:

	MulticastDestination();
	void addDestination(const unsigned int& iAnalyticInstanceId, const std::string& sInputName, const std::string& sInputQueueAddress);
	//bool getNextDestination(unsigned int& iAnalyticInstanceId, std::string& sInputName, mq::TcpMqSender*& pSender);
	bool getDestination(const unsigned int& iAnalyticInstanceId, std::string& sInputName, mq::TcpMqSender*& pSender);
	bool removeDestination(const unsigned int& iAnalyticInstanceId);
	void removeAllDestinations();
	std::map<unsigned int, AnalyticDestination>& getAllDestinations();
	unsigned int getNumberOfDestinations();

	virtual ~MulticastDestination();
};

} /* namespace opencctv */

#endif /* MULTICASTDESTINATION_HPP_ */
