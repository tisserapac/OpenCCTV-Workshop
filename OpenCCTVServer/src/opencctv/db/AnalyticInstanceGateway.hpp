/*
 * AnalyticInstanceGateway.hpp
 *
 *  Created on: Jul 9, 2015
 *      Author: anjana
 */

#ifndef ANALYTICINSTANCEGATEWAY_HPP_
#define ANALYTICINSTANCEGATEWAY_HPP_

#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>
#include "DbConnector.hpp"
#include "../dto/AnalyticInstance.hpp"
#include "../Exception.hpp"
#include "../util/Config.hpp"
#include "../util/Util.hpp"
#include "../ApplicationModel.hpp"

namespace opencctv {
namespace db {

class AnalyticInstanceGateway {
private:
	DbConnector* _pDbConnectorPtr;
	sql::Connection* _pConnectionPtr;
	//sql::PreparedStatement* _pStatementPtr;
	static const std::string _SELECT_ANALYTIC_INSTANCE_SQL;
	static const std::string _SELECT_ANALYTIC_INSTANCE_STATUS_SQL;
	static const std::string _UPDATE_ALL_SQL;
	static const std::string _UPDATE_ANALYTIC_INSTANCE_SQL;
public:
	AnalyticInstanceGateway();
	void findAnalyticInstance(const unsigned int iAnalyticInstanceId, opencctv::dto::AnalyticInstance& ai);
	void updateAllAnalyticInstanceStatus();
	void updateAnalyticInstanceStatus(const unsigned int iAnalyticInstanceId, const std::string& sStatus);
	std::string getAnalyticInstanceStatus(const unsigned int iAnalyticInstanceId);
	virtual ~AnalyticInstanceGateway();
};

} /* namespace db */
} /* namespace opencctv */

#endif /* ANALYTICINSTANCEGATEWAY_HPP_ */
