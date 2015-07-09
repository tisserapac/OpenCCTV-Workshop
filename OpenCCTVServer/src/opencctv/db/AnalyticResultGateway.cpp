/*
 * AnalyticResultGateway.cpp
 *
 *  Created on: Jun 4, 2015
 *      Author: anjana
 */

#include "AnalyticResultGateway.hpp"

namespace opencctv {
namespace db {

const std::string AnalyticResultGateway::_INSERT_ANALYTIC_RESULT_SQL = "INSERT INTO results (analytic_id, timestamp, result_text) VALUES (?, ?, ?)";

AnalyticResultGateway::AnalyticResultGateway()
{
	try
	{
		_pDbConnectorPtr = new DbConnector();
		_pConnectionPtr = (*_pDbConnectorPtr).getConnection_ResultsDB();
		_pStatementPtr = (*_pConnectionPtr).prepareStatement(_INSERT_ANALYTIC_RESULT_SQL);

	}catch(sql::SQLException &e)
	{
		std::string sErrorMsg = "Error while initializing the AnalyticResultGateway - .";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
	}
	catch(opencctv::Exception& e)
	{
		throw opencctv::Exception(e);
	}
}

int AnalyticResultGateway::insertResults(unsigned int iAnalyticInstanceId,analytic::AnalyticResult analyticResult)
{
	int result = 0;

	try{
		(*_pStatementPtr).setInt(1, iAnalyticInstanceId);
		(*_pStatementPtr).setString(2, analyticResult.getTimestamp());
		(*_pStatementPtr).setString(3, analyticResult.getCustomText());
		result = (*_pStatementPtr).executeUpdate();

	}catch(sql::SQLException &e)
	{
		result = -1;
		std::string sErrorMsg = "Error while Inserting analytics results to the database. - ";
		throw opencctv::Exception(sErrorMsg.append(e.what()));
	}

	return result;

}

AnalyticResultGateway::~AnalyticResultGateway()
{
	(*_pStatementPtr).close();
	delete _pStatementPtr; _pStatementPtr = NULL;
	delete _pDbConnectorPtr; _pDbConnectorPtr = NULL;
	delete _pDbConnectorPtr; _pDbConnectorPtr = NULL;
}



} /* namespace db */
} /* namespace opencctv */
