/*
 * AnalyticInstanceStreamGateway.hpp
 *
 *  Created on: Jun 3, 2015
 *      Author: anjana
 */

#ifndef ANALYTICINSTANCESTREAMGATEWAY_HPP_
#define ANALYTICINSTANCESTREAMGATEWAY_HPP_

#include <vector>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include "DbConnector.hpp"
#include "../dto/AnalyticInstanceStream.hpp"
#include "../Exception.hpp"
#include "../util/Config.hpp"
#include "../util/Util.hpp"

namespace opencctv {
namespace db {

class AnalyticInstanceStreamGateway {
private:
	DbConnector* _pDbConnectorPtr;
	sql::Connection* _pConnectionPtr;
	sql::PreparedStatement* _pStatementPtr;
	static const std::string _SELECT_ALL_FOR_STREAM_SQL;

public:
	AnalyticInstanceStreamGateway();
	void findAllForStream(unsigned int iStreamId, std::vector<opencctv::dto::AnalyticInstanceStream>& vToStoreAIS);
	//void findAllForAnalyticInstance(unsigned int iAnalyticInstanceId, std::vector<opencctv::dto::AnalyticInstanceStream>& vAnalyticInstanceStream);
	virtual ~AnalyticInstanceStreamGateway();
};

} /* namespace db */
} /* namespace opencctv */

#endif /* ANALYTICINSTANCESTREAMGATEWAY_HPP_ */
